#pragma once
#include "engine.h"
#include "timer.h"

// * 主函数中使用的一些工具函数

/*
* ------------------------------------------
* 解析命令行参数
* ------------------------------------------
* /inType: argc 参数个数, 
* /inType: char** argv 参数数组
* /outType: ARG 结构体
* ------------------------------------------
*/
inline ARG parse_args(int argc, char** argv)
{
    ARG args;
    
    // 解析命令行参数
    if (argc > 1)
    {
        for (size_t i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0)
                args.input_obj_json = argv[++i];
            else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
                args.output_img_file = argv[++i];
            else if(strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0)
                args.width = atoi(argv[++i]);
            else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--height") == 0)
                args.height = atoi(argv[++i]);
        }
    }

    return args;
}

/*
* ------------------------------------------
* 保存渲染图像
* ------------------------------------------
*/
inline void saveImage(Image& image, const char* filePath, bool saveDepth = true)
{
    Timer timer("Save Image");
    if (saveDepth)
    {
        // 保存深度图，保存在同一文件路径下
        Image depth_image(image.width, image.height, Image::Channel::GRAY);
        Engine::getInstance()->getDepthImage(depth_image);
        depth_image.flipVertical();
        std::string depth_path = filePath;
        size_t last_slash = depth_path.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            depth_path = depth_path.substr(0, last_slash + 1) + "depth.png";
        } else {
            depth_path = "depth.png";
        }
        depth_image.save(depth_path.c_str());
    }
    image.save(filePath);
    image.init(); // 保存图像后内存被删除了，需要重新初始化
}

/*
* ------------------------------------------
* 加载多个模型及对应的纹理、法向贴图
* ------------------------------------------
*/
inline void loadAsset(const char* input_json, std::vector<std::tuple<Model, Image, Image>>& loadmodels)
{
    Timer timer("Load Asset");
    // 使用 json 文件多个模型
    using json = nlohmann::json;
    json data = json::parse(std::ifstream(input_json));
    for (auto& model: data["model"])
    {
        bool load = model.value("load", true);
        if (load)
        {
            Model objmodel;
            Image texture, normal;
            const std::string name = model.value("name", "");
            const std::string obj_file = model.value("obj", "");
            const std::string texture_file = model.value("texture", "");
            const std::string normal_file = model.value("normal", "");
            objmodel.loadFrom(obj_file.c_str());
            if (!texture_file.empty())
            {
                texture.load(texture_file.c_str());
                texture.flipVertical();
            }
            if (!normal_file.empty())
            {
                normal.load(normal_file.c_str());
                normal.flipVertical();
            }
            loadmodels.emplace_back(objmodel, texture, normal);
        }
    }
}

/*
* ------------------------------------------
* 渲染载入的所有模型
* ------------------------------------------
*/
inline void renderAll(Image& image, Shader* shader, std::vector<std::tuple<Model, Image, Image>>& loadmodels, bool drawWireframe)
{
    for (auto& [model, texture_map, normal_map]: loadmodels)
    {
        if (drawWireframe)
        {
            Engine::getInstance()->wireframe(image, model, Image::Color::RED);
        }
        else
        {
            if (texture_map.image_buffer) shader->texture_map = texture_map;
            if (normal_map.image_buffer) shader->normal_map = normal_map;
            Engine::getInstance()->render(image, model, shader);
        }
    }
}