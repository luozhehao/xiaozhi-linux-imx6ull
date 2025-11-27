#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

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
#include <memory>

#include <opus/opus.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "audio_capture.h"
#include "audio_play.h"
#include "opus_decoder.h"
#include "opus_encoder.h"


#define OPUS_FRAME_DURATION_MS 60
// #define MAX_ENCODE_TASKS_IN_QUEUE 2
#define MAX_ENCODE_TASKS_IN_QUEUE 10
#define MAX_PLAYBACK_TASKS_IN_QUEUE 2
#define MAX_DECODE_PACKETS_IN_QUEUE (2400 / OPUS_FRAME_DURATION_MS)
#define MAX_SEND_PACKETS_IN_QUEUE (2400 / OPUS_FRAME_DURATION_MS)
#define AUDIO_TESTING_MAX_DURATION_MS 10000
#define MAX_TIMESTAMPS_IN_QUEUE 3

#define AUDIO_POWER_TIMEOUT_MS 15000
#define AUDIO_POWER_CHECK_INTERVAL_MS 1000

struct AudioStreamPacket {
    int sample_rate = 0;
    int frame_duration = 0;
    uint32_t timestamp = 0;
    std::vector<uint8_t> payload;
    // std::vector<int16_t> payload;
};


enum AudioTaskType {
    kAudioTaskTypeEncodeToSendQueue,
    kAudioTaskTypeEncodeToTestingQueue,
    kAudioTaskTypeDecodeToPlaybackQueue,
};

struct AudioTask {
    AudioTaskType type;
    std::vector<int16_t> pcm;
    uint32_t timestamp;
};

struct DebugStatistics {
    uint32_t input_count = 0;
    uint32_t decode_count = 0;
    uint32_t encode_count = 0;
    uint32_t playback_count = 0;
};



class AudioService{
public:
    AudioService(){};
    ~AudioService(){};


    static AudioService& GetInstance() {
        static AudioService instance;
        return instance;
    }

    void Initialize();
    void Start();
    void Stop();
    void AudioInputTask();
    void AudioOutputTask();
    void OpusCodecTask();
    bool ReadAudioData(std::vector<int16_t> &data, int sample_rate, int samples);
    void PushTaskToEncodeQueue(AudioTaskType type, std::vector<int16_t> &&pcm);
    bool PushPacketToDecodeQueue(std::unique_ptr<AudioStreamPacket> packet, bool wait);
    std::unique_ptr<AudioStreamPacket> PopPacketFromSendQueue();
    void UpdateOutputTimestamp();

    
    AudioPlayer play;
    AudioPlayer &GetAudioPlayer() { return play; }
    // static AudioPlayer instance;
    // return instance; }

    AudioCapture capture;
    

    std::thread input_thread;
    std::thread output_thread;
    std::thread opus_thread;

    
    std::mutex audio_queue_mutex_;
    std::condition_variable audio_queue_cv_;
    std::deque<std::unique_ptr<AudioStreamPacket>> audio_decode_queue_;
    std::deque<std::unique_ptr<AudioStreamPacket>> audio_send_queue_;
    std::deque<std::unique_ptr<AudioStreamPacket>> audio_testing_queue_;
    std::deque<std::unique_ptr<AudioTask>> audio_encode_queue_;
    std::deque<std::unique_ptr<AudioTask>> audio_playback_queue_;
    std::deque<uint32_t> timestamp_queue_;

private:
    std::unique_ptr<OpusEncoderWrapper> opus_encoder_;
    std::unique_ptr<OpusDecoderWrapper> opus_decoder_;
    bool service_stopped_ = true;
    DebugStatistics debug_statistics_;

    std::chrono::steady_clock::time_point last_input_time_;
    std::chrono::steady_clock::time_point last_output_time_;

};



#endif