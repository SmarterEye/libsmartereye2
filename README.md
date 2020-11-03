# libsmartereye2

[![License](https://img.shields.io/badge/License-Apache%202.0-green.svg)](https://github.com/SmarterEye/libsmartereye2/blob/master/LICENSE)
[![Language](https://img.shields.io/badge/language-c++-red.svg)](https://en.cppreference.com/)
[![Platform](https://img.shields.io/badge/platform-linux%20%7C%20windows-lightgrey.svg)]()

**SmarterEye SDK 2.0** 是一个跨平台的C++库，用于中科慧眼双子座平台的双目相机，提供接口从相机获取图像、相机参数等数据。
同时也能获取处理过后的数据（点云，深度图渲染等）。

## Installation

**SmarterEye SDK 2.0** 通过CMake进行构建，使用方法请参考 [使用教程](docs/build.md)。

## Documentation

说明文档 [here](docs/README.md)。

### Sample

```c++
// Create a Pipeline - this serves as a top-level API for streaming and processing frames
se2::Pipeline p;

// Configure and start the pipeline
p.start();

int k = 0;
cv::namedWindow("calib_left");
while (k != 27) {
// Block program until frames arrive
se2::FrameSet frames = p.waitForFrames();

// Try to get frames
auto calib_left_color = frames.getVideoFrame(FrameId::CalibLeftCamera);

if (!calib_left_color) {
  continue;
}

// calib left
cv::Mat calib_left_yuv(calib_left_color.height(), calib_left_color.width(), CV_8UC2,
                       (void *) (calib_left_color.data()));
cv::Mat calib_left_bgr(calib_left_yuv.rows, calib_left_yuv.cols, CV_8UC3);
cv::cvtColor(calib_left_yuv, calib_left_bgr, cv::COLOR_YUV2BGR_UYVY); // yuv422p

cv::imshow("calib_left", calib_left_bgr);
k = cv::waitKey(80);
}

p.stop();
cv::destroyAllWindows();
```

更多示例代码可以参考 [examples](examples)。

### Q&A

常见问题参考 [此处](docs/qa.md)。

### License

This project is licensed under the [Apache License, Version 2.0](LICENSE).
Copyright 2020 Smarter Eye Co.,Ltd. All Rights Reserved.
