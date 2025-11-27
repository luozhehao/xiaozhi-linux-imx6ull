// SPDX-License-Identifier: GPL-3.0-only
/*

 */
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <random>
#include <sstream>
#include <iomanip>

// Include nlohmann/json library
#include "json.hpp"

// #include "websocket_client.h"
#include "http.h"
#include "ipc_udp.h"

#include "uuid.h"
#include "uuid_class.h"

#include "cfg.h"
#include "mcp_server.h"
#include <cJSON.h>

#include "volume_alsa.h"
#include "websocket_protocol.h"
#include "debug.h"
#include "logger.h"
// #include "audio_process.h"
#include "audio_service.h"
#include "application.h"

#include "MusicService.h"

// extern int AudioProcessInit();

#if 0
using json = nlohmann::json;
static int g_audio_upload_enable = 1;
static std::string g_session_id;
std::string mcp_session_id;

typedef enum ListeningMode {
    kListeningModeAutoStop,
    kListeningModeManualStop,
    kListeningModeAlwaysOn // 需要 AEC 支持
} ListeningMode;

// 定义设备状态枚举类型
typedef enum DeviceState {
    kDeviceStateUnknown,
    kDeviceStateStarting,
    kDeviceStateWifiConfiguring,
    kDeviceStateIdle,
    kDeviceStateConnecting,
    kDeviceStateListening,
    kDeviceStateSpeaking,
    kDeviceStateUpgrading,
    kDeviceStateActivating,
    kDeviceStateFatalError
} DeviceState;

static p_ipc_endpoint_t g_ipc_ep_audio;
static p_ipc_endpoint_t g_ipc_ep_ui;
static DeviceState g_device_state = kDeviceStateUnknown;

static void set_device_state(DeviceState state)
{
    g_device_state = state;
}

static void send_device_state(void)
{
    std::string stateString = "{\"state\":" + std::to_string(g_device_state) + "}";
    g_ipc_ep_ui->send(g_ipc_ep_ui, stateString.data(), stateString.size());
}

static void send_stt(const std::string& text)
{
    if (!g_ipc_ep_ui) {
        std::cerr << "Error: g_ipc_ep_ui is nullptr" << std::endl;
        return;
    }

    try {
        json j;
        j["text"] = text;
        std::string textString = j.dump();
        g_ipc_ep_ui->send(g_ipc_ep_ui, textString.data(), textString.size());
    } catch (const std::exception& e) {
        std::cerr << "Error creating JSON string: " << e.what() << std::endl;
    }
}

static void process_opus_data_downloaded(const char *buffer, size_t size)
{
#if 0    
    std::cout << "Received opus data: " << size << " bytes" << std::endl;
    static int file_number = 1;
    // 构造文件名
    char filename[20];
    snprintf(filename, sizeof(filename), "test%03d.opus", file_number);

    // 打开文件
    FILE *file = fopen(filename, "wb");
    if (file) {
        // 写入Opus数据
        fwrite(buffer, 1, size, file);
        fclose(file);
        file_number++; // 增加文件编号
    } else {
        fprintf(stderr, "Failed to open file %s for writing\n", filename);
    }     
#endif    
    g_ipc_ep_audio->send(g_ipc_ep_audio, buffer, size);
}

static void send_start_listening_req(ListeningMode mode)
{
    std::string startString = "{\"session_id\":\"" + g_session_id + "\"";

    startString += ",\"type\":\"listen\",\"state\":\"start\"";

    if (mode == kListeningModeAutoStop) {
        startString += ",\"mode\":\"auto\"}";
    } else if (mode == kListeningModeManualStop) {
        startString += ",\"mode\":\"manual\"}";
    } else if (mode == kListeningModeAlwaysOn) {
        startString += ",\"mode\":\"realtime\"}";
    }

    try {
        //c->send(hdl, startString, websocketpp::frame::opcode::text);
        // auto& websk = WebsocketProtocol::GetInstance();
        WebsocketProtocol::GetInstance().websocket_send_text(startString.data(), startString.size());
        std::cout << "Send: " << startString << std::endl;    
    } catch (const websocketpp::lib::error_code& e) {
        std::cout << "Error sending message: " << e << " (" << e.message() << ")" << std::endl;
    }     
}

