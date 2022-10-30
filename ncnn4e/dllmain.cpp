// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"


void checkBuild()
{
    //查询opencv编译时配置
    std::cout << cv::getBuildInformation() << std::endl;
}

void checkSimd()
{
    //查询opencv线程
    int numTh = cv::getNumThreads(); //默认值是cpu的逻辑线程数
    int numCore = cv::getNumberOfCPUs();
    std::cout << "getNumThreads=" << numTh << std::endl;
    std::cout << "getNumberOfCPUs=" << numCore << std::endl;

    //查询opencv当前是否开启了并行优化功能
    bool opt = cv::useOptimized(); //默认值是true
    std::cout << "useOptimized=" << opt << std::endl;

    //查询opencv当前是否支持具体的CPU指令集
    bool check1 = cv::checkHardwareSupport(CV_CPU_SSE4_1);
    bool check2 = cv::checkHardwareSupport(CV_CPU_SSE4_2);
    bool check3 = cv::checkHardwareSupport(CV_CPU_AVX2);
    std::cout << "CV_CPU_SSE4_1=" << check1 << std::endl;
    std::cout << "CV_CPU_SSE4_2=" << check2 << std::endl;
    std::cout << "CV_CPU_AVX2=" << check3 << std::endl;

    //查询完整的硬件支持清单
    std::cout << "HardwareSupport:" << std::endl;
    std::cout << "CV_CPU_MMX: " << cv::checkHardwareSupport(CV_CPU_MMX) << std::endl;
    std::cout << "CV_CPU_SSE: " << cv::checkHardwareSupport(CV_CPU_SSE) << std::endl;
    std::cout << "CV_CPU_SSE2: " << cv::checkHardwareSupport(CV_CPU_SSE2) << std::endl;
    std::cout << "CV_CPU_SSE3: " << cv::checkHardwareSupport(CV_CPU_SSE3) << std::endl;
    std::cout << "CV_CPU_SSSE3: " << cv::checkHardwareSupport(CV_CPU_SSSE3) << std::endl;
    std::cout << "CV_CPU_SSE4_1: " << cv::checkHardwareSupport(CV_CPU_SSE4_1) << std::endl;
    std::cout << "CV_CPU_SSE4_2: " << cv::checkHardwareSupport(CV_CPU_SSE4_2) << std::endl;
    std::cout << "CV_CPU_POPCNT: " << cv::checkHardwareSupport(CV_CPU_POPCNT) << std::endl;
    std::cout << "CV_CPU_FP16: " << cv::checkHardwareSupport(CV_CPU_FP16) << std::endl;
    std::cout << "CV_CPU_AVX: " << cv::checkHardwareSupport(CV_CPU_AVX) << std::endl;
    std::cout << "CV_CPU_AVX2: " << cv::checkHardwareSupport(CV_CPU_AVX2) << std::endl;
    std::cout << "CV_CPU_FMA3: " << cv::checkHardwareSupport(CV_CPU_FMA3) << std::endl;
    std::cout << "CV_CPU_AVX_512F: " << cv::checkHardwareSupport(CV_CPU_AVX_512F) << std::endl;
    std::cout << "CV_CPU_AVX_512BW: " << cv::checkHardwareSupport(CV_CPU_AVX_512BW) << std::endl;
    std::cout << "CV_CPU_AVX_512CD: " << cv::checkHardwareSupport(CV_CPU_AVX_512CD) << std::endl;
    std::cout << "CV_CPU_AVX_512DQ: " << cv::checkHardwareSupport(CV_CPU_AVX_512DQ) << std::endl;
    std::cout << "CV_CPU_AVX_512ER: " << cv::checkHardwareSupport(CV_CPU_AVX_512ER) << std::endl;
    std::cout << "CV_CPU_AVX_512IFMA512: " << cv::checkHardwareSupport(CV_CPU_AVX_512IFMA512) << std::endl;
    std::cout << "CV_CPU_AVX_512IFMA: " << cv::checkHardwareSupport(CV_CPU_AVX_512IFMA) << std::endl;
    std::cout << "CV_CPU_AVX_512PF: " << cv::checkHardwareSupport(CV_CPU_AVX_512PF) << std::endl;
    std::cout << "CV_CPU_AVX_512VBMI: " << cv::checkHardwareSupport(CV_CPU_AVX_512VBMI) << std::endl;
    std::cout << "CV_CPU_AVX_512VL: " << cv::checkHardwareSupport(CV_CPU_AVX_512VL) << std::endl;
    std::cout << "CV_CPU_NEON: " << cv::checkHardwareSupport(CV_CPU_NEON) << std::endl;
    std::cout << "CV_CPU_VSX: " << cv::checkHardwareSupport(CV_CPU_VSX) << std::endl;
    std::cout << "CV_CPU_AVX512_SKX: " << cv::checkHardwareSupport(CV_CPU_AVX512_SKX) << std::endl;
    std::cout << "CV_HARDWARE_MAX_FEATURE: " << cv::checkHardwareSupport(CV_HARDWARE_MAX_FEATURE) << std::endl;
    std::cout << std::endl;

    //cv::setUseOptimized(false);
    //cv::setNumThreads(1);
}

void checkOpenCL() //Open Computing Language:开放计算语言,可以附加在主机处理器的CPU或GPU上执行
{
    std::vector<cv::ocl::PlatformInfo> info;
    getPlatfomsInfo(info);
    for (cv::ocl::PlatformInfo &sdk : info)
    {

        int number = sdk.deviceNumber();
        if (number < 1)
        {
            std::cout << "Number of devices:" << number << std::endl;
            return;
        }

        std::cout << "***********SDK************" << std::endl;
        std::cout << "Name:" << sdk.name() << std::endl;
        std::cout << "Vendor:" << sdk.vendor() << std::endl;
        std::cout << "Version:" << sdk.version() << std::endl;
        std::cout << "Version:" << sdk.version() << std::endl;
        std::cout << "Number of devices:" << number << std::endl;

        for (int i = 0; i < number; i++)
        {
            std::cout << std::endl;

            cv::ocl::Device device;
            sdk.getDevice(device, i);

            std::cout << "***********Device " << i + 1 << "***********" << std::endl;
            std::cout << "Vendor Id:" << device.vendorID() << std::endl;
            std::cout << "Vendor name:" << device.vendorName() << std::endl;
            std::cout << "Name:" << device.name() << std::endl;
            std::cout << "Driver version:" << device.vendorID() << std::endl;

            if (device.isAMD())
                std::cout << "Is AMD device" << std::endl;

            if (device.isIntel())
                std::cout << "Is Intel device" << std::endl;

            if (device.isNVidia())
                std::cout << "Is NVidia device" << std::endl;

            std::cout << "Global Memory size:" << device.globalMemSize() << std::endl;
            std::cout << "Memory cache size:" << device.globalMemCacheSize() << std::endl;
            std::cout << "Memory cache type:" << device.globalMemCacheType() << std::endl;
            std::cout << "Local Memory size:" << device.localMemSize() << std::endl;
            std::cout << "Local Memory type:" << device.localMemType() << std::endl;
            std::cout << "Max Clock frequency:" << device.maxClockFrequency() << std::endl;
        }
    }
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        checkBuild();
        checkSimd();
        checkOpenCL();
        std::cout <<"==============================" << std::endl;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

