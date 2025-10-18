#include "engine.h"
#include "timer.h"
#include <cstdlib>
#include <ctime>

static ARG parse_args(int argc, char** argv) // * 解析命令行参数
{
    ARG args;
    
    // 解析命令行参数
    if (argc > 1)
    {
        for (size_t i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
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

int main(int argc, char** argv) // * 主函数
{
    ARG args = parse_args(argc, argv);
    spdlog::info(
        "terminal args : output_img_file = {}, width = {}, height = {}, channel = {}", 
        args.output_img_file, args.width, args.height, (int) args.channel
    ); // 打印命令行参数

    Image img(args.width, args.height, args.channel); // 创建图像对象
    
    // * 绘制图像内容
    Image::Pixel start(0, 0), end(args.width / 2, args.height - 1);
    Engine::getInstance()->line(img, end, start, Image::WHITE); // 画对角线
    
    img.save(args.output_img_file); // 保存图像
}