#define STB_IMAGE_IMPLEMENTATION // 图片的处理和输出采用 stb_image 库
#include "external/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"
#include "external/spdlog/spdlog.h" // 日志打印采用 spdlog 库
#include "src/typedef.h" // 自定义的一些数据类型

struct ARG // * 命令行参数
{
    const char* output_img_file; // 输出文件路径
    stbi__uint32 width, height; // 图像宽高
    Image::Channel channel; // 图像通道数
};

static ARG parse_args(int argc, char** argv) // * 解析命令行参数
{
    // 定义命令行参数的默认值
    const char* arg_output_img_file = "output.png";
    stbi__uint32 arg_width = 64, arg_height = 64;
    Image::Channel arg_channel = Image::Channel::RGB;
    
    // 解析命令行参数
    if (argc > 1)
    {
        for (size_t i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
                arg_output_img_file = argv[++i];
            else if(strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0)
                arg_width = static_cast<stbi__uint32>(atoi(argv[++i]));
            else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--height") == 0)
                arg_height = static_cast<stbi__uint32>(atoi(argv[++i]));
            else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--channel") == 0)
                arg_channel = static_cast<Image::Channel>(atoi(argv[++i]));
        }
    }

    return { arg_output_img_file, arg_width, arg_height, arg_channel };
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
    img.setColor(0, 0, Image::BLUE);
    img.setColor(args.width - 1, 0, Image::GREEN);
    img.setColor(0, args.height - 1, Image::RED);
    img.setColor(args.width - 1, args.height - 1, Image::WHITE);
    
    img.save(args.output_img_file); // 保存图像
}