/**
 * @file lv_modbus_tool.h
 * This file exists only to be compatible with Arduino's library structure
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H


// #include <thread>

// #ifdef __cplusplus
// extern "C" {
// #endif

/*********************
 *      INCLUDES
 *********************/
#include <pthread.h>

#include "../lv_100ask_xz_ai.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/*
 *  注意，这里要与 lv_conf.h 中 LV_FS_STDIO_LETTER 或 LV_FS_POSIX_LETTER 指定的盘符一致
 *  示例(Windows)： 假设 LETTER 设置为 'D'，之后路径可 "D:/100ask/"
 *  示例(Linux)：   直接设置 LETTER 设置为 'A'，之后路径可为 "A:/mnt/"
 */
#define ASSETS_PATH  "A:/usr/share/xiaozhi/"

#define PATH_PREFIX "/usr/share/xiaozhi/"

void lv_100ask_xz_ai_main(void);

void SetStateString(const char *str);

/* 每次只能显示给定的str */
void SetText(const char *str);

/*
 *  注意，这里要与 lv_conf.h 中 LV_FS_STDIO_LETTER 或 LV_FS_POSIX_LETTER 指定的盘符一致
 *  示例(Windows)： 假设 LETTER 设置为 'D'，之后路径可 "D:/100ask/img_naughty.png"
 *  示例(Linux)：   直接设置 LETTER 设置为 'A'，之后路径可为 "A:/mnt/img_naughty.png"
 */
void SetEmotion(const char *jpgFile);

void OnClicked(void);


// 定义设备状态枚举类型
typedef enum DeviceState{
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


#define  background_color       lv_color_hex(0xFFFFFF)      //rgb(255, 255, 255)
#define  border_color           lv_color_hex(0x000000)      //rgb(0, 0, 0)
#define user_bubble_color       lv_color_hex(0x00FF00)      //rgb(0, 255, 0)
#define assistant_bubble_color  lv_color_hex(0x00FF00)      //rgb(0, 255, 0)
#define system_bubble_color     lv_color_hex(0xFFFFFF)      //rgb(255, 255, 255)
#define text_color              lv_color_hex(0x000000)      //rgb(0, 0, 0)

#define  red_color          lv_color_hex(0xFF0000)         //rgb(255, 0, 0)
#define  green_color        lv_color_hex(0x00FF00)         //rgb(0, 255, 0)
#define  blue_color         lv_color_hex(0x0000FF)         //rgb(0, 0, 255)
#define  white_color        lv_color_hex(0xFFFFFF)         //rgb(255, 255, 255)


class LcdDisplay{
protected:
    lv_obj_t* container_ = nullptr;
    lv_obj_t* status_bar_= nullptr;
    lv_obj_t* content_= nullptr;
    lv_obj_t* chat_message_label_= nullptr;
    lv_obj_t* test_label_chat = nullptr;  //state_label_
    lv_obj_t* state_label_ = nullptr;

protected:
    // 添加protected构造函数
    // LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width, int height);

public:
    static LcdDisplay& GetInstance() {
        static LcdDisplay instance;
        return instance;
    }

    LcdDisplay();
    ~LcdDisplay(){}
    void SetupUI();
    void SetChatMessage(const char *role, const char *content);
    void SetDeviceState(DeviceState state);
    void Start();
    void Stop();
    void refresh_ui();

    void add_message(const char * text) ;
    void create_chat_interface(void);

    lv_obj_t *create_chat_container(void);
    void test_add_message(lv_obj_t *container, const char *sender, const char *message, bool is_self);
    void test_scroll();

private:
    static void *threadEntry(void *arg);
    void refresh_func(void* arg);
    
    // std::thread refresh_thread;
    bool running_;
    pthread_t refresh_thread;
};

// #ifdef __cplusplus
// } /* extern "C" */
// #endif

#endif /*LV_100ASK_XZ_AI_MAIN_H*/
