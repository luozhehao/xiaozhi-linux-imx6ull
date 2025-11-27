
#ifndef BOARD_H
#define BOARD_H

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <variant>
#include <optional>
#include <stdexcept>
#include <thread>
#include <iostream>
#include <stdio.h>
#include <cJSON.h>

#include "volume_alsa.h"
#include "music.h"
#include "esp32_music.h"
// #include "esp_network.h"
#include "MusicService.h"


class Board {
private:
    Board(const Board&) = delete; // 禁用拷贝构造函数
    Board& operator=(const Board&) = delete; // 禁用赋值操作

protected:
    Board();
    std::string GenerateUuid();

    // 软件生成的设备唯一标识
    std::string uuid_;
    
    // // 音乐播放器实例
    // Music* music_;

public:
    static Board& GetInstance() {
        // static Board* instance = static_cast<Board*>(create_board());
        static Board instance;
        return instance;
    }

     ~Board();  // 改为非默认析构函数，用于清理 music_
     std::string GetBoardType();
     std::string GetUuid() { return uuid_; }
    // virtual Backlight* GetBacklight() { return nullptr; }
    // virtual Led* GetLed();
    // virtual AudioCodec* GetAudioCodec() = 0;
     int GetAudioCodec();
     bool GetTemperature(float& esp32temp);
    // virtual Display* GetDisplay();
    // virtual Camera* GetCamera();
    // virtual Music* GetMusic();
    // virtual NetworkInterface* GetNetwork() = 0;
     void StartNetwork();
     const char* GetNetworkStateIcon();
     bool GetBatteryLevel(int &level, bool& charging, bool& discharging);
     std::string GetJson();
     void SetPowerSaveMode(bool enabled) ;
     std::string GetBoardJson();
    // virtual std::string GetDeviceStatusJson() = 0;
    std::string GetDeviceStatusJson();
    VolumeController* GetAudioAmx();
    MusicService* GetMusic();

    // 音乐播放器实例
    Music* music_;
    MusicService* music_svr_;

    // NetworkInterface* GetNetwork();
};



#endif