
#include "audio_play.h"
// #include "main.h"


AudioPlayer::AudioPlayer() : running(false), decoder(nullptr), udp_socket(-1), alsa_handle(nullptr) {

    g_opus_decoder.inputSampleRate = 16000;
    g_opus_decoder.inputChannels = 1;
    g_opus_decoder.outputSampleRate = 16000;
    g_opus_decoder.outputChannels = 2;
    g_opus_decoder.duration_ms = 60;
    
    // 初始化ALSA播放设备
    init_alsa();
    init_speex_resampler();
    init_opus_decoder();
    init_udp();


    // on_incoming_audio_ = HandleAudioPacket;
    fifo_buffer.reserve(3840 * 50);        // 保留两倍于最大可能使用的空间
}


void AudioPlayer::init_opus_decoder() {
    // 初始化Opus解码器
    int error;
    decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    if (error != OPUS_OK) {
        throw std::runtime_error("Opus decoder creation failed");
    }
}



void AudioPlayer::init_speex_resampler() {
    // 初始化重采样器
    int resampleErr;
    resampler = speex_resampler_init(        //speex_resampler_process_int   未完成 2025.10.21  单声道 转 双声道
        g_opus_decoder.inputChannels,
        g_opus_decoder.inputSampleRate,     //16000
        g_opus_decoder.outputSampleRate,    // 24000
        SPEEX_RESAMPLER_QUALITY_DEFAULT,
        &resampleErr
    );

    // resampler = speex_resampler_init(        //speex_resampler_process_int   未完成 2025.10.21  单声道 转 双声道
    // g_opus_decoder.inputChannels,
    // g_opus_decoder.inputSampleRate,
    // g_opus_decoder.outputSampleRate,
    // SPEEX_RESAMPLER_QUALITY_DEFAULT,
    // &resampleErr
 
    if (resampleErr != RESAMPLER_ERR_SUCCESS) {
        std::cerr << "重采样器初始化失败 for decoder: " << resampleErr <<" inputSampleRate "<<  std::endl;
        //g_opus_decoder.inputSampleRate << "  "<< g_opus_decoder.outputSampleRate << "inputChannels "<< g_opus_decoder.inputChannels<< std::endl;
        return ;
    }
    std::cout << "重采样器初始化成功 for decoder: " << resampleErr <<" inputSampleRate "<<  std::endl;
}




void AudioPlayer::init_udp() {
    // 创建UDP套接字
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        throw std::system_error(errno, std::system_category(), "Socket creation failed");
    }
    
    // 绑定到本地端口
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(UDP_LISTEN_PORT);
    
    if (bind(udp_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::system_error(errno, std::system_category(), "Socket bind failed");
    }
    
    // 设置套接字超时
    timeval tv;
    tv.tv_sec = 1;  // 1秒超时
    tv.tv_usec = 0;
    setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

}

void AudioPlayer::start() {
    if (running) return;
    running = true;
      
    // 启动UDP接收线程
    receive_thread = std::thread(&AudioPlayer::receive_audio, this);
    receive_thread.detach();
    
    // 启动播放线程
    play_thread = std::thread(&AudioPlayer::play_audio, this);
    play_thread.detach();
}



void AudioPlayer::stop() {
    running = false;
    cond_var.notify_all();  // 唤醒所有等待线程
    
    if (receive_thread.joinable()) receive_thread.join();
    if (play_thread.joinable()) play_thread.join();
}

