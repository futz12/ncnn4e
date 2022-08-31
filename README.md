# ncnn4e
关于NCNN常见模型便于C的封装

当前最新版本~0.99a~

== 当前还不是正式版本 ==

# V0.99a更新日志
## 功能
1. 百度飞桨Ocr
2. Yolov7

## 已知bug
1. Ocr如果开启vulkan，将无法被销毁

项目使用以下项目的二进制文件或是源码，由于各部分开源协议不一致，请谨慎使用

1. https://github.com/nihui/ncnn
2. https://github.com/FeiGeChuanShu/ncnn_paddleocr
3. https://github.com/DayBreak-u/chineseocr_lite/tree/onnx/cpp_projects/OcrLiteNcnn
4. https://github.com/frotms/PaddleOCR2Pytorch
5. https://github.com/PaddlePaddle/PaddleOCR#PP-OCRv2
6. https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.5
7. https://github.com/WongKinYiu/yolov7

# 编译教程
请您配置并安装以下SDK
1. Opencv
2. Ncnn
3. Vulkan

如果您是易语言用户请选择32Bit的相关SDK

# To Do
1. 统一模型的载入方法，和获取处理结果
2. 引入人体姿态识别
3. 易语言采用wow64方法调用64bit的dll，以便支持更大的内存
4. 修复现有bug
