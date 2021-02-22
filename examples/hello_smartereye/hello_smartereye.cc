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
#include <utility>

// algorithm output
#include <smartereye2/alg/obstacleData.h>

//#define GET_SYNCED_FRAME

using namespace se2;

#ifndef GET_SYNCED_FRAME
void handleCurrentFrame(const se2::Frame &current_frame) {
  auto frameId = current_frame.getProfile().frameId();

  switch (frameId) {
    case FrameId::LeftCamera: {
      auto ori_left_color = se2::VideoFrame(current_frame); // 0
      cv::Mat ori_left_yuv(ori_left_color.height(), ori_left_color.width(), CV_8UC2, (void *) current_frame.data());
      cv::Mat ori_left_bgr(ori_left_yuv.rows, ori_left_yuv.cols, CV_8UC3);
      cv::cvtColor(ori_left_yuv, ori_left_bgr, cv::COLOR_YUV2BGR_YUYV); // yuv422
      cv::imshow("ori_left", ori_left_bgr);
    }
      break;
    case FrameId::RightCamera: {
      auto ori_right_gray = se2::VideoFrame(current_frame); // 1
      cv::Mat ori_right_mat(ori_right_gray.height(), ori_right_gray.width(), CV_8UC1, (void *) ori_right_gray.data());
      cv::imshow("ori_right", ori_right_mat);
    }
      break;
    case FrameId::Disparity: {
      auto disparity = se2::VideoFrame(current_frame);  // 4
      cv::Mat disparity_mat(disparity.height(), disparity.width(), CV_16U, (void *) disparity.data());
      cv::normalize(disparity_mat, disparity_mat, 0, 255, cv::NORM_MINMAX, CV_8U);
      cv::imshow("disparity", disparity_mat);
    }
      break;
    case FrameId::Obstacle: {
      auto obstacle_frame = se2::ObstacleFrame(current_frame);
//      std::cout << "obs num: " << obstacle_frame.num() << " : " << std::endl;
//      for (int i = 0; i < obstacle_frame.num(); i++) {
//        auto obs = obstacle_frame.obstacles()[i];
//        std::cout << (int)obs->trackId << " -- " << obs->avgDistanceZ << std::endl;
//      }
    }
      break;
    default:break;
  }
}
#endif

int main(int argc, char *argv[]) {
  cv::namedWindow("ori_left");
  cv::namedWindow("ori_right");
  cv::namedWindow("disparity");

  // Create a Pipeline - this serves as a top-level API for streaming and processing frames
  se2::Pipeline pipeline;

  // Configure and start the pipeline
#ifdef GET_SYNCED_FRAME
  pipeline.start();
#else
  auto frame_handler_func = [](SeFrame *frame) {
    se2::Frame current_frame(frame);
    handleCurrentFrame(current_frame);
  };
  FrameCallbackPtr frame_callback_ptr(new FramCallback<decltype(frame_handler_func)>(frame_handler_func));
  pipeline.start(frame_callback_ptr);
#endif

  // Register device changed callback for getting device connection state
  auto dev = pipeline.getActiveProfile().getDevice();
  DevicesChangedCallbackPtr cb(new DevicesChangedCallback(
      [&](DeviceChangedEvent &changed_event) {
        if (changed_event.wasAdded(dev)) {
          std::cout << "current device connected!!!" << std::endl;
        } else if (changed_event.wasRemoved(dev)) {
          std::cout << "current device disconnected!!!" << std::endl;
        }
      }
  ));
  auto dev_cb_id = pipeline.registerInternalDeviceCallback(cb);

  int k = 0;
  while (k != 27) {
#ifdef GET_SYNCED_FRAME
    // Block program until frames arrive
    se2::FrameSet frames = pipeline.waitForFrames();

    if (!frames) continue;

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
#endif

    k = cv::waitKey(1);
  }

  pipeline.unregisterDevicesChangedCallback(dev_cb_id);
  pipeline.stop();
  cv::destroyAllWindows();

  return 0;
}
