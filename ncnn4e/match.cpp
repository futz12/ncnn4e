#include "pch.h"
#ifdef MATCH

inline uint32_t swar(uint32_t i)
{
    i = (i & 0x55555555) + ((i >> 1) & 0x55555555); // 步骤1
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333); // 步骤2
    i = (i & 0x0F0F0F0F) + ((i >> 4) & 0x0F0F0F0F); // 步骤3
    i = (i * 0x01010101) >> 24;                     // 步骤4
    return i;
}

inline __m256i swar(__m256i x)
{
    const static __m256i var1(_mm256_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555));
    const static __m256i var2(_mm256_set_epi32(0x33333333, 0x33333333, 0x33333333, 0x33333333, 0x33333333, 0x33333333, 0x33333333, 0x33333333));
    const static __m256i var3(_mm256_set_epi32(0x0F0F0F0F, 0x0F0F0F0F, 0x0F0F0F0F, 0x0F0F0F0F, 0x0F0F0F0F, 0x0F0F0F0F, 0x0F0F0F0F, 0x0F0F0F0F));
    const static __m256i var4(_mm256_set_epi32(0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101));

    x = _mm256_add_epi32(_mm256_and_si256(x, var1), _mm256_and_si256(_mm256_srli_epi32(x, 1), var1));
    x = _mm256_add_epi32(_mm256_and_si256(x, var2), _mm256_and_si256(_mm256_srli_epi32(x, 2), var2));
    x = _mm256_add_epi32(_mm256_and_si256(x, var3), _mm256_and_si256(_mm256_srli_epi32(x, 4), var3));
    x = _mm256_srli_epi32(_mm256_mullo_epi32(x, var4), 24);
    return x;
}

extern "C" __declspec(dllexport) bool __stdcall match_pHash(const unsigned char *img_src, const int img_size, const int target_size, uint32_t *output) // target_size^2请为32的倍数
{
    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    if (src_mat.empty())
    {
        std::cout << "[Match]ERR Cant Read Img" << std::endl;
        return false;
    }
    // std::cout << "[DEBUG] Read Over" << std::endl;

    if (src_mat.channels() == 3)
    {
        cv::cvtColor(src_mat, src_mat, cv::COLOR_BGR2GRAY); // TO Gray
        // std::cout << "[DEBUG] To Gray Over1" << std::endl;
        src_mat = cv::Mat_<double>(src_mat);
        // std::cout << "[DEBUG] To Gray Over2" << std::endl;
    }
    // std::cout << "[DEBUG] To Gray Over" << std::endl;

    cv::resize(src_mat, src_mat, cv::Size(target_size, target_size));
    // std::cout << "[DEBUG] Resize Over" << std::endl;

    cv::dct(src_mat, src_mat);
    // std::cout << "[DEBUG] Calc Over" << std::endl;

    double *dIdex = new double[target_size * target_size];
    double mean = 0.0;

    for (int i = 0; i < target_size; ++i)
    {
        for (int j = 0; j < target_size; ++j)
        {
            *(dIdex++) = src_mat.at<double>(i, j);
            mean += src_mat.at<double>(i, j) / 64;
        }
    }

    int count = target_size * target_size;

    dIdex -= count;

    count /= 32;

    for (int i = 0; i < count; i++)
    {
        uint32_t buf = 0;
        for (int j = 0; j < 32; j++)
            buf |= (dIdex[i * 32 + j] >= mean ? 1u : 0u) << j;
        output[i] = buf;
    }
    delete[] dIdex;
    return true;
}

extern "C" __declspec(dllexport) uint32_t __stdcall match_sim(const uint32_t *phash1, const uint32_t *phash2, const int size) //返回汉明距离
{
    uint32_t count = 0;
    for (int i = 0; i < size; i++)
        count += swar(phash1[i] ^ phash2[i]);
    return count;
}

struct object
{
    int x;
    int y;
    int dist;
};

