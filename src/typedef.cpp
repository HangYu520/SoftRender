#include "typedef.h"

Image::Image(uint32_t w, uint32_t h, Channel c)
        : width(w), height(h), channel(c)
{
    image_buffer = new stbi_uc[w * h * c](); // 分配图像内存并初始化为0
}

void Image::setColor(const Pixel& pixel, const Color& color)
{
    // 设置指定像素的颜色
    stbi_uc* pixel_buffer = image_buffer + (pixel.y * width + pixel.x) * channel;
    pixel_buffer[0] = color.R;
    pixel_buffer[1] = color.G;
    pixel_buffer[2] = color.B;
}

void Image::save(const char* filename)
{
    bool is_saved = stbi_write_png(filename, width, height, channel, image_buffer, width * channel); // 写入图像文件
    // stbi_write_png() 会自动释放图像数据
    if (is_saved)
        spdlog::info("Image saved to {}", filename);
    else
        spdlog::error("Failed to save image to {}", filename);
}

// 初始化预定义静态颜色常量
const Image::Color Image::BLACK    =   {0, 0, 0};
const Image::Color Image::RED      =   {255, 0, 0};
const Image::Color Image::GREEN    =   {0, 255, 0};
const Image::Color Image::BLUE     =   {0, 0, 255};
const Image::Color Image::WHITE    =   {255, 255, 255};
const Image::Color Image::YELLOW   =   {255, 255, 0};