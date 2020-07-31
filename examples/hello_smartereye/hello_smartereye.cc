// Copyright 2020 Smarter Eye Co.,Ltd. All Rights Reserved.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <smartereye2/pipeline/pipeline.hpp>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[]) {
  // Create a Pipeline - this serves as a top-level API for streaming and processing frames
  se2::Pipeline p;

  // Configure and start the pipeline
  p.start();

  int k = 0;
  cv::namedWindow("calib_left");
  cv::namedWindow("calib_right");
  cv::namedWindow("ori_left");
  cv::namedWindow("ori_right");
  while (k != 27) {
    // Block program until frames arrive
    se2::FrameSet frames = p.waitForFrames();

    // Try to get a frames
    auto calib_left_color = frames.getVideoFrame(FrameId::CalibLeftCamera);
    auto calib_right_gray = frames[FrameId::CalibRightCamera];
    auto ori_left_color = frames.getVideoFrame(FrameId::LeftCamera);
    auto ori_right_gray = frames[FrameId::RightCamera];

    if (!calib_left_color || !calib_right_gray || !ori_left_color || !ori_right_gray) {
      continue;
    }

    // calib left
    cv::Mat calib_left_yuv(calib_left_color.height(), calib_left_color.width(), CV_8UC2, (void *) (calib_left_color.data()));
    cv::Mat calib_left_bgr(calib_left_yuv.rows, calib_left_yuv.cols, CV_8UC3);
    cv::cvtColor(calib_left_yuv, calib_left_bgr, cv::COLOR_YUV2BGR_YUYV);

    // calib right
    cv::Mat calib_right_mat(calib_right_gray.height(), calib_right_gray.width(), CV_8UC1, (void *) calib_right_gray.data());

    // ori left
    cv::Mat ori_left_yuv(ori_left_color.height(), ori_left_color.width(), CV_8UC2, (void *) ori_left_color.data());
    cv::Mat ori_left_bgr(ori_left_yuv.rows, ori_left_yuv.cols, CV_8UC3);
    cv::cvtColor(ori_left_yuv, ori_left_bgr, cv::COLOR_YUV2BGR_Y422);

    // ori right
    cv::Mat ori_right_mat(ori_right_gray.height(), ori_right_gray.width(), CV_8UC1, (void *) ori_right_gray.data());

    cv::imshow("calib_left", calib_left_bgr);
    cv::imshow("calib_right", calib_right_mat);
    cv::imshow("ori_left", ori_left_bgr);
    cv::imshow("ori_right", ori_right_mat);

    k = cv::waitKey(40);
  }

  cv::destroyAllWindows();

  return 0;
}
