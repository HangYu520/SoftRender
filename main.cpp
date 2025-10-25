#include <SFML/Graphics.hpp> // SFML 库显示窗口
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
* 保存渲染图像
* ------------------------------------------
*/
void saveImage(Image& image, const char* filePath, bool saveDepth = true)
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
* 主函数入口
* ------------------------------------------
*/
int main(int argc, char** argv)
{
    ARG args = parse_args(argc, argv);
    args.log(); // 打印命令行参数

    // TODO 1. 初始化 SFML 窗口 (使用 SFML 3.0 风格)
    sf::RenderWindow window(sf::VideoMode({args.width, args.height}), "Soft Renderer");
    window.setFramerateLimit(60); // 限制帧率

    Image image(args.width, args.height, args.channel); // 创建图像对象
    RandomShader shader; // 创建着色器对象
    
    // TODO 2. 载入 obj 模型
    Model model;
    {   Timer timer("Load Model"); // 计时器开始
    model.loadFrom(args.input_obj_file); // 加载模型文件
    }

    // TODO 3. 初始化 texture 用以和 image 交流
    sf::Texture texture(sf::Vector2u(args.width, args.height)); 
    sf::Sprite sprite(texture);
    float rotation = 0.0f; // 设置旋转角度可变
    float cameraHeight = 0.0f; // 设置相机高度可变
    float fov = 45.0f; // 设置视角范围可变
    bool drawWireframe = false; // 绘制线框
    sf::Clock clock; // 用于键盘平滑的交互

    spdlog::info(
        " Welcome to Soft Renderer !"
        " Press left / right to rotate model."
        " Press up / down to adjust camera height."
        " Scorll to zoom in / out."
        " Press 'W' to toggle wireframe."
        " Press 'R' to reset model."
        " Press 'S' to save image."); // 输出提示信息
    
    // TODO 4. SFML 窗口主循环
    {   Timer timer("Main Loop");
    Engine::getInstance()->centerModel(model); // ! 影响 z 缓存初始化
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // 事件处理 使用 SFML (3.0 风格)
        while (const auto event = window.pollEvent())
        {
            // 检查窗口关闭事件
            if (event->is<sf::Event::Closed>())
                window.close();
            
            // 检查按键事件
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                // 检查是否按下了 'S' 键
                if (keyPressed->scancode == sf::Keyboard::Scan::S)
                {
                    saveImage(image, args.output_img_file);
                }

                // 检查是否按下了 'R' 键
                if (keyPressed->scancode == sf::Keyboard::Scan::R)
                {
                    rotation = 0.0f; // 重置旋转角度
                    cameraHeight = 0.0f; // 重置相机高度
                    fov = 45.0f; // 重置视角范围
                    drawWireframe = false; // 重置绘制线框
                    spdlog::info("Reset rotation = {}, camera height = {}, and field of view = {}.", rotation, cameraHeight, fov);
                }

                // 检查是否按下了 'W' 键
                if (keyPressed->scancode == sf::Keyboard::Scan::W)
                {
                    drawWireframe = !drawWireframe; // 切换线框模式
                    spdlog::info("Toggle wireframe mode. Press 'W' again to toggle back.");
                }
            }

            // 检查滚轮事件
            if (const auto* scrolled = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                // scrolled->delta 包含了滚动的幅度 (正值通常是向上/远离用户, 负值是向下/朝向用户)
                fov += scrolled->delta * 0.1f; // 0.1f 是灵敏度因子
                
                // 防止缩放得太小
                if (fov < 10.0f) 
                    fov = 10.0f;
                
                // 防止缩放得太大
                if (fov > 120.0f) 
                    fov = 120.0f;
            }
        }
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) // 方向键左
            rotation -= 45.0f * deltaTime; // 交互式旋转模型
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) // 方向键右
            rotation += 45.0f * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) // 方向键上
            cameraHeight += 0.1f * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) // 方向键下
            cameraHeight -= 0.1f * deltaTime;

        // !! 调用渲染引擎 !!
        {
            image.clear(); // 清空图像
            Engine::getInstance()->freeZBuffer(); // 释放 z 缓存
            Engine::getInstance()->initZBuffer(image.width * image.height, -2.0f); // 初始化 z 缓存
            Engine::getInstance()->getTrans().mConfig.rotateAngle = rotation; // 旋转
            Engine::getInstance()->getTrans().vConfig.cameraPos.y = cameraHeight; // 升降
            Engine::getInstance()->getTrans().pConfig.fov = fov; // 视角
            if (drawWireframe) Engine::getInstance()->wireframe(image, model, Image::WHITE); // 绘制线框
            else Engine::getInstance()->render(image, model, &shader); // 渲染
        }

        // 更新 texture 并绘制到 SFML 窗口
        image.flipVertical(); // 翻转图像
        texture.update(image.image_buffer); // 更新 texture
        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.display();
    }
    }
}