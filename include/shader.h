#pragma once
#include "typedef.h"

/*
* -----------------------------
* 着色器头文件 (包含各种 shader )
* -----------------------------
*/

struct V2F // 顶点着色器输出 (封装的顶点属性)
{
    glm::vec4 eyePosition; // 相机空间的位置 (经过 ModelView 变换)
    glm::vec4 clipPosition; // 裁剪空间的位置 (经过透视变换)
    glm::vec3 color = glm::vec3(255.f, 255.f, 255.f); // 顶点颜色
    glm::vec4 normal = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // 法向量
    // glm::vec2 texcoord; // 纹理坐标
    // TODO 更多的属性
};

V2F lerp(const V2F& v1, const V2F& v2, float weight); // 线性插值两个顶点

struct Fragment // 片元着色器输入 (封装的片元属性)
{
    Image::Pixel pixel; // 片元像素位置
    float zBuffer; // 片元深度
    BarycentricCoord baryCoord; // 片元的重心坐标
};

/*
* -----------------------------
* 着色器抽象类 (所有 shader 都需继承该类)
* -----------------------------
*/
class Shader
{
public:
    /*
    * -----------------------------
    * 顶点着色器接口
    * -----------------------------
    * /inType const Model::attrib_t& rawVertex : 原始顶点数据
    * /outType V2F : 顶点着色器输出
    * -----------------------------
    */
    virtual V2F vertexShader(const Model::attrib_t& rawVertex) = 0; // 顶点着色器
    /*
    * -----------------------------
    * 片元着色器接口
    * -----------------------------
    * /inType const Fragment& fragment : 片元着色器输入
    * /outType std::pair<bool, Image::Color> : 片元着色器输出 (是否丢弃, 颜色)
    * -----------------------------
    */
    virtual std::pair<bool, Image::Color> fragmentShader(const Fragment& fragment) = 0; // 片元着色器
    
    /*
    * -----------------------------
    * 更新着色三角形
    * -----------------------------
    * /inType const std::array<V2F, 3>& Triangle : 要着色的三角形
    * -----------------------------
    */
    virtual void updateTriangle(const std::array<V2F, 3>& Triangle) // 更新着色的三角形
    {
        this->currentTriangle = Triangle;
    }
    // * 返回着色三角形
    virtual std::array<V2F, 3>& getTriangle()
    {
        return this->currentTriangle;
    }
    /*
    * -----------------------------
    * 设置 MVP 矩阵
    * -----------------------------
    * /inType const glm::mat4& modelMatrix : 模型矩阵
    * /inType const glm::mat4& viewMatrix : 视图矩阵
    * /inType const glm::mat4& projectionMatrix : 投影矩阵
    * -----------------------------
    */
    virtual void setMVP(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) // 设置 MVP
    {
        this->modelMatrix = modelMatrix;
        this->viewMatrix = viewMatrix;
        this->projectionMatrix = projectionMatrix;
    }
    virtual ~Shader() = default;

protected:
    std::array<V2F, 3>  currentTriangle; // 要着色的三角形
    glm::mat4           modelMatrix; // 模型矩阵
    glm::mat4           viewMatrix; // 视图矩阵
    glm::mat4           projectionMatrix; // 投影矩阵
};

/*
* -----------------------------
* 随机色填充 shader
* -----------------------------
*/
class RandomShader : public Shader
{ 
public:
    RandomShader() = default;
    V2F vertexShader(const Model::attrib_t& rawVertex) override; // 顶点着色器
    std::pair<bool, Image::Color> fragmentShader(const Fragment& fragment) override;// 片元着色器
};

/*
* -----------------------------
* Blinn-Phong shader
* -----------------------------
*/
class PhongShader : public Shader
{
public:
    struct Config // 参数配置
    {
        float       I = 3.0f; // 光照强度
        float       Ia = 0.5f; // 环境光强度
        int         p = 64; // 镜面反射指数
        float       kd = 0.65f; // 漫反射系数
        float       ks = 0.35f; // 镜面反射系数
        float       ka = 0.15f; // 环境反射系数
        glm::vec4   light_position = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        glm::vec4   camera_position = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    } config;

public:
    PhongShader() = default;
    V2F vertexShader(const Model::attrib_t& rawVertex) override; // 顶点着色器
    std::pair<bool, Image::Color> fragmentShader(const Fragment& fragment) override;// 片元着色器
    void updateTriangle(const std::array<V2F, 3>& Triangle) override; // CPU 优化方案, 同一个三角形所有的片元画同一个颜色

private:
    Image::Color precomputedFlatColor; // 为同一个三角形的片元缓存 Color
};