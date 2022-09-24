#include "pch.h"

extern "C" bool __declspec(dllexport) __stdcall ncnn4e_isVulkan() //判断是否支持GPU加速
{
    return ncnn::get_gpu_count() != 0;
}

extern "C" void __declspec(dllexport) __stdcall ncnn4e_initVulkan() //请在程序启动时开启
{
    if (ncnn::get_gpu_count() > 0)
        ncnn::create_gpu_instance();
}

extern "C" void __declspec(dllexport) __stdcall ncnn4e_destroyVulkan() //请在程序最终结束时销毁
{
    if (ncnn::get_gpu_count() > 0)
        ncnn::destroy_gpu_instance();
}