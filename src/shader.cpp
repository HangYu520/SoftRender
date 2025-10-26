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
    // TODO : 1. 漫反射颜色
    glm::vec4 fragment_position = (currentTriangle[0].eyePosition + currentTriangle[1].eyePosition + currentTriangle[2].eyePosition) / 3.0f; // 片元的位置
    glm::vec4 n = (currentTriangle[0].normal + currentTriangle[1].normal + currentTriangle[2].normal) / 3.0f; // 片元的法向量
    glm::vec4 l = config.light_position - fragment_position; // 光源方向
    float rr = glm::dot(l, l);
    l = glm::normalize(l); 
    float inv_light_distance_sq = config.I / rr;
    float ndotl = std::max(glm::dot(n, l), 0.0f);// 漫反射
    float Ld = config.kd * inv_light_distance_sq * ndotl;

    // TODO : 2. 镜面反射颜色
    glm::vec4 v = glm::normalize(config.camera_position - fragment_position); // 视线方向
    glm::vec4 h = glm::normalize(l + v); // 半向量
    float ndoth = std::max(glm::dot(n, h), 0.0f);
    float Ls = config.ks * inv_light_distance_sq * std::pow(ndoth, config.p);

    // TODO : 3. 环境光颜色
    float La = config.ka * config.Ia;

    // * 片元的最终颜色
    float L = La + Ld + Ls;
    glm::vec3 fragment_color = std::min(1.0f, L) * currentTriangle[0].color; // 片元颜色
    Image::Color color(fragment_color.x, fragment_color.y, fragment_color.z);
    return std::make_pair(false, color);
}