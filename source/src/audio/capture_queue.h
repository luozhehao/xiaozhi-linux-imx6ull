#ifndef CAPTURE_QUEUE_H
#define CAPTURE_QUEUE_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <cstring>
#include <system_error>
#include <atomic>
#include <fstream>

#include <opus/opus.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #include "audio_capture.h"



// 音频参数结构体
struct AudioCapParams {
    unsigned int sample_rate;      // 采样率 (Hz)
    snd_pcm_format_t format;       // 采样格式
    unsigned int channels;         // 通道数
    unsigned long period_size;      // 每次读取的帧数
    unsigned int buffer_time;      // 缓冲区时间 (ms)
    std::string device_name;       // ALSA设备名称
};


class AudioCapture {
public:
    AudioCapture();

    ~AudioCapture() {
        stop();
        if (encoder) opus_encoder_destroy(encoder);
        if (udp_socket >= 0) close(udp_socket);
        if (outputFile.is_open()) {
            outputFile.close();
        }  
        if (outputPcmFile.is_open()) {
            outputPcmFile.close();
        }
    }

    void start();

    void stop();

private:
    bool init();
    void init_opus_decoder();
    void init_speex_resampler();
    void init_udp();
    
    void capture_audio();

    void process_audio();

    // 线程控制
    std::atomic<bool> running;
    std::thread capture_thread;
    std::thread process_thread;
    
    // 音频队列
    std::queue<std::vector<int16_t>> audio_queue;
    std::mutex queue_mutex;
    std::condition_variable cond_var;
    
    // Opus编码器
    OpusEncoder* encoder;

    // 音频设备
    AudioCapParams params_;
    AudioCapParams actual_params_;
    std::atomic<bool> is_capturing_;
    snd_pcm_t* pcm_handle_;
    
    // 网络
    int udp_socket;
    struct sockaddr_in target_addr;

    std::ofstream outputFile; // 文件输出流成员变量
    std::ofstream outputOpusFile; // 文件输出流成员变量
    std::ofstream outputPcmFile; // 文件输出流成员变量


    // 音频配置  constexpr const
    const int SAMPLE_RATE = 16000;   // 48000
    const int CHANNELS = 1;          // 单声道
    const int FRAME_SIZE = 960;       // 960 = 20ms帧 (48000Hz * 0.02s)  960 = 16000*0.06  (60ms)
    const int BITRATE = 16000;        // 16kbps

    // 网络配置
    const  char *UDP_TARGET_IP = "127.0.0.1";     //"192.168.1.24";
    const int UDP_TARGET_PORT = 5676;   // AUDIO_PORT_UP    5676   

    // 环形缓冲区配置
    const int BUFFER_COUNT = 30;     // 缓冲10帧数据

};



#endif
