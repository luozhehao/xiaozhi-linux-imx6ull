

// simple_volume.h
#ifndef SIMPLE_VOLUME_H
#define SIMPLE_VOLUME_H

#include <string>
#include <vector>
#include <memory>
#include <cstdio>
#include <stdexcept>

class SimpleVolume {
public:
    // 获取音量（简单版本）
    static int get_speaker_volume() {
        std::string command = "amixer get Speaker | grep -oP 'Front Left: Playback \\K\\d+'";
        std::string result = exec_command(command);
        
        if (!result.empty()) {
            try {
                return std::stoi(result);
            } catch (const std::exception& e) {
                throw std::runtime_error("Failed to parse volume: " + std::string(e.what()));
            }
        }
        
        throw std::runtime_error("No volume data found");
    }
    
    // 获取左右声道音量
    static std::pair<int, int> get_stereo_volume() {
        std::string command = "amixer get Speaker";
        std::string result = exec_command(command);
        
        int left_vol = 0, right_vol = 0;
        size_t pos = 0;
        
        // 查找左声道
        size_t left_pos = result.find("Front Left: Playback");
        if (left_pos != std::string::npos) {
            left_pos = result.find_first_of("0123456789", left_pos);
            size_t end_pos = result.find_first_not_of("0123456789", left_pos);
            if (end_pos != std::string::npos) {
                left_vol = std::stoi(result.substr(left_pos, end_pos - left_pos));
            }
        }
        
        // 查找右声道
        size_t right_pos = result.find("Front Right: Playback");
        if (right_pos != std::string::npos) {
            right_pos = result.find_first_of("0123456789", right_pos);
            size_t end_pos = result.find_first_not_of("0123456789", right_pos);
            if (end_pos != std::string::npos) {
                right_vol = std::stoi(result.substr(right_pos, end_pos - right_pos));
            }
        }
        
        return std::make_pair(left_vol, right_vol);
    }
    
    // 设置音量
    static void set_speaker_volume(int volume) {
        std::string command = "amixer set Speaker " + std::to_string(volume);
        exec_command(command);
    }
    
    // 设置音量百分比
    static void set_speaker_volume_percent(int percent) {
        std::string command = "amixer set Speaker " + std::to_string(percent) + "%";
        exec_command(command);
    }
    
private:
    static std::string exec_command(const std::string& cmd) {
        std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        
        std::string result;
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            result += buffer;
        }
        
        return result;
    }
};

#endif // SIMPLE_VOLUME_H