extern "C" __declspec(dllexport) bool __stdcall match_scan(const unsigned char *img_src, const int img_size, const unsigned char *target_img_src, const int target_img_size, const int target_size, object *Ret)
{
    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    if (src_mat.empty())
    {
        std::cout << "[Match]ERR Cant Read Img" << std::endl;
        return false;
    }

    cv::_InputArray pic_arr_target(target_img_src, target_img_size);
    cv::Mat target_mat = cv::imdecode(pic_arr_target, cv::IMREAD_UNCHANGED);

    if (target_mat.empty())
    {
        std::cout << "[Match]ERR Cant Read Target Img" << std::endl;
        return false;
    }

    // INIT
    Ret->x = Ret->y = 0;
    Ret->dist = target_size * target_size;
    // INIT END

    float scale_x = ((float)target_size) / ((float)target_mat.cols);
    float scale_y = ((float)target_size) / ((float)target_mat.rows);

    //std::cout << "[DEBUG] " << scale_x << ' ' << scale_y << std::endl;

    cv::resize(target_mat, target_mat, cv::Size(target_size, target_size));
    cv::resize(src_mat, src_mat, cv::Size(round((float)src_mat.cols * scale_x), round((float)src_mat.rows * scale_y)));

    // PRE BEGIN

    cv::cvtColor(target_mat, target_mat, cv::COLOR_BGR2GRAY);
    target_mat = cv::Mat_<double>(target_mat);

    cv::cvtColor(src_mat, src_mat, cv::COLOR_BGR2GRAY);
    src_mat = cv::Mat_<double>(src_mat);

    // PRE END

    // DCT BEGIN
    uint32_t *pHash = new uint32_t[target_size * target_size / 32];

    dct(target_mat, target_mat);

    {
        double *dIdex = new double[target_size * target_size];
        double mean = 0.0;

        for (int i = 0; i < target_size; ++i)
        {
            for (int j = 0; j < target_size; ++j)
            {
                *(dIdex++) = target_mat.at<double>(i, j);
                mean += target_mat.at<double>(i, j) / 64;
            }
        }

        int count = target_size * target_size;

        dIdex -= count;

        count /= 32;

        for (int i = 0; i < count; i++)
        {
            uint32_t buf = 0;
            for (int j = 0; j < 32; j++)
                buf |= (dIdex[i * 32 + j] >= mean ? 1u : 0u) << j;
            pHash[i] = buf;
        }
        delete[] dIdex;
    }
    // DCT END

    // std::cout << "[DEBUG] " << src_mat.cols << ' ' << src_mat.rows << std::endl;
#pragma omp parallel for num_threads(6)

    for (int i = src_mat.rows- target_size; i >= 0; i--)
    {
        for (int j = 0; j + target_size <= src_mat.cols; j++)
        {
            // std::cout << "[DEBUG] " << i + target_size << ' ' << j + target_size << std::endl;
            cv::Mat buf_mat = cv::Mat(src_mat, cv::Range(i, i + target_size), cv::Range(j, j + target_size)).clone();
            // std::cout << "[DEBUG] " << '@' << std::endl;
            //  DCT BEGIN
            uint32_t *pHash2 = new uint32_t[target_size * target_size / 32];
            dct(buf_mat, buf_mat);
            {
                double *dIdex = new double[target_size * target_size];
                double mean = 0.0;

                for (int i = 0; i < target_size; ++i)
                {
                    for (int j = 0; j < target_size; ++j)
                    {
                        *(dIdex++) = buf_mat.at<double>(i, j);
                        mean += buf_mat.at<double>(i, j) / 64;
                    }
                }

                int count = target_size * target_size;

                dIdex -= count;

                count /= 32;

                for (int i = 0; i < count; i++)
                {
                    uint32_t buf = 0;
                    for (int j = 0; j < 32; j++)
                        buf |= (dIdex[i * 32 + j] >= mean ? 1u : 0u) << j;
                    pHash2[i] = buf;
                }
                delete[] dIdex;
            }
            // DCT END
            uint32_t dis = match_sim(pHash, pHash2, target_size * target_size / 32);
            // std::cout << pHash[0] << ' ' << pHash2[0] << std::endl;
            if (dis < Ret->dist)
            {
                Ret->x = j;
                Ret->y = i;
                Ret->dist = dis;
            }
            delete[] pHash2;
        }
    }

    Ret->x /= scale_x;
    Ret->y /= scale_y;

    delete[] pHash;
    return true;
}

extern "C" __declspec(dllexport) int __stdcall match_findcolor32(const unsigned char* img_src,const unsigned int color,const unsigned int dist,int *RetList)//保留API,暂时废弃
{
    int x, y;
    x = *((int*)(img_src + 18));
    y = *((int*)(img_src + 22));

    auto ref = RetList - 1;

    __m256i mode = _mm256_set_epi32(color, color, color, color, color, color, color, color);

#pragma omp parallel for num_threads(ncnn::get_big_cpu_count()*2)
    for (int i = 0; i < y; ++i)
    {
        __m256i* ptr = (__m256i*)(img_src + 54 + x * i * 4);
        for (int j = 0; j < x ; j+=8, ++ptr)
        {
            __m256i buf = _mm256_xor_si256(*ptr, mode);
            __m256i count = swar(buf);
            //__m256i count = buf;
            int _buf[8];
            _mm256_storeu_si256((__m256i*)_buf, count);

            if (_buf[0] <= dist)
            {
                *++ref = j;
                *++ref = y - i - 1;
            }
            if (_buf[1] <= dist)
            {
                *++ref = j+1;
                *++ref = y - i - 1;
            }
            if (_buf[2] <= dist)
            {
                *++ref = j+2;
                *++ref = y - i - 1;
            }
            if (_buf[3] <= dist)
            {
                *++ref = j+3;
                *++ref = y - i - 1;
            }
            if (_buf[4] <= dist)
            {
                *++ref = j+4;
                *++ref = y - i - 1;
            }
            if (_buf[5] <= dist)
            {
                *++ref = j+5;
                *++ref = y - i - 1;
            }
            if (_buf[6] <= dist)
            {
                *++ref = j+6;
                *++ref = y - i - 1;
            }
            if (_buf[7] <= dist)
            {
                *++ref = j+7;
                *++ref = y - i - 1;
            }

        }
    }
    return ((ref - RetList) + 1)>>1;
}

