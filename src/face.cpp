#include "pch.h"
#ifdef FACE
#include "blazeface.h"

struct _face
{
    std::vector<unsigned char> param;
    std::vector<unsigned char> model;

    BlazeFace net;
};

typedef _face *__face;

extern "C" __declspec(dllexport) __face __stdcall face_Init(const unsigned char *mem_param, int size_param, const unsigned char *mem_model, int size_model, int target_size, const bool use_vulkan)
{
    if (use_vulkan && ncnn::get_gpu_count() == 0)
    {
        // no gpu
        std::cout << "[Face]Err Your GPU count is Zero" << std::endl;
        return NULL;
    }

    _face *faceNet = new _face;

    faceNet->param.clear();
    faceNet->model.clear();

    faceNet->param.insert(faceNet->param.end(), mem_param, mem_param + size_param);
    faceNet->model.insert(faceNet->model.end(), mem_model, mem_model + size_model);

    faceNet->param.push_back(0);

    if (faceNet->net.load(faceNet->param.data(), faceNet->model.data(), target_size, use_vulkan) == 0)
        return faceNet;
    else
    {
        delete faceNet;
        return NULL;
    }
}

extern "C" int __declspec(dllexport) __stdcall face_Deal(__face face, const unsigned char *img_src, const int img_size, FaceObject **ResList)
{
    if (face == NULL)
    {
        std::cout << "[Face]Not Init" << std::endl;
        return 0;
    }

    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    if (src_mat.empty())
    {
        std::cout << "[Face]ERR Cant Read Img" << std::endl;
        return 0;
    }

    std::vector<FaceObject> faceobjects;
    face->net.detect(src_mat, faceobjects);
    int count = faceobjects.size();
    if (count == 0)
        return 0;
    *ResList = new FaceObject[count];
    memcpy(*ResList, faceobjects.data(), count * sizeof(FaceObject));
    return count;
}

extern "C" void __declspec(dllexport) __stdcall face_DestructRet(FaceObject *ResList)
{
    delete[] ResList;
}

extern "C" void __declspec(dllexport) __stdcall face_Destroy(__face face)
{
    face->net.blazeface.clear();
    delete face;
}

#endif