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

#include "se_util.h"

namespace libsmartereye2 {

struct SeVector3f {
  float x, y, z;
};

class OptionsInterface;
class ContextPrivate;
class DeviceInfo;
class DeviceInterface;
class DeviceHubPrivate;
class PipelineProfilePrivate;
class PipelinePrivate;

struct SeContext {
  std::shared_ptr<ContextPrivate> ctx;
};

struct SeOptions {
  explicit SeOptions(OptionsInterface *options) : options_interface(options) {}
  OptionsInterface *options_interface;
};

struct SeDevice {
  std::shared_ptr<ContextPrivate> ctx;
  std::shared_ptr<DeviceInfo> info;
  std::shared_ptr<DeviceInterface> device;
};

struct SeDeviceInfo {
  std::shared_ptr<ContextPrivate> ctx;
  std::shared_ptr<DeviceInfo> info;
};

struct SeDeviceList {
  std::shared_ptr<ContextPrivate> ctx;
  std::vector<SeDeviceInfo> list;
};

struct SeDeviceHub {
  std::shared_ptr<DeviceHubPrivate> hub;
};

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
      throw std::exception("Trying to return item to a heap that didn't allocate it!");
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
      throw std::exception("Could not flush one of the user controlled objects!");
    }
  }

  bool empty() const { return size_ == 0; }
  int size() const { return size_; }
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_SE_TYPES_H
