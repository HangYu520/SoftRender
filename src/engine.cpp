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
    // 获取模型的原始数据
    auto& attrib = model.getAttrib();
    auto trifaces_lst = model.getTrifaces();
    auto M = getModelMatrix(), V = getViewMatrix(), P = getProjectMatrix();
    
    // * 开启渲染流程
    for (auto& trifaces : trifaces_lst)
    { 
        for (const auto& triface : trifaces) // 遍历三角形
        {
            // * 获取顶点的原始数据
            auto [rawVertex0, rawVertex1, rawVertex2] = triface.Get(attrib);
            // * 投影矩阵变换三角形
            glm::vec4 clipPos0 = P * V * M * glm::vec4(rawVertex0._position.x, rawVertex0._position.y, rawVertex0._position.z, 1.0f);
            glm::vec4 clipPos1 = P * V * M * glm::vec4(rawVertex1._position.x, rawVertex1._position.y, rawVertex1._position.z, 1.0f);
            glm::vec4 clipPos2 = P * V * M * glm::vec4(rawVertex2._position.x, rawVertex2._position.y, rawVertex2._position.z, 1.0f);
            V2F vertex0 = {glm::vec4(0, 0, 0, 0), clipPos0, glm::vec3(0, 0, 0)};
            V2F vertex1 = {glm::vec4(0, 0, 0, 0), clipPos1, glm::vec3(0, 0, 0)};
            V2F vertex2 = {glm::vec4(0, 0, 0, 0), clipPos2, glm::vec3(0, 0, 0)};
            // * 对顶点数据进行裁剪
            std::vector<V2F> clippedVertices = clipTriangle(vertex0, vertex1, vertex2);
            if (clippedVertices.empty()) continue; // 若丢弃三角形, 则跳过该三角形
            for (int i = 0; i < clippedVertices.size() - 3 + 1; i++) // 裁剪完三角形的个数 : 1 or 2, 顶点个数: 3 or 4
            {
                // 获取裁剪后的顶点
                V2F v0 = clippedVertices[i+0];
                V2F v1 = clippedVertices[i+1];
                V2F v2 = clippedVertices[i+2];
                auto ndc = NDC({v0, v1, v2});
                auto screen = viewport(image, ndc);
                // * 画三角形的线框
                line(image, screen[0], screen[1], color);
                line(image, screen[1], screen[2], color);
                line(image, screen[2], screen[0], color);
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

void Engine::rasterize(Image& image, Shader* shader) // 光栅化画三角形
{
    // TODO : 1. 透视除法得到 NDC 坐标
    auto Triangle = shader->getTriangle();
    auto ndc = NDC(Triangle);
    // TODO : 2. 视口变换得到屏幕坐标
    auto screen = viewport(image, ndc);
    // TODO : 3. 计算边界框
    int min_x = std::min({screen[0].x, screen[1].x, screen[2].x});
    int max_x = std::max({screen[0].x, screen[1].x, screen[2].x});
    int min_y = std::min({screen[0].y, screen[1].y, screen[2].y});
    int max_y = std::max({screen[0].y, screen[1].y, screen[2].y});

    // * 遍历边界框内的像素
    for (int y = min_y; y <= max_y; ++y)
    {
        for (int x = min_x; x <= max_x; ++x)
        {
            // TODO : 4. 计算像素的重心坐标
            Image::Pixel p = {x, y};
            auto barycentric = getBarycentricCoord(screen[0], screen[1], screen[2], p);
            float u = barycentric.u, v = barycentric.v, w = barycentric.w;
            if (u >= 0 && v >= 0 && w >= 0) // 说明在三角形内
            {
                // TODO : 5. 插值深度构造片元
                float z = -1 * (u * Triangle[0].clipPosition.w + v * Triangle[1].clipPosition.w + w * Triangle[2].clipPosition.w);
                if (m_zBuffer && z < m_zBuffer[x + y * image.width]) continue; // 深度测试未通过，跳过该像素
                Fragment fragment = {Image::Pixel(x, y), z, barycentric};
                // TODO : 6. 调用片元着色器
                auto [isDiscard, color] = shader->fragmentShader(fragment);
                if (!isDiscard) image.setColor(p, color);
                // 更新深度缓冲区
                if (m_zBuffer) m_zBuffer[x + y * image.width] = z; 
            }
        }
    }
}

void Engine::render(Image& image, Model& model, Shader* shader) // 渲染 3D 模型的流程 (类 GPU 管线)
{
    // 获取模型的原始数据
    auto& attrib = model.getAttrib();
    auto trifaces_lst = model.getTrifaces();
    shader->setMVP(getModelMatrix(), getViewMatrix(), getProjectMatrix()); // 设置 MVP 矩阵
    
    // * 开启渲染流程
    for (auto& trifaces : trifaces_lst)
    { 
        for (const auto& triface : trifaces) // 遍历三角形
        {
            // TODO : 1. 获取顶点的原始数据
            auto [rawVertex0, rawVertex1, rawVertex2] = triface.Get(attrib);
            // TODO : 2. 调用顶点着色器
            V2F vertex0 = shader->vertexShader(rawVertex0);
            V2F vertex1 = shader->vertexShader(rawVertex1);
            V2F vertex2 = shader->vertexShader(rawVertex2);
            // TODO : 3. 对顶点着色器返回的顶点数据进行裁剪
            std::vector<V2F> clippedVertices = clipTriangle(vertex0, vertex1, vertex2);
            if (clippedVertices.empty()) continue; // 若丢弃三角形, 则跳过该三角形
            for (int i = 0; i < clippedVertices.size() - 3 + 1; i++) // 裁剪完三角形的个数 : 1 or 2, 顶点个数: 3 or 4
            {
                // 获取裁剪后的顶点
                V2F v0 = clippedVertices[0];
                V2F v1 = clippedVertices[i+1];
                V2F v2 = clippedVertices[i+2];
                // TODO : 4. 光栅化三角形
                shader->updateTriangle({v0, v1, v2});
                rasterize(image, shader);
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

std::array<glm::vec3, 3> Engine::NDC(const std::array<V2F, 3>& Triangle)
{
    // 裁剪坐标转 NDC : x/w, y/w, z/w
    glm::vec3 ndc0 = {
        Triangle[0].clipPosition.x / Triangle[0].clipPosition.w,
        Triangle[0].clipPosition.y / Triangle[0].clipPosition.w,
        Triangle[0].clipPosition.z / Triangle[0].clipPosition.w
    };

    glm::vec3 ndc1 = {
        Triangle[1].clipPosition.x / Triangle[1].clipPosition.w,
        Triangle[1].clipPosition.y / Triangle[1].clipPosition.w,
        Triangle[1].clipPosition.z / Triangle[1].clipPosition.w
    };

    glm::vec3 ndc2 = {
        Triangle[2].clipPosition.x / Triangle[2].clipPosition.w,
        Triangle[2].clipPosition.y / Triangle[2].clipPosition.w,
        Triangle[2].clipPosition.z / Triangle[2].clipPosition.w
    };

    return {ndc0, ndc1, ndc2};
}

std::array<Image::Pixel, 3> Engine::viewport(const Image& image, const std::array<glm::vec3, 3>& ndc)
{
    Image::Pixel pixel0 = {static_cast<int>((ndc[0].x + 1) * image.width / 2), static_cast<int>((ndc[0].y + 1) * image.height / 2)};
    Image::Pixel pixel1 = {static_cast<int>((ndc[1].x + 1) * image.width / 2), static_cast<int>((ndc[1].y + 1) * image.height / 2)};
    Image::Pixel pixel2 = {static_cast<int>((ndc[2].x + 1) * image.width / 2), static_cast<int>((ndc[2].y + 1) * image.height / 2)};

    return {pixel0, pixel1, pixel2};
}

bool Engine::isInside(const glm::vec4& plane, const glm::vec4& pos) // 判断点是否在平面内侧
{
    return plane.x * pos.x + plane.y * pos.y + plane.z * pos.z + plane.w * pos.w >= 0.0f;
}

bool Engine::allInside(const std::vector<V2F>& vertices)
{
    bool allInside = true;
    for (auto& vertex : vertices)
    {
        bool outside = vertex.clipPosition.x > vertex.clipPosition.w || vertex.clipPosition.y > vertex.clipPosition.w || vertex.clipPosition.z > vertex.clipPosition.w;
        outside = outside || vertex.clipPosition.x < -vertex.clipPosition.w || vertex.clipPosition.y < -vertex.clipPosition.w || vertex.clipPosition.z < -vertex.clipPosition.w;
        if (outside) return false;
    }
    return allInside;
}

V2F Engine::interSect(const glm::vec4& plane, const V2F& vertex0, const V2F& vertex1) // 获取平面和线段相交的点
{
    float d0 = plane.x * vertex0.clipPosition.x + plane.y * vertex0.clipPosition.y + plane.z * vertex0.clipPosition.z + plane.w * vertex0.clipPosition.w;
    float d1 = plane.x * vertex1.clipPosition.x + plane.y * vertex1.clipPosition.y + plane.z * vertex1.clipPosition.z + plane.w * vertex1.clipPosition.w;
    float weight = d0 / (d0 - d1);

    return lerp(vertex0, vertex1, weight);
}

std::vector<V2F> Engine::clipTriangle(const V2F& vertex0, const V2F& vertex1, const V2F& vertex2)
{
    std::vector<V2F> clipped = {vertex0, vertex1, vertex2};

    if (allInside(clipped))
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
        std::vector<V2F> input(clipped);
        clipped.clear();
        for (int j = 0; j < input.size(); j++)
        {
            V2F v0 = input[j];
            V2F v1 = input[(j + input.size() - 1) % input.size()];
            if (isInside(plane, v0.clipPosition))
            {
                if (!isInside(plane, v1.clipPosition))
                    clipped.push_back(interSect(plane, v0, v1));
                clipped.push_back(v0);
            }
            else if (isInside(plane, v1.clipPosition))
            {
                clipped.push_back(interSect(plane, v1, v0));
            }
        }
    }
    return clipped;
}

#if defined(SCANLINE)
//! 古早的三角形填充算法
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