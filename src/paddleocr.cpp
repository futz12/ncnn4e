#include "pch.h"
#ifdef OCR

struct _ocr
{
    ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
    ncnn::PoolAllocator g_workspace_pool_allocator;
    int dstHeight = 32; // when use PP-OCRv3 it should be 48

    ncnn::Net dbNet;

    ncnn::Net crnnNet;

    std::vector<std::string> keys;
};

typedef _ocr *__ocr;

struct TextBox
{
    std::vector<cv::Point> boxPoint;
    float score;
    std::string text;
};
struct TextLine
{
    std::string text;
    std::vector<float> charScores;
};

struct Angle
{
    int index;
    float score;
};

bool cvPointCompare(const cv::Point &a, const cv::Point &b)
{
    return a.x < b.x;
}

std::vector<cv::Point> getMinBoxes(const std::vector<cv::Point> &inVec, float &minSideLen, float &allEdgeSize)
{
    std::vector<cv::Point> minBoxVec;
    cv::RotatedRect textRect = cv::minAreaRect(inVec);
    cv::Mat boxPoints2f;
    cv::boxPoints(textRect, boxPoints2f);

    float *p1 = (float *)boxPoints2f.data;
    std::vector<cv::Point> tmpVec;
    for (int i = 0; i < 4; ++i, p1 += 2)
    {
        tmpVec.emplace_back(int(p1[0]), int(p1[1]));
    }

    std::sort(tmpVec.begin(), tmpVec.end(), cvPointCompare);

    minBoxVec.clear();

    int index1, index2, index3, index4;
    if (tmpVec[1].y > tmpVec[0].y)
    {
        index1 = 0;
        index4 = 1;
    }
    else
    {
        index1 = 1;
        index4 = 0;
    }

    if (tmpVec[3].y > tmpVec[2].y)
    {
        index2 = 2;
        index3 = 3;
    }
    else
    {
        index2 = 3;
        index3 = 2;
    }

    minBoxVec.clear();

    minBoxVec.push_back(tmpVec[index1]);
    minBoxVec.push_back(tmpVec[index2]);
    minBoxVec.push_back(tmpVec[index3]);
    minBoxVec.push_back(tmpVec[index4]);

    minSideLen = (std::min)(textRect.size.width, textRect.size.height);
    allEdgeSize = 2.f * (textRect.size.width + textRect.size.height);

    return minBoxVec;
}

float boxScoreFast(const cv::Mat &inMat, const std::vector<cv::Point> &inBox)
{
    std::vector<cv::Point> box = inBox;
    int width = inMat.cols;
    int height = inMat.rows;
    int maxX = -1, minX = 1000000, maxY = -1, minY = 1000000;
    for (int i = 0; i < box.size(); ++i)
    {
        if (maxX < box[i].x)
            maxX = box[i].x;
        if (minX > box[i].x)
            minX = box[i].x;
        if (maxY < box[i].y)
            maxY = box[i].y;
        if (minY > box[i].y)
            minY = box[i].y;
    }
    maxX = (std::min)((std::max)(maxX, 0), width - 1);
    minX = (std::max)((std::min)(minX, width - 1), 0);
    maxY = (std::min)((std::max)(maxY, 0), height - 1);
    minY = (std::max)((std::min)(minY, height - 1), 0);

    for (int i = 0; i < box.size(); ++i)
    {
        box[i].x = box[i].x - minX;
        box[i].y = box[i].y - minY;
    }

    std::vector<std::vector<cv::Point>> maskBox;
    maskBox.push_back(box);
    cv::Mat maskMat(maxY - minY + 1, maxX - minX + 1, CV_8UC1, cv::Scalar(0, 0, 0));
    cv::fillPoly(maskMat, maskBox, cv::Scalar(1, 1, 1), 1);
    return cv::mean(inMat(cv::Rect(cv::Point(minX, minY), cv::Point(maxX + 1, maxY + 1))).clone(),
                    maskMat)
        .val[0];
}

