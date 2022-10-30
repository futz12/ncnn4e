#include "pch.h"
#ifdef YOLOV7
#include "yolov7.h"

#define MAX_STRIDE 64

static inline float intersection_area(const Object& a, const Object& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

#pragma omp parallel sections
    {
#pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
#pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects)
{
    if (faceobjects.empty())
        return;

    qsort_descent_inplace(faceobjects, 0, faceobjects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold, bool agnostic = false)
{
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++)
    {
        const Object& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const Object& b = faceobjects[picked[j]];

            if (!agnostic && a.label != b.label)
                continue;

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            // float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}

static inline float sigmoid(float x)
{
    return static_cast<float>(1.f / (1.f + exp(-x)));
}

static void generate_proposals(const ncnn::Mat& anchors, int stride, const ncnn::Mat& in_pad, const ncnn::Mat& feat_blob, float prob_threshold, std::vector<Object>& objects)
{
    const int num_grid_x = feat_blob.w;
    const int num_grid_y = feat_blob.h;

    const int num_anchors = anchors.w / 2;

    const int num_class = 80;

    for (int q = 0; q < num_anchors; q++)
    {
        const float anchor_w = anchors[q * 2];
        const float anchor_h = anchors[q * 2 + 1];

        for (int i = 0; i < num_grid_y; i++)
        {
            for (int j = 0; j < num_grid_x; j++)
            {
                // find class index with max class score
                int class_index = 0;
                float class_score = -FLT_MAX;
                for (int k = 0; k < num_class; k++)
                {
                    float score = feat_blob.channel(q * 85 + 5 + k).row(i)[j];
                    if (score > class_score)
                    {
                        class_index = k;
                        class_score = score;
                    }
                }

                float box_score = feat_blob.channel(q * 85 + 4).row(i)[j];

                float confidence = sigmoid(box_score) * sigmoid(class_score);

                if (confidence >= prob_threshold)
                {
                    // yolov5/models/yolo.py Detect forward
                    // y = x[i].sigmoid()
                    // y[..., 0:2] = (y[..., 0:2] * 2. - 0.5 + self.grid[i].to(x[i].device)) * self.stride[i]  # xy
                    // y[..., 2:4] = (y[..., 2:4] * 2) ** 2 * self.anchor_grid[i]  # wh

                    float dx = sigmoid(feat_blob.channel(q * 85 + 0).row(i)[j]);
                    float dy = sigmoid(feat_blob.channel(q * 85 + 1).row(i)[j]);
                    float dw = sigmoid(feat_blob.channel(q * 85 + 2).row(i)[j]);
                    float dh = sigmoid(feat_blob.channel(q * 85 + 3).row(i)[j]);

                    float pb_cx = (dx * 2.f - 0.5f + j) * stride;
                    float pb_cy = (dy * 2.f - 0.5f + i) * stride;

                    float pb_w = pow(dw * 2.f, 2) * anchor_w;
                    float pb_h = pow(dh * 2.f, 2) * anchor_h;

                    float x0 = pb_cx - pb_w * 0.5f;
                    float y0 = pb_cy - pb_h * 0.5f;
                    float x1 = pb_cx + pb_w * 0.5f;
                    float y1 = pb_cy + pb_h * 0.5f;

                    Object obj;
                    obj.rect.x = x0;
                    obj.rect.y = y0;
                    obj.rect.width = x1 - x0;
                    obj.rect.height = y1 - y0;
                    obj.label = class_index;
                    obj.prob = confidence;

                    objects.push_back(obj);
                }
            }
        }
    }
}

extern "C" __declspec(dllexport) __yolov7 __stdcall yolov7_Init(const unsigned char *mem_param, const int size_param, const unsigned char *mem_model, const int size_model, const char *inputlayer, const char *outputlayer0, const char *outputlayer1, const char *outputlayer2, const bool use_vulkan)
{
    if (use_vulkan && ncnn::get_gpu_count() == 0)
    {
        // no gpu
        std::cout << "[Yolov7]Err Your GPU count is Zero" << std::endl;
        return NULL;
    }

    _yolov7 *yolov7Net = new _yolov7;

    yolov7Net->net.opt.use_vulkan_compute = use_vulkan;
    yolov7Net->net.opt.num_threads = ncnn::get_big_cpu_count();

    yolov7Net->param.clear();
    yolov7Net->model.clear();

    yolov7Net->param.insert(yolov7Net->param.end(), mem_param, mem_param + size_param);
    yolov7Net->model.insert(yolov7Net->model.end(), mem_model, mem_model + size_model);

    yolov7Net->param.push_back(0);

    if (yolov7Net->net.load_param_mem((char *)yolov7Net->param.data()) != 0)
    {
        std::cout << "[Yolov7]Err Read Param Failed" << std::endl;
        delete yolov7Net;
        return NULL;
    }
    if (yolov7Net->net.load_model(yolov7Net->model.data()) == 0)
    {
        std::cout << "[Yolov7]Err Read Model Failed" << std::endl;
        delete yolov7Net;
        return NULL;
    }
    yolov7Net->layerName[0] = inputlayer;
    yolov7Net->layerName[1] = outputlayer0;
    yolov7Net->layerName[2] = outputlayer1;
    yolov7Net->layerName[3] = outputlayer2;
    return yolov7Net;
}

extern "C" int __declspec(dllexport) __stdcall yolov7_Deal(__yolov7 yolov7, const unsigned char *img_src, const int img_size, const int target_size, const float prob_threshold, const float nms_threshold, Object **ResList)
{
    if (yolov7 == NULL)
    {
        std::cout << "[Yolov7]Not Init" << std::endl;
        return 0;
    }

    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    if (src_mat.empty())
    {
        std::cout << "[Yolov7]ERR Cant Read Img" << std::endl;
        return 0;
    }

    /*
        unsigned char *mat_data;
        if (!BMP24TOMAT(img_src, &mat_data))
        {
            std::cout << "[Yolov7]Err Read BMP Failed" << endl;
            return 0;
        }
        int img_w, img_h;
        GetBMPSize(img_src, img_w, img_h);
    */

    // letterbox pad to multiple of MAX_STRIDE
    int img_w = src_mat.cols;
    int img_h = src_mat.rows;

    // letterbox pad to multiple of MAX_STRIDE
    int w = img_w;
    int h = img_h;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)target_size / w;
        w = target_size;
        h = int(h * scale);
    }
    else
    {
        scale = (float)target_size / h;
        h = target_size;
        w = int(w * scale);
    }

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(src_mat.data, ncnn::Mat::PIXEL_BGR2RGB, img_w, img_h, w, h);

    int wpad = (w + MAX_STRIDE - 1) / MAX_STRIDE * MAX_STRIDE - w;
    int hpad = (h + MAX_STRIDE - 1) / MAX_STRIDE * MAX_STRIDE - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT, 114.f);

    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in_pad.substract_mean_normalize(0, norm_vals);

    ncnn::Extractor ex = ((__yolov7)yolov7)->net.create_extractor();

    ex.input(yolov7->layerName[0].c_str(), in_pad);

    std::vector<Object> proposals;

    // stride 8
    {
        ncnn::Mat out;
        ex.extract(yolov7->layerName[1].c_str(), out);

        ncnn::Mat anchors(6);
        anchors[0] = 12.f;
        anchors[1] = 16.f;
        anchors[2] = 19.f;
        anchors[3] = 36.f;
        anchors[4] = 40.f;
        anchors[5] = 28.f;

        std::vector<Object> objects8;
        generate_proposals(anchors, 8, in_pad, out, prob_threshold, objects8);

        proposals.insert(proposals.end(), objects8.begin(), objects8.end());
    }

    // stride 16
    {
        ncnn::Mat out;

        ex.extract(yolov7->layerName[2].c_str(), out);

        ncnn::Mat anchors(6);
        anchors[0] = 36.f;
        anchors[1] = 75.f;
        anchors[2] = 76.f;
        anchors[3] = 55.f;
        anchors[4] = 72.f;
        anchors[5] = 146.f;

        std::vector<Object> objects16;
        generate_proposals(anchors, 16, in_pad, out, prob_threshold, objects16);

        proposals.insert(proposals.end(), objects16.begin(), objects16.end());
    }

    // stride 32
    {
        ncnn::Mat out;

        ex.extract(yolov7->layerName[3].c_str(), out);

        ncnn::Mat anchors(6);
        anchors[0] = 142.f;
        anchors[1] = 110.f;
        anchors[2] = 192.f;
        anchors[3] = 243.f;
        anchors[4] = 459.f;
        anchors[5] = 401.f;

        std::vector<Object> objects32;
        generate_proposals(anchors, 32, in_pad, out, prob_threshold, objects32);

        proposals.insert(proposals.end(), objects32.begin(), objects32.end());
    }
    qsort_descent_inplace(proposals);
    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked, nms_threshold);

    int count = picked.size();

    //还原坐标
    std::vector<Object> objects;

    objects.resize(count);

    for (int i = 0; i < count; i++)
    {
        objects[i] = proposals[picked[i]];

        // adjust offset to original unpadded
        float x0 = (objects[i].rect.x - (wpad / 2)) / scale;
        float y0 = (objects[i].rect.y - (hpad / 2)) / scale;
        float x1 = (objects[i].rect.x + objects[i].rect.width - (wpad / 2)) / scale;
        float y1 = (objects[i].rect.y + objects[i].rect.height - (hpad / 2)) / scale;

        // clip
        x0 = std::max(std::min(x0, (float)(img_w - 1)), 0.f);
        y0 = std::max(std::min(y0, (float)(img_h - 1)), 0.f);
        x1 = std::max(std::min(x1, (float)(img_w - 1)), 0.f);
        y1 = std::max(std::min(y1, (float)(img_h - 1)), 0.f);

        objects[i].rect.x = x0;
        objects[i].rect.y = y0;
        objects[i].rect.width = x1 - x0;
        objects[i].rect.height = y1 - y0;
    }
    if (count == 0)
        return 0;

    *ResList = new Object[count];
    // std::cout << "[DEBUG] " << proposals.size() * sizeof(Object) <<' '<<_msize(*ResList) << std::endl;
    memcpy(*ResList, objects.data(), objects.size() * sizeof(Object));
    return objects.size();
}

extern "C" void __declspec(dllexport) __stdcall yolov7_DestructRet(Object *ResList)
{
    delete[] ResList;
}

extern "C" void __declspec(dllexport) __stdcall yolov7_Destroy(__yolov7 yolov7)
{
    yolov7->net.clear();
    delete yolov7;
}

#endif