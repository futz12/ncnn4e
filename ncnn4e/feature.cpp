#include "pch.h"
#ifdef FEATURE

struct _feat
{
    std::vector<unsigned char> param;
    std::vector<unsigned char> model;
    ncnn::Net net;
};

typedef _feat *__feat;

extern "C" __declspec(dllexport) __feat __stdcall feat_Init(const unsigned char *mem_param, int size_param, const unsigned char *mem_model, int size_model, const bool use_vulkan) // 112 Only
{
    if (use_vulkan && ncnn::get_gpu_count() == 0)
    {
        // no gpu
        std::cout << "[feat]Err Your GPU count is Zero" << std::endl;
        return NULL;
    }

    _feat *featNet = new _feat;

    featNet->net.opt.use_vulkan_compute = use_vulkan;
    featNet->net.opt.num_threads = ncnn::get_big_cpu_count();

    featNet->param.clear();
    featNet->model.clear();

    featNet->param.insert(featNet->param.end(), mem_param, mem_param + size_param);
    featNet->model.insert(featNet->model.end(), mem_model, mem_model + size_model);

    featNet->param.push_back(0);

    if (featNet->net.load_param_mem((char *)featNet->param.data()) != 0)
    {
        std::cout << "[Feature]Err Read Param Failed" << std::endl;
        delete featNet;
        return NULL;
    }
    if (featNet->net.load_model(featNet->model.data()) == 0)
    {
        std::cout << "[Feature]Err Read Model Failed" << std::endl;
        delete featNet;
        return NULL;
    }
    return featNet;
}

extern "C" bool __declspec(dllexport) __stdcall feat_Deal(__feat feat, const unsigned char *img_src, const int img_size, float *feature_mat)
{
    if (feat == NULL)
    {
        std::cout << "[Feature]Not Init" << std::endl;
        return false;
    }

    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    if (src_mat.empty())
    {
        std::cout << "[feat]ERR Cant Read Img" << std::endl;
        return false;
    }

    ncnn::Mat input = ncnn::Mat::from_pixels_resize(src_mat.data, ncnn::Mat::PIXEL_BGR2RGB, src_mat.cols, src_mat.rows, 112, 112);

    ncnn::Extractor ex = feat->net.create_extractor();
    ex.input("data", input);
    ncnn::Mat out;
    ex.extract("fc1", out);
    for (int i = 0; i < 128; i++)
    {
        feature_mat[i] = out[i];
    }
    return true;
}

extern "C" double __declspec(dllexport) __stdcall feat_Calc(const float *v1, const float *v2)
{
    double ret = 0.0, mod1 = 0.0, mod2 = 0.0;
    for (int i = 0; i < 128; ++i)
    {
        ret += v1[i] * v2[i];
        mod1 += v1[i] * v1[i];
        mod2 += v2[i] * v2[i];
    }
    return ret / sqrt(mod1) / sqrt(mod2);
}

extern "C" void __declspec(dllexport) __stdcall feat_Destroy(__feat feat)
{
    feat->net.clear();
    delete feat;
}

#endif