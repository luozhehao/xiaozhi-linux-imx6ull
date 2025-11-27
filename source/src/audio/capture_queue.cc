
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "capture_queue.h"
// #include "main.h"




AudioCapture::AudioCapture() : running(false), encoder(nullptr), udp_socket(-1) {
    // 初始化Opus编码器
    int error;
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &error);
    if (error != OPUS_OK) {
        throw std::runtime_error("Opus encoder creation failed");
    }
    
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(1));  // 低复杂度
    
    // 创建UDP套接字
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        throw std::system_error(errno, std::system_category(), "Socket creation failed");
    }
    
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(UDP_TARGET_PORT);
    if (inet_pton(AF_INET, UDP_TARGET_IP, &target_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address");
    }

    //音频设备 初始化
    params_.sample_rate = 16000;                  // 44.1kHz
    params_.format = SND_PCM_FORMAT_S16_LE;       // 16位小端格式
    params_.channels = 2;                          // 立体声
    params_.period_size = 1024;                    // 每次读取1024帧
    params_.buffer_time = 500000;                 // 500ms缓冲区
    params_.device_name = "hw:1,0";                // 使用第一个硬件设备  "hw:0,0";

    init();
}

// 初始化音频设备
bool AudioCapture::init() {
    int err;
    snd_pcm_hw_params_t *hw_params;

    // 打开PCM设备
    if ((err = snd_pcm_open(&pcm_handle_, params_.device_name.c_str(), 
                            SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        std::cerr << "Cannot open audio device: " << params_.device_name 
                    << " (" << snd_strerror(err) << ")" << std::endl;
        return false;
    }

    // 分配硬件参数对象
    snd_pcm_hw_params_alloca(&hw_params);

    // 初始化硬件参数
    if ((err = snd_pcm_hw_params_any(pcm_handle_, hw_params)) < 0) {
        std::cerr << "Cannot initialize hardware parameter structure: " 
                    << snd_strerror(err) << std::endl;
        return false;
    }

    // 设置访问类型为交错模式
    if ((err = snd_pcm_hw_params_set_access(pcm_handle_, hw_params, 
                                            SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        std::cerr << "Cannot set access type: " << snd_strerror(err) << std::endl;
        return false;
    }

    // 设置采样格式
    if ((err = snd_pcm_hw_params_set_format(pcm_handle_, hw_params, params_.format)) < 0) {
        std::cerr << "Cannot set sample format: " << snd_strerror(err) << std::endl;
        return false;
    }

    // 设置采样率
    unsigned int actual_rate = params_.sample_rate;
    if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle_, hw_params, 
                                                &actual_rate, 0)) < 0) {
        std::cerr << "Cannot set sample rate: " << snd_strerror(err) << std::endl;
        return false;
    }

    if (actual_rate != params_.sample_rate) {
        std::cerr << "Sample rate doesn't match (requested " << params_.sample_rate
                    << ", got " << actual_rate << ")" << std::endl;
        return false;
    }

    // 设置通道数
    // if ((err = snd_pcm_hw_params_set_channels(pcm_handle_, hw_params, params_.channels)) < 0) {
    //     std::cerr << "Cannot set channel count: " << snd_strerror(err) << std::endl;
    //     return false;
    // }

    /**********自动检测声道2025.10.12***********/
    // 获取设备支持的声道范围
    unsigned int min_channels, max_channels;
    snd_pcm_hw_params_get_channels_min(hw_params, &min_channels);
    snd_pcm_hw_params_get_channels_max(hw_params, &max_channels);
    
    printf("设备支持的声道范围: %u - %u\n", min_channels, max_channels);
    
    // 尝试设置最大声道数，如果失败则尝试最小声道数
    unsigned int snd_channels = max_channels;
    if (snd_pcm_hw_params_set_channels(pcm_handle_, hw_params, snd_channels) < 0) {
        printf("无法设置 %u 声道，尝试 %u 声道\n", snd_channels, min_channels);
        snd_channels = min_channels;
        if (snd_pcm_hw_params_set_channels(pcm_handle_, hw_params, snd_channels) < 0) {
            fprintf(stderr, "无法设置任何声道数\n");
            return 1;
        }
    }
    /*****************************************/
    // 设置缓冲区时间
    unsigned int buffer_time = params_.buffer_time;
    // if ((err = snd_pcm_hw_params_set_buffer_time_near(pcm_handle_, hw_params, 
    //                                                 &buffer_time, 0)) < 0) {
    //     std::cerr << "Cannot set buffer time: " << snd_strerror(err) << std::endl;
    //     return false;
    // }
    // 设置周期大小
     snd_pcm_uframes_t period_size = params_.period_size;
    // if ((err = snd_pcm_hw_params_set_period_size_near(pcm_handle_, hw_params, 
    //                                                     &period_size, 0)) < 0) {
    //     std::cerr << "Cannot set period size: " << snd_strerror(err) << std::endl;
    //     return false;
    // }

    // 应用参数
    if ((err = snd_pcm_hw_params(pcm_handle_, hw_params)) < 0) {
        std::cerr << "Cannot set parameters: " << snd_strerror(err) << std::endl;
        return false;
    }

    // 获取实际参数
    snd_pcm_hw_params_get_period_size(hw_params, &period_size, 0);
    snd_pcm_hw_params_get_buffer_time(hw_params, &buffer_time, 0);

    std::cout << "Audio device initialized: " << params_.device_name << std::endl;
    std::cout << "  Sample rate: " << actual_rate << " Hz" << std::endl;
    std::cout << "  Channels: " << params_.channels << std::endl;
    std::cout << "  Format: " << snd_pcm_format_name(params_.format) << std::endl;
    std::cout << "  Period size: " << period_size << " frames" << std::endl;
    std::cout << "  Buffer time: " << buffer_time / 1000 << " ms" << std::endl;

    /*********************************/
    // Retrieve and display audio parameters
    snd_pcm_uframes_t frames;
    snd_pcm_hw_params_get_period_size(hw_params, &actual_params_.period_size, 0);
    snd_pcm_hw_params_get_rate(hw_params, &actual_params_.sample_rate, 0);
    snd_pcm_hw_params_get_channels(hw_params, &actual_params_.channels);
    snd_pcm_hw_params_get_format(hw_params, &actual_params_.format);

    printf("####### frames = %d ************\n", period_size);
    printf("####### capture 最终配置参数: 格式=%s, 声道数=%u, 采样率=%uHz, 周期大小=%lu帧\n",
           snd_pcm_format_name(actual_params_.format), 
           actual_params_.channels, actual_params_.sample_rate, actual_params_.period_size);

    // Set output parameters
    // if (actual_sample_rate) *actual_sample_rate = sample_rate;
    // if (actual_channels) *actual_channels = channels;
    // if (actual_format) *actual_format = format;
    /***********************/
    return true;
}

void AudioCapture::start() {
    if (running) return;
    running = true;

    // 启动音频采集线程
    capture_thread = std::thread(&AudioCapture::capture_audio, this);
    capture_thread.detach();
    
    // 启动处理线程
    process_thread = std::thread(&AudioCapture::process_audio, this);
    process_thread.detach();
    std::cout << "### AudioCapture started... " << std::endl;
}

void AudioCapture::stop() {
    running = false;
    cond_var.notify_all();  // 唤醒所有等待线程
    
    if (capture_thread.joinable()) capture_thread.join();
    if (process_thread.joinable()) process_thread.join();
}


void AudioCapture::capture_audio() {
    // 配置ALSA音频采集
    // snd_pcm_t* handle = NULL;
    // snd_pcm_hw_params_t* params = NULL;
    // int rc;
    // rc = snd_pcm_open(&handle, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0);
    // if ( rc< 0) {
    //     std::cerr << "ALSA open error" << std::endl;
    //     return;
    // }
    // std::cout << "### capture_audio ... 001" << std::endl; 
    // snd_pcm_hw_params_alloca(&params);
    // std::cout << "### capture_audio ... 002" << std::endl;
    // snd_pcm_hw_params_any(handle, params);
    // std::cout << "### capture_audio ... 003" << std::endl;
    // rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    // if (rc < 0)
    // printf("set_access fail\n");
    // rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    //     if (rc < 0)
    // printf("set_format fail\n");
    // rc = snd_pcm_hw_params_set_channels(handle, params, CHANNELS);
    //     if (rc < 0)
    // printf("set_channels fail\n");
    // rc = snd_pcm_hw_params_set_rate_near(handle, params, (unsigned int*)&SAMPLE_RATE, 0);
    //     if (rc < 0)
    // printf("set_rate_near fail\n");
    // std::cout << "### capture_audio ... 004" << std::endl;
    // // 设置帧大小
    // snd_pcm_uframes_t frames = FRAME_SIZE;
    // snd_pcm_hw_params_set_period_size_near(handle, params, &frames, 0);
    // if (snd_pcm_hw_params(handle, params) < 0) {
    //     std::cerr << "ALSA params error" << std::endl;
    //     snd_pcm_close(handle);
    //     return;
    // }
    // std::cout << "### capture_audio ... 005" << std::endl;
    
    
    // 打开文件（二进制模式）
    // std::string filename = "outputPcm_001.pcm";
    // outputPcmFile.open(filename, std::ios::binary);  //  | std::ios::out
    // if (!outputFile.is_open()) {
    //     throw std::runtime_error("无法打开文件: " +  filename);
    // }

    // 音频采集循环
    std::vector<int16_t> buffer(FRAME_SIZE * CHANNELS);
    std::cout << "### capture_audio running ... " << std::endl;
    long int capture_cnt = 0;
    while (running) {
        int ret = snd_pcm_readi(pcm_handle_, buffer.data(), FRAME_SIZE);
        if (ret == -EPIPE) {
            // 处理overrun
            snd_pcm_prepare(pcm_handle_);
            continue;
        } else if (ret < 0) {
            std::cerr << "ALSA read error: " << snd_strerror(ret) << std::endl;
            break;
        }
        
        // 将数据添加到队列
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
            // 如果队列已满，等待空间
            if (audio_queue.size() >= BUFFER_COUNT) {
                cond_var.wait(lock, [this] { 
                    return !running || audio_queue.size() < BUFFER_COUNT; 
                });
            }
            
            if (!running) break;
            
            // 移动数据到队列（避免拷贝）
            audio_queue.emplace(buffer);
            capture_cnt++;
            // std::cout << "### push pcm_data to audio_queue, capture_cnt = " << capture_cnt << std::endl;

            // 写入PCM文件
            // outputPcmFile.write((const char*)buffer.data(), ret*2);

        }
        
        cond_var.notify_one();  // 通知处理线程
    }
    std::cout << "### capture_audio ... 009" << std::endl;
    
    snd_pcm_close(pcm_handle_);
}



