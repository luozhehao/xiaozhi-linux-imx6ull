/**
 ******************************************************************************
 * @file    lv_100ask_xz_ai_main.c
 * @author  百问科技
 * @version V1.0
 * @date    2025-3-17
 * @brief	100ask XiaoZhi AI base on LVGL
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2025-3-17     zhouyuebiao     First version
 ******************************************************************************
 * @attention
 *
 * Copyright (C) 2008-2025 深圳百问网科技有限公司<https://www.100ask.net/>
 * All rights reserved
 *
 ******************************************************************************
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// #include "lvgl/lvgl.h"
// #include "lvgl/demos/lv_demos.h"

#include "lcd_display.h"
#include "lang_config.h"
#include "../lvgl/src/font/lv_font.h"
#include "../lvgl/src/libs/freetype/lv_freetype.h"   // \gui\lvgl\src\libs\freetype\lv_freetype.h

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/
typedef struct _lv_100ask_xz_ai {
	lv_obj_t  * state_bar_img_wifi;
	lv_obj_t  * state_bar_label_state;
	lv_obj_t  * state_bar_img_battery;
	lv_obj_t  * img_emoji;
	lv_obj_t  * label_chat;
} T_lv_100ask_xz_ai, *PT_lv_100ask_xz_ai;

// extern lv_font_simsun_heiti_regular_16_cjk;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void init_freetype(void);
static void deinit_freetype(void);

static void init_style(void);

static void screen_onclicked_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
static PT_lv_100ask_xz_ai g_pt_lv_100ask_xz_ai;

lv_style_t g_style_chat_font;
lv_style_t g_style_state_font;

static lv_font_t * gp_chat_font_freetype;
static lv_font_t * gp_state_font_freetype;

// extern void lvgl_lock(void);
// extern void lvgl_unlock(void);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static pthread_mutex_t lvgl_mutex;

void lvgl_lock(void)
{
    pthread_mutex_lock(&lvgl_mutex);
}

void lvgl_unlock(void)
{
    pthread_mutex_unlock(&lvgl_mutex);
}


const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ? : dflt;
}


#if LV_USE_LINUX_FBDEV
void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t * disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t * disp = lv_linux_drm_create();

    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    const int width = atoi(getenv("LV_SDL_VIDEO_WIDTH") ? : "800");
    const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ? : "480");

    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif


static lv_obj_t* test02_label_chat = nullptr;

void lv_100ask_xz_ai_main(void)
{
    /* init */
    g_pt_lv_100ask_xz_ai = (T_lv_100ask_xz_ai *)lv_malloc(sizeof(T_lv_100ask_xz_ai));
    
    lv_fs_stdio_init();	
    init_freetype();
    init_style();

    auto screen = lv_screen_active();

/******************************/   
#if 0 
    printf("test ... 001\n");
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(screen, red_color, 0);  //blue
    // lv_obj_set_style_bg_opa(screen, LV_OPA_50, 0);
    lv_refr_now(NULL);
    sleep(2);

    lv_obj_set_style_bg_color(screen, green_color, 0); 
    lv_refr_now(NULL);    
    sleep(2);

    lv_obj_set_style_bg_color(screen, blue_color, 0); 
    // 强制刷新屏幕
    lv_refr_now(NULL);
    sleep(2);

    lv_obj_set_style_bg_color(screen, white_color, 0); 
    // 强制刷新屏幕
    lv_refr_now(NULL);
    sleep(2);

    /* chat message */
    printf("test ... 002\n");
    test02_label_chat = lv_label_create(screen);
    lv_obj_set_width(test02_label_chat, LV_PCT(90));
    lv_obj_add_style(test02_label_chat, &g_style_chat_font, 0);
    // lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->label_chat, &lv_font_simsun_16_cjk, 0); // 2025.10.12
    //lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->label_chat, &lv_font_simsun_heiti_normal_16_cjk, 0); // 2025.10.12
    lv_label_set_text(test02_label_chat, "It is 2025.11.25 today");
    // lv_obj_align_to(test_label_chat, g_pt_lv_100ask_xz_ai->img_emoji, 
    //         LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_align(test02_label_chat, LV_ALIGN_TOP_MID);  
    const char * test_txt = lv_label_get_text(test02_label_chat);
    LV_LOG_WARN("### test02_label_chat: %s", test_txt);
    printf("test ... 009\n");
    // sleep(10);
    lv_refr_now(NULL);
    sleep(2);
