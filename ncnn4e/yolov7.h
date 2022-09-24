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

    std::string layerName[4];

    std::vector<unsigned char> param;
    std::vector<unsigned char> model;
};

typedef _yolov7 *__yolov7;
