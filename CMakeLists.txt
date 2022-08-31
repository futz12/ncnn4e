cmake_minimum_required(VERSION 3.23.0)
project(ncnn4e VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(include)

aux_source_directory(./src DIR_SRCS)

# OpenCV
set(BUILD_SHARED_LIBS false)
include(${CMAKE_CURRENT_SOURCE_DIR}/opencv-static/OpenCVWrapperConfig.cmake)
find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
    message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
    message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "opencv Not Found!")
endif (OpenCV_FOUND)

# NCNN
include(${CMAKE_CURRENT_SOURCE_DIR}/ncnn-vulkan-static/NcnnWrapperConfig.cmake)

find_package(ncnn REQUIRED)
if (ncnn_FOUND)
    message(STATUS "ncnn Found!")
else ()
    message(FATAL_ERROR "ncnn Not Found!")
endif (ncnn_FOUND)

add_library(ncnn4e SHARED ${DIR_SRCS})

target_precompile_headers(ncnn4e PRIVATE include/pch.h)
target_link_libraries(ncnn4e ${OpenCV_LIBS})
target_link_libraries(ncnn4e ${NCNN_LIBS})
#target_link_libraries(ncnn4e libopenjp2 libjpeg-turbo libpng libtiff libwebp opencv_core460 opencv_features2d460 opencv_highgui460 opencv_imgproc460 opencv_photo460 opencv_video460)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
