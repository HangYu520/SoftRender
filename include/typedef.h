#pragma once
#include "preheader.h"

// * 自定义数据类型头文件

struct Image // * 图像结构体 (封装 stb_image)
{ 
    // 数据类型
    enum Channel // 图像通道数
    {
        GRAY = 1,
        GRAY_ALPHA = 2,
        RGB = 3,
        RGBA = 4
    };
    struct Color // 颜色结构体
    {
        unsigned char R, G, B;
    };
    struct Pixel // 像素结构体
    {
        uint32_t x, y;
    };   
    
    // 预定义颜色
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color WHITE;
    static const Color BLACK;
    static const Color YELLOW;

    stbi_uc* image_buffer; // 图像数据
    uint32_t width, height; // 图像宽高
    Channel channel; // 图像通道数

    Image(uint32_t w, uint32_t h, Channel c); // 分配图像内存并初始化为0
    
    void setColor(const Pixel& pixel, const Color& color); // 设置指定像素的颜色
    void save(const char* filename); // 写入图像文件
};

struct ARG // * 命令行参数
{
    const char* output_img_file  =  "output.png"; // 输出文件路径
    uint32_t width               =  64; // 图像宽
    uint32_t height              =  64; // 图像宽
    Image::Channel channel       =  Image::Channel::RGB; // 图像通道数
};