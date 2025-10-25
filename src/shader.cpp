#include "shader.h"

V2F lerp(const V2F& v1, const V2F& v2, float weight)
{
    V2F v;
    v.eyePosition = glm::lerp(v1.eyePosition, v2.eyePosition, weight);
    v.clipPosition = glm::lerp(v1.clipPosition, v2.clipPosition, weight);
    v.color = glm::lerp(v1.color, v2.color, weight);
    // TODO 其他属性插值
    return v;
}

V2F RandomShader::vertexShader(const Model::attrib_t& rawVertex) // 顶点着色器
{
    glm::vec4 eyePosition = viewMatrix * modelMatrix * glm::vec4(rawVertex._position.x, rawVertex._position.y, rawVertex._position.z, 1.0f);
    glm::vec4 clipPosition = projectionMatrix * eyePosition;
    Image::Color randcolor = Image::Color::randColor();
    glm::vec3 color(randcolor.R, randcolor.G, randcolor.B);
    return { eyePosition, clipPosition, color };
}

std::pair<bool, Image::Color> RandomShader::fragmentShader(const Fragment& fragment)// 片元着色器
{
    Image::Color color(currentTriangle[0].color.x, currentTriangle[0].color.y, currentTriangle[0].color.z);
    return std::make_pair(false, color);
}