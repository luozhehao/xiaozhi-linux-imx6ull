

#include "debug.h"
#include "volume_alsa.h"


void Debug::get_vol(){
    try {
        VolumeController volume;
        int vol = volume.get_volume();
        int percent = volume.get_volume_percent();
        int min, max;
        volume.get_volume_range(min, max);
        
        std::cout << "当前音量: " << vol << " (" << percent << "%)" << std::endl;
        std::cout << "音量范围: " << min << " - " << max << std::endl;
        volume.set_volume(75);
    } catch (const std::exception& e) {
        std::cerr << "ALSA测试失败: " << e.what() << std::endl;
    }    
}

void Debug::set_vol(int vol){
    try {
        VolumeController volume;
        int vol = volume.get_volume();
        int percent = volume.get_volume_percent();
        int min, max;
        volume.get_volume_range(min, max);
        
        std::cout << "当前音量: " << vol << " (" << percent << "%)" << std::endl;
        std::cout << "音量范围: " << min << " - " << max << std::endl;
        volume.set_volume(75);
    } catch (const std::exception& e) {
        std::cerr << "ALSA测试失败: " << e.what() << std::endl;
    }    
}



void Debug::debug_thread(){
    std::string input;
    while(1){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::getline(std::cin, input);
        if (input == "get_vol") {
            get_vol();
        }
        if (input == "set_vol") {
            set_vol(75);
        }
        if (input == "_vol") {
            get_vol();
        }        
    }
}

void Debug::start(){
    std::cout << "Debug ... 001" << std::endl;
    // std::thread td(&Debug::debug_thread, this, 100);    // pass by value
    int a = 100;
    debug_td = std::thread(&Debug::debug_thread, this);
    debug_td.detach();
    std::cout << "Debug ... 002" << std::endl;    
}