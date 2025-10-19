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
        Timer timer("Render Wireframe"); // 计时器开始
        Engine::getInstance()->wireframe(image, model, Image::WHITE); // 绘制模型线框
    }
    
    image.save(args.output_img_file); // 保存图像
}