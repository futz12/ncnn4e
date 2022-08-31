#pragma once

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

struct _yolov7
{
    ncnn::Net net;
    float nms_threshold;
    int target_size;
    std::string layername[3];
};

typedef _yolov7 *__yolov7;

extern "C" __declspec(dllexport) __yolov7 __stdcall yolov7_Init(const char *paramfile, const char *modelfile, float nms_threshold, int target_size, char *layer0, char *layer1, char *layer2, const bool use_vulkan);

extern "C" int __declspec(dllexport) __stdcall yolov7_Deal(__yolov7 yolov7, const unsigned char *img_src, const int img_size, Object **ResList);

extern "C" void __declspec(dllexport) __stdcall yolov7_Destroy(__yolov7 yolov7);
