#pragma once
#include "preheader.h"

// * 计时器头文件

/*
* ------------------------------
* 计时器类 (用于测量代码块执行时间)
* ------------------------------
*/
class Timer {
private:
    std::string name; // 计时器名称
    std::chrono::steady_clock::time_point start; // 开始时间点

public:
    /*
    * ------------------------------------------
    * 构造函数，启动计时器
    * ------------------------------------------
    * /inType: const std::string& name 计时器名称
    * ------------------------------------------
    */
    Timer(const std::string& name = "") : name(name), start(std::chrono::steady_clock::now()) {}

    /*
    * ------------------------------------------
    * 析构函数，停止计时器并输出耗时
    * ------------------------------------------
    */
    ~Timer() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (!name.empty()) 
            spdlog::info("Timer [{}] elapsed: {} ms", name, duration);
        else
            spdlog::info("Timer elapsed: {} ms", duration);
    }
};