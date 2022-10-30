#include "pch.h"
#ifdef SORT

struct _sort
{
    ncnn::Net net;

    std::vector<unsigned char> param;
    std::vector<unsigned char> model;
};

typedef _sort *__sort;

extern "C" __declspec(dllexport) __sort __stdcall sort_Init(const unsigned char* mem_param, int size_param, const unsigned char* mem_model, int size_model, const bool use_vulkan)
{
    if (use_vulkan && ncnn::get_gpu_count() == 0)
    {
        // no gpu
        std::cout << "[Sort]Err Your GPU count is Zero" << std::endl;
        return NULL;
    }

    __sort sort = new _sort;

    sort->net.opt.num_threads = ncnn::get_big_cpu_count();
    sort->net.opt.use_vulkan_compute = use_vulkan;

    sort->param.clear();
    sort->model.clear();

    sort->param.insert(sort->param.end(), mem_param, mem_param + size_param);
    sort->model.insert(sort->model.end(), mem_model, mem_model + size_model);

    sort->param.push_back(0);

    if (sort->net.load_param_mem((char*)sort->param.data()) != 0)
    {
        std::cout << "[Sort]Err Read Param Failed" << std::endl;
        delete sort;
        return NULL;
    }
    if (sort->net.load_model(sort->model.data()) == 0)
    {
        std::cout << "[Sort]Err Read Model Failed" << std::endl;
        delete sort;
        return NULL;
    }
    return sort;
}

struct Object
{
    int label;
    float prob;
};

extern "C" __declspec(dllexport) bool __stdcall sort_Deal(__sort sort, const unsigned char* img_src, const int img_size, int target_size, Object *ResList)
{
    if (sort == NULL)
    {
        std::cout << "[Sort]Not Init" << std::endl;
        return false;
    }

    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    if (src_mat.empty())
    {
        std::cout << "[Sort]ERR Cant Read Img" << std::endl;
        return 0;
    }

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(src_mat.data, ncnn::Mat::PIXEL_BGR2RGB, src_mat.cols, src_mat.rows, target_size, target_size);

    const float mean_vals[3] = { 104.f, 117.f, 123.f };
    in.substract_mean_normalize(mean_vals, 0);

    ncnn::Extractor ex = sort->net.create_extractor();

    ex.input("data", in);

    ncnn::Mat out;
    ex.extract("prob", out);

    std::vector<Object> cls_scores;

    cls_scores.resize(out.w);
    for (int i = 0; i < out.w; ++i)
    {
        cls_scores[i] = { i ,out[i] };
    }

    std::sort(cls_scores.begin(), cls_scores.end(), [](const auto a, const auto b) {
        return a.prob > b.prob;
    });

    memcpy(ResList, cls_scores.data(), cls_scores.size()*sizeof(Object));

    return true;
}

extern "C" void __declspec(dllexport) __stdcall sort_Destroy(__sort sort)
{
    sort->net.clear();
    delete sort;
}

#endif