#pragma once
#include "typedef.h"

/*
* ------------------------------------------
* 软渲染绘制算法引擎，单例类(禁止复制，构造)
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

    /*
    * ------------------------------------------
    * 计算有向三角形面积
    * ------------------------------------------
    * /inType: const Image::Pixel& p1 顶点1坐标
    * /inType: const Image::Pixel& p2 顶点2坐标
    * /inType: const Image::Pixel& p3 顶点3坐标
    * /outType: float 有向面积
    * ------------------------------------------
    */
    float signedTriangleArea(const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3);

    /*
    * ------------------------------------------
    * 计算重心坐标
    * ------------------------------------------
    * /inType: const Image::Pixel& p1 顶点1坐标
    * /inType: const Image::Pixel& p2 顶点2坐标
    * /inType: const Image::Pixel& p3 顶点3坐标
    * /inType: const Image::Pixel& p 待计算点坐标
    * /outType: BarycentricCoord 重心坐标
    * ------------------------------------------
    */
    BarycentricCoord getBarycentricCoord(const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3, const Image::Pixel& p);

    /*
    * ------------------------------------------
    * 画三角形 (顶点属性插值)
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: const std::array<Image::PixelWAttrib, 3>& Trianlge 三角形顶点及属性
    * ------------------------------------------
    */
    void triangle(Image& image, const std::array<Image::PixelwAttrib, 3>& Trianlge); // 画指定顶点属性的三角形

    /*
    * ------------------------------------------
    * 渲染 3D 模型到图像
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: Model& model 3D 模型对象
    * ------------------------------------------
    */
    void render(Image& image, Model& model); // 渲染 3D 模型到图像

    /*
    * ------------------------------------------
    * 获取深度图
    * ------------------------------------------
    */
    void getDepthImage(Image& image); // 获取深度图
    
    /*
    * ------------------------------------------
    * 手动释放深度缓冲区
    * ------------------------------------------
    */
    void freeZBuffer()
    {
        if (m_zBuffer)
        {
            delete[] m_zBuffer;
            m_zBuffer = nullptr;
        }
        m_zBufferSize = 0;
    }

private:
    Engine() {}; // 私有构造函数，防止实例化
    static std::unique_ptr<Engine> createInstance() // 创建引擎实例
    {
        return std::unique_ptr<Engine>(new Engine());
    }
    // 私有变量
    float* m_zBuffer = nullptr; // 深度缓冲区
    uint32_t m_zBufferSize = 0; // 深度缓冲区大小

    /*
    * ------------------------------------------
    * 画三角形 (边界框算法/扫描线算法， 无属性插值)
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: const Image::Pixel& p1 顶点1坐标
    * /inType: const Image::Pixel& p2 顶点2坐标
    * /inType: const Image::Pixel& p3 顶点3坐标
    * /inType: const Image::Color& color 颜色
    * ------------------------------------------
    */
    void triangle(Image& image, const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3, const Image::Color& color); // 画三角形
};