#endif
/******************************/

    /* state bar */
    lv_obj_t * cont_state_bar = lv_obj_create(screen);
    lv_obj_remove_style_all(cont_state_bar);
    lv_obj_set_size(cont_state_bar, LV_PCT(100), 40);
    lv_obj_set_align(cont_state_bar, LV_ALIGN_TOP_MID);
    lv_obj_set_style_radius(cont_state_bar, 0, 0);
    lv_obj_set_style_bg_opa(cont_state_bar, LV_OPA_60, 0);
    lv_obj_set_style_pad_hor(cont_state_bar, 10, 0);
    lv_obj_set_layout(cont_state_bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont_state_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont_state_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, 
                            LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // wifi
    g_pt_lv_100ask_xz_ai->state_bar_img_wifi = lv_image_create(cont_state_bar);
    lv_image_set_src(g_pt_lv_100ask_xz_ai->state_bar_img_wifi, LV_SYMBOL_WIFI);
    
    // state
    g_pt_lv_100ask_xz_ai->state_bar_label_state = lv_label_create(cont_state_bar);
    lv_obj_add_style(g_pt_lv_100ask_xz_ai->state_bar_label_state, &g_style_state_font, 0);
    lv_obj_set_width(g_pt_lv_100ask_xz_ai->state_bar_label_state, LV_PCT(70));
    //lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->state_bar_label_state, &lv_font_simsun_16_cjk, 0); // 2025.10.12 lv_font_simsun_heiti_regular_16_cjk
    //lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->state_bar_label_state, &lv_font_simsun_heiti_normal_16_cjk, 0);  // 2025.10.13 带标点
    lv_label_set_text(g_pt_lv_100ask_xz_ai->state_bar_label_state, "待命");
    //lv_label_set_text(g_pt_lv_100ask_xz_ai->state_bar_label_state, "wait...");
    const char * txt = lv_label_get_text(g_pt_lv_100ask_xz_ai->state_bar_label_state);
    LV_LOG_WARN("### Label text: %s", txt);
    

    // battery
    g_pt_lv_100ask_xz_ai->state_bar_img_battery = lv_image_create(cont_state_bar);
    lv_image_set_src(g_pt_lv_100ask_xz_ai->state_bar_img_battery, LV_SYMBOL_BATTERY_FULL);


    /* emoji */ 
    // https://www.iconfont.cn/search/index?searchType=icon&q=%E5%9C%86%E8%84%B8%E8%A1%A8%E6%83%85
    g_pt_lv_100ask_xz_ai->img_emoji = lv_image_create(lv_screen_active());
    lv_image_set_src(g_pt_lv_100ask_xz_ai->img_emoji, ASSETS_PATH"img_naughty.png");
    //lv_image_set_src(g_pt_lv_100ask_xz_ai->img_emoji, ASSETS_PATH"emoji_23.png");
    lv_obj_align(g_pt_lv_100ask_xz_ai->img_emoji, LV_ALIGN_CENTER, 0, 0);  // y = -40

    
    /* chat message */
    g_pt_lv_100ask_xz_ai->label_chat = lv_label_create(lv_screen_active());
    lv_obj_set_width(g_pt_lv_100ask_xz_ai->label_chat, LV_PCT(90));
    lv_obj_add_style(g_pt_lv_100ask_xz_ai->label_chat, &g_style_chat_font, 0);
    // lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->label_chat, &lv_font_simsun_16_cjk, 0); // 2025.10.12
    //lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->label_chat, &lv_font_simsun_heiti_normal_16_cjk, 0); // 2025.10.12
    lv_label_set_text(g_pt_lv_100ask_xz_ai->label_chat, "Hi！有什么可以帮到你呢？ 20251125");
    lv_obj_align_to(g_pt_lv_100ask_xz_ai->label_chat, g_pt_lv_100ask_xz_ai->img_emoji, 
            LV_ALIGN_OUT_BOTTOM_MID, 0, 10);


    // screen touch
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(lv_layer_top(), screen_onclicked_event_cb, LV_EVENT_CLICKED, NULL);

    lv_refr_now(NULL);
}


void SetStateString(const char *str)
{
    lvgl_lock();
    lv_label_set_text(g_pt_lv_100ask_xz_ai->state_bar_label_state, str);
    lvgl_unlock();
}

void SetText(const char *str)
{
    lvgl_lock();
    lv_label_set_text(g_pt_lv_100ask_xz_ai->label_chat, str);
    lvgl_unlock();
}

void SetEmotion(const char *jpgFile)
{
    lvgl_lock();
    lv_image_set_src(g_pt_lv_100ask_xz_ai->img_emoji, jpgFile);
    lvgl_unlock();
}


