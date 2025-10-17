#pragma once
#include "typedef.h"

class Engine // * 引擎单例类
{
public:
    Engine(const Engine&) = delete; // 禁止拷贝构造
    Engine& operator=(const Engine&) = delete; // 禁止赋值操作

    static std::unique_ptr<Engine>& getInstance() // 获取引擎实例
    {
        static auto instance = createInstance();
        return instance;
    }

    void line(
    Image& image, // 图像对象
    const Image::Pixel& start, // 起点坐标 
    const Image::Pixel& end, // 终点坐标
    const Image::Color& color // 颜色
    ); // 画线函数

private:
    Engine() {}; // 私有构造函数，防止实例化
    static std::unique_ptr<Engine> createInstance() // 创建引擎实例
    {
        return std::unique_ptr<Engine>(new Engine());
    }
};