#include "typedef.h"

/*
* -----------------------------
* 1. 图像结构体 (封装 stb_image)
* -----------------------------
*/
Image::Color Image::Color::randColor() // 生成随机颜色
{
    static thread_local std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dis(0, 255);
    return {
        static_cast<unsigned char>(dis(gen)),
        static_cast<unsigned char>(dis(gen)),
        static_cast<unsigned char>(dis(gen))};
};

Image::Image(uint32_t w, uint32_t h, Channel c)
        : width(w), height(h), channel(c)
{
    image_buffer = new stbi_uc[w * h * c](); // 分配图像内存并初始化为0
}

void Image::setColor(const Pixel& pixel, const Color& color)
{
    // 设置指定像素的颜色
    stbi_uc* pixel_buffer = image_buffer + (pixel.y * width + pixel.x) * channel;
    pixel_buffer[0] = color.R;
    pixel_buffer[1] = color.G;
    pixel_buffer[2] = color.B;
    pixel_buffer[3] = 255; // 透明度
}

void Image::flipVertical() {
    const size_t bytesPerRow = width * static_cast<size_t>(channel);
    stbi_uc* tempRow = new stbi_uc[bytesPerRow];
    
    for (uint32_t i = 0; i < height / 2; ++i) {
        const size_t topOffset = i * bytesPerRow;
        const size_t bottomOffset = (height - 1 - i) * bytesPerRow;
        
        // 交换上下行数据
        memcpy(tempRow, image_buffer + topOffset, bytesPerRow);
        memcpy(image_buffer + topOffset, image_buffer + bottomOffset, bytesPerRow);
        memcpy(image_buffer + bottomOffset, tempRow, bytesPerRow);
    }
    
    delete[] tempRow;
}

void Image::save(const char* filename)
{
    bool is_saved = stbi_write_png(filename, width, height, channel, image_buffer, width * channel); // 写入图像文件
    image_buffer = nullptr; // stbi_write_png() 会自动释放图像数据
    if (is_saved)
        spdlog::info("Image saved to {}", filename);
    else
        spdlog::error("Failed to save image to {}", filename);
}

// 初始化预定义静态颜色常量
const Image::Color Image::BLACK    =   {0, 0, 0};
const Image::Color Image::RED      =   {255, 0, 0};
const Image::Color Image::GREEN    =   {0, 255, 0};
const Image::Color Image::BLUE     =   {0, 0, 255};
const Image::Color Image::WHITE    =   {255, 255, 255};
const Image::Color Image::YELLOW   =   {255, 255, 0};

/*
* ---------------------------------
* 2. 3D 模型类 (封装 tinyobjloader)
* ---------------------------------
*/

const Model::attrib_t Model::point_t::Get(const Attrib& attrib) const // 获取 attrib 中对应的顶点属性 (常量版本)
{
    attrib_t point;
    if (!attrib._vertices.empty())
        point._position = attrib._vertices[vertex_index];
    if (!attrib._normals.empty())
        point._normal = attrib._normals[normal_index];
    if (!attrib._texcoords.empty())
        point._texcoord = attrib._texcoords[texcoord_index];
    
    return point;
}

const Model::line_attrib_t Model::line_t::Get(const Attrib& attrib) const // 获取 attrib 中对应的线段顶点属性 (常量版本)
{
    Model::attrib_t p0 = start.Get(attrib);
    Model::attrib_t p1 = end.Get(attrib);
    return std::make_tuple(p0, p1);
}

const Model::triface_attrib_t Model::triface_t::Get(const Attrib& attrib) const // 获取 attrib 中对应的三角形面顶点属性
{
    Model::attrib_t p0 = vertex[0].Get(attrib);
    Model::attrib_t p1 = vertex[1].Get(attrib);
    Model::attrib_t p2 = vertex[2].Get(attrib);
    return std::make_tuple(p0, p1, p2);
}

void Model::setLoadConfig(bool triangulate, bool vertex_color, std::string triangulation_method, std::string mtl_search_path)
{
    readerConfig.triangulate            =   triangulate;
    readerConfig.vertex_color           =   vertex_color;
    readerConfig.triangulation_method   =   triangulation_method;
    readerConfig.mtl_search_path        =   mtl_search_path;
}

