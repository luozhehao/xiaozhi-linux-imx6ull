
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
#include "board.h"
#include "audio_service.h"
#include "application.h"
#include "device_state.h"


using json = nlohmann::json;
static int g_audio_upload_enable = 1;
static std::string g_session_id;
std::string mcp_session_id;

typedef enum ListeningMode {
    kListeningModeAutoStop,
    kListeningModeManualStop,
    kListeningModeAlwaysOn // 需要 AEC 支持
} ListeningMode;


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
        std::cout << "[process_other_json] buffer: " << buffer << std::endl;
        // Parse JSON data
        json j = json::parse(buffer);
        if (!j.contains("type"))
            return;
        // {"type":"tts","state":"sentence_start","text":"好啊，来一个更炸的！","session_id":"b50a77b7"}
        if (j["type"] == "tts") {
            auto state = j["state"];
            if (state == "start") {
                // 下发语音, 可以关闭录音
                g_audio_upload_enable = 0;
                set_device_state(kDeviceStateListening);
                LcdDisplay::GetInstance().SetDeviceState(kDeviceStateListening);
                send_device_state();
            } else if (state == "stop") {
                // 本次交互结束, 可以继续上传声音
                // 等待一会以免她听到自己的话误以为再次对话
                sleep(1);
                send_start_listening_req(kListeningModeAutoStop);
                set_device_state(kDeviceStateListening);
                LcdDisplay::GetInstance().SetDeviceState(kDeviceStateListening);
                send_device_state();
                g_audio_upload_enable = 1;
            } else if (state == "sentence_start") {
                // 取出"text", 通知GUI
                // {"type":"tts","state":"sentence_start","text":"1加1等于2啦~","session_id":"eae53ada"}
                auto text = j["text"];
                // char *message = text.get<std::string>().c_str();
                LcdDisplay::GetInstance().SetChatMessage("assistant", text.get<std::string>().c_str());
                send_stt(text.get<std::string>());
                send_start_listening_req(kListeningModeAutoStop);
                set_device_state(kDeviceStateSpeaking);
                LcdDisplay::GetInstance().SetDeviceState(kDeviceStateSpeaking);
                send_device_state();
            }
        } else if (j["type"] == "stt") {
            // 表示服务器端识别到了用户语音, 取出"text", 通知GUI
            auto text = j["text"];
            LcdDisplay::GetInstance().SetChatMessage("user", text.get<std::string>().c_str());
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


void Application::Start(){
    // Logger consoleLogger;
    Logger::info("Application started");
    Logger::warning("Low memory");
    Logger::error("File not found");

    auto& display = LcdDisplay::GetInstance();
    // display.Start();
    display.SetChatMessage("assistant", "你好");
    display.SetChatMessage("user", "今天好累");
    display.SetChatMessage("assistant", "在干嘛");
    display.SetChatMessage("user", "哈哈，没干嘛");
    display.SetChatMessage("assistant", "最近还好吗");
    display.SetChatMessage("user", "就那样吧");
    display.SetChatMessage("assistant", "好吧");
    display.SetChatMessage("user", "谢谢关心");


/**********************/
Logger::info("[music] start");

    // std::cout << "[music] ... 001, inputfile = " + inputfile << std::endl;
    // music_.Play(inputfile, "");

Logger::info("[music] end");
/********************/
    char active_code[20] = "";
    auto& debug = Debug::GetInstance();
    debug.start();

    // auto& audio = AudioProcess::GetInstance();
    // audio.start();
    // auto& audio_service_ = AudioService::GetInstance();
    audio_service_.Initialize();
    audio_service_.Start();
    auto& board = Board::GetInstance();

#if 1
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
    Logger::warning("active_device before");
    while (0 != active_device(&uuidCreate.http_data, active_code)) {
        if (active_code[0]) {
            std::string auth_code = "Active-Code: " + std::string(active_code);
            set_device_state(kDeviceStateActivating);
            send_device_state();
            send_stt(auth_code);
        }
        Logger::warning("active_device ing ...");
        sleep(5);
    }
    Logger::warning("active_device finish");

    set_device_state(kDeviceStateIdle);
    send_device_state();
    send_stt("设备已经激活");

    // protocol_ = std::unique_ptr<WebsocketProtocol>(new WebsocketProtocol);
    // protocol_= McpServer::GetInstance();  //NG    
    // ws_protocol_ = WebsocketProtocol::GetInstance();
    // auto& websock = WebsocketProtocol::GetInstance();

    ws_protocol_.param_init();
    ws_protocol_.OnIncomingAudio([this](std::unique_ptr<AudioStreamPacket> packet, bool wait){
    // if (device_state_ == kDeviceStateSpeaking) {
    if (packet != nullptr) {
        audio_service_.PushPacketToDecodeQueue(std::move(packet), wait);
    } });

    ws_protocol_.OnIncomingJson(process_txt_data_downloaded);
    // ws_protocol_.OnIncomingJson([this, display](const char *buffer, size_t size) {
    //     // Parse the JSON message
    //     json j = json::parse(buffer);
    //     // Check if the message matches the expected structure
    //     if (j.contains("type") && j["type"] == "hello") {
    //         process_hello_json(buffer, size);
    //     } else {
    //         std::cout << "### websocket recv: " << buffer << std::endl;
    //         process_other_json(buffer, size);
    //     }
    // });


    ws_protocol_.OnData( [this](const char* data, size_t len, bool binary){
        if(binary){
            if (ws_protocol_.on_incoming_audio_ != nullptr) {
                auto packet = std::unique_ptr<AudioStreamPacket>(new AudioStreamPacket);
                packet->sample_rate = 16000;
                packet->frame_duration = 60;
                packet->timestamp = 0;
                packet->payload.assign((uint8_t *)data, (uint8_t *)data + len);
                ws_protocol_.on_incoming_audio_(std::move(packet), binary);


                // ws_protocol_.on_incoming_audio_( std::unique_ptr<AudioStreamPacket>(new AudioStreamPacket{
                //     16000,          //.sample_rate = 16000,
                //     60,             //.frame_duration = 60,
                //     0,              //.timestamp = 0,
                //     std::vector<uint8_t>((uint8_t *)data, (uint8_t *)data + len)    // .payload =                 
                // }), binary );
                // on_incoming_audio_(std::make_unique<AudioStreamPacket>(AudioStreamPacket{
                //     .sample_rate = server_sample_rate_,
                //     .frame_duration = server_frame_duration_,
                //     .timestamp = 0,
                //     .payload = std::vector<uint8_t>((uint8_t *)data, (uint8_t *)data + len)}));
            }}});
    // websock.websocket_set_callbacks(process_opus_data_downloaded, process_txt_data_downloaded, &websock.ws_data);
    // ws_protocol_.OnIncomingAudio(process_opus_data_downloaded);

    ws_protocol_.websocket_start();

#endif    

    // 启动主线程
    main_thread = std::thread(&Application::MainLoop, this);
    main_thread.detach(); 

}

void Application::MainLoop(){
    long int cnt = 0;
    while(true){
        cnt++;

        // if(music_.getPlayState()){
        //     std::cout <<  " 播放结束" << std::endl;
        //     // break;
        // }

        if(ws_protocol_.getConnectState() && ws_protocol_.getShakeState()){
            while (auto packet = audio_service_.PopPacketFromSendQueue()) {
                // Logger::info("MainLoop 001, packet->payload.size() = %d ", packet->payload.size());
                if (!ws_protocol_.SendAudio(std::move(packet))) {
                    break;
                }else{   // packet->payload.data(), packet->payload.size()
                    //Logger::info("MainLoop 003,  packet->payload.size() = %d ", packet->payload.size());//
                    // std::move(packet)：将packet转换为右值，传递给SendAudio函数，这意味着SendAudio函数接管了packet的所有权，可以移动其内容。在移动后，原来的packet对象变为有效但未定义的状态（通常为空或部分无效）。
                    //使用已被移动的对象：在else分支中，你尝试访问packet->payload.size()，但此时packet已经被移动，其内部的payload可能已经被转移，因此访问它会导致段错误。
                    // Logger::info("MainLoop 003,  send audio success.");
                }
            }            
        }
        // if(cnt%20 ==0){
        //     LcdDisplay::GetInstance().refresh_ui();
        // }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

#if 0
void Application::AddAudioData(AudioStreamPacket&& packet) {
    int bytes_left = 0;
    uint8_t* read_ptr = nullptr;
        // MP3解码器相关
    HMP3Decoder mp3_decoder_;
    MP3FrameInfo mp3_frame_info_;
    bool mp3_decoder_initialized_;

    // 解码MP3帧
    int16_t pcm_buffer[2304];
    int decode_result = MP3Decode(mp3_decoder_, &read_ptr, &bytes_left, pcm_buffer, 0);
}
#endif

// 新增：接收外部音频数据（如音乐播放） 2025.11.21 add
void Application::AddAudioData(PcmStreamPacket&& pcm_pkt) {

    auto& play = audio_service_.GetAudioPlayer();      // GetAudioPlayer
    // int mp3_SampleRate = music_.decoder.getMp3Samplerate();
    if(pcm_pkt.sample_rate != play.output_sample_rate()){
        play.SetOutputSampleRate(pcm_pkt.sample_rate);
    }

    // packet.payload包含的是原始PCM数据（int16_t）
    if (pcm_pkt.payload.size() >= 1) {
        // size_t num_samples = pcm_pkt.payload.size() / sizeof(int16_t);
        // std::vector<int16_t> pcm_data(num_samples);
        // memcpy(pcm_data.data(), pcm_pkt.payload.data(), pcm_pkt.payload.size());

        std::vector<int16_t> pcm_data = pcm_pkt.payload;
        std::cout << "[AddAudioData] pcm_data.size() = " << pcm_data.size() 
        << " pcm_pkt.sample_rate = " << pcm_pkt.sample_rate
        << std::endl;
        play.OutputData(pcm_data);
    }


}

#if 0
// 新增：接收外部音频数据（如音乐播放）
void Application::AddAudioData(AudioStreamPacket &&packet) {
    // auto codec = Board::GetInstance().GetAudioCodec();
    auto& play = audio_service_.GetAudioPlayer();      // GetAudioPlayer
    // if (device_state_ == kDeviceStateIdle && codec->output_enabled()) {
    if (true) {
        // packet.payload包含的是原始PCM数据（int16_t）
        if (packet.payload.size() >= 2) {
            size_t num_samples = packet.payload.size() / sizeof(int16_t);
            std::vector<int16_t> pcm_data(num_samples);
            memcpy(pcm_data.data(), packet.payload.data(), packet.payload.size());
            
            // // 检查采样率是否匹配，如果不匹配则进行简单重采样
            // if (packet.sample_rate != play.output_sample_rate()) {
            //     // ESP_LOGI(TAG, "Resampling music audio from %d to %d Hz", 
            //     //         packet.sample_rate, codec->output_sample_rate());        
            //     // 验证采样率参数
            //     if (packet.sample_rate <= 0 || play.output_sample_rate() <= 0) {
            //         Logger::info("Invalid sample rates: %d -> %d", 
            //                 packet.sample_rate, codec->output_sample_rate());
            //         return;
            //     }           
            //     std::vector<int16_t> resampled;    
            //     if (packet.sample_rate > play.output_sample_rate()) {
            //         Logger::info( "音乐播放：将采样率从 %d Hz 切换到 %d Hz", 
            //             play.output_sample_rate(), packet.sample_rate);

            //         // 尝试动态切换采样率
            //         if (play.SetOutputSampleRate(packet.sample_rate)) {
            //             Logger::info("成功切换到音乐播放采样率: %d Hz", packet.sample_rate);
            //         } else {
            //             Logger::info("无法切换采样率，继续使用当前采样率: %d Hz", codec->output_sample_rate());
            //         }
            //     } else {
            //         // 上采样：线性插值
            //         float upsample_ratio = codec->output_sample_rate() / static_cast<float>(packet.sample_rate);
            //         size_t expected_size = static_cast<size_t>(pcm_data.size() * upsample_ratio + 0.5f);
            //         resampled.reserve(expected_size);
                    
            //         for (size_t i = 0; i < pcm_data.size(); ++i) {
            //             // 添加原始样本
            //             resampled.push_back(pcm_data[i]);
                        
            //             // 计算需要插值的样本数
            //             int interpolation_count = static_cast<int>(upsample_ratio) - 1;
            //             if (interpolation_count > 0 && i + 1 < pcm_data.size()) {
            //                 int16_t current = pcm_data[i];
            //                 int16_t next = pcm_data[i + 1];
            //                 for (int j = 1; j <= interpolation_count; ++j) {
            //                     float t = static_cast<float>(j) / (interpolation_count + 1);
            //                     int16_t interpolated = static_cast<int16_t>(current + (next - current) * t);
            //                     resampled.push_back(interpolated);
            //                 }
            //             } else if (interpolation_count > 0) {
            //                 // 最后一个样本，直接重复
            //                 for (int j = 1; j <= interpolation_count; ++j) {
            //                     resampled.push_back(pcm_data[i]);
            //                 }
            //             }
            //         }
            //         Logger::info( "Upsampled %d -> %d samples (ratio: %.2f)", 
            //                 pcm_data.size(), resampled.size(), upsample_ratio);
            //     }
            //     pcm_data = std::move(resampled);
            // }
            
            // // 确保音频输出已启用
            // if (!codec->output_enabled()) {
            //     codec->EnableOutput(true);
            // }
            
            // 发送PCM数据到音频编解码器
            play.OutputData(pcm_data);
            
            audio_service_.UpdateOutputTimestamp();
        }
    }
}
#endif



void Application::SetDeviceState(DeviceState state) {
    if (device_state_ == state) {
        return;
    }
    
    // clock_ticks_ = 0;
    auto previous_state = device_state_;
    device_state_ = state;
}