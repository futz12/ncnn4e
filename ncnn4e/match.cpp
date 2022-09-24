#include "pch.h"
#ifdef MATCH

uint32_t swar(uint32_t i)
{
    i = (i & 0x55555555) + ((i >> 1) & 0x55555555); // 步骤1
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333); // 步骤2
    i = (i & 0x0F0F0F0F) + ((i >> 4) & 0x0F0F0F0F); // 步骤3
    i = (i * 0x01010101) >> 24;                     // 步骤4
    return i;
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

#endif