std::vector<cv::Point> unClip(const std::vector<cv::Point> &inBox, float perimeter, float unClipRatio)
{
    std::vector<cv::Point> outBox;
    ClipperLib::Path poly;

    for (int i = 0; i < inBox.size(); ++i)
    {
        poly.push_back(ClipperLib::IntPoint(inBox[i].x, inBox[i].y));
    }

    double distance = unClipRatio * ClipperLib::Area(poly) / (double)perimeter;

    ClipperLib::ClipperOffset clipperOffset;
    clipperOffset.AddPath(poly, ClipperLib::JoinType::jtRound, ClipperLib::EndType::etClosedPolygon);
    ClipperLib::Paths polys;
    polys.push_back(poly);
    clipperOffset.Execute(polys, distance);

    outBox.clear();
    std::vector<cv::Point> rsVec;
    for (int i = 0; i < polys.size(); ++i)
    {
        ClipperLib::Path tmpPoly = polys[i];
        for (int j = 0; j < tmpPoly.size(); ++j)
        {
            outBox.emplace_back(tmpPoly[j].X, tmpPoly[j].Y);
        }
    }
    return outBox;
}

std::vector<TextBox> findRsBoxes(const cv::Mat &fMapMat, const cv::Mat &norfMapMat,
                                 const float boxScoreThresh, const float unClipRatio)
{
    float minArea = 3;
    std::vector<TextBox> rsBoxes;
    rsBoxes.clear();
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(norfMapMat, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    for (int i = 0; i < contours.size(); ++i)
    {
        float minSideLen, perimeter;
        std::vector<cv::Point> minBox = getMinBoxes(contours[i], minSideLen, perimeter);
        if (minSideLen < minArea)
            continue;
        float score = boxScoreFast(fMapMat, contours[i]);
        if (score < boxScoreThresh)
            continue;
        //---use clipper start---
        std::vector<cv::Point> clipBox = unClip(minBox, perimeter, unClipRatio);
        std::vector<cv::Point> clipMinBox = getMinBoxes(clipBox, minSideLen, perimeter);
        //---use clipper end---

        if (minSideLen < minArea + 2)
            continue;

        for (int j = 0; j < clipMinBox.size(); ++j)
        {
            clipMinBox[j].x = (clipMinBox[j].x / 1.0);
            clipMinBox[j].x = (std::min)((std::max)(clipMinBox[j].x, 0), norfMapMat.cols);

            clipMinBox[j].y = (clipMinBox[j].y / 1.0);
            clipMinBox[j].y = (std::min)((std::max)(clipMinBox[j].y, 0), norfMapMat.rows);
        }

        rsBoxes.emplace_back(TextBox{clipMinBox, score});
    }
    reverse(rsBoxes.begin(), rsBoxes.end());

    return rsBoxes;
}

std::vector<TextBox> getTextBoxes(__ocr ocr, const cv::Mat &src, float boxScoreThresh, float boxThresh, float unClipRatio)
{
    int width = src.cols;
    int height = src.rows;
    int target_size = 640;
    // pad to multiple of 32
    int w = width;
    int h = height;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)target_size / w;
        w = target_size;
        h = h * scale;
    }
    else
    {
        scale = (float)target_size / h;
        h = target_size;
        w = w * scale;
    }

    ncnn::Mat input = ncnn::Mat::from_pixels_resize(src.data, ncnn::Mat::PIXEL_RGB, width, height, w, h);

    // pad to target_size rectangle
    int wpad = (w + 31) / 32 * 32 - w;
    int hpad = (h + 31) / 32 * 32 - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(input, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT, 0.f);

    const float meanValues[3] = {0.485 * 255, 0.456 * 255, 0.406 * 255};
    const float normValues[3] = {1.0 / 0.229 / 255.0, 1.0 / 0.224 / 255.0, 1.0 / 0.225 / 255.0};

    in_pad.substract_mean_normalize(meanValues, normValues);
    ncnn::Extractor extractor = ocr->dbNet.create_extractor();

    extractor.input("input0", in_pad);
    ncnn::Mat out;
    extractor.extract("out1", out);

    cv::Mat fMapMat(in_pad.h, in_pad.w, CV_32FC1, (float *)out.data);
    cv::Mat norfMapMat;
    norfMapMat = fMapMat > boxThresh;

    cv::dilate(norfMapMat, norfMapMat, cv::Mat(), cv::Point(-1, -1), 1);

    std::vector<TextBox> result = findRsBoxes(fMapMat, norfMapMat, boxScoreThresh, 2.0f);
    for (int i = 0; i < result.size(); i++)
    {
        for (int j = 0; j < result[i].boxPoint.size(); j++)
        {
            float x = (result[i].boxPoint[j].x - (wpad / 2)) / scale;
            float y = (result[i].boxPoint[j].y - (hpad / 2)) / scale;
            x = std::max(std::min(x, (float)(width - 1)), 0.f);
            y = std::max(std::min(y, (float)(height - 1)), 0.f);
            result[i].boxPoint[j].x = x;
            result[i].boxPoint[j].y = y;
        }
    }

    return result;
}

