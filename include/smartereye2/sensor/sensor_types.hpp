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

#ifndef LIBSMARTEREYE2_SENSOR_TYPES_HPP
#define LIBSMARTEREYE2_SENSOR_TYPES_HPP

enum NotificationCategory {
  NOTIFICATION_CATEGORY_FRAMES_TIMEOUT,               /**< Frames didn't arrived within 5 seconds */
  NOTIFICATION_CATEGORY_FRAME_CORRUPTED,              /**< Received partial/incomplete frame */
  NOTIFICATION_CATEGORY_HARDWARE_ERROR,               /**< Error reported from the device */
  NOTIFICATION_CATEGORY_HARDWARE_EVENT,               /**< General Hardeware notification that is not an error */
  NOTIFICATION_CATEGORY_UNKNOWN_ERROR,                /**< Received unknown error from the device */
  NOTIFICATION_CATEGORY_FIRMWARE_UPDATE_RECOMMENDED,  /**< Current firmware version installed is not the latest available */
  NOTIFICATION_CATEGORY_POSE_RELOCALIZATION,          /**< A relocalization event has updated the pose provided by a pose sensor */
  NOTIFICATION_CATEGORY_COUNT                         /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
};

#endif //LIBSMARTEREYE2_SENSOR_TYPES_HPP
