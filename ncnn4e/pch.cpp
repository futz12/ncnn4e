// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"
//Opencv
#pragma comment(lib,"opencv_world460.lib")
#pragma comment(lib,"ippicvmt.lib")
#pragma comment(lib,"ippiw.lib")
#pragma comment(lib,"libopenjp2.lib")
#pragma comment(lib,"ittnotify.lib")
#pragma comment(lib,"libjpeg-turbo.lib")
#pragma comment(lib,"libpng.lib")
#pragma comment(lib,"libtiff.lib")
#pragma comment(lib,"libwebp.lib")
#pragma comment(lib,"zlib.lib")

#pragma comment(lib,"ade.lib")
#pragma comment(lib,"IlmImf.lib")


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
