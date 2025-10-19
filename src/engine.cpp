#include "engine.h"
#include "preheader.h"
void Engine::line(Image& image, const Image::Pixel& start, const Image::Pixel& end, const Image::Color& color)
{
    // 起点和终点的 x, y
    int xStart = start.x,  yStart = start.y;
    int xEnd   = end.x,      yEnd = end.y;
    
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

void Engine::wireframe(Image& image, Model& model, const Image::Color& color) // 画 3D 模型的线框
{
    // TODO : 1. 平移和缩放模型到视口范围内
    auto [min_pos, max_pos] = model.getBoundingBox(); // 获取模型包围盒
    model.translate(-min_pos.x, -min_pos.y, -min_pos.z); // 平移模型到原点附近

    float model_width  =   max_pos.x - min_pos.x;
    float model_height =   max_pos.y - min_pos.y;
    if (model_width > model_height)
    {
        float scale = (image.width - 1) / model_width;
        model.resize(scale, scale, scale); // 按比例缩放模型
    }
    else
    {
        float scale = (image.height - 1) / model_height;
        model.resize(scale, scale, scale); // 按比例缩放模型
    }
    
    // TODO : 2. 投影到 2D 平面 (简单正交投影)
    auto& attrib = model.getAttrib(); // 投影时直接忽略 z 坐标
    auto trifaces_lst = model.getTrifaces();

    // TODO : 3. 绘制线框
    for (auto& trifaces : trifaces_lst)
    { 
        for (const auto& triface : trifaces)
        {
            auto [v0, v1, v2] = triface.Get(attrib);
            // 提取顶点位置
            Image::Pixel p0 = {static_cast<uint32_t>(v0._position.x), image.height - static_cast<uint32_t>(v0._position.y)};
            Image::Pixel p1 = {static_cast<uint32_t>(v1._position.x), image.height - static_cast<uint32_t>(v1._position.y)};
            Image::Pixel p2 = {static_cast<uint32_t>(v2._position.x), image.height - static_cast<uint32_t>(v2._position.y)};
            // 画三角形的三条边
            line(image, p0, p1, color);
            line(image, p1, p2, color);
            line(image, p2, p0, color);
        }
    }
}