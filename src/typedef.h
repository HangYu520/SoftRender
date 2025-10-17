#pragma once

struct Image // * 图像结构体 (封装 stb_image)
{ 
    // 图像通道数
    enum Channel
    {
        GRAY = 1,
        GRAY_ALPHA = 2,
        RGB = 3,
        RGBA = 4
    };
    // 颜色结构体
    struct Color
    {
        unsigned char R, G, B;
    };
    // 预定义颜色
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color WHITE;
    static const Color BLACK;

    stbi_uc* image_buffer; // 图像数据
    stbi__uint32 width, height; // 图像宽高
    Channel channel; // 图像通道数

    Image(stbi__uint32 w, stbi__uint32 h, Channel c)
        : width(w), height(h), channel(c)
    {
        image_buffer = new stbi_uc[w * h * c](); // 分配图像内存并初始化为0
    }
    void setColor(stbi__uint32 x, stbi__uint32 y, const Image::Color color)
    {
        // 设置指定像素的颜色
        stbi_uc* pixel = image_buffer + (y * width + x) * channel;
        pixel[0] = color.R;
        pixel[1] = color.G;
        pixel[2] = color.B;
    }
    void save(const char* filename)
    {
        bool is_saved = stbi_write_png(filename, width, height, channel, image_buffer, width * channel); // 写入图像文件
        // stbi_write_png() 会自动释放图像数据
        if (is_saved)
            spdlog::info("Image saved to {}", filename);
        else
            spdlog::error("Failed to save image to {}", filename);
    }
};

const Image::Color Image::RED   = {255, 0, 0};
const Image::Color Image::GREEN = {0, 255, 0};
const Image::Color Image::BLUE  = {0, 0, 255};
const Image::Color Image::WHITE = {255, 255, 255};
const Image::Color Image::BLACK = {0, 0, 0};

struct ARG // * 命令行参数
{
    const char* output_img_file = "output.png"; // 输出文件路径
    stbi__uint32 width = 64, height = 64; // 图像宽高
    Image::Channel channel = Image::Channel::RGB; // 图像通道数
};