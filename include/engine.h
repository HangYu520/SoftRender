#pragma once
#include "shader.h"

/*
* ------------------------------------------
* 软渲染绘制算法引擎，单例类(禁止复制，构造)
* ------------------------------------------
*/
class Engine
{
public:
    Engine(const Engine&) = delete; // 禁止拷贝构造
    Engine& operator=(const Engine&) = delete; // 禁止赋值操作

    /*
    * ------------------------------------------
    * 获取引擎实例(单例模式)
    * ------------------------------------------
    * /outType: unique_ptr<Engine>& 唯一的实例指针
    * ------------------------------------------
    */
    static std::unique_ptr<Engine>& getInstance() // 获取引擎实例
    {
        static auto instance = createInstance();
        return instance;
    }

    /*
    * ------------------------------------------
    * 画直线算法 (Bresenham 直线算法)
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: const Image::Pixel& start 起点坐标
    * /inType: const Image::Pixel& end 终点坐标
    * /inType: const Image::Color& color 颜色
    * ------------------------------------------
    */
    void line(Image& image, const Image::Pixel& start, const Image::Pixel& end, const Image::Color& color); // 画起点和终点之间的直线

    /*
    * ------------------------------------------
    * 画 3D 模型的线框
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: Model& model 3D 模型对象
    * /inType: const Image::Color& color 颜色
    * ------------------------------------------
    */
    void wireframe(Image& image, Model& model, const Image::Color& color); // 画 3D 模型的线框

    /*
    * ------------------------------------------
    * 计算有向三角形面积
    * ------------------------------------------
    * /inType: const Image::Pixel& p1 顶点1坐标
    * /inType: const Image::Pixel& p2 顶点2坐标
    * /inType: const Image::Pixel& p3 顶点3坐标
    * /outType: float 有向面积
    * ------------------------------------------
    */
    float signedTriangleArea(const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3);

    /*
    * ------------------------------------------
    * 计算重心坐标
    * ------------------------------------------
    * /inType: const Image::Pixel& p1 顶点1坐标
    * /inType: const Image::Pixel& p2 顶点2坐标
    * /inType: const Image::Pixel& p3 顶点3坐标
    * /inType: const Image::Pixel& p 待计算点坐标
    * /outType: BarycentricCoord 重心坐标
    * ------------------------------------------
    */
    BarycentricCoord getBarycentricCoord(const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3, const Image::Pixel& p);

    /*
    * ------------------------------------------
    * 光栅化画三角形到图像
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: Shader* Shader 着色器指针
    * ------------------------------------------
    */
    void rasterize(Image& image, Shader* shader); // 画指定顶点属性的三角形

    /*
    * ------------------------------------------
    * 渲染 3D 模型到图像
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: Model& model 3D 模型对象
    * /inType: Shader* shader 着色器指针
    * ------------------------------------------
    */
    void render(Image& image, Model& model, Shader* shader); // 渲染 3D 模型到图像

    /*
    * ------------------------------------------
    * 平移模型到中心
    * ------------------------------------------
    * /inType: Model& model 3D 模型对象
    * ------------------------------------------
    */
    void centerModel(Model& model); // 平移模型到中心

    /*
    * ------------------------------------------
    * 获取模型矩阵
    * ------------------------------------------
    * /outType: glm::mat4 模型矩阵
    * ------------------------------------------
    */
    glm::mat4 getModelMatrix(); // 获取模型矩阵

    /*
    * ------------------------------------------
    * 获取视图矩阵
    * ------------------------------------------
    * /outType: glm::mat4 视图矩阵
    * ------------------------------------------
    */
    glm::mat4 getViewMatrix(); // 获取视图矩阵

    /*
    * ------------------------------------------
    * 获取投影矩阵
    * ------------------------------------------
    * /outType: glm::mat4 投影矩阵
    * ------------------------------------------
    */
    glm::mat4 getProjectMatrix(); // 获取投影矩阵
    
    /*
    * ------------------------------------------
    * 获取深度图
    * ------------------------------------------
    */
    void getDepthImage(Image& image); // 获取深度图

    /*
    * ------------------------------------------
    * 初始化深度缓冲区
    * ------------------------------------------
    * /inType: int zBufferSize 深度缓冲区大小
    * /inType: float initZValue 深度缓冲区初始值
    * ------------------------------------------
    */
    void initZBuffer(int zBufferSize, float initZValue)
    {
        m_zBufferSize = zBufferSize;
        m_zBuffer = new float[m_zBufferSize]; // 初始化深度缓冲区
        std::fill(m_zBuffer, m_zBuffer + m_zBufferSize, initZValue);
    }
    
