#include "engine.h"
#include "timer.h"

/*
* ------------------------------------------
* 解析命令行参数
* ------------------------------------------
* /inType: argc 参数个数, 
* /inType: char** argv 参数数组
* /outType: ARG 结构体
* ------------------------------------------
*/
static ARG parse_args(int argc, char** argv)
{
    ARG args;
    
    // 解析命令行参数
    if (argc > 1)
    {
        for (size_t i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--model") == 0)
                args.input_obj_file = argv[++i];
            else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
                args.output_img_file = argv[++i];
            else if(strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0)
                args.width = atoi(argv[++i]);
            else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--height") == 0)
                args.height = atoi(argv[++i]);
            else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--channel") == 0)
                args.channel = static_cast<Image::Channel>(atoi(argv[++i]));
        }
    }

    return args;
}
#include "glm_test.h"
/*
* ------------------------------------------
* 主函数入口
* ------------------------------------------
*/
int main(int argc, char** argv)
{
    ARG args = parse_args(argc, argv);
    args.log(); // 打印命令行参数

    Image image(args.width, args.height, args.channel); // 创建图像对象
    
    Model model;
    {
        Timer timer("Load Model"); // 计时器开始
        model.loadFrom(args.input_obj_file); // 加载模型文件
    }

    // TODO : 在此处添加绘制代码
    {
        Timer timer("Render"); // 计时器开始
        Engine::getInstance()->render(image, model); // 渲染 3D 模型到图像
    }
    
    {
        Timer timer("Save Image");
        Image depth_image = Image(args.width, args.height, Image::Channel::GRAY);
        Engine::getInstance()->getDepthImage(depth_image);
        depth_image.flipVertical();
        std::string depth_path = args.output_img_file;
        size_t last_slash = depth_path.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            depth_path = depth_path.substr(0, last_slash + 1) + "depth.png";
        } else {
            depth_path = "depth.png"; // 无目录时使用当前目录
        }
        depth_image.save(depth_path.c_str()); // 保存深度图像
        image.flipVertical(); // 垂直翻转图像, 让 y 轴向上为正方向
        image.save(args.output_img_file); // 保存图像
    }
}