#include "engine.h"
#include "preheader.h"
void Engine::line
(
    Image& image, // 图像对象
    const Image::Pixel& start, // 起点坐标 
    const Image::Pixel& end, // 终点坐标
    const Image::Color& color // 颜色
)
{
    auto _start = start; // 局部变量，允许修改
    auto _end   = end;
    
    if (_start.x > _end.x)
    {
        // 确保从左到右绘制
        std::swap(_start.x, _end.x);
        std::swap(_start.y, _end.y);
    }

    bool steep  = _end.y - _start.y > _end.x - _start.x; // 判断是否陡峭
    if (steep)
    {
        // 如果陡峭，交换 x 和 y 坐标
        std::swap(_start.x, _start.y);
        std::swap(_end.x, _end.y);
    }

    for (uint32_t x = _start.x; x <= _end.x; ++x)
    {
        float t = (x - _start.x) / static_cast<float>(_end.x - _start.x); // 比例
        uint32_t y = _start.y + t * (_end.y - _start.y); // 计算 y 坐标
        
        if (steep)
            image.setColor(Image::Pixel(y, x), color);
        else
            image.setColor(Image::Pixel(x, y), color);
    }
}