void Model::loadFrom(const tinyobj::attrib_t& tinyattrib) // 从 tinyobjloader 加载属性
{
    for (size_t i = 0; i < tinyattrib.vertices.size(); i += 3)
    {
        attrib._vertices.push_back({
            tinyattrib.vertices[i + 0], // x
            tinyattrib.vertices[i + 1], // y
            tinyattrib.vertices[i + 2] // z
        });
    }
    for (size_t i = 0; i < tinyattrib.normals.size(); i += 3)
    {
        attrib._normals.push_back({
            tinyattrib.normals[i + 0], // nx
            tinyattrib.normals[i + 1], // ny
            tinyattrib.normals[i + 2] // nz
        });
    }
    for (size_t i = 0; i < tinyattrib.texcoords.size(); i += 2)
    {
        attrib._texcoords.push_back({
            tinyattrib.texcoords[i + 0], // u
            tinyattrib.texcoords[i + 1] // v
        });
    }
}

void Model::loadFrom(const std::vector<tinyobj::shape_t>& tinyshapes) // 从 tinyobjloader 加载形状列表
{
    for (size_t s = 0; s < tinyshapes.size(); s++) // 遍历每个形状
    {
        size_t index_offset = 0;
        TriMeshes trimesh; // 当前形状的三角网格
        for (size_t f = 0; f < tinyshapes[s].mesh.num_face_vertices.size(); f++) // 遍历每个面
        { 
            size_t fv = 3; // 三角面的顶点数
            triface_t face;
            for (size_t v = 0; v < fv; v++) // 遍历每个顶点
            {
                tinyobj::index_t idx = tinyshapes[s].mesh.indices[index_offset + v];
                face.vertex[v] = {idx.vertex_index, idx.normal_index, idx.texcoord_index};
            }
            trimesh.trifaces.push_back(face);
            trimesh.material_ids.push_back(tinyshapes[s].mesh.material_ids[f]);
            index_offset += fv;
        }

        index_offset = 0;
        Lines lines; // 当前形状的线段列表
        for (size_t l = 0; l < tinyshapes[s].lines.num_line_vertices.size(); l++) // 遍历每个线段
        {
            line_t line;
            tinyobj::index_t idx0 = tinyshapes[s].lines.indices[index_offset + 0];
            tinyobj::index_t idx1 = tinyshapes[s].lines.indices[index_offset + 1];
            line.start = { idx0.vertex_index, idx0.normal_index, idx0.texcoord_index };
            line.end = { idx1.vertex_index, idx1.normal_index, idx1.texcoord_index };
            lines.push_back(line);
            index_offset += 2;
        }

        Points points; // 当前形状的点列表
        for (size_t p = 0; p < tinyshapes[s].points.indices.size(); p++)
        {
            point_t point;
            tinyobj::index_t idx = tinyshapes[s].points.indices[p];
            point = { idx.vertex_index, idx.normal_index, idx.texcoord_index };
            points.push_back(point);
        }
        shapes.push_back({ tinyshapes[s].name, trimesh, lines, points }); // 添加形状到模型中
    }
}

void Model::loadFrom(const tinyobj::attrib_t& tinyattrib, const std::vector<tinyobj::shape_t>& tinyshapes) // 从 tinyobjloader 属性加载自定义数据结构
{
    // 加载 tinyattrib 顶点数据到自定义顶点列表
    loadFrom(tinyattrib);
    
    // 加载 tinyshapes 到自定义形状列表
    loadFrom(tinyshapes);
}

void Model::loadFrom(const std::string& filename) // 从文件加载模型
{
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filename, readerConfig)) 
    {
        // 加载失败，打印错误信息
        if (!reader.Error().empty())
            spdlog::error("TinyObjReader: {}", reader.Error());
        
        exit(1); // 退出程序
    }

    if (!reader.Warning().empty()) 
    {
        // 打印警告信息
        spdlog::warn("TinyObjReader: {}", reader.Warning());
    }

    auto& tinyattrib = reader.GetAttrib();
    auto& tinyshapes = reader.GetShapes();
    loadFrom(tinyattrib, tinyshapes); // 加载自定义数据结构
    
    materials = reader.GetMaterials(); // 获取材质列表

    // 打印加载信息
    spdlog::info(
        "Loaded model from {}: "
        "{} vertices, {} shapes, {} materials",
        filename,
        attrib._vertices.size(),
        shapes.size(),
        materials.size()
    );

    logShapes(); // 打印模型形状信息
}

