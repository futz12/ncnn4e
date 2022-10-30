#define __AVX__ 1//Ç¿ÖÆ¿ªÆô
#define __SSE2__ 1

#include <iostream>
#include <vector>
#include <map>
#include <bitset>

#include <ncnn/net.h>
#include <ncnn/layer.h>
#include <ncnn/c_api.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

#include <float.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>

#include <cstring>

#include <algorithm>
#include <functional>

#include "clipper.hpp"
#include "nanodet.h"

#include <ncnn/cpu.h>

#include <immintrin.h>

#define YOLOV7
#define OCR
#define POSE
#define FACE
#define FEATURE
#define YOLOV4
#define YOLOV5
#define MATCH
#define SORT
#define FASTESTDET