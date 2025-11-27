
#include <dirent.h>
#include <fstream>
#include <iostream>
#include "MusicService.h"
#include "application.h"



int MusicService::Start(){
    std::cout << "### MusicService start ... " << std::endl;
    service_stopped_ = false;
    // 启动音频decode线程
    decode_thread = std::thread(&MusicService::AudioDecodeThread, this);
    decode_thread.detach();

    // play_thread = std::thread(&MusicService::AudioOutputThread, this);
    // play_thread.detach();
    
    // // 启动处理线程
    // output_thread = std::thread(&MusicService::AudioOutputTask, this);
    // output_thread.detach();

    // opus_thread = std::thread(&MusicService::OpusCodecTask, this);
    // opus_thread.detach();

    std::cout << "### MusicService finish. " << std::endl;
    return 0;
}

void MusicService::Stop() {
    service_stopped_ = true;
    audio_queue_cv_.notify_all();  // 唤醒所有等待线程
    
    if (decode_thread.joinable()) decode_thread.join();
    if (play_thread.joinable()) play_thread.join();
}

bool MusicService::Play(std::string song_name, std::string artist_name) {
    std::string outfile = "001.pcm";
    std::string artist = artist_name;
    scanMusicLibrary("music");
    std::string song_name_path = findSongFile(song_name);
    decoder.setInfile(song_name_path);
    decoder.setOutfile(outfile);
    Start();
    return true;
}

void MusicService::AudioDecodeThread(){
    std::ifstream mp3(decoder.getInfile(), std::ios::binary);
    int skip_bytes = decoder.skip_id3v2(mp3);
    if(skip_bytes){
        std::cout << "[decode] skip_bytes = " << skip_bytes << std::endl;
    }
    
    std::ofstream pcmfile(decoder.getOutfile(), std::ios::binary);

    const size_t buf_size = 1024;
    std::vector<unsigned char> buf(buf_size);
    std::vector<short> pcm_out;
    long int total_read = 0;
    auto& app = Application::GetInstance();
    long int total_played = 0;

    while (true) {
        pcm_out.clear();
        mp3.read((char*)buf.data(), buf_size);
        size_t read_bytes = mp3.gcount();
        if (read_bytes == 0){
            decoding_finished_ = true;
            std::cout << "[decode] AudioDecode, end of file." << std::endl;
            break;
        }
            
        total_read += read_bytes ;
        // std::cout << "[decode] read_bytes = " << read_bytes 
        // << " total_read = " << total_read << std::endl;
        
        decoder.decode(buf.data(), read_bytes, pcm_out);
        // std::cout << "[decode] pcm_out.size() =  " << pcm_out.size() << std::endl;
        playChannels = decoder.getMp3Channels();
        int playSamplerate = decoder.getMp3Samplerate();

        if (!pcm_out.empty()){
            // pcmfile.write((char*)pcm_out.data(), pcm_out.size() * sizeof(short));
            // std::unique_lock<std::mutex> lock(audio_queue_mutex_);          
            // audio_playback_queue_.push(std::move(pcm_out));
            // audio_queue_cv_.notify_all();
            /****************/
            // 创建AudioStreamPacket
            PcmStreamPacket packet;
            packet.sample_rate = playSamplerate;
            packet.frame_duration = 60;  // 使用Application默认的帧时长
            packet.timestamp = 0;
            // 将int16_t PCM数据转换为uint8_t字节数组
            // size_t pcm_size_bytes = pcm_out.size() * sizeof(int16_t);
            packet.payload.resize(pcm_out.size());
            // memcpy(packet.payload.data(), final_pcm_data, pcm_size_bytes);
            packet.payload = pcm_out;
            // 发送到Application的音频解码队列
            app.AddAudioData(std::move(packet));
            total_played += pcm_out.size();
        }
        else{
            std::cout << "[decode] AudioDecode finish.  pcm_out empty" << std::endl;
            decoding_finished_ = true;
            break;
        }
    }

    // flush end of stream
    decoder.decode(nullptr, 0, pcm_out);
    if (!pcm_out.empty())
        pcmfile.write((char*)pcm_out.data(), pcm_out.size() * sizeof(short));  
    std::cout << "[decode] AudioDecodeThread ... 009 "  << std::endl; 
}



