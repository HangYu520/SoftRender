#include "shader.h"

V2F lerp(const V2F& v1, const V2F& v2, float weight)
{
    V2F v;
    v.eyePosition = glm::lerp(v1.eyePosition, v2.eyePosition, weight);
    v.clipPosition = glm::lerp(v1.clipPosition, v2.clipPosition, weight);
    v.color = glm::lerp(v1.color, v2.color, weight);
    v.normal = glm::lerp(v1.normal, v2.normal, weight);
    v.texcoord = glm::lerp(v1.texcoord, v2.texcoord, weight);
    // TODO 其他属性插值
    return v;
}

/*
* -----------------------------
* 随机色填充 shader
* -----------------------------
*/
V2F RandomShader::vertexShader(const Model::attrib_t& rawVertex) // 顶点着色器
{
    V2F v;
    v.eyePosition = viewMatrix * modelMatrix * glm::vec4(rawVertex._position.x, rawVertex._position.y, rawVertex._position.z, 1.0f);
    v.clipPosition = projectionMatrix * v.eyePosition;
    Image::Color randcolor = Image::Color::randColor();
    v.color = glm::vec3(randcolor.R, randcolor.G, randcolor.B);
    return v;
}

std::pair<bool, Image::Color> RandomShader::fragmentShader(const Fragment& fragment)// 片元着色器
{
    Image::Color color(currentTriangle[0].color.x, currentTriangle[0].color.y, currentTriangle[0].color.z);
    return std::make_pair(false, color);
}

/*
* -----------------------------
* Blinn-Phong shader
* -----------------------------
*/
V2F PhongShader::vertexShader(const Model::attrib_t& rawVertex) // 顶点着色器
{
    V2F v;
    v.eyePosition = viewMatrix * modelMatrix * glm::vec4(rawVertex._position.x, rawVertex._position.y, rawVertex._position.z, 1.0f);
    v.clipPosition = projectionMatrix * v.eyePosition;
    v.normal = glm::transpose(glm::inverse(modelMatrix)) * glm::vec4(rawVertex._normal.nx, rawVertex._normal.ny, rawVertex._normal.nz, 0.0f);
    v.normal = glm::normalize(v.normal);
    v.texcoord = glm::vec2(rawVertex._texcoord.u, rawVertex._texcoord.v);
    return v;
}

std::pair<bool, Image::Color> PhongShader::fragmentShader(const Fragment& fragment)// 片元着色器
{
    float PhongCoeff = this->precomputedFlatCoeff;
    if (smooth) // 逐片元着色
    {   // ! CPU 上运行速度较慢
        // 片元位置 （重心坐标插值）
        auto v0 = currentTriangle[0].eyePosition, v1 = currentTriangle[1].eyePosition, v2 = currentTriangle[2].eyePosition;
        glm::vec4 fragment_position = fragment.baryCoord.u * v0 + fragment.baryCoord.v * v1 + fragment.baryCoord.w * v2;
        // 片元法向量
        glm::vec4 fragment_normal;
        if (normal_map.image_buffer && use_normal_map)
        {
            // 法向贴图
            auto t0 = currentTriangle[0].texcoord, t1 = currentTriangle[1].texcoord, t2 = currentTriangle[2].texcoord;
            auto fragment_textcoord = fragment.baryCoord.u * t0 + fragment.baryCoord.v * t1 + fragment.baryCoord.w * t2; // 片元纹理坐标
            // 纹理坐标映射到图像
            int x = fragment_textcoord.x * normal_map.width, y = fragment_textcoord.y * normal_map.height;
            Image::Pixel pixel(x, y);
            Image::Color normal_color = normal_map.getColor(pixel).first;
            // 颜色映射到法向
            fragment_normal.x = float(normal_color.R) * 2.0 / 255.0f - 1.0f;
            fragment_normal.y = float(normal_color.G) * 2.0 / 255.0f - 1.0f;
            fragment_normal.z = float(normal_color.B) * 2.0 / 255.0f - 1.0f;
            // 变换切线空间法向到世界空间
            if (use_tangent_map)
            { 
                auto TBN = getTBN(fragment);
                fragment_normal = TBN * fragment_normal;
            }
        }
        else
        {
            // 重心坐标插值
            auto n0 = currentTriangle[0].normal, n1 = currentTriangle[1].normal, n2 = currentTriangle[2].normal;
            fragment_normal = fragment.baryCoord.u * n0 + fragment.baryCoord.v * n1 + fragment.baryCoord.w * n2;
        }
        PhongCoeff = _fragmentShader(fragment_position, fragment_normal);
    }
    // 片元颜色
    glm::vec3 color_vec = currentTriangle[0].color; // 基础颜色
    if (texture_map.image_buffer && use_texture_map)
    {
        // 片元纹理颜色
        auto t0 = currentTriangle[0].texcoord, t1 = currentTriangle[1].texcoord, t2 = currentTriangle[2].texcoord;
        auto fragment_textcoord = fragment.baryCoord.u * t0 + fragment.baryCoord.v * t1 + fragment.baryCoord.w * t2; // 片元纹理坐标
        // 纹理坐标映射到图像
        int x = fragment_textcoord.x * texture_map.width, y = fragment_textcoord.y * texture_map.height;
        Image::Pixel pixel(x, y);
        Image::ColorAlpha texture_color = texture_map.getColor(pixel);
        color_vec = glm::vec3(texture_color.first.R, texture_color.first.G, texture_color.first.B);
    }
    color_vec = color_vec * PhongCoeff;
    Image::Color color(color_vec.x, color_vec.y, color_vec.z);
    return std::make_pair(false, color);
}

