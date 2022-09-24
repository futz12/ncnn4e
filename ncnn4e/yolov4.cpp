#include "pch.h"
#ifdef YOLOV4

struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

struct _yolov4
{
    ncnn::Net net;

    std::vector<unsigned char> param;
    std::vector<unsigned char> model;
};

typedef _yolov4 *__yolov4;

extern "C" __declspec(dllexport) __yolov4 __stdcall yolov4_Init(const unsigned char *mem_param, int size_param, const unsigned char *mem_model, int size_model, const bool use_vulkan)
{
    if (use_vulkan && ncnn::get_gpu_count() == 0)
    {
        // no gpu
        std::cout << "[Yolov4]Err Your GPU count is Zero" << std::endl;
        return NULL;
    }

    __yolov4 yolov4 = new _yolov4;

    yolov4->net.opt.num_threads = ncnn::get_big_cpu_count();
    yolov4->net.opt.use_vulkan_compute = use_vulkan;

    yolov4->net.opt.use_winograd_convolution = true;
    yolov4->net.opt.use_sgemm_convolution = true;
    yolov4->net.opt.use_fp16_packed = true;
    yolov4->net.opt.use_fp16_storage = true;
    yolov4->net.opt.use_fp16_arithmetic = true;
    yolov4->net.opt.use_packing_layout = true;
    yolov4->net.opt.use_shader_pack8 = false;
    yolov4->net.opt.use_image_storage = false;

    yolov4->param.clear();
    yolov4->model.clear();

    yolov4->param.insert(yolov4->param.end(), mem_param, mem_param + size_param);
    yolov4->model.insert(yolov4->model.end(), mem_model, mem_model + size_model);

    yolov4->param.push_back(0);

    if (yolov4->net.load_param_mem((char *)yolov4->param.data()) != 0)
    {
        std::cout << "[Yolov4]Err Read Param Failed" << std::endl;
        delete yolov4;
        return NULL;
    }
    if (yolov4->net.load_model(yolov4->model.data()) == 0)
    {
        std::cout << "[Yolov4]Err Read Model Failed" << std::endl;
        delete yolov4;
        return NULL;
    }
    return yolov4;
}

extern "C" int __declspec(dllexport) __stdcall yolov4_Deal(__yolov4 yolov4, const unsigned char *img_src, const int img_size, int target_size, Object **ResList)
{
    if (yolov4 == NULL)
    {
        std::cout << "[Yolov4]Not Init" << std::endl;
        return 0;
    }

    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    if (src_mat.empty())
    {
        std::cout << "[Yolov4]ERR Cant Read Img" << std::endl;
        return 0;
    }

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(src_mat.data, ncnn::Mat::PIXEL_BGR2RGB, src_mat.cols, src_mat.rows, target_size, target_size);

    const float mean_vals[3] = {0, 0, 0};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};

    in.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = yolov4->net.create_extractor();

    ex.input("data", in);
    ncnn::Mat out;

    ex.extract("output", out);

    std::vector<Object> objects;

    for (int i = 0; i < out.h; i++)
    {
        const float *values = out.row(i);

        Object object;
        object.label = values[0];
        object.prob = values[1];
        object.rect.x = values[2] * src_mat.cols;
        object.rect.y = values[3] * src_mat.rows;
        object.rect.width = values[4] * src_mat.cols - object.rect.x;
        object.rect.height = values[5] * src_mat.rows - object.rect.y;

        objects.push_back(object);
    }

    *ResList = new Object[objects.size()];
    // std::cout << "[DEBUG] " << proposals.size() * sizeof(Object) <<' '<<_msize(*ResList) << std::endl;
    memcpy(*ResList, objects.data(), objects.size() * sizeof(Object));
    return objects.size();
}

extern "C" void __declspec(dllexport) __stdcall yolov4_DestructRet(Object *ResList)
{
    delete[] ResList;
}

extern "C" void __declspec(dllexport) __stdcall yolov4_Destroy(__yolov4 yolov4)
{
    yolov4->net.clear();
    delete yolov4;
}

#endif
