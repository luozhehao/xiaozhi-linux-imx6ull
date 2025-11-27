
#ifndef AUDIO_PROCESS_H
#define AUDIO_PROCESS_H

#include <thread>
#include <iostream>
#include <stdio.h>

#include "capture_queue.h"
#include "play_queue.h"


class AudioProcess{
public:
    static AudioProcess& GetInstance() {
        static AudioProcess instance;
        return instance;
    }
    int AudioProcessInit();
    int process_thread();
    int start();

private:
    AudioProcess(){};
    ~AudioProcess(){};

public:
    std::thread process_td;
    AudioCapture capture;
    AudioPlayer player;
};


#endif