template <class ForwardIterator>
inline static size_t argmax(ForwardIterator first, ForwardIterator last)
{
    return std::distance(first, std::max_element(first, last));
}

TextLine scoreToTextLine(__ocr ocr, const std::vector<float> &outputData, int h, int w)
{
    int keySize = ocr->keys.size();
    std::string strRes;
    std::vector<float> scores;
    int lastIndex = 0;
    int maxIndex;
    float maxValue;

    for (int i = 0; i < h; i++)
    {
        maxIndex = 0;
        maxValue = -1000.f;

        maxIndex = int(argmax(outputData.begin() + i * w, outputData.begin() + i * w + w));
        maxValue = float(*std::max_element(outputData.begin() + i * w, outputData.begin() + i * w + w)); // / partition;
        if (maxIndex > 0 && maxIndex < keySize && (!(i > 0 && maxIndex == lastIndex)))
        {
            scores.emplace_back(maxValue);
            strRes.append(ocr->keys[maxIndex - 1]);
        }
        lastIndex = maxIndex;
    }
    return {strRes, scores};
}

TextLine getTextLine(__ocr ocr, const cv::Mat &src)
{
    float scale = (float)ocr->dstHeight / (float)src.rows;
    int dstWidth = int((float)src.cols * scale);

    cv::Mat srcResize;
    cv::resize(src, srcResize, cv::Size(dstWidth, ocr->dstHeight));
    // if you use PP-OCRv3 you should change PIXEL_RGB to PIXEL_RGB2BGR
    ncnn::Mat input = ncnn::Mat::from_pixels(srcResize.data, ncnn::Mat::PIXEL_RGB, srcResize.cols, srcResize.rows);
    const float mean_vals[3] = {127.5, 127.5, 127.5};
    const float norm_vals[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
    input.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor extractor = ocr->crnnNet.create_extractor();
    // extractor.set_num_threads(2);
    extractor.input("input", input);

    ncnn::Mat out;
    extractor.extract("out", out);
    float *floatArray = (float *)out.data;
    std::vector<float> outputData(floatArray, floatArray + out.h * out.w);
    TextLine res = scoreToTextLine(ocr, outputData, out.h, out.w);
    return res;
}

std::vector<TextLine> getTextLines(__ocr ocr, std::vector<cv::Mat> &partImg)
{
    int size = partImg.size();
    std::vector<TextLine> textLines(size);
    for (int i = 0; i < size; ++i)
    {
        TextLine textLine = getTextLine(ocr, partImg[i]);
        textLines[i] = textLine;
    }
    return textLines;
}

bool compareBoxWidth(const TextBox &a, const TextBox &b)
{
    return abs(a.boxPoint[0].x - a.boxPoint[1].x) > abs(b.boxPoint[0].x - b.boxPoint[1].x);
}

cv::Mat getRotateCropImage(const cv::Mat &src, std::vector<cv::Point> box)
{
    cv::Mat image;
    src.copyTo(image);
    std::vector<cv::Point> points = box;

    int collectX[4] = {box[0].x, box[1].x, box[2].x, box[3].x};
    int collectY[4] = {box[0].y, box[1].y, box[2].y, box[3].y};
    int left = int(*std::min_element(collectX, collectX + 4));
    int right = int(*std::max_element(collectX, collectX + 4));
    int top = int(*std::min_element(collectY, collectY + 4));
    int bottom = int(*std::max_element(collectY, collectY + 4));

    cv::Mat imgCrop;
    image(cv::Rect(left, top, right - left, bottom - top)).copyTo(imgCrop);

    for (int i = 0; i < points.size(); i++)
    {
        points[i].x -= left;
        points[i].y -= top;
    }

    int imgCropWidth = int(sqrt(pow(points[0].x - points[1].x, 2) +
                                pow(points[0].y - points[1].y, 2)));
    int imgCropHeight = int(sqrt(pow(points[0].x - points[3].x, 2) +
                                 pow(points[0].y - points[3].y, 2)));

    cv::Point2f ptsDst[4];
    ptsDst[0] = cv::Point2f(0., 0.);
    ptsDst[1] = cv::Point2f(imgCropWidth, 0.);
    ptsDst[2] = cv::Point2f(imgCropWidth, imgCropHeight);
    ptsDst[3] = cv::Point2f(0.f, imgCropHeight);

    cv::Point2f ptsSrc[4];
    ptsSrc[0] = cv::Point2f(points[0].x, points[0].y);
    ptsSrc[1] = cv::Point2f(points[1].x, points[1].y);
    ptsSrc[2] = cv::Point2f(points[2].x, points[2].y);
    ptsSrc[3] = cv::Point2f(points[3].x, points[3].y);

    cv::Mat M = cv::getPerspectiveTransform(ptsSrc, ptsDst);

    cv::Mat partImg;
    cv::warpPerspective(imgCrop, partImg, M,
                        cv::Size(imgCropWidth, imgCropHeight),
                        cv::BORDER_REPLICATE);

    if (float(partImg.rows) >= float(partImg.cols) * 1.5)
    {
        cv::Mat srcCopy = cv::Mat(partImg.rows, partImg.cols, partImg.depth());
        cv::transpose(partImg, srcCopy);
        cv::flip(srcCopy, srcCopy, 0);
        return srcCopy;
    }
    else
    {
        return partImg;
    }
}

std::vector<cv::Mat> getPartImages(const cv::Mat &src, std::vector<TextBox> &textBoxes)
{
    std::sort(textBoxes.begin(), textBoxes.end(), compareBoxWidth);
    std::vector<cv::Mat> partImages;
    if (textBoxes.size() > 0)
    {
        for (int i = 0; i < textBoxes.size(); ++i)
        {
            cv::Mat partImg = getRotateCropImage(src, textBoxes[i].boxPoint);
            partImages.emplace_back(partImg);
        }
    }

    return partImages;
}

char *readKeys(const char *keyfile)
{
    FILE *fp;
    fp = fopen(keyfile, "rt");
    if (fp == NULL)
    {
        std::cout << "[OCR]Err Read Keys Failed" << std::endl;
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);

    char *ar = (char *)malloc(sizeof(char) * size);
    fread(ar, 1, size, fp);

    fclose(fp);
    return ar;
}

extern "C" __ocr __declspec(dllexport) __stdcall ocr_InitPaddleOcr(const char *dbNet_param, const char *crnnNet_param, const char *dbNet_bin, const char *crnnNet_bin, const char *keylist, const int dstHeight, const bool use_vulkan)
{
    __ocr ocr = new _ocr;

    ocr->dstHeight = dstHeight;

    ncnn::Option opt;
    opt.lightmode = true;
    opt.num_threads = 4;
    opt.blob_allocator = &ocr->g_blob_pool_allocator;
    opt.workspace_allocator = &ocr->g_workspace_pool_allocator;
    opt.use_packing_layout = true;

    opt.use_vulkan_compute = use_vulkan;

    ocr->dbNet.opt = opt;
    ocr->crnnNet.opt = opt;

    // init param
    {
        int ret = ocr->dbNet.load_param(dbNet_param);
        if (ret != 0)
        {
            std::cout << "[OCR]Err Read dbNet_param Failed" << std::endl;
            delete ocr;
            return NULL;
        }

        ret = ocr->crnnNet.load_param(crnnNet_param);

        if (ret != 0)
        {
            std::cout << "[OCR]Err Read crnnNet_param Failed" << std::endl;
            delete ocr;
            return NULL;
        }
    }

    // init bin
    {
        int ret = ocr->dbNet.load_model(dbNet_bin);
        if (ret != 0)
        {
            std::cout << "[OCR]Err Read dbNet_bin Failed" << std::endl;
            delete ocr;
            return NULL;
        }

        ret = ocr->crnnNet.load_model(crnnNet_bin);

        if (ret != 0)
        {
            std::cout << "[OCR]Err Read crnnNet_bin Failed" << std::endl;
            delete ocr;
            return NULL;
        }
    }

    // load keys
    char *buffer = readKeys(keylist);
    if (buffer != NULL)
    {
        std::istringstream inStr(buffer);
        std::string line;
        int size = 0;
        while (getline(inStr, line))
        {
            ocr->keys.emplace_back(line);
            size++;
        }
        free(buffer);
    }
    else
    {
        delete ocr;
        return NULL;
    }
    return ocr;
}

struct Object
{
    cv::Rect_<int> box;
    float score;
    char *Text;
};

extern "C" int __declspec(dllexport) __stdcall ocr_Deal(__ocr ocr, unsigned char *img_src, int img_size, Object *ret) // ret要开大来
{
    cv::_InputArray pic_arr(img_src, img_size);
    cv::Mat src_mat = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);

    ncnn::Mat in = ncnn::Mat::from_pixels(src_mat.data, ncnn::Mat::PIXEL_BGR2RGB, src_mat.cols, src_mat.rows);

    cv::Mat rgb = cv::Mat::zeros(in.h, in.w, CV_8UC3);
    in.to_pixels(rgb.data, ncnn::Mat::PIXEL_RGB);

    std::vector<TextBox> objects;
    objects = getTextBoxes(ocr, rgb, 0.4, 0.3, 2.0);

    std::vector<cv::Mat> partImages = getPartImages(rgb, objects);
    std::vector<TextLine> textLines = getTextLines(ocr, partImages);

    if (textLines.size() > 0)
    {
        for (int i = 0; i < textLines.size(); i++)
            objects[i].text = textLines[i].text;
    }
    int count = objects.size();
    for (int i = 0; i < count; i++)
    {
        ret[i].box.x = objects[i].boxPoint[0].x;
        ret[i].box.y = objects[i].boxPoint[0].y;

        ret[i].box.width = objects[i].boxPoint[2].x - objects[i].boxPoint[0].x;
        ret[i].box.height = objects[i].boxPoint[2].y - objects[i].boxPoint[0].y;

        memcpy(ret[i].Text, objects[i].text.c_str(), objects[i].text.size());
        ret[i].Text[objects[i].text.size()] = '\0';

        ret[i].score = objects[i].score;
    }
    return count;
}

extern "C" void __declspec(dllexport) __stdcall ocr_Destroy(__ocr ocr)
{
    delete ocr;
}

#endif