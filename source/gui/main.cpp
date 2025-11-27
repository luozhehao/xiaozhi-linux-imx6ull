#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "display/lv_100ask_xz_ai.h"
#include "display/src/ui_system.h"
#include "display/src/lcd_display.h"



static const wchar_t * title = L"百问网LVGL课程案例  © Copyright 2025, Shenzhen Baiwenwang Technology Co., Ltd.   https://www.100ask.net | https://lvgl.100ask.net";
// static pthread_mutex_t lvgl_mutex;
// static const char *getenv_default(const char *name, const char *dflt)
// {
//     return getenv(name) ? : dflt;
// }

// #if LV_USE_LINUX_FBDEV
// static void lv_linux_disp_init(void)
// {
//     const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
//     lv_display_t * disp = lv_linux_fbdev_create();

//     lv_linux_fbdev_set_file(disp, device);
// }
// #elif LV_USE_LINUX_DRM
// static void lv_linux_disp_init(void)
// {
//     const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
//     lv_display_t * disp = lv_linux_drm_create();

//     lv_linux_drm_set_file(disp, device, -1);
// }
// #elif LV_USE_SDL
// static void lv_linux_disp_init(void)
// {
//     const int width = atoi(getenv("LV_SDL_VIDEO_WIDTH") ? : "800");
//     const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ? : "480");

//     lv_sdl_window_create(width, height);
// }
// #else
// #error Unsupported configuration
// #endif

// void lvgl_lock(void)
// {
//     pthread_mutex_lock(&lvgl_mutex);
// }

// void lvgl_unlock(void)
// {
//     pthread_mutex_unlock(&lvgl_mutex);
// }

int gui_main(void)
{
    printf("main ... 001\n");
    // pthread_mutex_init(&lvgl_mutex, NULL);
    // lv_init();
    // /*Linux display device init*/
    // lv_linux_disp_init();
    /* 初始化UI交互系统 */
    // ui_system_init();
    /*Create a Demo*/
    //lv_demo_widgets();
   // lv_demo_widgets_start_slideshow();
    // lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event1");
    printf("main ... 002\n");

    // // sleep(10);
    // printf("main ... 001\n");
    // // LcdDisplay display;
    auto display = LcdDisplay::GetInstance();
    // display.SetupUI();
    // printf("main ... 002\n");
    // display.SetChatMessage("assistant", "你好");
    // display.SetChatMessage("user", "今天好累");
    // display.SetChatMessage("assistant", "在干嘛");
    // display.SetChatMessage("user", "哈哈，没干嘛");
    // display.SetChatMessage("assistant", "最近还好吗");
    // display.SetChatMessage("user", "就那样吧");
    // display.SetChatMessage("assistant", "好吧");
    // display.SetChatMessage("user", "谢谢关心");
    // // display.create_chat_interface();
    // // display.test_scroll();
    // // display.SetChatMessage("system", "系统测试");
    // printf("main ... 007\n");
    // sleep(3);

    printf("main ... 003\n");
    /* 初始化UI交互系统 */
    ui_system_init();


    // lv_100ask_xz_ai_main();

    printf("main ... 004\n");
    /*Handle LVGL tasks*/
    while(1) {
        // lvgl_lock();
        // lv_timer_handler();
        // lvgl_unlock();
        usleep(5000);
    }

    return 0;
}