void MusicService::AudioOutputThread(){

    // 设置实时优先级（需要root权限）
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 5;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    // Logger::info("AudioOutputTask ... 001");
    while (true) {
        std::cout << "[AudioOutput] while loop begin ..."  << std::endl;
        std::unique_lock<std::mutex> lock(audio_queue_mutex_);
        audio_queue_cv_.wait(lock, [this]() { return !audio_playback_queue_.empty() || 
            service_stopped_ || 
            (decoding_finished_ && audio_playback_queue_.empty()); 
        });
        // 退出条件1：服务停止
        if (service_stopped_) {
            std::cout << "[AudioOutput] service_stopped_ == true "  << std::endl; 
            break;
        }
        // 退出条件2：解码完成且队列已空
        if (decoding_finished_ && audio_playback_queue_.empty()) {
            std::cout << "[AudioOutput] 所有音频数据播放完成，退出线程" << std::endl;
            playback_finished_ = true;
            break;
        }

        auto pcm = std::move(audio_playback_queue_.front());
        audio_playback_queue_.pop();
        audio_queue_cv_.notify_all();
        lock.unlock();

        // if (!codec_->output_enabled()) {
        //     codec_->EnableOutput(true);
        //     esp_timer_start_periodic(audio_power_timer_, AUDIO_POWER_CHECK_INTERVAL_MS * 1000);
        // }

        // int mp3_SampleRate = decoder.getMp3Samplerate();
        // if(mp3_SampleRate != play.output_sample_rate()){
        //     play.SetOutputSampleRate(mp3_SampleRate);
        // }
        // std::cout << "[AudioOutput] pcm.size() = " << pcm.size() << std::endl;
        // play.OutputData(pcm);
        // std::cout << "[AudioOutput] OutputData 1 sample pcm " << std::endl;

        /* Update the last output time */
        // last_output_time_ = std::chrono::steady_clock::now();
        // debug_statistics_.playback_count++;

    #if CONFIG_USE_SERVER_AEC
        /* Record the timestamp for server AEC */
        if (task->timestamp > 0) {
            lock.lock();
            timestamp_queue_.push_back(task->timestamp);
        }
    #endif
    }
    std::cout << "[AudioOutput] AudioOutputThread ... 009 "  << std::endl;
}

void MusicService::reportStatus(const std::string& message) {
    if (status_callback_) {
        status_callback_(message);
    } else {
        std::cout << "[MusicPlayer] " << message << std::endl;
    }
}


bool MusicService::scanMusicLibrary(const std::string& musicDir) {
    // reportStatus("扫描音乐库: " + musicDir);
    std::cout << "扫描音乐库: " + musicDir << std::endl;
    playlist_.clear();
    song_map_.clear();
    
    DIR* dir = opendir(musicDir.c_str());
    if (!dir) {
        reportStatus("无法打开目录: " + musicDir);

        return false;
    }
    
    struct dirent* entry;
    int fileCount = 0;
    
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        
        // 检查文件扩展名
        if (filename.length() > 4 && 
            filename.substr(filename.length() - 4) == ".mp3") {
            
            std::string fullpath = musicDir + "/" + filename;
            std::string songName = filename.substr(0, filename.length() - 4); // 去掉 .mp3
            
            playlist_.push_back(songName);
            song_map_[songName] = fullpath;
            fileCount++;
            
            // reportStatus("找到歌曲: " + songName);
            std::cout << "找到歌曲: " + songName << std::endl;
        }
    }
    
    closedir(dir);
    
    // if (playlist_callback_) {
    //     playlist_callback_(playlist_);
    // }
    for(auto song : playlist_){
        std::cout << "扫描音乐: " + song << std::endl;
    }
    
    reportStatus("扫描完成，找到 " + std::to_string(fileCount) + " 首歌曲");
    return fileCount > 0;
}


std::string MusicService::findSongFile(const std::string& songName) const {
    auto it = song_map_.find(songName);
    if (it != song_map_.end()) {
        return it->second;
    }
    
    // 模糊匹配：不区分大小写
    std::string lowerSongName = songName;
    std::transform(lowerSongName.begin(), lowerSongName.end(), lowerSongName.begin(), ::tolower);
    
    for (const auto& pair : song_map_) {
        std::string key = pair.first;
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        
        if (key.find(lowerSongName) != std::string::npos) {
            return pair.second;
        }
    }
    
    return "";
}