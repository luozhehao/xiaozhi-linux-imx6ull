
#ifndef APPLICATION_H
#define APPLICATION_H

#include "audio_service.h"
#include "websocket_protocol.h"
#include "device_state.h"
#include "MusicService.h"
#include "lcd_display.h"

class Application{

public:
    static Application& GetInstance() {
        static Application instance;
        return instance;
    }
    void Start();
    void MainLoop();
    DeviceState GetDeviceState() const { return device_state_; }
    void SetDeviceState(DeviceState state);
    void AddAudioData(AudioStreamPacket &&packet);
    // void AddAudioData(std::vector<short> packet);
    void AddAudioData(PcmStreamPacket &&pcm_pkt);

    std::string inputfile;

private:
    Application():ws_protocol_(WebsocketProtocol::GetInstance()){
        // ws_protocol_ = WebsocketProtocol::GetInstance();
    };
    ~Application(){};

    AudioService audio_service_;
    // MusicService music_;
    // std::unique_ptr<WebsocketProtocol> protocol_;
    WebsocketProtocol& ws_protocol_;  // 引用成员
    std::thread main_thread;
    volatile DeviceState device_state_ = kDeviceStateUnknown;
};

#endif