void AudioPlayer::init_alsa() {
    int err;
    snd_pcm_hw_params_t* params;
    
    // 打开PCM设备                                  //SND_PCM_NONBLOCK: 非阻塞；  0：阻塞
    if ((err = snd_pcm_open(&alsa_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        throw std::runtime_error("ALSA open error: " + std::string(snd_strerror(err)));
    }
    
    // 分配硬件参数对象
    snd_pcm_hw_params_alloca(&params);
    
    // 填充默认参数
    if ((err = snd_pcm_hw_params_any(alsa_handle, params)) < 0) {
        throw std::runtime_error("ALSA init error: " + std::string(snd_strerror(err)));
    }
    
    // 设置交错模式
    if ((err = snd_pcm_hw_params_set_access(alsa_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        throw std::runtime_error("ALSA access error: " + std::string(snd_strerror(err)));
    }
    
    // 设置16位小端格式
    if ((err = snd_pcm_hw_params_set_format(alsa_handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
        throw std::runtime_error("ALSA format error: " + std::string(snd_strerror(err)));
    }
    
    // 设置通道数
    if ((err = snd_pcm_hw_params_set_channels(alsa_handle, params, PLAY_CHANNELS)) < 0) {     // CHANNELS
        throw std::runtime_error("ALSA channels error: " + std::string(snd_strerror(err)));
    }
    
    // 设置采样率
    unsigned int rate = PLAY_SAMPLE_RATE;
    if ((err = snd_pcm_hw_params_set_rate_near(alsa_handle, params, &rate, 0)) < 0) {
        throw std::runtime_error("ALSA rate error: " + std::string(snd_strerror(err)));
    }
    
    // 设置周期大小
    snd_pcm_uframes_t frames = 1440;   // FRAME_SIZE;
    
    if ((err = snd_pcm_hw_params_set_period_size_near(alsa_handle, params, &frames, 0)) < 0) {
        throw std::runtime_error("ALSA period size error: " + std::string(snd_strerror(err)));
    }
    // // 设置缓冲区大小为4倍的周期大小（即4个周期）
    // snd_pcm_uframes_t buffer_size = frames * 4;
    // if ((err = snd_pcm_hw_params_set_buffer_size_near(alsa_handle, params, &buffer_size)) < 0) {
    //     std::cerr << "ALSA buffer size error: " << snd_strerror(err) << std::endl;
    // } else {
    //     std::cout << "ALSA buffer size set to " << buffer_size << " frames" << std::endl;
    // }
    
    // 应用参数
    if ((err = snd_pcm_hw_params(alsa_handle, params)) < 0) {
        throw std::runtime_error("ALSA apply params error: " + std::string(snd_strerror(err)));
    }
    
    // 准备PCM设备
    if ((err = snd_pcm_prepare(alsa_handle)) < 0) {
        throw std::runtime_error("ALSA prepare error: " + std::string(snd_strerror(err)));
    }

    // Retrieve and display audio parameters
    snd_pcm_uframes_t buf_size = 0;
    snd_pcm_hw_params_get_period_size(params, &actual_params_.period_size, 0);
    snd_pcm_hw_params_get_rate(params, &actual_params_.sample_rate, 0);
    snd_pcm_hw_params_get_channels(params, &actual_params_.channels);
    snd_pcm_hw_params_get_format(params, &actual_params_.format);
    snd_pcm_hw_params_get_buffer_size(params, &actual_params_.buf_size);

    output_channels_ = actual_params_.channels;
    output_sample_rate_ = actual_params_.sample_rate;
    original_output_sample_rate_ = output_sample_rate_;

    printf("####### play 最终配置参数: 格式=%s, 声道数=%u, 采样率=%uHz, 周期大小=%lu帧\n",
           snd_pcm_format_name(actual_params_.format), 
           actual_params_.channels, actual_params_.sample_rate, actual_params_.period_size);
    printf("$$$ actual_params_.buf_size = %d\n\n", actual_params_.buf_size);
}

void AudioPlayer::close_player() {
    if (alsa_handle) {
        // if (state_ == State::Running || state_ == State::Paused) {
            snd_pcm_drop(alsa_handle);
        // }
        snd_pcm_close(alsa_handle);
        alsa_handle = nullptr;
    }
    // state_ = State::Closed;
}

void AudioPlayer::open_player(int new_sample_rate){   //, int new_channels, int new_format) {
    int err;
    snd_pcm_hw_params_t* params;   
    // 打开PCM设备                                  //SND_PCM_NONBLOCK: 非阻塞；  0：阻塞
    if ((err = snd_pcm_open(&alsa_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        throw std::runtime_error("ALSA open error: " + std::string(snd_strerror(err)));
    }
    // 分配硬件参数对象
    snd_pcm_hw_params_alloca(&params);  
    // 填充默认参数
    if ((err = snd_pcm_hw_params_any(alsa_handle, params)) < 0) {
        throw std::runtime_error("ALSA init error: " + std::string(snd_strerror(err)));
    } 
    // 设置交错模式
    if ((err = snd_pcm_hw_params_set_access(alsa_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        throw std::runtime_error("ALSA access error: " + std::string(snd_strerror(err)));
    }
    // 设置16位小端格式
    if ((err = snd_pcm_hw_params_set_format(alsa_handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
        throw std::runtime_error("ALSA format error: " + std::string(snd_strerror(err)));
    }
    // 设置通道数
    if ((err = snd_pcm_hw_params_set_channels(alsa_handle, params, PLAY_CHANNELS)) < 0) {     // CHANNELS
        throw std::runtime_error("ALSA channels error: " + std::string(snd_strerror(err)));
    }
    // 设置采样率
    unsigned int rate = new_sample_rate;
    if ((err = snd_pcm_hw_params_set_rate_near(alsa_handle, params, &rate, 0)) < 0) {
        throw std::runtime_error("ALSA rate error: " + std::string(snd_strerror(err)));
    }
    // 设置周期大小
    snd_pcm_uframes_t frames = 1440;   // FRAME_SIZE;
    
    if ((err = snd_pcm_hw_params_set_period_size_near(alsa_handle, params, &frames, 0)) < 0) {
        throw std::runtime_error("ALSA period size error: " + std::string(snd_strerror(err)));
    }
    // // 设置缓冲区大小为4倍的周期大小（即4个周期）
    // snd_pcm_uframes_t buffer_size = frames * 4;
    // if ((err = snd_pcm_hw_params_set_buffer_size_near(alsa_handle, params, &buffer_size)) < 0) {
    //     std::cerr << "ALSA buffer size error: " << snd_strerror(err) << std::endl;
    // } else {
    //     std::cout << "ALSA buffer size set to " << buffer_size << " frames" << std::endl;
    // }
    
    // 应用参数
    if ((err = snd_pcm_hw_params(alsa_handle, params)) < 0) {
        throw std::runtime_error("ALSA apply params error: " + std::string(snd_strerror(err)));
    }
    // 准备PCM设备
    if ((err = snd_pcm_prepare(alsa_handle)) < 0) {
        throw std::runtime_error("ALSA prepare error: " + std::string(snd_strerror(err)));
    }

}

// void AudioPlayer::OnIncomingAudio(std::function<void(std::unique_ptr<AudioPlayer> packet)> callback) {
//     on_incoming_audio_ = callback;
// }
// void AudioPlayer::HandleAudioPacket(std::unique_ptr<AudioPlayer> packet){
//     packet = NULL;
// }
    // // 定义回调类型
    // using AudioCallback = std::function<void(const std::string&, int)>;
    // // 设置回调函数
    // void set_callback(AudioCallback cb) {
    //     callback_ = cb;
    // }


void AudioPlayer::receive_audio() {
    std::vector<uint8_t> opus_buffer(MAX_UDP_PACKET_SIZE);
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int recvFrameCnt = 0;

    // 计算目标 PCM 数据大小  targetFrameSize = 16000 * 60 /1000 = 960
    //                       targetPcmSize = 960 * 2 * 2 = 3840
    // int targetFrameSize = g_opus_decoder.outputSampleRate * g_opus_decoder.duration_ms / 1000;
    // int targetPcmSize = targetFrameSize * g_opus_decoder.outputChannels * sizeof(opus_int16);
    int targetFrameSize = g_opus_decoder.outputSampleRate * g_opus_decoder.duration_ms / 1000;   // 960
    int targetPcmSize = targetFrameSize * g_opus_decoder.inputChannels * sizeof(opus_int16);
    //std::vector<opus_int16> resampledFrame(960);   //

    // int pcmFrameSize = actual_params_.format * actual_params_.period_size;
    std::cout << "receive_audio start ..." << std::endl;
    while (running) {
        // 接收UDP数据包
        ssize_t packet_size = recvfrom(udp_socket, 
                                        opus_buffer.data(), 
                                        opus_buffer.size(),
                                        0, 
                                        (struct sockaddr*)&client_addr, 
                                        &addr_len);
        
        if (packet_size < 0) {
            // 超时或其他错误，继续尝试
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            std::cerr << "UDP recv error: " << strerror(errno) << std::endl;
            continue;
        }
        
        // 解码Opus数据
        std::vector<int16_t> pcm_data(FRAME_SIZE);  // FRAME_SIZE
        std::vector<opus_int16> resampledFrame(FRAME_SIZE);   //
        int decoded_samples = opus_decode(decoder, 
                                            opus_buffer.data(), 
                                            packet_size,
                                            pcm_data.data(), 
                                            FRAME_SIZE,   // FRAME_SIZE
                                            0);
        
        if (decoded_samples < 0) {
            std::cerr << "Opus decode error: " << opus_strerror(decoded_samples) << std::endl;
            continue;
        }
        /********************/
        // 计算解码后的 PCM 数据大小
        int decodedBytes = decoded_samples * sizeof(opus_int16) * 1;
        // printf("$$$ opus2pcm, decodedSamples = %d\n", decoded_samples);  //960

        // 执行重采样
        spx_uint32_t in_len = decoded_samples;
        spx_uint32_t out_len = targetFrameSize;
        int resampleErr = speex_resampler_process_int(
            resampler,  // g_opus_decoder.resampler
            0,
            pcm_data.data(),
            &in_len,
            resampledFrame.data(),
            &out_len
        );
        // printf("$$$ resampleErr = %d, in_len =%d, out_len =%d\n", resampleErr, in_len, out_len);
        // std::cout << "### receive_audio, resampledFrame.size() = " << resampledFrame.size() << std::endl;

        if (resampleErr != RESAMPLER_ERR_SUCCESS)
        {
            std::cerr << "重采样失败: " << resampleErr << std::endl;
            return ;
        }
        // 检查重采样结果
        if (in_len != decoded_samples || out_len != targetFrameSize) {
            std::cerr << "重采样样本数不匹配" << std::endl;
            return ;
        }
        /**********************/

        recvFrameCnt++;
        // std::cout << "### receive_audio, begin... recvFrameCnt = " << recvFrameCnt << " packet_size  =  " << packet_size << std::endl;
        // std::cout << "### receive_audio, decoded_samples = " << decoded_samples << std::endl;
        
        // 将解码后的PCM数据添加到队列
        {
            std::unique_lock<std::mutex> lock(queue_mutex);    
            // 如果队列已满，丢弃最旧的数据
            if (pcm_queue.size() >= PCM_BUFFER_COUNT) {
                pcm_queue.pop();
            }

            int stero_size = resampledFrame.size() * 2;
            std::vector<int16_t> stero_data(stero_size);
            for (int i =0; i< resampledFrame.size(); i++)
            {
                stero_data[2 * i] = resampledFrame[i];
                stero_data[2 * i + 1] = resampledFrame[i];
            }
            
            pcm_queue.push(std::move(stero_data));
            // std::cout << "### receive_audio, end... pcm_queue.size() = " << pcm_queue.size() << std::endl;
            std::cout << std::endl;
        }
        // usleep(20000);
        // msleep(20);
        cond_var.notify_one();  // 通知播放线程有新数据
    }
}

void AudioPlayer::decBufToPlayBuf(std::vector<int16_t>pcm_data, unsigned char *playBuffer){
    static int play_buffer_offset = 0, dec_size = 1920, play_size = 1280 * PLAY_CHANNELS;
    // unsigned char *p = (unsigned char *)pcm_data.data();
    std::cout << "PlayBuf 001, pcm_data.size()*2 = " << pcm_data.size() *2 << std::endl;
    std::cout << "PlayBuf 002, play_buffer_offset = " << play_buffer_offset << " play_size = "<< play_size<< std::endl;
    int pcm_data_size = pcm_data.size() * 2;

    while(play_buffer_offset < play_size){
        // for (int i = 0; i< pcm_data.size(); i++){
        //     g_play_buffer[i] = pcm_data[i];
        // }
        std::cout << "PlayBuf 003, pcm_data to g_play_buffer " << std::endl;
        memcpy(g_play_buffer + play_buffer_offset, pcm_data.data(), pcm_data_size);
        play_buffer_offset += pcm_data_size;
        std::cout << "PlayBuf 004, play_buffer_offset = " << play_buffer_offset << std::endl;
    }

    std::cout << "PlayBuf 004, memcpy to buf, play_size = "<< play_size << std::endl;
    memcpy(playBuffer, g_play_buffer, play_size);
    std::cout << "decBufToPlayBuf ... 008" << std::endl;
    memmove(g_play_buffer, g_play_buffer+play_size, play_buffer_offset - play_size);
    play_buffer_offset -= play_size;
    std::cout << "decBufToPlayBuf ... 009, play_buffer_offset = "<< play_buffer_offset << std::endl;
}


void AudioPlayer::OutputData(std::vector<int16_t>& data) {
    // Write(data.data(), data.size());
    printf("[play] OutputData data.size() = %d\n", data.size());

    // int stero_size = data.size() * 2;
    // std::vector<int16_t> stero_data(stero_size);
    // for (int i =0; i< data.size(); i++)
    // {
    //     stero_data[2 * i] = data[i];
    //     stero_data[2 * i + 1] = data[i];
    // }

    auto t1 = std::chrono::steady_clock::now();
    int err = snd_pcm_writei(alsa_handle, data.data(), data.size() /2); // 640帧
    auto t2 = std::chrono::steady_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    // std::cout << "[PLAY] writei took "<< dt << " ms, wrote " << err << " frames\n" << std::endl;
    // 错误处理...
    if (err == -EPIPE) {
        // 处理underrun
        // err_cnt++;
        // std::cout << "[ERROR] err_cnt = "<< err_cnt << std::endl;
        std::cerr << "ALSA underrun occurred" << std::endl;
        snd_pcm_prepare(alsa_handle);
    } else if (err == -EAGAIN) {
        // 缓冲区已满，等待一段时间后重试
        std::cout << "[ERROR] buffer is full " << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // continue;
    }else if (err < 0) {
        std::cerr << "ALSA write error: " << snd_strerror(err) << std::endl;
    }    
}

void AudioPlayer::play_audio() {
    // 设置实时优先级（需要root权限）
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 5;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    sleep(1);

    int playFrameCnt = 0 , errCnt =0, err_cnt = 0;
    playBuffer = (unsigned char *)malloc(actual_params_.period_size * actual_params_.format * PLAY_CHANNELS);
    while (running) {
        std::vector<int16_t> pcm_data;
        std::vector<int16_t> pcm_data1;
        std::vector<int16_t> pcm_data2;
        std::vector<int16_t> combinedPcm_data;

        std::vector<std::vector<int16_t>> frames; // 用于存储从队列中取出的所有帧
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
             // 等待数据可用
            cond_var.wait(lock, [this] { 
                return !running || !pcm_queue.empty();//  || timeout_occurred; 
            });
            
            if (!running) break;
            
            // 从队列获取数据
            // std::cout << "### play_audio, pcm_queue.size() = " << pcm_queue.size() << std::endl;
            pcm_data = std::move(pcm_queue.front());
            pcm_queue.pop();   
        }
        playFrameCnt++;

/**************** chatGPT  ****************************/
/**************** luo  ****************************/
#if 0
        int dec_size = pcm_data.size();  //1920
        int idx=0;
        while(idx < 1280){
            // 填补 carry_buf
            std::cout << "[PLAY] 001, dec_size = "<< dec_size << " carry_buf.size() = " << carry_buf.size() << std::endl;
            while(carry_buf.size()<1280 && idx < 1280){
                carry_buf.push_back(pcm_data[idx++]);
            }
            std::cout << "[PLAY] 002, idx = "<< idx << " carry_buf.size() =  " << carry_buf.size() << std::endl;
            // 满了一个period则写入ALSA
            if (carry_buf.size() == 640 * PLAY_CHANNELS) {
                int err = snd_pcm_writei(alsa_handle, carry_buf.data(), 640);
                std::cout << "[PLAY] 003 writei took " << "  wrote " << err << " frames\n";
                if (err == -EPIPE) {
                // 处理underrun
                std::cerr << "ALSA underrun occurred" << std::endl;
                errCnt++;
                snd_pcm_prepare(alsa_handle);
                } else if (err < 0) {
                    std::cerr << "ALSA write error: " << snd_strerror(err) << std::endl;
                }
                carry_buf.clear();
            }
        }
        std::cout << "[PLAY] 004, idx = "<< idx <<std::endl;
        int written_size = idx;
        for (int i = 0; i < dec_size - written_size; i++){
            carry_buf.push_back(pcm_data[idx++]);
        }
        std::cout << "[PLAY] 005, idx = "<< idx << " carry_buf.size() = " << carry_buf.size() << std::endl;
        if(carry_buf.size() == 640 * PLAY_CHANNELS){
            int err = snd_pcm_writei(alsa_handle, carry_buf.data(), 640);
            std::cout << "[PLAY] 006 writei took " << "  wrote " << err << " frames\n";
            if (err == -EPIPE) {
            // 处理underrun
            std::cerr << "ALSA underrun occurred " << std::endl;
            errCnt++;
            snd_pcm_prepare(alsa_handle);
            } else if (err < 0) {
                std::cerr << "ALSA write error: " << snd_strerror(err) << std::endl;
            }
            carry_buf.clear();
        }
        std::cout << "[PLAY] 007 carry_buf.size() =  " << carry_buf.size() << std::endl;
        std::cout << "[PLAY] 009 int playFrameCnt = "<< playFrameCnt << " errCnt = " << errCnt << "\n" << std::endl;
#endif
/********** luo ***********/  

#if 0
            std::cout << "### play_audio, pcm_data.size() = " << pcm_data.size()  << std::endl;
            auto t1 = std::chrono::steady_clock::now();     
            int err = snd_pcm_writei(alsa_handle, pcm_data.data(), 960);   // pcm_data FRAME_SIZE
            auto t2 = std::chrono::steady_clock::now();
            auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            std::cout << "[PLAY] writei took " << dt << " ms, wrote " << err << " frames\n";

            // std::cout << "### play_audio, err = " << err  << std::endl;
            std::cout << std::endl;
            if (err == -EPIPE) {
                // 处理underrun
                std::cerr << "ALSA underrun occurred" << std::endl;
                snd_pcm_prepare(alsa_handle);
            } else if (err < 0) {
                std::cerr << "ALSA write error: " << snd_strerror(err) << std::endl;
            }
#endif 
#if  0
// 24000 采样率
        if(pcm_data.size()>0){
            std::cout << "[PLAY] pcm_data.size() = " << pcm_data.size() << " \n";
            // 写入ALSA
            int err = snd_pcm_writei(alsa_handle, pcm_data.data(), 640); // 640帧
            // 错误处理...
            std::cout << "[PLAY] writei took ms, wrote " << err << " frames\n";
            // std::cout << "### play_audio, err = " << err  << std::endl;
            if (err == -EPIPE) {
                // 处理underrun
                err_cnt++;
                std::cout << "[ERROR] err_cnt = "<< err_cnt << std::endl;
                std::cerr << "ALSA underrun occurred" << std::endl;
                snd_pcm_prepare(alsa_handle);
                // err = snd_pcm_writei(alsa_handle, chunk.data(), 640); // 640帧
                // if (err < 0) {
                // std::cerr << "ALSA write error after prepare: " << snd_strerror(err) << std::endl;
                // }
            } else if (err == -EAGAIN) {
                // 缓冲区已满，等待一段时间后重试
                std::cout << "[ERROR] buffer is full " << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }else if (err < 0) {
                std::cerr << "ALSA write error: " << snd_strerror(err) << std::endl;
            }
            std::cout << std::endl;
        }
#endif

#if 1    /**  current  **/
        // 将stereo_data添加到carry_buf（残差缓冲区）
        // std::cout << "start 000, playFrameCnt = " << playFrameCnt << std::endl;
        // std::cout << "start 001, carry_buf.size() = " << carry_buf.size() << std::endl;
        carry_buf.insert(carry_buf.end(), pcm_data.begin(), pcm_data.end());
        // std::cout << "start 002, carry_buf.size() = " << carry_buf.size() << std::endl;
        // 然后，从carry_buf中取出整块数据（每次1280样本）写入ALSA
        int play_times = 0;
        int stereo_period_size = actual_params_.period_size * PLAY_CHANNELS;
        while (carry_buf.size() >= stereo_period_size) {
            // std::vector<int16_t> move_buf(carry_buf.begin(), carry_buf.begin() + 3840);
            // while(move_buf.size()>= 1280){
            // std::cout << "[PLAY] play_times =  " << play_times << " frames\n";
            // 取出1280个样本（640帧立体声）
            // std::cout << "start 003, carry_buf.size() = " << carry_buf.size() << std::endl;
            std::vector<int16_t> chunk(carry_buf.begin(), carry_buf.begin() + stereo_period_size);
            carry_buf.erase(carry_buf.begin(), carry_buf.begin() + stereo_period_size);
            // std::cout << "start 004, carry_buf.size() = " << carry_buf.size() << std::endl;
            // std::cout << "start 005, chunk.size() = " << chunk.size() << std::endl;
            
            // 写入ALSA
            auto t1 = std::chrono::steady_clock::now();
            int err = snd_pcm_writei(alsa_handle, chunk.data(), actual_params_.period_size); // 640帧
            auto t2 = std::chrono::steady_clock::now();
            auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            // std::cout << "[PLAY] writei took "<< dt << " ms, wrote " << err << " frames\n" << std::endl;
            // 错误处理...
            if (err == -EPIPE) {
                // 处理underrun
                err_cnt++;
                std::cout << "[ERROR] err_cnt = "<< err_cnt << std::endl;
                std::cerr << "ALSA underrun occurred" << std::endl;
                snd_pcm_prepare(alsa_handle);
            } else if (err == -EAGAIN) {
                // 缓冲区已满，等待一段时间后重试
                std::cout << "[ERROR] buffer is full " << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }else if (err < 0) {
                std::cerr << "ALSA write error: " << snd_strerror(err) << std::endl;
            }
            std::cout << std::endl;
        }
#endif
        // }
        // playFrameCnt++;
        // std::cout << "### play_audio, playFrameCnt = " << playFrameCnt << std::endl;
        // int play_size = combinedPcm_data.size() * 2;
        // std::vector<int16_t> play_data(combinedPcm_data.size() * 2);
        // for (int i =0; i< combinedPcm_data.size(); i++)
        // {
        //     play_data[2 * i] = combinedPcm_data[i];
        //     play_data[2 * i + 1] = combinedPcm_data[i];
        // }
        // // 将本次解码的数据追加到FIFO
        // std::cout << "### play_audio, "<< " fifo_buffer.size() === " << fifo_buffer.size() << std::endl;
        // fifo_buffer.insert(fifo_buffer.end(), play_data.begin(), play_data.end());
        // std::cout << "### play_audio, play_data.size() = "<< play_data.size() << " fifo_buffer.size() = " << fifo_buffer.size() << std::endl;
        // // 每个周期需要的采样点数：640帧 * 2声道 = 1280个采样值
        // const int samples_per_period = 640 * PLAY_CHANNELS;
        // std::vector<int16_t> period_data(fifo_buffer.begin(), fifo_buffer.begin() + samples_per_period);
        // // 从FIFO中移除这些数据
        // fifo_buffer.erase(fifo_buffer.begin(), fifo_buffer.begin() + samples_per_period);
        // std::cout << "### play_audio, period_data.size() = " << period_data.size() << std::endl;
        // int pcmFrameSize = actual_params_.format * actual_params_.channels * actual_params_.period_size;  //2*2*640
        // std::cout << "### play_audio, play_data.size() = " << play_data.size() << std::endl;
        // // 播放PCM数据 
        // //FRAME_SIZE=960：声音断断续续，声音丢失； pcmFrameSize=2560:声音沙哑   
        // //play_data.size()=1920:声音沙哑          1280:比较流畅,有杂音
        // int err = snd_pcm_writei(alsa_handle, play_data.data(), 3840);   // pcm_data FRAME_SIZE
        // std::cout << "### play_audio, err = " << err  << std::endl;
        // if (err == -EPIPE) {
        //     // 处理underrun
        //     std::cerr << "ALSA underrun occurred" << std::endl;
        //     snd_pcm_prepare(alsa_handle);
        // } else if (err < 0) {
        //     std::cerr << "ALSA write error: " << snd_strerror(err) << std::endl;
        // }
    
    }
}



bool AudioPlayer::SetOutputSampleRate(int sample_rate) {
    // 特殊处理：如果传入 -1，表示重置到原始采样率
    if (sample_rate == -1) {
        if (original_output_sample_rate_ > 0) {
            sample_rate = original_output_sample_rate_;
            // ESP_LOGI(TAG, "Resetting to original output sample rate: %d Hz", sample_rate);
        } else {
            // ESP_LOGW(TAG, "Original sample rate not available, cannot reset");
            return false;
        }
    }
    
    if (sample_rate <= 0 || sample_rate > 192000) {
        // ESP_LOGE(TAG, "Invalid sample rate: %d", sample_rate);
        return false;
    }
    
    if (output_sample_rate_ == sample_rate) {
        // ESP_LOGI(TAG, "Sample rate already set to %d Hz", sample_rate);
        return true;
    }

    // 设置采样率
    unsigned int rate = sample_rate;
    close_player();
    open_player(rate);
    output_sample_rate_ = sample_rate;
    return true;
}



int play_main() {
    try {
        AudioPlayer player;
        player.start();
        
        std::cout << "Audio player started. Receiving on port " << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();
        
        player.stop();
        std::cout << "Stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}