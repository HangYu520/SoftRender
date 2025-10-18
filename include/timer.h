#pragma once
#include "preheader.h"

// * 计时器头文件

class Timer {
private:
    std::string name;
    std::chrono::steady_clock::time_point start;

public:
    Timer(const std::string& name = "") : name(name), start(std::chrono::steady_clock::now()) {}

    ~Timer() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (!name.empty()) 
            spdlog::info("Timer [{}] elapsed: {} ms", name, duration);
        else
            spdlog::info("Timer elapsed: {} ms", duration);
    }
};