void PhongShader::updateTriangle(const std::array<V2F, 3>& Triangle)
{
    this->currentTriangle = Triangle;
    if (!smooth) // 逐三角形着色
    {
        auto v0 = Triangle[0], v1 = Triangle[1], v2 = Triangle[2];
        // 片元的位置 (使用三角形中心)
        glm::vec4 fragment_position = (v0.eyePosition + v1.eyePosition + v2.eyePosition) / 3.0f; 
        // 片元的法向量 (平均法向量)
        glm::vec4 fragment_normal = v0.normal + v1.normal + v2.normal;
        // ! 将最终颜色存入成员变量
        this->precomputedFlatCoeff = _fragmentShader(fragment_position, fragment_normal);
    }
}

float PhongShader::_fragmentShader(const glm::vec4& fragment_position, const glm::vec4& fragment_normal) // 着色器主代码
{
    // TODO : 1. 漫反射颜色
    // 片元的法向量 (平均法向量)
    glm::vec4 n = glm::normalize(fragment_normal); 
    
    // 光源方向
    glm::vec4 l = config.light_position - fragment_position; 
    float rr = glm::dot(l, l);
    l = glm::normalize(l); 
    float inv_light_distance_sq = config.I / rr;
    float ndotl = std::max(glm::dot(n, l), 0.0f);
    float Ld = config.kd * inv_light_distance_sq * ndotl;

    // TODO : 2. 镜面反射颜色
    glm::vec4 v = glm::normalize(config.camera_position - fragment_position); 
    glm::vec4 h = glm::normalize(l + v); 
    float ndoth = std::max(glm::dot(n, h), 0.0f);
    float Ls = config.ks * inv_light_distance_sq * std::pow(ndoth, config.p);

    // TODO : 3. 环境光颜色
    float La = config.ka * config.Ia;

    // * 片元的最终颜色
    float L = La + Ld + Ls;
    
    return std::min(1.0f, L);
}

glm::mat4 PhongShader::getTBN(const Fragment& fragment)
{
    glm::vec3 e0 = glm::vec3(currentTriangle[1].eyePosition - currentTriangle[0].eyePosition);
    glm::vec3 e1 = glm::vec3(currentTriangle[2].eyePosition - currentTriangle[0].eyePosition);
    glm::vec2 u0 = currentTriangle[1].texcoord - currentTriangle[0].texcoord;
    glm::vec2 u1 = currentTriangle[2].texcoord - currentTriangle[0].texcoord;
    
    float inv_det = 1.0f / (u0.x * u1.y - u0.y * u1.x);
    
    glm::vec3 t = inv_det * (u1.y * e0 - u0.y * e1);
    glm::vec3 b = inv_det * (-u1.x * e0 + u0.x * e1);
    
    t = glm::normalize(t);
    b = glm::normalize(b);
    
    auto n0 = glm::vec3(currentTriangle[0].normal);
    auto n1 = glm::vec3(currentTriangle[1].normal);
    auto n2 = glm::vec3(currentTriangle[2].normal);
    auto n = fragment.baryCoord.u * n0 + fragment.baryCoord.v * n1 + fragment.baryCoord.w * n2;
    n = glm::normalize(n);
    
    glm::mat3 TBN(t, b, n);
    return glm::mat4(TBN); // 或者直接返回mat3
}
