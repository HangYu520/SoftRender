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

void Engine::wireframe(Image& image, Model& model, const Image::Color& color) // 画 3D 模型的线框
{
    auto& attrib = model.getAttrib();
    auto trifaces_lst = model.getTrifaces();

    for (auto& trifaces : trifaces_lst)
    { 
        for (const auto& triface : trifaces)
        {
            auto [v0, v1, v2] = triface.Get(attrib);
            // TODO : 1. 位置变换, 视图变换和投影
            auto [pos0, pos1, pos2] = MVPTrans(v0, v1, v2);
            // TODO : 2. 裁剪视锥外的面
            std::vector<glm::vec4> clippedVertices = clipTriangle(pos0, pos1, pos2);
            if (clippedVertices.empty()) continue;
            for (int i = 0; i < clippedVertices.size() - 3 + 1; i++) // 裁剪完三角形的个数 : 0 or 1 or 2
            {
                glm::vec4 vec0 = clippedVertices[i+0];
                glm::vec4 vec1 = clippedVertices[i+1];
                glm::vec4 vec2 = clippedVertices[i+2];
                // TODO : 3. 透视除法和视口变换, 转换到屏幕坐标 
                auto [p0, p1, p2] = ViewportTrans(image, vec0, vec1, vec2);
                // TODO ; 4. 画线
                line(image, p0, p1, color);
                line(image, p1, p2, color);
                line(image, p2, p0, color);
            }
        }
    }
}

float Engine::signedTriangleArea(const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3)
{
    // 计算有向三角形面积, 正值表示朝向内，负值表示朝向外
    return 0.5f * ((p2.y - p1.y)*(p2.x + p1.x) + (p3.y - p2.y)*(p3.x + p2.x) + (p1.y - p3.y)*(p1.x + p3.x));
}

BarycentricCoord Engine::getBarycentricCoord(const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3, const Image::Pixel& p)
{
    // 计算三角形的带符号面积
    float area_total = signedTriangleArea(p1, p2, p3);
    float area_u     = signedTriangleArea(p, p2, p3);
    float area_v     = signedTriangleArea(p1, p, p3);
    float area_w     = signedTriangleArea(p1, p2, p);

    // 计算重心坐标
    return {area_u / area_total, area_v / area_total, area_w / area_total};
}

void Engine::triangle(Image& image, const std::array<Image::PixelwAttrib, 3>& Trianlge) // 画指定顶点属性的三角形
{
    // 获取三角形顶点和属性
    Image::Pixel p1 = Trianlge[0].pixel; Image::Color c1 = Trianlge[0].color; float z1 = Trianlge[0].z;
    Image::Pixel p2 = Trianlge[1].pixel; Image::Color c2 = Trianlge[1].color; float z2 = Trianlge[1].z;
    Image::Pixel p3 = Trianlge[2].pixel; Image::Color c3 = Trianlge[2].color; float z3 = Trianlge[2].z;

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
            // TODO : 3. 计算像素的重心坐标
            Image::Pixel p = {x, y};
            auto barycentric = getBarycentricCoord(p1, p2, p3, p);
            float u = barycentric.u; 
            float v = barycentric.v; 
            float w = barycentric.w;
            if (u >= 0 && v >= 0 && w >= 0) // 说明在三角形内
            {
                // TODO : 4. 插值深度
                float z = u * z1 + v * z2 + w * z3;
                if (m_zBuffer && z < m_zBuffer[x + y * image.width])
                    continue; // 深度测试未通过，跳过该像素
                
                // TODO : 5. 插值颜色
                float R = u * c1.R + v * c2.R + w * c3.R;
                float G = u * c1.G + v * c2.G + w * c3.G;
                float B = u * c1.B + v * c2.B + w * c3.B;
                image.setColor(p, Image::Color{static_cast<unsigned char>(R), static_cast<unsigned char>(G), static_cast<unsigned char>(B)});

                if (m_zBuffer)
                    m_zBuffer[x + y * image.width] = z; // 更新深度缓冲区
            }
        }
    }
}

