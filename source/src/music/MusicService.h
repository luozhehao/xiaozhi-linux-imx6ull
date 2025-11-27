

#ifndef MUSICSERVICE_H
#define MUSICSERVICE_H

#include <thread>
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <condition_variable>
#include <functional>
#include <algorithm>

#include <stdio.h>

#include "Mp3Decoder.h"
#include "audio_play.h"
// #include "PcmPlayer.h"


struct PcmStreamPacket {
    int sample_rate = 0;
    int frame_duration = 0;
    uint32_t timestamp = 0;
    // std::vector<uint8_t> payload;
    std::vector<int16_t> payload;
};

class MusicService{
public:
    MusicService(){}
    ~MusicService() { Stop();}

    static MusicService& GetInstance() {
        static MusicService instance;
        return instance;
    }

    // 回调函数类型
    using StatusCallback = std::function<void(const std::string& message)>;

    // int MusicServiceInit();
    // int process_thread();

    int Start();
    void Stop();
    void AudioDecodeThread();
    void AudioOutputThread();
    bool Play(std::string song_name, std::string artist_name);

    // 回调设置
    void setStatusCallback(StatusCallback callback) { status_callback_ = callback; }
    bool scanMusicLibrary(const std::string &musicDir);
    std::string findSongFile(const std::string &songName) const ;
    std::vector<std::string> getPlaylist() { return playlist_; }
    bool getPlayState() { return playback_finished_; }

private:

    // 回调函数
    StatusCallback status_callback_;

    void reportStatus(const std::string& message);

    bool service_stopped_ = true;
    int playChannels;
    std::atomic<bool> playback_finished_{false};    // 播放完成标志
    std::atomic<bool> decoding_finished_{false};    // 解码完成标志

    // 播放列表
    std::vector<std::string> playlist_;
    std::map<std::string, std::string> song_map_; // 歌曲名 -> 文件路径

public:
    std::thread decode_thread;
    std::thread play_thread;
    Mp3Decoder decoder;
    // AudioPlayer play;
    // PcmPlayer player;

    std::mutex audio_queue_mutex_;
    std::condition_variable audio_queue_cv_;
    std::queue<std::vector<int16_t>> audio_playback_queue_;
// std::deque<std::unique_ptr<AudioTask>> audio_playback_queue_;
    std::queue<std::vector<int16_t>> pcmQueue;
    std::mutex mtx;
    std::condition_variable cv;
    bool finished = false;

};


#endif