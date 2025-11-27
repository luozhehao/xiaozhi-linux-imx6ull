



#include "audio_process.h"

// int AudioProcessInit(int argc, char** argv)

int AudioProcess::process_thread(){
    try {
        AudioCapture capture;
        capture.start();
        std::cout << "Audio capture started. Press Enter to stop..." << std::endl;

        AudioPlayer player;
        player.start();
        std::cout << "Audio player started. Receiving on port " << std::endl;

        int a = 1;
        while(1){
        sleep(1);

        if(a==0){
            std::cout << "Press Enter to stop..." << std::endl;
            std::cin.get();
            player.stop();
            capture.stop();
            std::cout << "Stopped" << std::endl;
        }
    }
    // std::cout << "Press Enter to stop..." << std::endl;
    // std::cin.get();
    // player.stop();
    // capture.stop();
    // std::cout << "Stopped" << std::endl;
    } catch (const std::exception& e) {

        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


int AudioProcess::start(){
    // process_td = std::thread(&AudioProcess::process_thread, this);
    // process_td.detach();
    try {
        // AudioCapture capture;
        capture.start();
        std::cout << "Audio capture started. Press Enter to stop..." << std::endl;

        // AudioPlayer player;
        player.start();
        std::cout << "Audio player started. Receiving on port " << std::endl;
    } catch (const std::exception& e) {

        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}



int AudioProcessInit(){
    try {
        AudioCapture capture;
        capture.start();
        std::cout << "Audio capture started. Press Enter to stop..." << std::endl;

        AudioPlayer player;
        player.start();
        std::cout << "Audio player started. Receiving on port " << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}