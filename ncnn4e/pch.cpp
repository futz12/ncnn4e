// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"
//Opencv
#pragma comment(lib,"opencv_core460.lib")
#pragma comment(lib,"opencv_imgcodecs460.lib")
#pragma comment(lib,"opencv_imgproc460.lib")
#pragma comment(lib,"opencv_photo460.lib")
#pragma comment(lib,"opencv_features2d460.lib")
#pragma comment(lib,"opencv_highgui460.lib")
#pragma comment(lib,"opencv_video460.lib")
#pragma comment(lib,"zlib.lib")
#pragma comment(lib,"IlmImf.lib")
#pragma comment(lib,"libpng.lib")
#pragma comment(lib,"libwebp.lib")
#pragma comment(lib,"libtiff.lib")
#pragma comment(lib,"libopenjp2.lib")
#pragma comment(lib,"libjpeg-turbo.lib")

//NCNN
#pragma comment(lib,"ncnn.lib")

//Vulkan
#pragma comment(lib,"glslang.lib")
#pragma comment(lib,"vulkan-1.lib")
#pragma comment(lib,"SPIRV.lib")
#pragma comment(lib,"GenericCodeGen.lib")
#pragma comment(lib,"MachineIndependent.lib")
#pragma comment(lib,"OGLCompiler.lib")
#pragma comment(lib,"OSDependent.lib")

// 当使用预编译的头时，需要使用此源文件，编译才能成功。
