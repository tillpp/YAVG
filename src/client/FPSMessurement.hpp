#pragma once

//FPS counter
#include <chrono>
#include <iostream>
struct FPSMessurement{
    
    std::chrono::steady_clock::time_point lastSecond = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastFrame = std::chrono::steady_clock::now();
public:
    size_t frames = 0;
    float delta = 0;

    FPSMessurement(){

    }
    void update(){
        auto now = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now-lastSecond).count()>=1000){
            float frameTime = std::chrono::duration<float, std::chrono::microseconds::period>(now - lastFrame).count();
            std::cout << "[FPS]" << frames << std::endl;
            frames = 0;
            lastSecond = now;
        }
        frames++;
        // {
        //     const int FPSLimit = 120;
        //     auto now = std::chrono::steady_clock::now();
        //     float delta = std::chrono::duration<float, std::chrono::seconds::period>(now - lastFrame).count();
        //     if(delta < 1.f/FPSLimit){
        //         float waittime = 1.f/FPSLimit-delta;
        //         //std::this_thread::sleep_for(std::chrono::milliseconds((int)(waittime*1000)));
        //     }
        // }

        auto currentTime = std::chrono::steady_clock::now();
        delta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrame).count();
        lastFrame = currentTime;
    }
};