static void process_hello_json(const char *buffer, size_t size)
{
    json j = json::parse(buffer);
    int sample_rate = j["audio_params"]["sample_rate"];
    int channels = j["audio_params"]["channels"];
    std::cout << "Received valid 'hello' message with sample_rate: " << sample_rate << " and channels: " << channels << std::endl;     

    g_session_id = j["session_id"];
    std::cout << "### g_session_id: " << g_session_id  << std::endl;

    // std::string desc = R"(
    // {"session_id":"","type":"iot","update":true,"descriptors":[{"name":"Speaker","description":"扬声器","properties":{"volume":{"description":"当前音量值","type":"number"}},"methods":{"SetVolume":{"description":"设置音量","parameters":{"volume":{"description":"0到100之间的整数","type":"number"}}}}}]}
    // )";

    // // Send the new message              
    // try {
    //     //c->send(hdl, desc, websocketpp::frame::opcode::text);
    //     websocket_send_text(desc.data(), desc.size());
    //     std::cout << "### process_hello_json, Send: \n" << desc << std::endl;    
    // } catch (const websocketpp::lib::error_code& e) {
    //     std::cout << "Error sending message: " << e << " (" << e.message() << ")" << std::endl;
    // }

    // std::string desc2 = R"(
    // {"session_id":"","type":"iot","update":true,"descriptors":[{"name":"Backlight","description":"屏幕背光","properties":{"brightness":{"description":"当前亮度百分比","type":"number"}},"methods":{"SetBrightness":{"description":"设置亮度","parameters":{"brightness":{"description":"0到100之间的整数","type":"number"}}}}}]}
    // )";

    // // Send the new message 
    // try {
    //     //c->send(hdl, desc2, websocketpp::frame::opcode::text);
    //     websocket_send_text(desc2.data(), desc2.size());
    //     std::cout << "Send: " << desc2 << std::endl;    
    // } catch (const websocketpp::lib::error_code& e) {
    //     std::cout << "Error sending message: " << e << " (" << e.message() << ")" << std::endl;
    // }	

    // std::string desc3 = R"(
    // {"session_id":"","type":"iot","update":true,"descriptors":[{"name":"Battery","description":"电池管理","properties":{"level":{"description":"当前电量百分比","type":"number"},"charging":{"description":"是否充电中","type":"boolean"}},"methods":{}}]}
    //  )";
    // // Send the new message
    // try {
    //     //c->send(hdl, desc3, websocketpp::frame::opcode::text);
    //     websocket_send_text(desc3.data(), desc3.size());
    //     std::cout << "Send: " << desc3 << std::endl;    
    // } catch (const websocketpp::lib::error_code& e) {
    //     std::cout << "Error sending message: " << e << " (" << e.message() << ")" << std::endl;
    // }			

    std::string startString = R"(
        {"session_id":"","type":"listen","state":"start","mode":"auto"}
    )";
    
    try {
        //c->send(hdl, startString, websocketpp::frame::opcode::text);
        WebsocketProtocol::GetInstance().websocket_send_text(startString.data(), startString.size());
        std::cout << "Send: " << startString << std::endl;    
    } catch (const websocketpp::lib::error_code& e) {
        std::cout << "Error sending message: " << e << " (" << e.message() << ")" << std::endl;
    }
    g_audio_upload_enable = 1;

    // std::string state = R"(
    //     {"session_id":"","type":"iot","update":true,"states":[{"name":"Speaker","state":{"volume":80}},{"name":"Backlight","state":{"brightness":75}},{"name":"Battery","state":{"level":100,"charging":false}}]}
    // )";
    // try {
    //     //c->send(hdl, state, websocketpp::frame::opcode::text);
    //     websocket_send_text(state.data(), state.size());
    //     std::cout << "Send: " << state << std::endl;    
    // } catch (const websocketpp::lib::error_code& e) {
    //     std::cout << "Error sending message: " << e << " (" << e.message() << ")" << std::endl;
    // }
 
}

