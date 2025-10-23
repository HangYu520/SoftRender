#pragma once 

// * 预编译头文件，包含常用的第三方库
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp> // 矩阵和向量计算采用 glm 库
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp> // 提供 glm::to_string 功能
#include <stb/stb_image.h> // 图片的处理和输出采用 stb_image 库
#include <stb/stb_image_write.h>
#include <tiny_obj_loader.h> // 3D 模型加载采用 tinyobjloader 库
#include <spdlog/spdlog.h> // 日志打印采用 spdlog 库
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip> // 提供 std::setprecision 功能
#include <memory>
#include <utility>
#include <string>
#include <chrono>
#include <vector>
#include <array>
#include <tuple>
#include <limits>
#include <random>