// ! render 函数还需要完善
void Engine::render(Image& image, Model& model) // 渲染 3D 模型到图像
{
    auto& attrib = model.getAttrib();
    auto trifaces_lst = model.getTrifaces();

    // TODO : 1. 初始化深度缓冲区
    if (m_zBuffer) delete[] m_zBuffer; // 释放旧深度缓冲区
    m_zBufferSize = image.width * image.height;
    m_zBuffer = new float[m_zBufferSize]; // 初始化深度缓冲区
    std::fill(m_zBuffer, m_zBuffer + m_zBufferSize, -2.0f);
    
    for (auto& trifaces : trifaces_lst)
    { 
        for (const auto& triface : trifaces)
        {
            auto [v0, v1, v2] = triface.Get(attrib);
            // TODO : 2. 位置变换, 视图变换和投影
            auto [pos0, pos1, pos2] = MVPTrans(v0, v1, v2);
            // TODO : 3. 裁剪视锥外的面
            std::vector<glm::vec4> clippedVertices = clipTriangle(pos0, pos1, pos2);
            if (clippedVertices.empty()) continue;
            for (int i = 0; i < clippedVertices.size() - 3 + 1; i++) // 裁剪完三角形的个数 : 0 or 1 or 2
            {
                glm::vec4 vec0 = clippedVertices[i+0];
                glm::vec4 vec1 = clippedVertices[i+1];
                glm::vec4 vec2 = clippedVertices[i+2];
                // TODO : 4. 透视除法和视口变换, 转换到屏幕坐标 
                auto [p0, p1, p2] = ViewportTrans(image, vec0, vec1, vec2);
                // 提取顶点深度, 变换后的 w 存储着视图变换后的深度 : - z
                float z0 = -vec0.w, z1 = -vec1.w, z2 = -vec2.w;
                // TODO : 5. 画三角形的面
                Image::Color color = Image::WHITE;
                triangle(image, {Image::PixelwAttrib{p0, color, z0}, Image::PixelwAttrib{p1, color, z1}, Image::PixelwAttrib{p2, color, z2}});
            }
        }
    }
}

void Engine::centerModel(Model& model) 
{
    // 1. 获取模型包围盒
    auto [min_pos, max_pos] = model.getBoundingBox();
    
    // 2. 计算模型尺寸
    float model_width  = max_pos.x - min_pos.x;
    float model_height = max_pos.y - min_pos.y;
    float model_depth  = max_pos.z - min_pos.z;

    // 3. 缩放
    float scale = 1.0f / std::max({model_width, model_height, model_depth});
    m_transConfig.mConfig.scale = glm::vec3(scale, scale, scale);
    
    // 4. 中心化
    m_transConfig.mConfig.translate = glm::vec3(
        - scale * (min_pos.x + model_width/2.0f),
        - scale * (min_pos.y + model_height/2.0f),
        - scale * model_depth/2.0f
    );
}

void Engine::getDepthImage(Image& image) // 获取深度图
{
    if (!m_zBuffer)
    {
        spdlog::error("Can not get the depth image as zBuffer is null !");
        exit(1);
    }

    if (image.channel != Image::Channel::GRAY)
    {
        spdlog::error("Can not get the depth image as image channel is not gray !");
        exit(1);
    }
    
    auto width = image.width;
    auto height = image.height;
    
    if (width * height != m_zBufferSize)
    {
        spdlog::error("The depth image's width * height is not equal to zBuffer size !");
        exit(1);
    }
    
    // 找到最大和最小深度
    float max_z = *std::max_element(m_zBuffer, m_zBuffer + m_zBufferSize);
    float min_z = *std::min_element(m_zBuffer, m_zBuffer + m_zBufferSize);

    // 生成深度图
    for (int i = 0; i < m_zBufferSize; ++i)
    {
        float z = m_zBuffer[i];
            
        unsigned char GRAY = static_cast<unsigned char>(255 * (z - min_z) / (max_z - min_z));

        image.image_buffer[i] = GRAY;
    }
}

glm::mat4 Engine::getModelMatrix() // 获取模型矩阵
{
    TransConfig::M config = m_transConfig.mConfig;
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), config.scale);
    glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), config.translate);

    float angle = glm::radians(config.rotateAngle); // 转换为弧度
    auto axis = config.rotateAxis;
    glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);

    auto model = translateMatrix * rotateMatrix * scaleMatrix;

    return model;
}

glm::mat4 Engine::getViewMatrix() // 获取视图矩阵
{
    TransConfig::V config = m_transConfig.vConfig;
    glm::mat4 view = glm::lookAt(config.cameraPos, config.cameraTarget, config.cameraUp);

    return view;
}

glm::mat4 Engine::getProjectMatrix() // 获取投影矩阵
{
    TransConfig::P config = m_transConfig.pConfig;
    glm::mat4 projection = glm::perspective(glm::radians(config.fov), config.aspect, config.near, config.far);

    return projection;
}

