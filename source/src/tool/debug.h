#ifndef DEBUG_H
#define DEBUG_H

#include <thread>
#include <iostream>
#include <stdio.h>


class Debug{
public:
    static Debug& GetInstance() {
        static Debug instance;
        return instance;
    }

    void start();
    void get_vol();
    void set_vol(int vol);
    void debug_thread();

private:
    Debug(){};
    ~Debug(){};

public:
    std::thread debug_td;
};

#endif