Model::bounding_box_t Model::getBoundingBox() const // 获取模型的边界框
{
    auto& vertices = attrib._vertices;
    if (vertices.empty())
    {
        spdlog::error("Model::getBoundingBox(): no vertices");
        exit(1);
    }
    
    // 初始化最小值和最大值
    float min_x = std::numeric_limits<float>::max();
    float min_y = std::numeric_limits<float>::max();
    float min_z = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float max_y = std::numeric_limits<float>::lowest();
    float max_z = std::numeric_limits<float>::lowest();
    
    // 遍历所有顶点
    for (const auto& vertex : vertices)
    {
        // 更新x的最小最大值
        if (vertex.x < min_x) min_x = vertex.x;
        if (vertex.x > max_x) max_x = vertex.x;
        
        // 更新y的最小最大值
        if (vertex.y < min_y) min_y = vertex.y;
        if (vertex.y > max_y) max_y = vertex.y;
        
        // 更新z的最小最大值
        if (vertex.z < min_z) min_z = vertex.z;
        if (vertex.z > max_z) max_z = vertex.z;
    }

    return std::make_tuple(
        attrib_t::position{min_x, min_y, min_z}, // 最小点
        attrib_t::position{max_x, max_y, max_z}  // 最大点
    );
}

void Model::translate(float x_offset, float y_offset, float z_offset) // 平移模型
{
    auto& vertices = attrib._vertices;
    for (auto& vertex : vertices)
    {
        vertex.x += x_offset;
        vertex.y += y_offset;
        vertex.z += z_offset;
    }
}

void Model::resize(float x_scale, float y_scale, float z_scale) // 按比例缩放模型
{
    auto& vertices = attrib._vertices;
    for (auto& vertex : vertices)
    {
        vertex.x *= x_scale;
        vertex.y *= y_scale;
        vertex.z *= z_scale;
    }
}

std::vector<Model::Trifaces>  Model::getTrifaces()
{
    std::vector<Trifaces> trifaces_list;
    for (const auto& shape : shapes)
    {
        if (!shape.trimeshes.trifaces.empty())
            trifaces_list.push_back(shape.trimeshes.trifaces);
    }
    return trifaces_list;
}

void Model::saveOBJ(const char* filename, const std::vector<attrib_t::position>& vertices, const Trifaces& trifaces)
{
    if (vertices.empty())
    {
        spdlog::error("Model::saveOBJ(): no vertices");
        exit(1);
    }
    std::ofstream obj_file(filename, std::ios::out);
    if (!obj_file.is_open())
    {
        spdlog::error("Model::saveOBJ(): failed to open file {}", filename);
        exit(1);
    }
    for (size_t i = 0; i < vertices.size(); i++)
    {
        obj_file << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << std::endl;
    }
    for (auto &face : trifaces)
    {
        obj_file << "f " << (face.vertex[0].vertex_index + 1) << " " 
             << (face.vertex[1].vertex_index + 1) << " " 
             << (face.vertex[2].vertex_index + 1) << std::endl;
    }
    obj_file.close();
}

void Model::logShapes() const // 打印模型形状信息
{
    for (size_t i = 0; i < shapes.size(); i++)
    {
        const auto& shape = shapes[i];
        spdlog::info(
            "Shape [{}]: name = {}, "
            "num_faces = {}, num_lines = {}, num_points = {}",
            i+1, shape.name,
            shape.trimeshes.trifaces.size(),
            shape.lines.size(),
            shape.points.size()
        );
    }
}

/*
* -------------------
* 3. 命令行参数结构体
* -------------------
*/
void ARG::log() const // 打印命令行参数
{
    spdlog::info(
        "terminal args : "
        "input_obj_file = {}, "
        "output_img_file = {}, "
        "width = {}, height = {}, channel = {}",
        input_obj_file,
        output_img_file, 
        width, height, 
        (int) channel
    ); // 打印命令行参数
}