#include "shader.h"

V2F lerp(const V2F& v1, const V2F& v2, float weight)
{
    V2F v;
    v.eyePosition = glm::lerp(v1.eyePosition, v2.eyePosition, weight);
    v.clipPosition = glm::lerp(v1.clipPosition, v2.clipPosition, weight);
    v.color = glm::lerp(v1.color, v2.color, weight);
    v.normal = glm::lerp(v1.normal, v2.normal, weight);
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
    v.normal = viewMatrix * modelMatrix * glm::vec4(rawVertex._normal.nx, rawVertex._normal.ny, rawVertex._normal.nz, 0.0f);
    v.normal = glm::normalize(v.normal);
    return v;
}

std::pair<bool, Image::Color> PhongShader::fragmentShader(const Fragment& fragment)// 片元着色器
{
    Image::Color color = this->precomputedFlatColor;
    if (smooth) // 逐片元着色
    {   // ! CPU 上运行速度较慢
        auto v0 = currentTriangle[0].eyePosition, v1 = currentTriangle[1].eyePosition, v2 = currentTriangle[2].eyePosition;
        auto n0 = currentTriangle[0].normal, n1 = currentTriangle[1].normal, n2 = currentTriangle[2].normal;
        // 片元位置 （重心坐标插值）
        glm::vec4 fragment_position = fragment.baryCoord.u * v0 + fragment.baryCoord.v * v1 + fragment.baryCoord.w * v2;
        // 片元的法向量 (重心坐标插值)
        glm::vec4 fragment_normal = fragment.baryCoord.u * n0 + fragment.baryCoord.v * n1 + fragment.baryCoord.w * n2;
        color = _fragmentShader(fragment_position, fragment_normal);
    }
    return std::make_pair(false, color); // 片元着色器移动到 updateTriangle 中
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
        this->precomputedFlatColor = _fragmentShader(fragment_position, fragment_normal);
    }
}

Image::Color PhongShader::_fragmentShader(glm::vec4& fragment_position, glm::vec4& fragment_normal) // 着色器主代码
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
    glm::vec3 fragment_color = std::min(1.0f, L) * currentTriangle[0].color; 
    
    return Image::Color(fragment_color.x, fragment_color.y, fragment_color.z);
}