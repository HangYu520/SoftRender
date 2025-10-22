#include <preheader.h>

/*
* ------------------------------------------
* 测试GLM库的常用功能
* ------------------------------------------
*/
inline void test_glm() {
    std::cout << "===== GLM 功能测试 =====" << std::endl;
    
    // 1. 向量操作测试
    std::cout << "\n1. 向量操作测试:" << std::endl;
    glm::vec3 v1(1.0f, 2.0f, 3.0f);
    glm::vec3 v2(4.0f, 5.0f, 6.0f);
    
    std::cout << "v1 = (" << v1.x << ", " << v1.y << ", " << v1.z << ")" << std::endl;
    std::cout << "v2 = (" << v2.x << ", " << v2.y << ", " << v2.z << ")" << std::endl;
    std::cout << "v1 + v2 = " << glm::to_string(v1 + v2) << std::endl;
    std::cout << "v1 - v2 = " << glm::to_string(v1 - v2) << std::endl;
    std::cout << "v1 * 2.0 = " << glm::to_string(v1 * 2.0f) << std::endl;
    std::cout << "点积 v1·v2 = " << glm::dot(v1, v2) << std::endl;
    std::cout << "叉积 v1×v2 = " << glm::to_string(glm::cross(v1, v2)) << std::endl;
    std::cout << "v1 长度 = " << glm::length(v1) << std::endl;
    std::cout << "v1 归一化 = " << glm::to_string(glm::normalize(v1)) << std::endl;
    
    // 2. 矩阵操作测试
    std::cout << "\n2. 矩阵操作测试:" << std::endl;
    glm::mat4 identity = glm::mat4(1.0f);
    std::cout << "单位矩阵:" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  ";
        for (int j = 0; j < 4; j++) {
            std::cout << std::fixed << std::setprecision(2) << identity[j][i] << " ";
        }
        std::cout << std::endl;
    }
    
    // 3. 变换矩阵测试
    std::cout << "\n3. 变换矩阵测试:" << std::endl;
    glm::vec3 translation(2.0f, 3.0f, 4.0f);
    glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), translation);
    
    glm::vec3 scale(2.0f, 0.5f, 1.0f);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    
    float angle = glm::radians(45.0f); // 转换为弧度
    glm::vec3 axis(0.0f, 1.0f, 0.0f); // 绕Y轴旋转
    glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
    
    std::cout << "平移矩阵 (2, 3, 4):" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  ";
        for (int j = 0; j < 4; j++) {
            std::cout << std::fixed << std::setprecision(2) << translateMatrix[j][i] << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "缩放矩阵 (2, 0.5, 1):" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  ";
        for (int j = 0; j < 4; j++) {
            std::cout << std::fixed << std::setprecision(2) << scaleMatrix[j][i] << " ";
        }
        std::cout << std::endl;
    }
    
    // 4. 组合变换
    std::cout << "\n4. 组合变换测试:" << std::endl;
    glm::mat4 transform = translateMatrix * rotateMatrix * scaleMatrix;
    std::cout << "组合变换矩阵 (T * R * S):" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  ";
        for (int j = 0; j < 4; j++) {
            std::cout << std::fixed << std::setprecision(2) << transform[j][i] << " ";
        }
        std::cout << std::endl;
    }
    
    // 5. 向量变换
    std::cout << "\n5. 向量变换测试:" << std::endl;
    glm::vec4 point(1.0f, 0.0f, 0.0f, 1.0f); // 齐次坐标
    glm::vec4 transformedPoint = transform * point;
    std::cout << "原始点 (1, 0, 0):" << std::endl;
    std::cout << "变换后的点 = (" << transformedPoint.x << ", " << transformedPoint.y 
              << ", " << transformedPoint.z << ", " << transformedPoint.w << ")" << std::endl;
    
    // 6. 投影矩阵
    std::cout << "\n6. 投影矩阵测试:" << std::endl;
    float fov = 45.0f;
    float aspect = 16.0f / 9.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    glm::mat4 perspective = glm::perspective(glm::radians(fov), aspect, near, far);
    std::cout << "透视投影矩阵 (fov=" << fov << "°, aspect=" << aspect << ", near=" << near << ", far=" << far << "):" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  ";
        for (int j = 0; j < 4; j++) {
            std::cout << std::fixed << std::setprecision(2) << perspective[j][i] << " ";
        }
        std::cout << std::endl;
    }
    
    // 7. 视图矩阵
    std::cout << "\n7. 视图矩阵测试:" << std::endl;
    glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    std::cout << "视图矩阵 (camera at (0,0,5), looking at origin):" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  ";
        for (int j = 0; j < 4; j++) {
            std::cout << std::fixed << std::setprecision(2) << view[j][i] << " ";
        }
        std::cout << std::endl;
    }
    
    // 8. 四元数测试
    std::cout << "\n8. 四元数测试:" << std::endl;
    glm::quat rotation = glm::angleAxis(angle, axis);
    std::cout << "四元数表示的45°Y轴旋转: " << glm::to_string(rotation) << std::endl;
    
    // 修复：使用四元数乘法旋转向量（原错误调用 glm::rotate）
    glm::vec3 rotatedVec = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    std::cout << "旋转后的向量 (1,0,0) = (" << rotatedVec.x << ", " << rotatedVec.y << ", " << rotatedVec.z << ")" << std::endl;
    
    // 9. 欧拉角转换
    std::cout << "\n9. 欧拉角转换测试:" << std::endl;
    glm::vec3 euler(30.0f, 45.0f, 60.0f);
    glm::quat fromEuler = glm::eulerAngleXYZ(glm::radians(euler.x), 
                                            glm::radians(euler.y), 
                                            glm::radians(euler.z));
    glm::vec3 toEuler = glm::eulerAngles(fromEuler);
    
    std::cout << "欧拉角 (30°, 45°, 60°) -> 四元数 -> 欧拉角:" << std::endl;
    std::cout << "  转换回的欧拉角 = (" << glm::degrees(toEuler.x) << "°, " 
              << glm::degrees(toEuler.y) << "°, " << glm::degrees(toEuler.z) << "°)" << std::endl;
    
    // 10. 矩阵与OpenGL交互
    std::cout << "\n10. OpenGL交互测试:" << std::endl;
    float matrixArray[16];
    std::memcpy(matrixArray, glm::value_ptr(transform), sizeof(float) * 16);
    std::cout << "矩阵转换为OpenGL数组格式 (前4个元素): ";
    for (int i = 0; i < 4; i++) {
        std::cout << std::fixed << std::setprecision(2) << matrixArray[i] << " ";
    }
    std::cout << "..." << std::endl;
    
    std::cout << "\n===== GLM 测试完成 =====" << std::endl;
}