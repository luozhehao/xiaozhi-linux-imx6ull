

#include "board.h"

#include "volume_alsa.h"

Board::Board() {
    // music_ = nullptr;  // 先初始化为空指针
    
    // // Settings settings("board", true);
    // uuid_ = settings.GetString("uuid");
    // if (uuid_.empty()) {
    //     uuid_ = GenerateUuid();
    //     settings.SetString("uuid", uuid_);
    // }
    // // ESP_LOGI(TAG, "UUID=%s SKU=%s", uuid_.c_str(), BOARD_NAME);
    
    // // 初始化音乐播放器
    // music_ = new Esp32Music();
    // ESP_LOGI(TAG, "Music player initialized for all boards");
}

Board::~Board() {
    // if (music_) {
    //     delete music_;
    //     music_ = nullptr;
    //     ESP_LOGI(TAG, "Music player destroyed");
    // }
}

std::string Board::GenerateUuid() {
    // // UUID v4 需要 16 字节的随机数据
    // uint8_t uuid[16];
    
    // // 使用 ESP32 的硬件随机数生成器
    // esp_fill_random(uuid, sizeof(uuid));
    
    // // 设置版本 (版本 4) 和变体位
    // uuid[6] = (uuid[6] & 0x0F) | 0x40;    // 版本 4
    // uuid[8] = (uuid[8] & 0x3F) | 0x80;    // 变体 1
    
    // // 将字节转换为标准的 UUID 字符串格式
    // char uuid_str[37];
    // snprintf(uuid_str, sizeof(uuid_str),
    //     "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    //     uuid[0], uuid[1], uuid[2], uuid[3],
    //     uuid[4], uuid[5], uuid[6], uuid[7],
    //     uuid[8], uuid[9], uuid[10], uuid[11],
    //     uuid[12], uuid[13], uuid[14], uuid[15]);
    
    // return std::string(uuid_str);
}



std::string Board::GetBoardType(){}
int Board::GetAudioCodec() {}
bool Board::GetTemperature(float& esp32temp){}
void Board::StartNetwork() {}
const char* Board::GetNetworkStateIcon() {}
bool Board::GetBatteryLevel(int &level, bool& charging, bool& discharging){}
std::string Board::GetJson(){}
void Board::SetPowerSaveMode(bool enabled) {}
std::string Board::GetBoardJson() {}


std::string Board::GetDeviceStatusJson() {
    /*
        * 返回设备状态JSON
        * 
        * 返回的JSON结构如下：
        * {
        *     "audio_speaker": {
        *         "volume": 70
        *     },
        *     "screen": {
        *         "brightness": 100,
        *         "theme": "light"
        *     },
        *     "battery": {
        *         "level": 50,
        *         "charging": true
        *     },
        *     "network": {
        *         "type": "wifi",
        *         "ssid": "Xiaozhi",
        *         "rssi": -60
        *     },
        *     "chip": {
        *         "temperature": 25
        *     }
        * }
        */  


    // try {
    //     VolumeController volume;
    //     int vol = volume.get_volume();
    //     int percent = volume.get_volume_percent();
    //     int min, max;
    //     volume.get_volume_range(min, max);
        
    //     std::cout << "当前音量: " << vol << " (" << percent << "%)" << std::endl;
    //     std::cout << "音量范围: " << min << " - " << max << std::endl;
    // } catch (const std::exception& e) {
    //     std::cerr << "ALSA测试失败: " << e.what() << std::endl;
    // }
    std::cout<< "GetDeviceStatusJson ... 001" << std::endl;
    VolumeController volume;
    int vol = volume.get_volume();

    auto& board = Board::GetInstance();
    auto root = cJSON_CreateObject();

    // Audio speaker
    auto audio_speaker = cJSON_CreateObject();
    // auto audio_codec = board.GetAudioCodec();
    if (vol) {
        cJSON_AddNumberToObject(audio_speaker, "volume", vol);
    }
    cJSON_AddItemToObject(root, "audio_speaker", audio_speaker);

    // Screen brightness
    // auto backlight = board.GetBacklight();
    // auto screen = cJSON_CreateObject();
    // if (backlight) {
    //     cJSON_AddNumberToObject(screen, "brightness", backlight->brightness());
    // }
    // auto display = board.GetDisplay();
    // if (display && display->height() > 64) { // For LCD display only
    //     cJSON_AddStringToObject(screen, "theme", display->GetTheme().c_str());
    // }
    // cJSON_AddItemToObject(root, "screen", screen);     


    auto json_str = cJSON_PrintUnformatted(root);
    std::string json(json_str);
    std::cout<< "GetDeviceStatusJson ... 009, json: " << json << std::endl;

    cJSON_free(json_str);
    cJSON_Delete(root);
    return json;
}



VolumeController* Board::GetAudioAmx(){
    static VolumeController instance;
    return &instance;
}


