#include <iostream>
#include <vector>
#include <map>

#include <ncnn/net.h>
#include <ncnn/layer.h>
#include <ncnn/c_api.h>
#include <opencv2/opencv.hpp>

#include <float.h>
#include <stdio.h>
#include <malloc.h>

#include <cstring>

#include <algorithm>
#include <functional>

#include "clipper.hpp"
#include "nanodet.h"

#include <ncnn/cpu.h>

#define YOLOV7
#define OCR
#define POSE
#define FACE
#define FEATURE