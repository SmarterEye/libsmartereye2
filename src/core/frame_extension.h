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

#ifndef LIBSMARTEREYE2_FRAME_EXTENSION_H
#define LIBSMARTEREYE2_FRAME_EXTENSION_H

#include <array>

namespace libsmartereye2 {

using se_time_t = double;
static const size_t kMaxMetadataSize(0xff);

enum class TimestampDomain {
  HARDWARE_CLOCK, /**< Frame timestamp was measured in relation to the camera clock */
  SYSTEM_TIME,    /**< Frame timestamp was measured in relation to the OS system clock */
  GLOBAL_TIME,    /**< Frame timestamp was measured in relation to the camera clock and converted to OS system clock by constantly measure the difference*/
  COUNT
};

enum class FrameMetadataValue {
  FRAME_COUNTER, /**< A sequential index managed per-stream. Integer value*/
  FRAME_TIMESTAMP, /**< Timestamp set by device clock when data readout and transmit commence. usec*/
  SENSOR_TIMESTAMP, /**< Timestamp of the middle of sensor's exposure calculated by device. usec*/
  ACTUAL_EXPOSURE, /**< Sensor's exposure width. When Auto Exposure (AE) is on the value is controlled by firmware. usec*/
  GAIN_LEVEL, /**< A relative value increasing which will increase the Sensor's gain factor. \
                                                              When AE is set On, the value is controlled by firmware. Integer value*/
  AUTO_EXPOSURE, /**< Auto Exposure Mode indicator. Zero corresponds to AE switched off. */
  WHITE_BALANCE, /**< White Balance setting as a color temperature. Kelvin degrees*/
  TIME_OF_ARRIVAL, /**< Time of arrival in system clock */
  TEMPERATURE, /**< Temperature of the device, measured at the time of the frame capture. Celsius degrees */
  BACKEND_TIMESTAMP, /**< Timestamp get from uvc driver. usec*/
  ACTUAL_FPS, /**< Actual fps */
  FRAME_LASER_POWER, /**< Laser power value 0-360. */
  FRAME_LASER_POWER_MODE, /**< Laser power mode. Zero corresponds to Laser power switched off and one for switched on. deprecated, replaced by FRAME_EMITTER_MODE*/
  EXPOSURE_PRIORITY, /**< Exposure priority. */
  EXPOSURE_ROI_LEFT, /**< Left region of interest for the auto exposure Algorithm. */
  EXPOSURE_ROI_RIGHT, /**< Right region of interest for the auto exposure Algorithm. */
  EXPOSURE_ROI_TOP, /**< Top region of interest for the auto exposure Algorithm. */
  EXPOSURE_ROI_BOTTOM, /**< Bottom region of interest for the auto exposure Algorithm. */
  BRIGHTNESS, /**< Color image brightness. */
  CONTRAST, /**< Color image contrast. */
  SATURATION, /**< Color image saturation. */
  SHARPNESS, /**< Color image sharpness. */
  AUTO_WHITE_BALANCE_TEMPERATURE, /**< Auto white balance temperature Mode indicator. Zero corresponds to automatic mode switched off. */
  BACKLIGHT_COMPENSATION, /**< Color backlight compensation. Zero corresponds to switched off. */
  HUE, /**< Color image hue. */
  GAMMA, /**< Color image gamma. */
  MANUAL_WHITE_BALANCE, /**< Color image white balance. */
  POWER_LINE_FREQUENCY, /**< Power Line Frequency for anti-flickering Off/50Hz/60Hz/Auto. */
  LOW_LIGHT_COMPENSATION, /**< Color lowlight compensation. Zero corresponds to switched off. */
  FRAME_EMITTER_MODE, /**< Emitter mode: 0 - all emitters disabled. 1 - laser enabled. 2 - auto laser enabled (opt). 3 - LED enabled (opt).*/
  FRAME_LED_POWER, /**< Led power value 0-360. */
  RAW_FRAME_SIZE, /**< The number of transmitted payload bytes, not including metadata */
  COUNT
};

struct FrameExtension {
  se_time_t timestamp = 0;
  uint64_t frame_number = 0;
  TimestampDomain timestamp_domain = TimestampDomain::HARDWARE_CLOCK;
  se_time_t system_time = 0; // sys-clock at the time the frame was received from the backend
  se_time_t frame_callback_started = 0; // time when the frame was sent to user callback
  uint32_t metadata_size = 0;
  std::array<uint8_t, kMaxMetadataSize> metadata_blob;
  se_time_t backend_timestamp = 0; // time when the frame arrived to the backend (OS dependent)
  se_time_t last_timestamp = 0;
  uint64_t last_frame_number = 0;
  bool is_blocking = false; // when running from recording, this bit indicates
  uint32_t raw_size = 0;   // The frame transmitted size (payload only)
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_EXTENSION_H
