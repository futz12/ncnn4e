#include "pch.h"
#ifdef POSE

struct _pose
{
    NanoDet g_nanodet;

    std::vector<unsigned char> param;
    std::vector<unsigned char> model;
};

typedef _pose *__pose;

const float _mean_val[3] = {127.5f, 127.5f, 127.5f};

const float _norm_vals[3] = {1 / 127.5f, 1 / 127.5f, 1 / 127.5f};

const float _multipose_scale[][5] = {
    {0.02083333395421505, 0.015625, 0.02083333395421505, 0.015625, -0.069444440305233},
    {0.015625, 0.012500000186264515, 0.015625, 0.012500000186264515, -0.0520833320915699},
};

extern "C" __pose __declspec(dllexport) __stdcall pose_InitPose(const unsigned char *mem_param, int size_param, const unsigned char *mem_model, int size_model, const int target_size, const bool use_vulkan) // target_size is 192/256 Only
{
    if (use_vulkan && ncnn::get_gpu_count() == 0)
    {
        // no gpu
        std::cout << "[POSE]Err Your GPU count is Zero" << std::endl;
        return NULL;
    }
    if (target_size != 192 && target_size != 256)
    {
        std::cout << "[POSE]Err Unexpect Size" << std::endl;
        return NULL;
    }

    __pose pose = new _pose;

    pose->param.clear();
    pose->model.clear();

    pose->param.insert(pose->param.end(), mem_param, mem_param + size_param);
    pose->model.insert(pose->model.end(), mem_model, mem_model + size_model);

    pose->param.push_back(0);

    pose->g_nanodet.load(pose->param.data(), pose->model.data(), target_size, _mean_val, _norm_vals, _multipose_scale[((target_size == 192) ? 0 : 1)], use_vulkan);

    return pose;
}

struct Person_C
{
    Keypoint *points;
    int x;
    int y;
    int width;
    int height;
    float score;
};

extern "C" int __declspec(dllexport) __stdcall pose_Deal(__pose pose, const unsigned char *img_src, const int img_size, Person_C **ResList)
{
    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat rgb = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    if (rgb.empty())
    {
        std::cout << "[POSE]ERR Cant Read Img" << std::endl;
        return 0;
    }

    std::vector<Person> objects;
    pose->g_nanodet.detect(rgb, objects);

    int count = objects.size();
    if (count == 0)
        return 0;

    *ResList = new Person_C[count];
    for (int i = 0; i < count; i++)
    {
        (*ResList)[i].points = new Keypoint[18];
        memcpy((*ResList)[i].points, objects[i].points.data(), 18 * sizeof(Keypoint));
        (*ResList)[i].x = objects[i].x;
        (*ResList)[i].y = objects[i].y;
        (*ResList)[i].width = objects[i].width;
        (*ResList)[i].height = objects[i].height;

        (*ResList)[i].score = objects[i].score;
    }
    return count;
}

extern "C" void __declspec(dllexport) __stdcall pose_DestructRet(Person_C *ResList)
{
    int count = _msize(ResList) / sizeof(Person_C);
    for (int i = 0; i < count; i++)
    {
        delete[] ResList[i].points;
    }
    delete[] ResList;
}

extern "C" void __declspec(dllexport) __stdcall pose_Destroy(__pose pose)
{
    pose->g_nanodet.poseNet.clear();
    delete pose;
}

#endif