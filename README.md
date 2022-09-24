# ncnn4e
关于NCNN常见模型便于C的封装

当前最新版本~1.2.0b~

交流Q群：904511841
点击链接加入群聊【ZMPI Development Insider】：https://jq.qq.com/?_wv=1027&k=17AMp3X1

# V1.2.0b更新日志
## 特别
1. 改变项目为sln项目
2. 改变了Yolov4/v7的调用API请注意

## 功能
1. 百度飞桨Ocr
2. Yolov7
3. MoveNet(多对象肢体识别)
4. 人脸检测
5. 人脸特征提取与识别
6. Yolov4
7. pHash&Match

## 注意
项目使用以下项目的二进制文件或是源码，由于各部分开源协议不一致，请谨慎使用

1. https://github.com/nihui/ncnn
2. https://github.com/FeiGeChuanShu/ncnn_paddleocr
3. https://github.com/DayBreak-u/chineseocr_lite/tree/onnx/cpp_projects/OcrLiteNcnn
4. https://github.com/frotms/PaddleOCR2Pytorch
5. https://github.com/PaddlePaddle/PaddleOCR#PP-OCRv2
6. https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.5
7. https://github.com/WongKinYiu/yolov7
8. https://github.com/FeiGeChuanShu/ncnn_Android_MoveNet
9. https://github.com/nihui/ncnn-android-nanodet
10. Google MoveNet
11. https://github.com/FeiGeChuanShu/ncnn_Android_blazeface
12. https://github.com/WegenPan/faceRecognition-jna
13. https://github.com/Tianxiaomo/pytorch-YOLOv4
## 已知BUG
1. yolov4开始vulkan可能有bug
2. ~ UP没有往头文件写东西 ~
3. Ocr可能会因为ex缓存过大而崩溃，目前仅仅只完成了初步的修复，建议定期销毁Ocr实例

## To Do
1. 易语言采用wow64方法调用64bit的dll，以便支持更大的内存
2. 修复现有bug
3. 语义分割
4. Yolov5

================这是一条分割线==================

# V1.1.1a更新日志
## 功能
1. 百度飞桨Ocr
2. Yolov7
3. MoveNet(多对象肢体识别)
4. 人脸检测
5. 人脸特征提取与识别
6. Yolov4

## 注意
项目使用以下项目的二进制文件或是源码，由于各部分开源协议不一致，请谨慎使用

1. https://github.com/nihui/ncnn
2. https://github.com/FeiGeChuanShu/ncnn_paddleocr
3. https://github.com/DayBreak-u/chineseocr_lite/tree/onnx/cpp_projects/OcrLiteNcnn
4. https://github.com/frotms/PaddleOCR2Pytorch
5. https://github.com/PaddlePaddle/PaddleOCR#PP-OCRv2
6. https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.5
7. https://github.com/WongKinYiu/yolov7
8. https://github.com/FeiGeChuanShu/ncnn_Android_MoveNet
9. https://github.com/nihui/ncnn-android-nanodet
10. Google MoveNet
11. https://github.com/FeiGeChuanShu/ncnn_Android_blazeface
12. https://github.com/WegenPan/faceRecognition-jna
13. https://github.com/Tianxiaomo/pytorch-YOLOv4
## 已知BUG
1. yolov4开始vulkan可能有bug
2. ~UP没有往头文件写东西~

## To Do
1. 易语言采用wow64方法调用64bit的dll，以便支持更大的内存
2. 修复现有bug
3. 语义分割

================这是一条分割线==================

# V1.1a更新日志
## 功能
1. 百度飞桨Ocr
2. Yolov7
3. MoveNet(多对象肢体识别)
4. 人脸检测
5. 人脸特征提取与识别

## 注意
项目使用以下项目的二进制文件或是源码，由于各部分开源协议不一致，请谨慎使用

1. https://github.com/nihui/ncnn
2. https://github.com/FeiGeChuanShu/ncnn_paddleocr
3. https://github.com/DayBreak-u/chineseocr_lite/tree/onnx/cpp_projects/OcrLiteNcnn
4. https://github.com/frotms/PaddleOCR2Pytorch
5. https://github.com/PaddlePaddle/PaddleOCR#PP-OCRv2
6. https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.5
7. https://github.com/WongKinYiu/yolov7
8. https://github.com/FeiGeChuanShu/ncnn_Android_MoveNet
9. https://github.com/nihui/ncnn-android-nanodet
10. Google MoveNet
11. https://github.com/FeiGeChuanShu/ncnn_Android_blazeface
12. https://github.com/WegenPan/faceRecognition-jna

## 已知BUG
~不详~
~UP没有往头文件写东西~

## To Do
1. 易语言采用wow64方法调用64bit的dll，以便支持更大的内存
2. 修复现有bug
3. 语义分割

================这是一条分割线==================

# V1.0a更新日志
## 功能
1. 百度飞桨Ocr
2. Yolov7
3. MoveNet(多对象肢体识别)

## 注意
项目使用以下项目的二进制文件或是源码，由于各部分开源协议不一致，请谨慎使用

1. https://github.com/nihui/ncnn
2. https://github.com/FeiGeChuanShu/ncnn_paddleocr
3. https://github.com/DayBreak-u/chineseocr_lite/tree/onnx/cpp_projects/OcrLiteNcnn
4. https://github.com/frotms/PaddleOCR2Pytorch
5. https://github.com/PaddlePaddle/PaddleOCR#PP-OCRv2
6. https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.5
7. https://github.com/WongKinYiu/yolov7
8. https://github.com/FeiGeChuanShu/ncnn_Android_MoveNet
9. https://github.com/nihui/ncnn-android-nanodet
10. Google MoveNet

## 已知BUG
~不详~
~UP没有往头文件写东西~

## To Do
1. 统一模型的载入方法，和获取处理结果 (Y)
2. 引入人体姿态识别 (Y)
3. 易语言采用wow64方法调用64bit的dll，以便支持更大的内存
4. 修复现有bug

================这是一条分割线==================

# V0.99a更新日志
## 功能
1. 百度飞桨Ocr
2. Yolov7

## 已知bug
1. Ocr如果开启vulkan，将无法被销毁

## 注意
项目使用以下项目的二进制文件或是源码，由于各部分开源协议不一致，请谨慎使用

1. https://github.com/nihui/ncnn
2. https://github.com/FeiGeChuanShu/ncnn_paddleocr
3. https://github.com/DayBreak-u/chineseocr_lite/tree/onnx/cpp_projects/OcrLiteNcnn
4. https://github.com/frotms/PaddleOCR2Pytorch
5. https://github.com/PaddlePaddle/PaddleOCR#PP-OCRv2
6. https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.5
7. https://github.com/WongKinYiu/yolov7

## 编译教程
请您配置并安装以下SDK
1. Opencv
2. Ncnn
3. Vulkan

如果您是易语言用户请选择32Bit的相关SDK

## To Do
1. 统一模型的载入方法，和获取处理结果
2. 引入人体姿态识别
3. 易语言采用wow64方法调用64bit的dll，以便支持更大的内存
4. 修复现有bug