static void process_other_json(const char *buffer, size_t size)
{
    try {
        // Parse JSON data
        json j = json::parse(buffer);
        
        if (!j.contains("type"))
            return;
        
        if (j["type"] == "tts") {
            auto state = j["state"];
            if (state == "start") {
                // 下发语音, 可以关闭录音
                g_audio_upload_enable = 0;
                set_device_state(kDeviceStateListening);
                send_device_state();
            } else if (state == "stop") {
                // 本次交互结束, 可以继续上传声音
                // 等待一会以免她听到自己的话误以为再次对话
                sleep(2);
                send_start_listening_req(kListeningModeAutoStop);
                set_device_state(kDeviceStateListening);
                send_device_state();
                g_audio_upload_enable = 1;
            } else if (state == "sentence_start") {
                // 取出"text", 通知GUI
                // {"type":"tts","state":"sentence_start","text":"1加1等于2啦~","session_id":"eae53ada"}
                auto text = j["text"];
                send_stt(text.get<std::string>());
                send_start_listening_req(kListeningModeAutoStop);
                set_device_state(kDeviceStateSpeaking);
                send_device_state();
            }
        } else if (j["type"] == "stt") {
            // 表示服务器端识别到了用户语音, 取出"text", 通知GUI
            auto text = j["text"];
            send_stt(text.get<std::string>());
        } else if (j["type"] == "llm") {
            // 有"happy"等取值
        /*
            "neutral",
            "happy",
            "laughing",
            "funny",
            "sad",
            "angry",
            "crying",
            "loving",
            "embarrassed",
            "surprised",
            "shocked",
            "thinking",
            "winking",
            "cool",
            "relaxed",
            "delicious",
            "kissy",
            "confident",
            "sleepy",
            "silly",
            "confused"
        */          
            auto emotion = j["emotion"];
        } else if (j["type"] == "iot") {
            
        } else if (j["type"] == "mcp") {
            std::cout << "### recv mcp data " << std::endl;    // 2025.11.02 02:10  
            auto session_id = j["session_id"];
            if (session_id == nullptr || !session_id.is_string()) {
                // ESP_LOGE(TAG, "Missing method");
                return;
            }
            std::cout << "### session_id: " << session_id << std::endl;
            mcp_session_id = session_id;

#if 1
            auto payload = j["payload"];
            // const cJSON *root;
            // auto payload = cJSON_GetObjectItem(root, "payload");
            std::cout << "### recv mcp data 002" << std::endl;
            // if (cJSON_IsObject(payload)) {
            if (payload.is_object()){
                std::cout << "### recv mcp data 003" << std::endl;
                McpServer::GetInstance().ParseMessage(payload);
            }
            std::cout << "### recv mcp data finish." << std::endl;
#endif
        }

    } catch (json::parse_error& e) {
        std::cout << "Failed to parse JSON message: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cout << "Error processing message: " << e.what() << std::endl;
    }
}

static void process_txt_data_downloaded(const char *buffer, size_t size)
{
    try {
        // Parse the JSON message
        json j = json::parse(buffer);

        // Check if the message matches the expected structure
        if (j.contains("type") && j["type"] == "hello") {
            process_hello_json(buffer, size);
        } else {
            std::cout << "### websocket recv: " << buffer << std::endl;
            process_other_json(buffer, size);
        }
         
    } catch (json::parse_error& e) {
        std::cout << "Failed to parse JSON message: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cout << "Error processing message: " << e.what() << std::endl;
    }
}

