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

#ifndef LIBSMARTEREYE2_DEVICE_TYPES_HPP
#define LIBSMARTEREYE2_DEVICE_TYPES_HPP

// Stereo camera’s intrinsic and extrinsic params.
struct Intrinsics {
  int width;     /**< Width of the image in pixels */
  int height;    /**< Height of the image in pixels */
  float ppx;     /**< Horizontal coordinate of the principal point of the image, as a pixel offset from the left edge */
  float ppy;     /**< Vertical coordinate of the principal point of the image, as a pixel offset from the top edge */
  float fx;      /**< Focal length of the image plane, as a multiple of pixel width */
  float fy;      /**< Focal length of the image plane, as a multiple of pixel height */
  float coeffs[5]; /**< Distortion coefficients */
};

struct Extrinsics {
  float rotation[9];
  float translation[3];
};

struct MotionDeviceIntrinsics {
  /* \internal
  * Scale X       cross axis  cross axis  Bias X \n
  * cross axis    Scale Y     cross axis  Bias Y \n
  * cross axis    cross axis  Scale Z     Bias Z */
  float data[3][4];          /**< Interpret data array values */

  float noise_variances[3];  /**< Variance of noise for X, Y, and Z axis */
  float bias_variances[3];   /**< Variance of bias for X, Y, and Z axis */
};

enum PlaybackStatus {
  PLAYBACK_STATUS_UNKNOWN, /**< Unknown state */
  PLAYBACK_STATUS_PLAYING, /**< One or more sensors were started, playback is reading and raising data */
  PLAYBACK_STATUS_PAUSED,  /**< One or more sensors were started, but playback paused reading and paused raising data*/
  PLAYBACK_STATUS_STOPPED, /**< All sensors were stopped, or playback has ended (all data was read). This is the initial playback status*/
  PLAYBACK_STATUS_COUNT
};

#endif //LIBSMARTEREYE2_DEVICE_TYPES_HPP