struct objectEx
{
    cv::Rect_<float> rect;
    float prob;
};

static void qsort_descent_inplace(std::vector<objectEx>& objects, int left, int right)
{
    int i = left;
    int j = right;
    float p = objects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (objects[i].prob > p)
            i++;

        while (objects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(objects[i], objects[j]);

            i++;
            j--;
        }
    }

#pragma omp parallel sections
    {
#pragma omp section
        {
            if (left < j)
                qsort_descent_inplace(objects, left, j);
        }
#pragma omp section
        {
            if (i < right)
                qsort_descent_inplace(objects, i, right);
        }
    }
}

static void qsort_descent_inplace(std::vector<objectEx>& objects)
{
    if (objects.empty())
        return;

    qsort_descent_inplace(objects, 0, objects.size() - 1);
}

static inline float intersection_area(const objectEx& a, const objectEx& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void nms_sorted_bboxes(const std::vector<objectEx>& faceobjects, std::vector<int>& picked, float nms_threshold)
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
        const objectEx& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const objectEx& b = faceobjects[picked[j]];


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

extern "C" __declspec(dllexport) int __stdcall match_scanEx(const unsigned char* img_src, const int img_size, const unsigned char* target_img_src,const int target_img_size,const float prob_threshold,const float nms_threshold,objectEx **ResList)
{
    cv::_InputArray pic_arr(img_src, img_size);
    cv::UMat src_mat_cl;
    {
        cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_GRAYSCALE);

        if (src_mat.empty())
        {
            std::cout << "[Match]ERR Cant Read Img" << std::endl;
            return false;
        }
        src_mat.copyTo(src_mat_cl);
    }
    cv::UMat target_mat_cl;
    {
        cv::_InputArray pic_arr_target(target_img_src, target_img_size);
        cv::Mat target_mat = cv::imdecode(pic_arr_target, cv::IMREAD_GRAYSCALE);

        if (target_mat.empty())
        {
            std::cout << "[Match]ERR Cant Read Target Img" << std::endl;
            return false;
        }
        target_mat.copyTo(target_mat_cl);
    }
    if (target_mat_cl.cols > src_mat_cl.cols || target_mat_cl.rows > src_mat_cl.rows)
    {
        std::cout << "[Match]ERR Target is too large" << std::endl;
        return false;
    }

    cv::UMat result(src_mat_cl.cols - target_mat_cl.cols + 1, src_mat_cl.rows - target_mat_cl.rows + 1, CV_32FC1);

    
    cv::matchTemplate(src_mat_cl, target_mat_cl, result, cv::TM_CCOEFF_NORMED);

    cv::Mat res = result.getMat(cv::ACCESS_READ);

    std::vector <objectEx> proposals;

    for (int i = 0; i < result.rows; ++i)
        for (int j = 0; j < result.cols; ++j)
        {
            if (res.at<float>(cv::Point(j, i)) >= prob_threshold)
            {
                objectEx buf;
                buf.prob = res.at<float>(cv::Point(j, i));
                buf.rect.x = j;
                buf.rect.y = i;
                buf.rect.height = target_mat_cl.rows;
                buf.rect.width = target_mat_cl.cols;
                proposals.push_back(buf);
            }
        }
    std::vector<int> picked;
    qsort_descent_inplace(proposals);
    nms_sorted_bboxes(proposals, picked, nms_threshold);

    std::vector <objectEx> objects;

    for (auto x : picked)
        objects.emplace_back(proposals[x]);

    *ResList = new objectEx[objects.size()];
    memcpy(*ResList, objects.data(), objects.size() * sizeof(objectEx));
    return objects.size();
}

extern "C" void __declspec(dllexport) __stdcall match_DestructRet(objectEx * ResList)
{
    delete[] ResList;
}

extern "C" __declspec(dllexport) int __stdcall match_Canny(const unsigned char* img_src, const int img_size,unsigned char **img_out)//成功返回bmp图像大小，请手动释放,失败返回0
{
    cv::_InputArray pic_arr(img_src, img_size);
    cv::UMat src_mat;
    cv::imdecode(pic_arr, cv::IMREAD_GRAYSCALE).copyTo(src_mat);

    if (src_mat.empty())
    {
        std::cout << "[Match]ERR Cant Read Img" << std::endl;
        return false;
    }
    cv::UMat result;

    cv::blur(src_mat, src_mat, cv::Size(3, 3));
    cv::Canny(src_mat, result, 50, 100,3, true);

    std::vector<uchar> out_buf;
    cv::imencode(".bmp", result, out_buf);

    *img_out = new uchar[out_buf.size()];
    memcpy(*img_out, out_buf.data(), out_buf.size());
    return out_buf.size();
}
#endif