int process_opus_data_uploaded(char *buffer, size_t size, void *user_data)
{
#if 0    
    static int file_number = 1;
    // 构造文件名
    char filename[20];
    snprintf(filename, sizeof(filename), "test%03d.opus", file_number);

    // 打开文件
    FILE *file = fopen(filename, "wb");
    if (file) {
        // 写入Opus数据
        fwrite(buffer, 1, size, file);
        fclose(file);
        file_number++; // 增加文件编号
    } else {
        fprintf(stderr, "Failed to open file %s for writing\n", filename);
    }   
#endif
    if (g_audio_upload_enable) {
        static int cnt = 0;
        if ((cnt++ % 100) == 0)
            std::cout << "Send opus data to server: " << size <<" count: "<< cnt << std::endl;
        WebsocketProtocol::GetInstance().websocket_send_binary(buffer, size);
    }
    return 0;
}

int process_ui_data(char *buffer, size_t size, void *user_data)
{
    return 0;
}
#endif 

#if 0
int main(int argc, char **argv)
{
    // Logger consoleLogger;
    Logger::info("Application started");
    Logger::warning("Low memory");
    Logger::error("File not found");
    char active_code[20] = "";

    auto& debug = Debug::GetInstance();
    debug.start();

    // auto& audio = AudioProcess::GetInstance();
    // audio.start();

    auto& audio_service_ = AudioService::GetInstance();
    audio_service_.Initialize();
    audio_service_.Start();


    McpServer::GetInstance().AddCommonTools();
    // Add MCP common tools before initializing the protocol
    // auto& mcp_server = McpServer::GetInstance();
    // mcp_server.AddCommonTools();
    std::cout << "main ... 003" << std::endl;


    g_ipc_ep_audio = ipc_endpoint_create_udp(AUDIO_PORT_UP, AUDIO_PORT_DOWN, process_opus_data_uploaded, NULL);
    g_ipc_ep_ui = ipc_endpoint_create_udp(UI_PORT_UP, UI_PORT_DOWN, process_ui_data, NULL);

    auto& uuidCreate = UuidGenerate::GetInstance();
    // uuidCreate.get_wireless_mac_address();
    // uuidCreate.generate_uuid();
    
    uuidCreate.httpParam_init();

    while (0 != active_device(&uuidCreate.http_data, active_code)) {
        if (active_code[0]) {
            std::string auth_code = "Active-Code: " + std::string(active_code);
            set_device_state(kDeviceStateActivating);
            send_device_state();
            send_stt(auth_code);
        }
        sleep(5);
    }

    set_device_state(kDeviceStateIdle);
    send_device_state();
    send_stt("设备已经激活");

    auto& websock = WebsocketProtocol::GetInstance();

    websock.param_init();
    //  websock.websocket_set_callbacks(process_opus_data_downloaded, process_txt_data_downloaded, &websock.ws_data);
    websock.OnIncomingBin(process_opus_data_downloaded);
    websock.OnIncomingTxt(process_txt_data_downloaded);
    websock.websocket_start();

    while (1)
    {
        sleep(1);
    }
}
#endif


int main(int argc, char* argv[]){

    // if(argc!=2){
    //     printf("usage: %s input.mp3 \n", argv[0]);
    //     // printf("usage: %s input.mp3  output.pcm \n", argv[0]);
    //     return -1;
    // }
    
    // Launch the application
    std::cout << " main ... 001" << std::endl;
    auto& app = Application::GetInstance();
    std::cout << " main ... 002" << std::endl;
    // app.inputfile = argv[1];
    app.Start();
    std::cout << " main ... 003" << std::endl;
    app.MainLoop();
    std::cout << " main ... 004" << std::endl;

    // std::cout << " main ... 001" << std::endl;
    // std::string infile = argv[1];
    // std::string outfile = "001.pcm";
    // auto& music = MusicService::GetInstance();
    // music.Play(infile);

    while(1){
        // if(music.getPlayState()){
        //     std::cout <<  " 播放结束" << std::endl;
        //     break;
        // }
        sleep(1);
    }
}


