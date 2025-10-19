// * 第三方库的实现放在一个单独的 cpp 文件

#define STB_IMAGE_IMPLEMENTATION // 图片的处理和输出采用 stb_image 库
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h> // 3D 模型加载采用 tinyobjloader 库