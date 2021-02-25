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

#ifndef LIBSMARTEREYE2_SE_TYPES_H
#define LIBSMARTEREYE2_SE_TYPES_H

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

enum ProductCode {
  SE_PRODUCT_GEMINI = 0x01,
  SE_PRODUCT_ANY = 0xff
};

enum class SeExtension {
  EXTENSION_UNKNOWN,
  EXTENSION_VIDEO_FRAME,
  EXTENSION_MOTION_FRAME,
  EXTENSION_COMPOSITE_FRAME,
  EXTENSION_POINTS,
  EXTENSION_DEPTH_FRAME,
  EXTENSION_DISPARITY_FRAME,
  EXTENSION_POSE_FRAME,
  EXTENSION_JOURNEY_FRAME,
  EXTENSION_OBSTACLE_FRAME,
  EXTENSION_LANE_FRAME,
  EXTENSION_SMALL_OBS_FRAME
};

enum CameraInfo {
  CAMERA_INFO_NAME, /**< Friendly name */
  CAMERA_INFO_SERIAL_NUMBER, /**< Device serial number */
  CAMERA_INFO_FIRMWARE_VERSION, /**< Primary firmware version */
  CAMERA_INFO_PHYSICAL_PORT, /**< Unique identifier of the port the device is connected to (platform specific) */
  CAMERA_INFO_DEBUG_OP_CODE, /**< If device supports firmware logging, this is the command to send to get logs from firmware */
  CAMERA_INFO_PRODUCT_ID, /**< Product ID as reported in the USB descriptor */
  CAMERA_INFO_CAMERA_LOCKED, /**< True iff EEPROM is locked */
  CAMERA_INFO_USB_TYPE_DESCRIPTOR, /**< Designated USB specification: USB2/USB3 */
  CAMERA_INFO_PRODUCT_LINE, /**< Device product line D400/SR300/L500/T200 */
  CAMERA_INFO_ASIC_SERIAL_NUMBER, /**< ASIC serial number */
  CAMERA_INFO_IP_ADDRESS, /**< IP address for remote camera. */
  CAMERA_INFO_COUNT                            /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
};

struct SeVector3f {
  float x, y, z;
};

typedef struct SeDevice SeDevice;
typedef struct SeDeviceInfo SeDeviceInfo;
typedef struct SeDeviceList SeDeviceList;

typedef struct SeSensor SeSensor;
typedef struct SeNotification SeNotification;

typedef struct SeContext SeContext;
typedef struct SeOptions SeOptions;

namespace libsmartereye2 { class FrameInterface; }
typedef class libsmartereye2::FrameInterface SeFrame;

typedef struct SeFrameQueue SeFrameQueue;
typedef struct SeSyntheticSource SeSyntheticSource;

typedef struct SeStreamProfile SeStreamProfile;

typedef struct SePipeline SePipeline;
typedef struct SePipelineProfile SePipelineProfile;
typedef struct SePipelineConfig SePipelineConfig;

typedef struct SeProcessingBlock SeProcessingBlock;

template<class T, int C>
class SmallHeap {
  T buffer_[C];
  bool is_free_[C];
  std::mutex mutex_;
  bool keep_allocating_ = true;
  std::condition_variable cv_;
  int size_ = 0;

 public:
  static const int CAPACITY = C;

  SmallHeap() {
    for (auto i = 0; i < C; i++) {
      is_free_[i] = true;
      buffer_[i] = std::move(T());
    }
  }

  T *allocate() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!keep_allocating_) return nullptr;

    for (auto i = 0; i < C; i++) {
      if (is_free_[i]) {
        is_free_[i] = false;
        size_++;
        return &buffer_[i];
      }
    }
    return nullptr;
  }

  void deallocate(T *item) {
    if (item < buffer_ || item >= buffer_ + C) {
      throw std::runtime_error("Trying to return item to a heap that didn't allocate it!");
    }
    auto i = item - buffer_;
    auto old_value = std::move(buffer_[i]);
    buffer_[i] = std::move(T());

    {
      std::unique_lock<std::mutex> lock(mutex_);

      is_free_[i] = true;
      size_--;

      if (size_ == 0) {
        lock.unlock();
        cv_.notify_one();
      }
    }
  }

  void stopAllocating() {
    std::unique_lock<std::mutex> lock(mutex_);
    keep_allocating_ = false;
  }

  void waitUntilEmpty() {
    std::unique_lock<std::mutex> lock(mutex_);

    const auto ready = [this]() {
      return empty();
    };
    if (!ready() && !cv_.wait_for(lock,
                                  std::chrono::hours(1000),
                                  ready)) // for some reason passing std::chrono::duration::max makes it return instantly
    {
//      throw std::exception("Could not flush one of the user controlled objects!");
    }
  }

  bool empty() const { return size_ == 0; }
  int size() const { return size_; }
};

#endif  // LIBSMARTEREYE2_SE_TYPES_H