    /*
    * ------------------------------------------
    * 手动释放深度缓冲区
    * ------------------------------------------
    */
    void freeZBuffer()
    {
        if (m_zBuffer)
        {
            delete[] m_zBuffer;
            m_zBuffer = nullptr;
        }
        m_zBufferSize = 0;
    }

    TransConfig& getTrans() // 获取矩阵变换设置
    {
        return m_transConfig;
    }

private:
    Engine() {}; // 私有构造函数，防止实例化
    static std::unique_ptr<Engine> createInstance() // 创建引擎实例
    {
        return std::unique_ptr<Engine>(new Engine());
    }
    // 私有变量
    float* m_zBuffer = nullptr; // 深度缓冲区
    uint32_t m_zBufferSize = 0; // 深度缓冲区大小
    TransConfig m_transConfig; // 矩阵变换设置

    /*
    * ------------------------------------------
    * 对三角形进行视口变换到屏幕坐标
    * ------------------------------------------
    * //inType: const Image& image 屏幕图像
    * /inType: const std::array<glm::vec3, 3>& ndc 三角形 ndc 坐标
    * /outType: std::array<Image::Pixel, 3> 三角形屏幕坐标
    * ------------------------------------------
    */
    std::array<Image::Pixel, 3> viewport(const Image& image, const std::array<glm::vec3, 3>& ndc);

    /*
    * ------------------------------------------
    * 判断点是否在平面内侧
    * ------------------------------------------
    * /inType: const glm::vec4& plane 平面方程
    * /inType: const glm::vec4& pos 待判断点
    * /outType: bool 点是否在平面内侧
    * ------------------------------------------
    */
    bool isInside(const glm::vec4& plane, const glm::vec4& pos);

    /*
    * ------------------------------------------
    * 判断一组顶点是否全部在平面内侧 (无需裁剪)
    * ------------------------------------------
    * /inType: const std::vector<V2F>& vertices 一组顶点列表
    * /outType: bool 顶点是否全部在平面内侧
    * ------------------------------------------
    */
    bool allInside(const std::vector<V2F>& vertices);

    /*
    * ------------------------------------------
    * 获取平面与裁剪线段的相交点
    * ------------------------------------------
    * /inType: const glm::vec4& plane 平面方程
    * /inType: const V2F& vertex0 裁剪线段的起点
    * /inType: const V2F& vertex1 裁剪线段的终点
    * /outType: V2F 裁剪线段与平面的交点
    * ------------------------------------------
    */
    V2F interSect(const glm::vec4& plane, const V2F& vertex0, const V2F& vertex1);

    /*
    * ------------------------------------------
    * 剪裁三角形 (Sutherland-Hodgeman裁剪算法)
    * ------------------------------------------
    * /inType: const V2F& vertex0 顶点0
    * /inType: const V2F& vertex1 顶点1
    * /inType: const V2F& vertex2 顶点2
    * /outType: std::vector<V2F> 剪裁后的顶点
    * ------------------------------------------
    */
    std::vector<V2F> clipTriangle(const V2F& vertex0, const V2F& vertex1, const V2F& vertex2);

    /*
    * ------------------------------------------
    * 获取三角形的NDC坐标
    * ------------------------------------------
    * /inType: const std::array<V2F, 3>& Triangle 三角形顶点列表
    * /outType: std::array<glm::vec3, 3> 三角形的NDC坐标
    * ------------------------------------------
    */
    std::array<glm::vec3, 3> NDC(const std::array<V2F, 3>& Triangle);

    /*
    * ------------------------------------------
    * 画三角形 (边界框算法/扫描线算法， 无属性插值)
    * ------------------------------------------
    * /inType: Image& image 图像对象
    * /inType: const Image::Pixel& p1 顶点1坐标
    * /inType: const Image::Pixel& p2 顶点2坐标
    * /inType: const Image::Pixel& p3 顶点3坐标
    * /inType: const Image::Color& color 颜色
    * ------------------------------------------
    */
    void triangle(Image& image, const Image::Pixel& p1, const Image::Pixel& p2, const Image::Pixel& p3, const Image::Color& color); // 画三角形
};