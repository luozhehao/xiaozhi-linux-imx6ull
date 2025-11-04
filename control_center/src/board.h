
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


class Board {
private:
    Board(const Board&) = delete; // 禁用拷贝构造函数
    Board& operator=(const Board&) = delete; // 禁用赋值操作

protected:
    Board();
    std::string GenerateUuid();

    // 软件生成的设备唯一标识
    std::string uuid_;
    
    // 音乐播放器实例
    // Music* music_;

public:
    static Board& GetInstance() {
        // static Board* instance = static_cast<Board*>(create_board());
        static Board* instance;
        return *instance;
    }

    virtual ~Board();  // 改为非默认析构函数，用于清理 music_
    virtual std::string GetBoardType() = 0;
    virtual std::string GetUuid() { return uuid_; }
    // virtual Backlight* GetBacklight() { return nullptr; }
    // virtual Led* GetLed();
    // virtual AudioCodec* GetAudioCodec() = 0;
    virtual int GetAudioCodec() = 0;
    virtual bool GetTemperature(float& esp32temp);
    // virtual Display* GetDisplay();
    // virtual Camera* GetCamera();
    // virtual Music* GetMusic();
    // virtual NetworkInterface* GetNetwork() = 0;
    virtual void StartNetwork() = 0;
    virtual const char* GetNetworkStateIcon() = 0;
    virtual bool GetBatteryLevel(int &level, bool& charging, bool& discharging);
    virtual std::string GetJson();
    virtual void SetPowerSaveMode(bool enabled) = 0;
    virtual std::string GetBoardJson() = 0;
    // virtual std::string GetDeviceStatusJson() = 0;
    std::string GetDeviceStatusJson();
    VolumeController* GetAudioAmx();

    // 音乐播放器实例
    Music* music_;
};



#endif