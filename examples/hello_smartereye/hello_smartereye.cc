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

#include <smartereye2/pipeline/pipeline.hpp>
#include <opencv2/opencv.hpp>

using namespace se2;

int main(int argc, char *argv[]) {
  // Create a Pipeline - this serves as a top-level API for streaming and processing frames
  se2::Pipeline p;

  // Configure and start the pipeline
  p.start();

  int k = 0;
  cv::namedWindow("ori_left");
  cv::namedWindow("ori_right");
  cv::namedWindow("disparity");

  while (k != 27) {
    // Block program until frames arrive
    se2::FrameSet frames = p.waitForFrames();

    // Try to get a frames
    auto ori_left_color = frames.getVideoFrame(FrameId::LeftCamera); // 0
    auto ori_right_gray = frames[FrameId::RightCamera]; // 1
    auto disparity = frames[FrameId::Disparity];  // 4

    if (!ori_left_color || !ori_right_gray || !disparity) {
      break;
    }

    // ori left
    cv::Mat ori_left_yuv(ori_left_color.height(), ori_left_color.width(), CV_8UC2, (void *) ori_left_color.data());
    cv::Mat ori_left_bgr(ori_left_yuv.rows, ori_left_yuv.cols, CV_8UC3);
    cv::cvtColor(ori_left_yuv, ori_left_bgr, cv::COLOR_YUV2BGR_YUYV); // yuv422

    // ori right
    cv::Mat ori_right_mat(ori_right_gray.height(), ori_right_gray.width(), CV_8UC1, (void *) ori_right_gray.data());

    // disparity
    cv::Mat disparity_mat(disparity.height(), disparity.width(), CV_16U, (void *) disparity.data());
    cv::normalize(disparity_mat, disparity_mat, 0, 255, cv::NORM_MINMAX, CV_8U);

    cv::imshow("ori_left", ori_left_bgr);
    cv::imshow("ori_right", ori_right_mat);
    cv::imshow("disparity", disparity_mat);

    k = cv::waitKey(1);
  }

  p.stop();
  cv::destroyAllWindows();

  return 0;
}
