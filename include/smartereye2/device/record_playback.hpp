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

#ifndef LIBSMARTEREYE2_RECORD_PLAYBACK_HPP
#define LIBSMARTEREYE2_RECORD_PLAYBACK_HPP

#include "device.hpp"
#include "device_types.hpp"
#include "se_callbacks.hpp"

#include <utility>

namespace se2 {

template<class T>
class StatusChangedCallback : public SePlaybackChangedCallback {
  T on_status_changed_function_;
 public:
  explicit StatusChangedCallback(T on_status_changed) : on_status_changed_function_(on_status_changed) {}

  void onPlaybackStatusChanged(PlaybackStatus status) override {
    on_status_changed_function_(status);
  }

  void release() override { delete this; }
};

class Playback : public Device {
 public:
  explicit Playback(Device device) {}

  void pause();
  void resume();
  void stop();
  void seek();

  std::string fileName() const { return file_; }
  uint64_t position() const;
  std::chrono::nanoseconds duration() const;

  bool isRealTime() const;
  void setRealTime(bool is_real_time);
  void setPlaybackSpeed(float speed);

  template<class T>
  void setStatusChangedCallback();

  PlaybackStatus currentStatus() const;

 protected:
  friend class Context;
  explicit Playback(std::shared_ptr<SeDevice> dev) : Device(std::move(dev)) {}

 private:
  std::string file_;
};

class Recorder : public Device {
 public:
  explicit Recorder(const Device& device) : Recorder(device.get()) {}

  Recorder(const std::string &file, const Device& dev) {}

 protected:
  explicit Recorder(std::shared_ptr<SeDevice> dev) : Device(std::move(dev)) {}
};

}  // namespace se2

#endif //LIBSMARTEREYE2_RECORD_PLAYBACK_HPP
