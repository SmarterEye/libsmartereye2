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

#ifndef LIBSMARTEREYE2_MOTION_H
#define LIBSMARTEREYE2_MOTION_H

#include "se_util.h"

namespace libsmartereye2 {

struct MotionDeviceIntrinsics {
  /* \internal
  * Scale X       cross axis  cross axis  Bias X \n
  * cross axis    Scale Y     cross axis  Bias Y \n
  * cross axis    cross axis  Scale Z     Bias Z */
  float data[3][4];          /**< Interpret data array values */

  float noise_variances[3];  /**< Variance of noise for X, Y, and Z axis */
  float bias_variances[3];   /**< Variance of bias for X, Y, and Z axis */
};

struct MotionData {
  double accel_x;
  double accel_y;
  double accel_z;
  double gyro_x;
  double gyro_y;
  double gyro_z;
  int64_t timestamp;
};

class Motion {

};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_MOTION_H
