
// volume_alsa.h
#ifndef VOLUME_ALSA_H
#define VOLUME_ALSA_H

#include <alsa/asoundlib.h>
#include <string>
#include <iostream>
#include <stdexcept>

class VolumeController {
private:
    snd_mixer_t* mixer_;
    snd_mixer_elem_t* element_;
    snd_mixer_elem_t* element_headphone;
    std::string device_;
    std::string control_;
    std::string control_headphone;
    
public:
    VolumeController(const std::string& device = "default", 
                    const std::string& control = "Speaker")
        : mixer_(nullptr), element_(nullptr), device_(device), control_(control) {
        open_mixer();
    }
    
    ~VolumeController() {
        close_mixer();
    }
    
    // 获取音量（返回平均值或指定声道）
    int get_volume(bool average = true) {
        if (!element_) {
            throw std::runtime_error("Mixer element not opened");
        }
        
        long min, max;
        snd_mixer_selem_get_playback_volume_range(element_, &min, &max);
        
        if (average) {
            // 获取左右声道平均值
            long left_vol, right_vol;
            if (snd_mixer_selem_get_playback_volume(element_, SND_MIXER_SCHN_FRONT_LEFT, &left_vol) == 0 &&
                snd_mixer_selem_get_playback_volume(element_, SND_MIXER_SCHN_FRONT_RIGHT, &right_vol) == 0) {
                return static_cast<int>((left_vol + right_vol) / 2);
            }
        } else {
            // 只获取左声道
            long volume;
            if (snd_mixer_selem_get_playback_volume(element_, SND_MIXER_SCHN_FRONT_LEFT, &volume) == 0) {
                return static_cast<int>(volume);
            }
        }
        
        throw std::runtime_error("Failed to get volume");
    }
    
    // 获取音量范围
    void get_volume_range(int& min, int& max) {
        if (!element_) {
            throw std::runtime_error("Mixer element not opened");
        }
        
        long min_val, max_val;
        snd_mixer_selem_get_playback_volume_range(element_, &min_val, &max_val);
        min = static_cast<int>(min_val);
        max = static_cast<int>(max_val);
    }
    
    // 获取音量百分比
    int get_volume_percent() {
        int volume = get_volume();
        int min, max;
        get_volume_range(min, max);
        
        return static_cast<int>((volume - min) * 100 / (max - min));
    }
    
    // 设置音量
    void set_volume(int volume) {
        if (!element_) {
            throw std::runtime_error("Mixer element not opened");
        }
        
        long min, max;
        snd_mixer_selem_get_playback_volume_range(element_, &min, &max);
        
        if (volume < min || volume > max) {
            throw std::out_of_range("Volume out of range");
        }
        
        snd_mixer_selem_set_playback_volume_all(element_, volume);
        snd_mixer_selem_set_playback_switch_all(element_, 1); // 确保开启
    }
    
private:
    void open_mixer() {
        int err;
        
        // 打开混音器
        if ((err = snd_mixer_open(&mixer_, 0)) < 0) {
            throw std::runtime_error("Cannot open mixer: " + std::string(snd_strerror(err)));
        }
        
        // 附加到默认设备
        if ((err = snd_mixer_attach(mixer_, device_.c_str())) < 0) {
            snd_mixer_close(mixer_);
            throw std::runtime_error("Cannot attach mixer: " + std::string(snd_strerror(err)));
        }
        
        // 注册简单元素类
        if ((err = snd_mixer_selem_register(mixer_, NULL, NULL)) < 0) {
            snd_mixer_close(mixer_);
            throw std::runtime_error("Cannot register mixer: " + std::string(snd_strerror(err)));
        }
        
        // 加载混音器元素
        if ((err = snd_mixer_load(mixer_)) < 0) {
            snd_mixer_close(mixer_);
            throw std::runtime_error("Cannot load mixer: " + std::string(snd_strerror(err)));
        }
        
        // 查找控制元素
        element_ = find_control_element(control_);
        if (!element_) {
            snd_mixer_close(mixer_);
            throw std::runtime_error("Cannot find control: " + control_);
        }
    }
    
    void close_mixer() {
        if (mixer_) {
            snd_mixer_close(mixer_);
            mixer_ = nullptr;
            element_ = nullptr;
        }
    }
    
    snd_mixer_elem_t* find_control_element(const std::string& control_name) {
        snd_mixer_elem_t* elem;
        snd_mixer_selem_id_t* sid;
        
        snd_mixer_selem_id_alloca(&sid);
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, control_name.c_str());
        
        elem = snd_mixer_find_selem(mixer_, sid);
        return elem;
    }
};

#endif // VOLUME_ALSA_H