std::tuple<glm::vec4, glm::vec4, glm::vec4> Engine::MVPTrans(const Model::attrib_t& v0, const Model::attrib_t& v1, const Model::attrib_t& v2)
{
    glm::mat4 M = getModelMatrix();
    glm::mat4 V = getViewMatrix();
    glm::mat4 P = getProjectMatrix();

    glm::vec4 pos0 = P * V * M * glm::vec4{v0._position.x, v0._position.y, v0._position.z, 1.0f};
    glm::vec4 pos1 = P * V * M * glm::vec4{v1._position.x, v1._position.y, v1._position.z, 1.0f};
    glm::vec4 pos2 = P * V * M * glm::vec4{v2._position.x, v2._position.y, v2._position.z, 1.0f};

    return std::make_tuple(pos0, pos1, pos2);
}

std::tuple<Image::Pixel, Image::Pixel, Image::Pixel> Engine::ViewportTrans(const Image& image, const glm::vec4& pos0, const glm::vec4& pos1, const glm::vec4& pos2)
{
    Image::Pixel p0 = {static_cast<int>((pos0.x / pos0.w + 1)*image.width/2), static_cast<int>((pos0.y / pos0.w + 1)*image.height/2)};
    Image::Pixel p1 = {static_cast<int>((pos1.x / pos1.w + 1)*image.width/2), static_cast<int>((pos1.y / pos1.w + 1)*image.height/2)};
    Image::Pixel p2 = {static_cast<int>((pos2.x / pos2.w + 1)*image.width/2), static_cast<int>((pos2.y / pos2.w + 1)*image.height/2)};
    
    return std::make_tuple(p0, p1, p2);
}

bool Engine::isInside(const glm::vec4& plane, const glm::vec4& pos) // 判断点是否在平面内侧
{
    return plane.x * pos.x + plane.y * pos.y + plane.z * pos.z + plane.w * pos.w >= 0.0f;
}

glm::vec4 Engine::interSect(const glm::vec4& plane, const glm::vec4& pos0, const glm::vec4& pos1) // 获取平面和线段相交的点
{
    float d0 = plane.x * pos0.x + plane.y * pos0.y + plane.z * pos0.z + plane.w * pos0.w;
    float d1 = plane.x * pos1.x + plane.y * pos1.y + plane.z * pos1.z + plane.w * pos1.w;
    float weight = d0 / (d0 - d1);

    return glm::lerp(pos0, pos1, weight);
}

std::vector<glm::vec4> Engine::clipTriangle(const glm::vec4& pos0, const glm::vec4& pos1, const glm::vec4& pos2)
{
    std::vector<glm::vec4> clipped = {pos0, pos1, pos2};

    bool allInside = true;
    for (int i = 0; i < 3; ++i)
    {
        bool outside = clipped[i].x > clipped[i].w || clipped[i].y > clipped[i].w || clipped[i].z > clipped[i].w;
        outside = outside || clipped[i].x < -clipped[i].w || clipped[i].y < -clipped[i].w || clipped[i].z < -clipped[i].w;
        if (outside)
        {
            allInside = false;
            break;
        }
    }
    if (allInside)
        return clipped; // 全部在内侧, 无需裁剪, 直接返回

    const std::vector<glm::vec4> Planes =  // 需要裁剪的平面方程
    {   //near
        glm::vec4(0,0,1,1),
        //far
        glm::vec4(0,0,-1,1),
        //left
        glm::vec4(1,0,0,1),
        //right
        glm::vec4(-1,0,0,1),
        //top 
        glm::vec4(0,-1,0,1),
        //bottom
        glm::vec4(0,1,0,1)
    };
    
    // 逐边裁剪
    for (auto& plane: Planes)
    {
        std::vector<glm::vec4> input(clipped);
        clipped.clear();
        for (int j = 0; j < input.size(); j++)
        {
            glm::vec4 pos0 = input[j];
            glm::vec4 pos1 = input[(j + input.size() - 1) % input.size()];
            if (isInside(plane, pos0))
            {
                if (!isInside(plane, pos1))
                    clipped.push_back(interSect(plane, pos0, pos1));
                clipped.push_back(pos0);
            }
            else if (isInside(plane, pos1))
            {
                clipped.push_back(interSect(plane, pos1, pos0));
            }
        }
    }
    return clipped;
}

#if defined(SCANLINE)
//! 古早的三角形填充算法，建议使用上面的带属性插值的版本
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
            // TODO : 3. 判断像素是否在三角形内
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