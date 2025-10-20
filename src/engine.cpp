#include "engine.h"
#include "preheader.h"

void Engine::line(Image& image, const Image::Pixel& start, const Image::Pixel& end, const Image::Color& color)
{
    // 起点和终点的 x, y
    int xStart = start.x,  yStart = start.y;
    int xEnd   = end.x,      yEnd = end.y;
    
    // TODO : 1. 判断是否陡峭
    bool steep  = std::abs(yEnd - yStart) > std::abs(xEnd - xStart); 
    if (steep)
    {
        // 如果陡峭，交换 x 和 y 坐标
        std::swap(xStart, yStart);
        std::swap(xEnd, yEnd);
    }

    // TODO : 2. 确保从左到右绘制
    if (xStart > xEnd)
    {
        std::swap(xStart, xEnd);
        std::swap(yStart, yEnd);
    }

    float error = 0; // 跟当前位置的垂直偏差值，用以累计小数部分
    int ierror = 0; // 使用整数避免浮点数运算, ierror = error * 2 * (xEnd - xStart)
    int y = yStart;
    
    for (int x = xStart; x <= xEnd; ++x)
    {
        #if 0
        // * 方法A: 按比例计算 y 坐标
        float t = (x - xStart) / static_cast<float>(xEnd - xStart); // 比例
        int y = yStart + t * (yEnd - yStart); // 计算 y 坐标
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

        // TODO : 3. 方法C: Brensenham 算法
        ierror += 2 * std::abs(yEnd - yStart); // 避免浮点数运算
        if (ierror > xEnd - xStart)
        {
            y += yEnd > yStart ? 1 : -1;
            ierror -= 2 * (xEnd - xStart);
        }
    }
    
}

#if defined(SCANLINE)
void Engine::triangle(Image& image, const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3, const Image::Color& color) // 画三角形
{
    // TODO : 1. 排序顶点按 y 坐标从小到大
    Image::Pixel v[3] = {p1, p2, p3};
    // 冒泡排序
    for (int i = 0; i < 3; ++i)
    {
        for (int j = i + 1; j < 3; ++j)
        {
            if (v[i].y > v[j].y)
            {
                std::swap(v[i], v[j]);
            }
        }
    }
    // TODO : 2. 扫描线填充
    for (int y = v[0].y; y <= v[2].y; ++y)
    {
        int x_left, x_right;
        // v0 - v2 边界
        x_left  = v[0].x + (v[2].x - v[0].x) * (y - v[0].y) / (v[2].y - v[0].y);
        
        if (y <v[1].y) // 下半部分
            // v0 - v1 边界
            x_right = v[0].x + (v[1].x - v[0].x) * (y - v[0].y) / (v[1].y - v[0].y);
        else // 上半部分
            // v1 - v2 边界
            x_right = v[1].x + (v[2].x - v[1].x) * (y - v[1].y) / (v[2].y - v[1].y);
        
        line(image, Image::Pixel{x_left, y}, Image::Pixel{x_right, y}, color);
    }
}
#else

void Engine::triangle(Image& image, const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3, const Image::Color& color) // 画三角形
{
    // TODO : 1. 计算边界框
    int min_x = std::min({p1.x, p2.x, p3.x});
    int max_x = std::max({p1.x, p2.x, p3.x});
    int min_y = std::min({p1.y, p2.y, p3.y});
    int max_y = std::max({p1.y, p2.y, p3.y});

    // TODO : 2. 遍历边界框内的像素
    for (int y = min_y; y <= max_y; ++y)
    {
        for (int x = min_x; x <= max_x; ++x)
        {
            // TODO : 2. 判断像素是否在三角形内
            Image::Pixel p = {x, y};
            int cross1 = (p2.x - p1.x) * (p.y - p1.y) - (p2.y - p1.y) * (p.x - p1.x); // (p1,p2) X (p1, p)
            int cross2 = (p3.x - p2.x) * (p.y - p2.y) - (p3.y - p2.y) * (p.x - p2.x); // (p2,p3) X (p2, p)
            int cross3 = (p1.x - p3.x) * (p.y - p3.y) - (p1.y - p3.y) * (p.x - p3.x); // (p3,p1) X (p3, p)
            if ((cross1 >= 0 && cross2 >= 0 && cross3 >= 0) || (cross1 <= 0 && cross2 <= 0 && cross3 <= 0))
            {
                // 叉积符号相同，说明在三角形内
                image.setColor(p, color); // 设置像素颜色
            }
        }
    }
}
#endif

// ! wireframe 还需要完善
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
            Image::Pixel p0 = {static_cast<int>(v0._position.x), static_cast<int>(v0._position.y)};
            Image::Pixel p1 = {static_cast<int>(v1._position.x), static_cast<int>(v1._position.y)};
            Image::Pixel p2 = {static_cast<int>(v2._position.x), static_cast<int>(v2._position.y)};
            // 画三角形的三条边
            line(image, p0, p1, color);
            line(image, p1, p2, color);
            line(image, p2, p0, color);
        }
    }
}

// ! render 函数还需要完善
void Engine::render(Image& image, Model& model) // 渲染 3D 模型到图像
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

    // TODO : 3. 绘制模型
    for (auto& trifaces : trifaces_lst)
    { 
        for (const auto& triface : trifaces)
        {
            auto [v0, v1, v2] = triface.Get(attrib);
            // 提取顶点位置
            Image::Pixel p0 = {static_cast<int>(v0._position.x), static_cast<int>(v0._position.y)};
            Image::Pixel p1 = {static_cast<int>(v1._position.x), static_cast<int>(v1._position.y)};
            Image::Pixel p2 = {static_cast<int>(v2._position.x), static_cast<int>(v2._position.y)};
            // 画三角形的面
            triangle(image, p0, p1, p2, Image::Color::randColor());
        }
    }
}