#include <SFML/Graphics.hpp> // SFML 库显示窗口
#include <imgui.h> // ImGui 库
#include <imgui-SFML.h> // ImGui SFML 库
#include "utils.h"

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
    bool initImGui = ImGui::SFML::Init(window); // 初始化 ImGui

    Image image(args.width, args.height, args.channel); // 创建渲染图像对象
    PhongShader shader; // 创建着色器对象
    
    // TODO 2. 载入 obj 模型
    std::vector<std::tuple<Model, Image, Image>> loadmodels;
    loadAsset(args.input_obj_json, loadmodels);
    if (loadmodels.empty())
    {
        spdlog::error("No model found.");
        return -1;
    }
    auto [model, texture_map, normal_map] = loadmodels[0]; // 获取第一个模型进行 center

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
        " Press 'S' to smooth model."); // 输出提示信息
    
    // TODO 4. SFML 窗口主循环
    {   Timer timer("Main Loop");
    Engine::getInstance()->centerModel(model); // ! 影响 z 缓存初始化
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // 事件处理 使用 SFML (3.0 风格)
        while (const auto event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event); // 处理 ImGui 事件
            // 检查窗口关闭事件
            if (event->is<sf::Event::Closed>())
                window.close();
            
            // 检查按键事件
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                // 检查是否按下了 'S' 键
                if (keyPressed->scancode == sf::Keyboard::Scan::S)
                {
                    shader.smooth = !shader.smooth; // 切换平滑模式
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
                if (fov < 10.0f) fov = 10.0f;
                // 防止缩放得太大
                if (fov > 120.0f) fov = 120.0f;
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
            Engine::getInstance()->initZBuffer(image.width * image.height, -3.0f); // 初始化 z 缓存
            Engine::getInstance()->getTrans().mConfig.rotateAngle = rotation; // 旋转
            Engine::getInstance()->getTrans().vConfig.cameraPos.y = cameraHeight; // 升降
            Engine::getInstance()->getTrans().pConfig.fov = fov; // 视角
            renderAll(image, &shader, loadmodels, drawWireframe); // 渲染
        }

        // 更新 texture 并绘制到 SFML 窗口
        image.flipVertical(); // 翻转图像
        texture.update(image.image_buffer); // 更新 texture
        // 创建 ImGui 窗口
        ImGui::SFML::Update(window, sf::seconds(deltaTime));
        // 控制面板
        ImGui::Begin("Control Panel");
        ImGui::Checkbox("Wireframe", &drawWireframe);
        ImGui::SameLine();
        if (ImGui::Button("Save")) saveImage(image, args.output_img_file);
        ImGui::Text("Rotation: %.2f", rotation);
        ImGui::Text("Camera Height: %.2f", cameraHeight);
        ImGui::Text("Field of View: %.2f", fov);
        ImGui::Separator();
        ImGui::Text("Phong Shader");
        ImGui::SliderFloat("I", &shader.config.I, 0.0f, 5.0f);
        ImGui::SliderFloat("ka", &shader.config.ka, 0.0f, 1.0f);
        ImGui::SliderFloat("kd", &shader.config.kd, 0.0f, 1.0f);
        ImGui::SliderFloat("ks", &shader.config.ks, 0.0f, 1.0f);
        ImGui::SliderInt("p", &shader.config.p, 0, 100);
        ImGui::Text("Light Position");
        ImGui::SliderFloat("x", &shader.config.light_position.x, -2.0f, 2.0f);
        ImGui::SliderFloat("y", &shader.config.light_position.y, -2.0f, 2.0f);
        ImGui::SliderFloat("z", &shader.config.light_position.z, -2.0f, 2.0f);
        ImGui::Checkbox("Texture", &shader.use_texture_map);
        ImGui::SameLine();
        ImGui::Checkbox("Smooth", &shader.smooth);
        ImGui::Checkbox("NormalMap", &shader.use_normal_map);
        ImGui::SameLine();
        ImGui::Checkbox("TangentMap", &shader.use_tangent_map);
        ImGui::End();
        
        window.clear(sf::Color::Black);
        window.draw(sprite);
        ImGui::SFML::Render(window); // 绘制 ImGui
        window.display();
    }
    ImGui::SFML::Shutdown(); // 销毁 ImGui
    }
}