void OnClicked(void)
{
    static uint16_t index = 0;
    static char *str[][3] = {
        {"待命", "聆听", "回答"},
        {"现在是待命状态哦。", "现在是聆听状态哦。", "现在是回答状态哦。"},
        {ASSETS_PATH "img_joke.png", ASSETS_PATH "img_naughty.png", ASSETS_PATH "img_think.png"},
    };
#if 0
    lvgl_lock();
    lv_label_set_text(g_pt_lv_100ask_xz_ai->state_bar_label_state, str[0][index]);
    lv_label_set_text(g_pt_lv_100ask_xz_ai->label_chat, str[1][index]);

    lv_image_set_src(g_pt_lv_100ask_xz_ai->img_emoji, str[2][index]);
    lvgl_unlock();
#endif
    if(index >= 2) index = 0;
    else index++;

    LV_LOG_USER("Clicked, index: %d", index);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
static void lv_100ask_xz_ai_main_deinit(void)
{
    deinit_freetype();
    lv_free(g_pt_lv_100ask_xz_ai);
    lv_deinit();
}



static void init_freetype(void)
{
    lv_freetype_init(16);

    /*Create a font*/       
    LV_LOG_WARN("### init_freetype start ...");                                                  //   /usr/share/fonts/ttf/SourceHanSansCN-Regular.otf
    gp_chat_font_freetype = lv_freetype_font_create(PATH_PREFIX "HarmonyOS_Sans_SC_Regular.ttf", //   PATH_PREFIX "HarmonyOS_Sans_SC_Regular.ttf"    // 
                                                    LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                                    20,  //26
                                                    LV_FREETYPE_FONT_STYLE_NORMAL);
    LV_LOG_WARN("### init_freetype start ... 001");                                               
                                                    
    gp_state_font_freetype = lv_freetype_font_create(PATH_PREFIX "HarmonyOS_Sans_SC_Regular.ttf",
                                                    LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                                    20,
                                                    LV_FREETYPE_FONT_STYLE_NORMAL);
    LV_LOG_WARN("### init_freetype start ... 002");                                            

    if((!gp_chat_font_freetype) || (!gp_state_font_freetype)) {
        LV_LOG_ERROR("freetype font create failed.");
        exit(-1);
    }
    LV_LOG_WARN("### init_freetype finish.");
}

static void deinit_freetype(void)
{
    lv_freetype_font_delete(gp_chat_font_freetype);
    lv_freetype_font_delete(gp_state_font_freetype);
}


static void init_style(void)
{
    /*Create style with the new font*/;
    lv_style_init(&g_style_chat_font);
    lv_style_set_text_font(&g_style_chat_font, gp_chat_font_freetype);
    lv_style_set_text_align(&g_style_chat_font, LV_TEXT_ALIGN_CENTER);

    lv_style_init(&g_style_state_font);
    lv_style_set_text_font(&g_style_state_font, gp_state_font_freetype);
    lv_style_set_text_align(&g_style_state_font, LV_TEXT_ALIGN_CENTER);
}

static void screen_onclicked_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        OnClicked();
    }
}


// 将设备状态转换为本地字符串
const char* DeviceStateToLocalString(DeviceState state)
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



LcdDisplay::LcdDisplay(){
    pthread_mutex_init(&lvgl_mutex, NULL);
    lv_init();
    /*Linux display device init*/
    lv_linux_disp_init();
    
    /*Create a Demo*/
    //lv_demo_widgets();
   // lv_demo_widgets_start_slideshow();
    lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event1");

    SetupUI();
}

