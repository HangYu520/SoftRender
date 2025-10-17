#pragma once

#define STB_IMAGE_IMPLEMENTATION // 图片的处理和输出采用 stb_image 库
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <spdlog/spdlog.h>// 日志打印采用 spdlog 库