#pragma once
#include "typedef.h"

/*
* ------------------------------------------
* 软渲染绘制算法引擎(类GPU)，单例类(禁止复制，构造)
* ------------------------------------------
*/
class Engine
{
public:
    Engine(const Engine&) = delete; // 禁止拷贝构造
    Engine& operator=(const Engine&) = delete; // 禁止赋值操作

    /*
    * ------------------------------------------
    * 获取引擎实例(单例模式)
    * ------------------------------------------
    * /outType: unique_ptr<Engine>& 唯一的实例指针
    * ------------------------------------------
    */
    static std::unique_ptr<Engine>& getInstance() // 获取引擎实例
    {
        static auto instance = createInstance();
        return instance;
    }

    /*
    * ------------------------------------------
    * 画直线算法 (Bresenham 直线算法)
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: const Image::Pixel& start 起点坐标
    * /inType: const Image::Pixel& end 终点坐标
    * /inType: const Image::Color& color 颜色
    * ------------------------------------------
    */
    void line(Image& image, const Image::Pixel& start, const Image::Pixel& end, const Image::Color& color); // 画起点和终点之间的直线

    /*
    * ------------------------------------------
    * 画 3D 模型的线框
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: Model& model 3D 模型对象
    * /inType: const Image::Color& color 颜色
    * ------------------------------------------
    */
    void wireframe(Image& image, Model& model, const Image::Color& color); // 画 3D 模型的线框

private:
    Engine() {}; // 私有构造函数，防止实例化
    static std::unique_ptr<Engine> createInstance() // 创建引擎实例
    {
        return std::unique_ptr<Engine>(new Engine());
    }
};