void LcdDisplay::SetupUI() {

    lv_fs_stdio_init();	
    init_freetype();
    init_style();

    auto screen = lv_screen_active();

/**************************************/    
    printf("SetupUI ... 001\n");
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(screen, white_color, 0); 
    // lv_obj_set_style_bg_opa(screen, LV_OPA_50, 0);
    // 强制刷新屏幕
    lv_refr_now(NULL);
    // sleep(2);

    /* chat message */
    printf("SetupUI ... 002\n");
    test02_label_chat = lv_label_create(screen);
    lv_obj_set_width(test02_label_chat, LV_PCT(90));
    lv_obj_add_style(test02_label_chat, &g_style_chat_font, 0);
    // lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->label_chat, &lv_font_simsun_16_cjk, 0); // 2025.10.12
    //lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->label_chat, &lv_font_simsun_heiti_normal_16_cjk, 0); // 2025.10.12
    lv_label_set_text(test02_label_chat, "It is 2025.11.25 today");
    // lv_obj_align_to(test_label_chat, g_pt_lv_100ask_xz_ai->img_emoji, 
    //         LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_align(test02_label_chat, LV_ALIGN_TOP_MID);  
    const char * test_txt = lv_label_get_text(test02_label_chat);
    LV_LOG_WARN("### test02_label_chat: %s", test_txt);
    printf("SetupUI ... 003\n");
    lv_refr_now(NULL);
    // sleep(2);
/**************************************/

    // auto screen = lv_screen_active();
    /* Container */
    container_ = lv_obj_create(screen);
    printf("SetupUI ... 004, LV_HOR_RES = %d, LV_VER_RES = %d, LV_SIZE_CONTENT = %d\n", LV_HOR_RES, LV_VER_RES, LV_SIZE_CONTENT);
    lv_obj_set_size(container_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_radius(container_, 0, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    // lv_obj_set_style_bg_color(container_, lvgl_theme->background_color(), 0);  // background_color
    lv_obj_set_style_bg_color(container_, background_color, 0);
    // lv_obj_set_style_border_color(container_, lvgl_theme->border_color(), 0);
    lv_obj_set_style_border_color(container_, border_color, 0);

    /* Status bar */
    status_bar_ = lv_obj_create(container_);
    lv_obj_set_size(status_bar_, LV_HOR_RES, 40);  // LV_SIZE_CONTENT
    lv_obj_set_style_radius(status_bar_, 0, 0);
    // lv_obj_set_style_bg_color(status_bar_, lvgl_theme->background_color(), 0);
    lv_obj_set_style_bg_color(status_bar_, background_color, 0);
    // lv_obj_set_style_text_color(status_bar_, lvgl_theme->text_color(), 0);
    lv_obj_set_style_text_color(status_bar_,  text_color, 0);   

    /* state bar */
    // lv_obj_t * 
    // status_bar_ = lv_obj_create(container_);
    // lv_obj_remove_style_all(status_bar_);
    // lv_obj_set_size(status_bar_, LV_PCT(100), 40);
    // lv_obj_set_align(status_bar_, LV_ALIGN_TOP_MID);
    // lv_obj_set_style_radius(status_bar_, 0, 0);
    // lv_obj_set_style_bg_color(status_bar_, background_color, 0);
    // // lv_obj_set_style_bg_opa(status_bar_, LV_OPA_60, 0);
    // lv_obj_set_style_pad_hor(status_bar_, 10, 0);
    // lv_obj_set_layout(status_bar_, LV_LAYOUT_FLEX);
    // lv_obj_set_flex_flow(status_bar_, LV_FLEX_FLOW_ROW);
    // lv_obj_set_flex_align(status_bar_, LV_FLEX_ALIGN_SPACE_BETWEEN, 
    //                         LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);    

    // state
    state_label_ = lv_label_create(status_bar_);
    lv_obj_add_style(state_label_, &g_style_state_font, 0);
    lv_obj_set_width(state_label_, LV_PCT(70));
    lv_obj_set_align(state_label_, LV_ALIGN_CENTER);     // LV_ALIGN_CENTER
    //lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->state_bar_label_state, &lv_font_simsun_16_cjk, 0); // 2025.10.12 lv_font_simsun_heiti_regular_16_cjk
    //lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->state_bar_label_state, &lv_font_simsun_heiti_normal_16_cjk, 0);  // 2025.10.13 带标点
    lv_label_set_text(state_label_, "待命");
    //lv_label_set_text(g_pt_lv_100ask_xz_ai->state_bar_label_state, "wait...");
    const char * txt = lv_label_get_text(state_label_);
    LV_LOG_WARN("### Label text: %s", txt);

    /* Content - Chat area */
    content_ = lv_obj_create(container_);
    lv_obj_set_style_radius(content_, 0, 0);
    lv_obj_set_width(content_, LV_HOR_RES);
    lv_obj_set_flex_grow(content_, 1);
    // lv_obj_set_style_pad_all(content_, lvgl_theme->spacing(4), 0);
    lv_obj_set_style_pad_all(content_, 4*2, 0);
    lv_obj_set_style_border_width(content_, 0, 0);
    // lv_obj_set_style_bg_color(content_, lvgl_theme->chat_background_color(), 0); // Background for chat area
    lv_obj_set_style_bg_color(content_, background_color, 0);

    // Enable scrolling for chat content
    lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_AUTO);  // LV_SCROLLBAR_MODE_OFF
    lv_obj_set_scroll_dir(content_, LV_DIR_VER);
    
    // Create a flex container for chat messages
    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    // lv_obj_set_style_pad_row(content_, lvgl_theme->spacing(4), 0); // Space between messages
    lv_obj_set_style_pad_row(content_, 4*2, 0);

    // We'll create chat messages dynamically in SetChatMessage
    chat_message_label_ = nullptr;

    printf("SetupUI ... 007\n");
    /* chat message */
    // test_label_chat = lv_label_create(screen);
    // lv_obj_set_width(test_label_chat, LV_PCT(90));
    // lv_obj_add_style(test_label_chat, &g_style_chat_font, 0);
    // // lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->label_chat, &lv_font_simsun_16_cjk, 0); // 2025.10.12
    // //lv_obj_set_style_text_font(g_pt_lv_100ask_xz_ai->label_chat, &lv_font_simsun_heiti_normal_16_cjk, 0); // 2025.10.12
    // lv_label_set_text(test_label_chat, "It is Nov.25 2025 today.");
    // // lv_obj_align_to(test_label_chat, g_pt_lv_100ask_xz_ai->img_emoji, 
    // //         LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    // lv_obj_set_align(test_label_chat, LV_ALIGN_TOP_MID);  // y = -40
    
    lv_refr_now(NULL);
    // sleep(2);
    if(content_){
        printf("009 content_ is not null\n");
    }else{
        printf("009 content_ is  null\n");
    }
    printf("SetupUI ... 009\n");
}


void LcdDisplay::refresh_ui(){
    lv_refr_now(NULL);
}

void LcdDisplay::refresh_func(void* arg){
    while(1){
        lvgl_lock();
        lv_timer_handler();
        lvgl_unlock();
        usleep(5000);
    }
}

// 静态线程入口函数
void* LcdDisplay::threadEntry(void* arg) {
    LcdDisplay* self = static_cast<LcdDisplay*>(arg);
    self->refresh_func(arg);
    return NULL;
}

void LcdDisplay::Start(){

    running_ = true;
    int id1 = 1;
    // 创建线程
    pthread_create(&refresh_thread, NULL, &LcdDisplay::threadEntry, &id1);
}

// 停止线程
void LcdDisplay::Stop() {
    if (running_) {
        running_ = false;
        pthread_join(refresh_thread, NULL);
    }
}


#define  MAX_MESSAGES 20

void LcdDisplay::SetChatMessage(const char* role, const char* content) {
    printf("SetChatMessage ... 001\n");
    // DisplayLockGuard lock(this);
    // lv_obj_t* container_ = nullptr;
    // lv_obj_t* status_bar_= nullptr;
    // lv_obj_t* content_= nullptr;
    // lv_obj_t* chat_message_label_= nullptr;
    // lv_obj_t* test_label_chat = nullptr;

    if (container_ == nullptr) printf("container_ == nullptr\n");
    if (status_bar_ == nullptr) printf("status_bar_ == nullptr\n");
    if (content_ == nullptr) printf("content_ == nullptr\n");
    if (chat_message_label_ == nullptr) printf("chat_message_label_ == nullptr\n");
    if (test_label_chat == nullptr) printf("test_label_chat == nullptr\n");

    if (content_ == nullptr) {
        printf("SetChatMessage ... content_ == nullptr\n");
        return;
    }
    printf("SetChatMessage ... 001.002\n");
    
    // 检查消息数量是否超过限制
    uint32_t child_count = lv_obj_get_child_cnt(content_);
    printf("SetChatMessage ... 002, child_count = %d\n", child_count);
    if (child_count >= MAX_MESSAGES) {
        // 删除最早的消息（第一个子对象）
        lv_obj_t* first_child = lv_obj_get_child(content_, 0);
        lv_obj_t* last_child = lv_obj_get_child(content_, child_count - 1);
        if (first_child != nullptr) {
            lv_obj_del(first_child);
        }
        // Scroll to the last message immediately
        if (last_child != nullptr) {
            lv_obj_scroll_to_view_recursive(last_child, LV_ANIM_OFF);
        }
    }
    printf("SetChatMessage ... 003\n");
    // 折叠系统消息（如果是系统消息，检查最后一个消息是否也是系统消息）
    if (strcmp(role, "system") == 0) {
        if (child_count > 0) {
            // 获取最后一个消息容器
            lv_obj_t* last_container = lv_obj_get_child(content_, child_count - 1);
            if (last_container != nullptr && lv_obj_get_child_cnt(last_container) > 0) {
                // 获取容器内的气泡
                lv_obj_t* last_bubble = lv_obj_get_child(last_container, 0);
                if (last_bubble != nullptr) {
                    // 检查气泡类型是否为系统消息
                    void* bubble_type_ptr = lv_obj_get_user_data(last_bubble);
                    if (bubble_type_ptr != nullptr && strcmp((const char*)bubble_type_ptr, "system") == 0) {
                        // 如果最后一个消息也是系统消息，则删除它
                        lv_obj_del(last_container);
                    }
                }
            }
        }
    } else {
        // 隐藏居中显示的 AI logo
        // lv_obj_add_flag(emoji_label_, LV_OBJ_FLAG_HIDDEN);
    }

    printf("SetChatMessage ... 004\n");
    //避免出现空的消息框
    if(strlen(content) == 0) {
        return;
    }

    // auto lvgl_theme = static_cast<LvglTheme*>(current_theme_);
    // auto text_font = lvgl_theme->text_font()->font();
    auto text_font  = gp_chat_font_freetype;    // add 2025.11.25

    printf("SetChatMessage ... 005\n");
    // Create a message bubble
    lv_obj_t* msg_bubble = lv_obj_create(content_);
    lv_obj_set_style_radius(msg_bubble, 8, 0);
    lv_obj_set_scrollbar_mode(msg_bubble, LV_SCROLLBAR_MODE_AUTO); // LV_SCROLLBAR_MODE_OFF
    lv_obj_set_scroll_dir(msg_bubble, LV_DIR_VER);  // add 2025.11.25
    lv_obj_set_style_border_width(msg_bubble, 0, 0);
    // lv_obj_set_style_pad_all(msg_bubble, lvgl_theme->spacing(4), 0);
    lv_obj_set_style_pad_all(msg_bubble, 4*2, 0);

    // Create the message text
    lv_obj_t* msg_text = lv_label_create(msg_bubble);
    lv_obj_add_style(msg_text, &g_style_chat_font, 0);
    lv_label_set_text(msg_text, content);

    printf("SetChatMessage ... 006\n");
    
    // 计算文本实际宽度
    lv_coord_t text_width = lv_txt_get_width(content, strlen(content), text_font, 0);

    // 计算气泡宽度
    lv_coord_t max_width = LV_HOR_RES * 85 / 100 - 16;  // 屏幕宽度的85%
    lv_coord_t min_width = 20;  
    lv_coord_t bubble_width;
    
    // 确保文本宽度不小于最小宽度
    if (text_width < min_width) {
        text_width = min_width;
    }

    // 如果文本宽度小于最大宽度，使用文本宽度
    if (text_width < max_width) {
        bubble_width = text_width; 
    } else {
        bubble_width = max_width;
    }
    printf("SetChatMessage ... 007\n");

    // 设置消息文本的宽度
    lv_obj_set_width(msg_text, bubble_width);  // 减去padding
    lv_label_set_long_mode(msg_text, LV_LABEL_LONG_WRAP);

    // 设置气泡宽度
    lv_obj_set_width(msg_bubble, bubble_width);
    lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);

    printf("SetChatMessage ... 008\n");
    // Set alignment and style based on message role
    if (strcmp(role, "user") == 0) {
        printf("SetChatMessage ... 009\n");
        // User messages are right-aligned with green background
        // lv_obj_set_style_bg_color(msg_bubble, lvgl_theme->user_bubble_color(), 0); 
        lv_obj_set_style_bg_color(msg_bubble, user_bubble_color, 0);   // rgb(0, 128, 0)
        printf("SetChatMessage ... 009.001\n");
        lv_obj_set_style_bg_opa(msg_bubble, LV_OPA_70, 0);
        printf("SetChatMessage ... 009.002\n");
        // Set text color for contrast
        // lv_obj_set_style_text_color(msg_text, lvgl_theme->text_color(), 0);  // lv_color_hex(0x000000)
        lv_obj_set_style_text_color(msg_text, text_color, 0);   //  rgb(0, 0, 0)
        printf("SetChatMessage ... 009.003\n");
        // 设置自定义属性标记气泡类型
        lv_obj_set_user_data(msg_bubble, (void*)"user");
        printf("SetChatMessage ... 009.004\n");
        // Set appropriate width for content
        lv_obj_set_width(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
        printf("SetChatMessage ... 009.005\n");
        // Don't grow
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
        printf("SetChatMessage ... 009.009\n");
    } else if (strcmp(role, "assistant") == 0) {
        // Assistant messages are left-aligned with white background
        // lv_obj_set_style_bg_color(msg_bubble, lvgl_theme->assistant_bubble_color(), 0);  // lv_color_hex(0x00FF00)
        lv_obj_set_style_bg_color(msg_bubble, assistant_bubble_color, 0);
        lv_obj_set_style_bg_opa(msg_bubble, LV_OPA_70, 0);
        // Set text color for contrast
        // lv_obj_set_style_text_color(msg_text, lvgl_theme->text_color(), 0);
        lv_obj_set_style_text_color(msg_text, text_color, 0);
        

        // 设置自定义属性标记气泡类型
        lv_obj_set_user_data(msg_bubble, (void*)"assistant");
        
        // Set appropriate width for content
        lv_obj_set_width(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
        
        // Don't grow
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
    } else if (strcmp(role, "system") == 0) {
        // System messages are center-aligned with light gray background
        // lv_obj_set_style_bg_color(msg_bubble, lvgl_theme->system_bubble_color(), 0);
        lv_obj_set_style_bg_color(msg_bubble, system_bubble_color, 0);
        lv_obj_set_style_bg_opa(msg_bubble, LV_OPA_70, 0);
        // Set text color for contrast
        // lv_obj_set_style_text_color(msg_text, lvgl_theme->system_text_color(), 0);
        lv_obj_set_style_text_color(msg_text, text_color, 0);
        
        // 设置自定义属性标记气泡类型
        lv_obj_set_user_data(msg_bubble, (void*)"system");
        
        // Set appropriate width for content
        lv_obj_set_width(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
        
        // Don't grow
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
    }
    
    printf("SetChatMessage ... 010\n");
    // Create a full-width container for user messages to ensure right alignment
    if (strcmp(role, "user") == 0) {
        // Create a full-width container
        lv_obj_t* container = lv_obj_create(content_);
        lv_obj_set_width(container, LV_HOR_RES);
        lv_obj_set_height(container, LV_SIZE_CONTENT);
        
        // Make container transparent and borderless
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        
        // Move the message bubble into this container
        lv_obj_set_parent(msg_bubble, container);
        
        // Right align the bubble in the container
        lv_obj_align(msg_bubble, LV_ALIGN_RIGHT_MID, -25, 0);
        
        // Auto-scroll to this container
        // lv_obj_scroll_to_view_recursive(msg_bubble, LV_ANIM_OFF);
        lv_obj_scroll_to_view_recursive(container, LV_ANIM_OFF);
    
        printf("SetChatMessage ... 011\n");
        // 检查容器属性
        printf("容器高度: %d\n", lv_obj_get_height(container));
        // printf("容器内容高度: %d\n", lv_obj_get_scroll_height(container));
        printf("滚动位置Y: %d\n", lv_obj_get_scroll_y(container));

    } else if (strcmp(role, "system") == 0) {
        // 为系统消息创建全宽容器以确保居中对齐
        lv_obj_t* container = lv_obj_create(content_);
        lv_obj_set_width(container, LV_HOR_RES);
        lv_obj_set_height(container, LV_SIZE_CONTENT);
        
        // 使容器透明且无边框
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        
        // 将消息气泡移入此容器
        lv_obj_set_parent(msg_bubble, container);
        
        // 将气泡居中对齐在容器中
        lv_obj_align(msg_bubble, LV_ALIGN_CENTER, 0, 0);
        
        // 自动滚动底部
        // lv_obj_scroll_to_view_recursive(msg_bubble, LV_ANIM_OFF);
        lv_obj_scroll_to_view_recursive(container, LV_ANIM_OFF);
    } else {    // "assistant"
        // For assistant messages
        // Left align assistant messages
        lv_obj_align(msg_bubble, LV_ALIGN_LEFT_MID, 0, 0);

        // Auto-scroll to the message bubble
        lv_obj_scroll_to_view_recursive(msg_bubble, LV_ANIM_OFF);

        // 检查容器属性
        printf("容器高度: %d\n", lv_obj_get_height(msg_bubble));
        // printf("容器内容高度: %d\n", lv_obj_get_scroll_height(msg_bubble));
        printf("滚动位置Y: %d\n", lv_obj_get_scroll_y(msg_bubble));        
    }
    
    // Store reference to the latest message label
    chat_message_label_ = msg_text;

    lv_refr_now(NULL);
    printf("SetChatMessage ... 019\n");

    // // 检查容器属性
    // printf("容器高度: %d\n", lv_obj_get_height(container));
    // printf("容器内容高度: %d\n", lv_obj_get_scroll_height(container));
    // printf("滚动位置Y: %d\n", lv_obj_get_scroll_y(container));

    // // 检查滚动功能
    // if(lv_obj_get_scroll_height(container) > lv_obj_get_height(container)) {
    //     printf("内容超出容器高度，应该可以滚动\n");
    // } else {
    //     printf("内容未超出容器高度，不需要滚动\n");
    // }
}


void LcdDisplay::SetDeviceState(DeviceState state){  
    const char* state_str = DeviceStateToLocalString(state);
    lv_label_set_text(state_label_, state_str);
    lv_refr_now(NULL);
}


/***************************/
static lv_obj_t * chat_container;

void LcdDisplay::add_message(const char * text) {
    lv_obj_t * label = lv_label_create(chat_container);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, LV_PCT(100));

    // 滚动到新消息
    lv_obj_scroll_to_view_recursive(label, LV_ANIM_OFF);
    lv_refr_now(NULL);
}

void LcdDisplay::create_chat_interface(void) {
    // 创建聊天容器
    chat_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(chat_container, 480, 272);
    lv_obj_align(chat_container, LV_ALIGN_TOP_MID, 0, 10);

    // 设置滚动
    lv_obj_set_scrollbar_mode(chat_container, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(chat_container, LV_DIR_VER);

    // 设置布局
    lv_obj_set_flex_flow(chat_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(chat_container, 10, 0);

    // 添加测试消息
    for (int i = 0; i < 20; i++) {

        add_message("this is a test text");
    }

    printf("create_chat_interface ... 009\n");
}

/****************************/

// 创建聊天容器
lv_obj_t* LcdDisplay::create_chat_container(void) {
    lv_obj_t* container = lv_obj_create(lv_scr_act());
    
    // 设置容器大小和位置
    lv_obj_set_size(container, 300, 200);
    lv_obj_align(container, LV_ALIGN_TOP_MID, 0, 20);
    
    // 启用滚动
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(container, LV_DIR_VER);
    
    // 使用Flex布局管理消息顺序
    // lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // 设置布局
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_set_style_pad_all(container, 10, 0);
    lv_obj_set_style_pad_gap(container, 5, 0);  // 消息间距
    
    return container;
}

// 添加消息函数
void LcdDisplay::test_add_message(lv_obj_t* container, const char* sender, const char* message, bool is_self) {
    // 创建消息容器
    lv_obj_t* msg_container = lv_obj_create(container);
    lv_obj_set_size(msg_container, LV_PCT(90), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(msg_container, 8, 0);
    lv_obj_set_style_radius(msg_container, 10, 0);
    
    // 根据发送者设置对齐和颜色
    if(is_self) {
        lv_obj_set_style_bg_color(msg_container, lv_palette_main(LV_PALETTE_BLUE), 0);
        lv_obj_align(msg_container, LV_ALIGN_TOP_RIGHT, 0, 0);
    } else {
        lv_obj_set_style_bg_color(msg_container, lv_palette_main(LV_PALETTE_GREY), 0);
        lv_obj_align(msg_container, LV_ALIGN_TOP_LEFT, 0, 0);
    }
        // 启用滚动
    lv_obj_set_scrollbar_mode(msg_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(msg_container, LV_DIR_VER);
    
    // 添加消息内容
    lv_obj_t* label = lv_label_create(msg_container);
    lv_label_set_text_fmt(label, "%s: %s", sender, message);
    lv_obj_set_width(label, LV_PCT(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    //     // 对齐到容器底部（新消息在底部）
    // lv_obj_align(msg_container, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_update_layout(msg_container);
    lv_obj_scroll_to_y(msg_container, LV_COORD_MAX, LV_ANIM_OFF);

    // 滚动到新消息
    lv_obj_scroll_to_view_recursive(msg_container, LV_ANIM_OFF);
    
    // 强制刷新确保立即显示
    lv_refr_now(NULL);
}

void LcdDisplay::test_scroll(){
    // 使用示例
    lv_obj_t* chat_container = create_chat_container();

    // 添加测试消息
    test_add_message(chat_container, "Alice", "1.Hello!", false);
    sleep(1);
    test_add_message(chat_container, "You", "Hi there!", true);
    test_add_message(chat_container, "Alice", "How are you today?", false);
    sleep(1);
    test_add_message(chat_container, "Alice", "2.Hello!", false);
    test_add_message(chat_container, "You", "Hi there!", true);
    sleep(1);
    test_add_message(chat_container, "Alice", "How are you today?", false);
    test_add_message(chat_container, "Alice", "3.Hello!", false);
    sleep(1);
    test_add_message(chat_container, "You", "Hi there!", true);
    test_add_message(chat_container, "Alice", "How are you today?", false);
    sleep(1);
    test_add_message(chat_container, "Alice", "4.Hello!", false);
    test_add_message(chat_container, "You", "Hi there!", true);
    sleep(1);
    test_add_message(chat_container, "Alice", "How are you today?", false);
}