void AudioCapture::process_audio() {
    // 设置实时优先级（需要root权限）
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 10;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    
    std::vector<uint8_t> opus_buffer(FRAME_SIZE * sizeof(int16_t)); // 足够大的缓冲区

    // // 打开文件（二进制模式）******************/
    // std::string filename = "output_001.opus";
    // outputFile.open(filename, std::ios::binary);  //  | std::ios::out
    // if (!outputFile.is_open()) {
    //     throw std::runtime_error("无法打开文件: " +  filename);
    // }
    /****************************************/

    long int send_cnt = 0, encoded_total=0;
    std::cout << "### process_audio running ... " << std::endl;
    long int  encodeCount = 0;
    while (running) {
        std::vector<int16_t> audio_data;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);     
            // 等待数据可用
            cond_var.wait(lock, [this] { 
                return !running || !audio_queue.empty(); 
            });
            if (!running) break;
            // 从队列获取数据
            audio_data = std::move(audio_queue.front());
            audio_queue.pop();
            // std::cout << "### take pcm_data from audio_queue" << std::endl;
        }
        cond_var.notify_one();  // 通知采集线程有空间了
        
        // Opus编码
        int encoded_size = opus_encode(encoder, 
                                        audio_data.data(), 
                                        FRAME_SIZE,
                                        opus_buffer.data(), 
                                        opus_buffer.size());
        if (encoded_size < 0) {
            std::cerr << "Opus encoding error: " << opus_strerror(encoded_size) << std::endl;
            continue;
        }
        encoded_total += encoded_size;
        // std::cout << "### opus_encode opus_buffer.size() = " << opus_buffer.size() <<  std::endl;
        // std::cout << "### encoded_size = " << encoded_size << "  encoded_total = " << encoded_total << std::endl;
        // std::cout << "### opus_encode a audio_data ... " << std::endl;
        /***************************/
        // 生成文件名
        // std::ostringstream filename;
        // filename << "test" << std::setw(3) << std::setfill('0') << (frameCount + 1) << ".opus";
        // // 写入文件
        // std::ofstream frameFile(filename.str(), std::ios::binary);
        // frameFile.write(reinterpret_cast<char*>(opus_buffer.data()), opus_buffer.size());
        // frameFile.close();
        // frameCount++;
        /**************************/
        // outputFile.write(reinterpret_cast<char*>(opus_buffer.data()), encoded_size);
        
        // UDP发送
        int udp_err = sendto(udp_socket, 
                    opus_buffer.data(), 
                    encoded_size,
                    0, 
                    (struct sockaddr*)&target_addr, 
                    sizeof(target_addr));
        send_cnt++;
        // std::cout << "[process_audio] UDP sendto opus_buffer, udp_err = " << udp_err << " send_cnt = "<< send_cnt << std::endl;
    }
    std::cout << "[process_audio] ... 009 " << std::endl;
}


int audioIn_main() {
    try {
        AudioCapture processor;
        processor.start();
        
        std::cout << "Audio processing started. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        processor.stop();
        std::cout << "Stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}