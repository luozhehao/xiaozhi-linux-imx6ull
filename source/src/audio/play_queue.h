
#ifndef PLAY_QUEUE_H
#define PLAY_QUEUE_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <cstring>
#include <atomic>
#include <system_error>
#include <opus/opus.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <speex/speex_resampler.h>  // 新增重采样头文件
#include <unistd.h>

// 音频参数结构体
struct AudioPlayParams {
    unsigned int sample_rate;      // 采样率 (Hz)
    snd_pcm_format_t format;       // 采样格式
    unsigned int channels;         // 通道数
    unsigned long period_size;      // 每次读取的帧数
    unsigned int buffer_time;      // 缓冲区时间 (ms)
    std::string device_name;       // ALSA设备名称
    unsigned long buf_size;
};

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer() {
        stop();
        if (decoder) opus_decoder_destroy(decoder);
        if (udp_socket >= 0) close(udp_socket);
        if (alsa_handle) {
            snd_pcm_drain(alsa_handle);
            snd_pcm_close(alsa_handle);
        }
    }
    void start();
    void stop();
    // void OnIncomingAudio(std::function<void(std::unique_ptr<AudioPlayer> packet)> callback);
    // void HandleAudioPacket(std::unique_ptr<AudioPlayer> packet);

private:
    void init_opus_decoder();
    void init_udp();
    void init_alsa();
    void init_speex_resampler();
    void receive_audio();
    void play_audio();
    void decBufToPlayBuf(std::vector<int16_t> pcm_data, unsigned char *playBuffer);

    // std::function<void(std::unique_ptr<AudioPlayer> packet)> on_incoming_audio_;

    struct opus_encoder {
        unsigned int inputSampleRate;
        unsigned int inputChannels;
        unsigned int outputSampleRate;
        unsigned int outputChannels;
        unsigned int duration_ms;
        SpeexResamplerState* resampler;
        OpusEncoder* encoder;
    };

    struct opus_decoder {
        int inputSampleRate;
        int inputChannels;
        int outputSampleRate;
        int outputChannels;
        int duration_ms;
        SpeexResamplerState* resampler;
        OpusDecoder* decoder;
    };

    opus_encoder g_opus_encoder;
    opus_decoder g_opus_decoder;

    // 线程控制
    std::atomic<bool> running;
    std::thread receive_thread;
    std::thread play_thread;
    
    // PCM数据队列
    std::queue<std::vector<int16_t>> pcm_queue;
    std::mutex queue_mutex;
    std::condition_variable cond_var;
    
    // Opus解码器
    OpusDecoder* decoder;
    SpeexResamplerState* resampler;
    
    // 网络
    int udp_socket;
    AudioPlayParams actual_params_;
    // ALSA播放设备
    snd_pcm_t* alsa_handle;
    std::vector<int16_t> fifo_buffer;
    std::mutex fifo_mutex;
    std::vector<int16_t> carry_buf;
    unsigned char *playBuffer = NULL;
    int16_t g_play_buffer[3*1024];
    std::vector<int16_t> playBuf;

    // 音频配置 const  constexpr
    const int SAMPLE_RATE = 16000;   // 16kHz采样率
    const int CHANNELS = 1;          // 单声道
    const int FRAME_SIZE = 960;      // 60ms帧 (16000Hz * 0.06s)
    const int PLAY_CHANNELS = 2;          // 单声道
    const int PLAY_SAMPLE_RATE = 16000;   // 16kHz采样率
    // 网络配置
    const int UDP_LISTEN_PORT = 5677;  // AUDIO_PORT_DOWN = 5677 
    const int MAX_UDP_PACKET_SIZE = 1500; // 典型MTU大小
    // 环形缓冲区配置
    const int PCM_BUFFER_COUNT = 10; // 约1秒的缓冲 (15 * 60ms = 900ms)

};


#endif