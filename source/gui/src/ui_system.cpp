// SPDX-License-Identifier: GPL-3.0-only
/*

 *-----------------------------------------------------
 */
#include <string.h>
#include "cfg.h"
#include "ipc_udp.h"
#include "cJSON.h"
#include "lcd_display.h"
#include "lang_config.h"

// // 定义设备状态枚举类型
// typedef enum DeviceState {
//     kDeviceStateUnknown,
//     kDeviceStateStarting,
//     kDeviceStateWifiConfiguring,
//     kDeviceStateIdle,
//     kDeviceStateConnecting,
//     kDeviceStateListening,
//     kDeviceStateSpeaking,
//     kDeviceStateUpgrading,
//     kDeviceStateActivating,
//     kDeviceStateFatalError
// } DeviceState;

// 静态变量，用于存储IPC端点
static p_ipc_endpoint_t g_ipc_ep;

// 将设备状态转换为本地字符串
static const char* ConvertToLocalString(DeviceState state)
{
    switch (state) {
        case kDeviceStateUnknown:
            return UNKNOWN_STATUS;
        case kDeviceStateStarting:
            return INITIALIZING;
        case kDeviceStateWifiConfiguring:
            return NETWORK_CFG;
        case kDeviceStateIdle:
            return STANDBY;
        case kDeviceStateConnecting:
            return CONNECTING;
        case kDeviceStateListening:
            return LISTENING;
        case kDeviceStateSpeaking:
            return SPEAKING;
        case kDeviceStateUpgrading:
            return UPGRADING;
        case kDeviceStateActivating:
            return ACTIVATION;
        case kDeviceStateFatalError:
            return ERROR_STR;
    }

    return "未知状态";
}

/*
 * 处理从IPC接收到的UI数据。
 * 处理的数据格式:
 * 1. 状态: {"state": 0}等, 取值对应DeviceState的取值
 * 2. 要显示的文本: {"text": "你好"}
 * 3. 要显示的emotion: {"emotion": "happy"}, 有这些取值:
 *           "neutral","happy","laughing","funny","sad","angry","crying","loving",
 *           "embarrassed","surprised","shocked","thinking","winking","cool","relaxed",
 *           "delicious","kissy","confident","sleepy","silly","confused"
 * 4. WIFI强度: {"wifi": "100"}
 * 5. 电量: {"battery": "100"}
 *
 * @param buffer 包含JSON格式数据的字符串缓冲区
 * @param size 缓冲区的大小
 * @param user_data 用户数据指针（未使用）
 * @return 0 表示成功，-1 表示解析错误
 */
static int process_ui_data(char *buffer, size_t size, void *user_data)
{
    cJSON *root;
    auto& display = LcdDisplay::GetInstance();

    // LV_LOG_USER("cJSON_Parse [buffer]:\n %s ", buffer);
    // 解析JSON数据
    root = cJSON_Parse(buffer);
    if (!root) {
        LV_LOG_USER("cJSON_Parse err: %s ", buffer);
        return -1;
    }
    printf("process_ui_data 001,  buffer: %s \n", buffer);

    // 获取状态字段
    cJSON *state = cJSON_GetObjectItem(root, "state");
    if (state) {
        LV_LOG_USER("get state = %d, %s ", state->valueint, ConvertToLocalString((DeviceState)state->valueint));
        // SetStateString(ConvertToLocalString((DeviceState)state->valueint));
        // if (state->valueint == kDeviceStateSpeaking)
        //     SetEmotion(ASSETS_PATH "img_joke.png");
        // if (state->valueint == kDeviceStateListening)
        //     SetEmotion(ASSETS_PATH "img_naughty.png");      
    }

    printf("process_ui_data ... 004\n");
    // 获取文本字段
    cJSON *text = cJSON_GetObjectItem(root, "text");
    if (text) {
        LV_LOG_USER("get text = %s ", text->valuestring);
        // SetText(text->valuestring);
        display.SetChatMessage("user", text->valuestring);
    }

    printf("process_ui_data ... 005\n");
#if 0
    // Parse JSON data
    cJSON *type = cJSON_GetObjectItem(root, "type");
    printf("process_ui_data ... 006\n");
    if (type) {
        if (strcmp(type->valuestring, "tts") == 0) {
            printf("process_ui_data ... 007\n");
            cJSON *text = cJSON_GetObjectItem(root, "text");
            if (cJSON_IsString(text)) {
                // ESP_LOGI(TAG, ">> %s", text->valuestring);
                // Schedule([this, display, message = std::string(text->valuestring)]() {
                //     display->SetChatMessage("user", message.c_str());
                // });

                printf("tts, text->valuestring: \n", text->valuestring);
                SetText(text->valuestring);
            }
        }else if (strcmp(type->valuestring, "stt") == 0) {
            printf("process_ui_data ... 008\n");
            cJSON *text = cJSON_GetObjectItem(root, "text");
            if (cJSON_IsString(text)) {
                // ESP_LOGI(TAG, ">> %s", text->valuestring);
                // Schedule([this, display, message = std::string(text->valuestring)]() {
                //     display->SetChatMessage("user", message.c_str());
                // });

                printf("stt, text->valuestring: \n", text->valuestring);
                SetText(text->valuestring);
            }
        }else{
            printf("type->valuestring unkown\n");
        }
    }else{
        printf("type is null\n");
    }

#endif 
    printf("process_ui_data ... 009\n");

    // 获取WIFI强度字段（未处理）
    cJSON *wifi = cJSON_GetObjectItem(root, "wifi");
    if (wifi) {
        // 处理WIFI强度
    }

    // 获取电量字段（未处理）
    cJSON *battery = cJSON_GetObjectItem(root, "battery");
    if (battery) {
        // 处理电量
    }

    // 释放JSON对象
    cJSON_Delete(root);

    return 0;
}

/*
 * 初始化UI系统。
 * 创建IPC端点，用于接收和处理UI数据。
 *
 * @return 0 表示成功，-1 表示创建IPC端点失败
 */
int ui_system_init(void)
{
    // 创建UDP IPC端点
    g_ipc_ep = ipc_endpoint_create_udp(UI_PORT_DOWN, UI_PORT_UP, process_ui_data, NULL);
    if (!g_ipc_ep) {
        LV_LOG_ERROR("Failed to create IPC endpoint\n");
        return -1;
    }
    return 0;
}