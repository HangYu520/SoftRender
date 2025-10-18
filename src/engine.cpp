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
    // 起点和终点的 x, y
    int xStart = start.x,  yStart = start.y;
    int xEnd = end.x,        yEnd = end.y;
    
    // * 判断是否陡峭
    bool steep  = std::abs(yEnd - yStart) > std::abs(xEnd - xStart); 
    if (steep)
    {
        // 如果陡峭，交换 x 和 y 坐标
        std::swap(xStart, yStart);
        std::swap(xEnd, yEnd);
    }

    // * 确保从左到右绘制
    if (xStart > xEnd)
    {
        std::swap(xStart, xEnd);
        std::swap(yStart, yEnd);
    }

    float error = 0; // 跟当前位置的垂直偏差值，用以累计小数部分
    int ierror = 0; // 使用整数避免浮点数运算, ierror = error * 2 * (xEnd - xStart)
    uint32_t y = yStart;
    
    for (uint32_t x = xStart; x <= xEnd; ++x)
    {
        #if 0
        // * 方法A: 按比例计算 y 坐标
        float t = (x - xStart) / static_cast<float>(xEnd - xStart); // 比例
        uint32_t y = yStart + t * (yEnd - yStart); // 计算 y 坐标
        #endif
        
        if (steep)
            image.setColor(Image::Pixel(y, x), color);
        else
            image.setColor(Image::Pixel(x, y), color);

        // y += dy; // 增量更新 y 坐标, 与按比例计算等价
        
        #if 0
        // * 方法B: 避免浮点数四舍五入
        float dy = std::abs(yEnd - yStart) / static_cast<float>(xEnd - xStart); // 斜率绝对值
        error += dy; // 累计偏差
        if (error > 0.5f)
        {
            y += yEnd > yStart ? 1 : -1; // 根据斜率方向更新 y 坐标
            error -= 1.0f; // 减去移动后位置的偏差
        }
        #endif

        // * 方法C: Brensenham 算法
        ierror += 2 * std::abs(yEnd - yStart); // 避免浮点数运算
        if (ierror > xEnd - xStart)
        {
            y += yEnd > yStart ? 1 : -1;
            ierror -= 2 * (xEnd - xStart);
        }
    }
    
}