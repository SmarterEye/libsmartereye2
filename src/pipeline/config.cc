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

#include "config.h"

#include <memory>
#include <utility>
#include "pipeline_profile.h"
#include "device/device.h"
#include "se_types.h"
#include "easylogging++.h"

namespace libsmartereye2 {

class PipelineConfigPrivate {
 public:
  PipelineConfigPrivate() = default;

  void enableStream(StreamType stream,
                    int index,
                    uint32_t width,
                    uint32_t height,
                    StreamFormat format,
                    uint32_t framerate) {
    std::lock_guard<std::mutex> lock(mutex_);
    resolved_profile_.reset();
    stream_requests_[{stream, index}] = StreamProfileConfig{format, stream, index, width, height, framerate};
  }

  void enableAllStreams() {
    std::lock_guard<std::mutex> lock(mutex_);
    resolved_profile_.reset();
    stream_requests_.clear();
    enable_all_streams_ = true;
  }

  void enableDevice(const std::string &serial) {
    std::lock_guard<std::mutex> lock(mutex_);
    resolved_profile_.reset();
    device_request_.serial = serial;
  }

  void enableDeviceFromFile(const std::string &file, bool repeat_playback) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (device_request_.record_output.empty()) {
      throw std::runtime_error("Configuring both device from file, and record to file is unsupported");
    }
    resolved_profile_.reset();
    device_request_.filename = file;
    playback_loop_ = repeat_playback;
  }

  void enableRecordToFile(const std::string &file) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (device_request_.record_output.empty()) {
      throw std::runtime_error("Configuring both device from file, and record to file is unsupported");
    }
    resolved_profile_.reset();
    device_request_.filename = file;
  }

  void disableStream(StreamType stream, int index = -1) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = stream_requests_.begin();
    while (iter != stream_requests_.end()) {
      if (iter->first.first == stream
          && (index == -1 || iter->first.second == index)) {
        iter = stream_requests_.erase(iter);
      } else {
        ++iter;
      }
    }
    resolved_profile_.reset();
  }

  void disableAllStreams() {
    std::lock_guard<std::mutex> lock(mutex_);
    stream_requests_.clear();
    enable_all_streams_ = false;
    resolved_profile_.reset();
  }

  std::shared_ptr<PipelineProfilePrivate> resolve(std::shared_ptr<PipelinePrivate> pipe,
                                                  const std::chrono::milliseconds &timeout = std::chrono::milliseconds(0)) {
    return nullptr;
  }

  bool canResolve(std::shared_ptr<PipelinePrivate> pipe) {
    try {
      resolve(std::move(pipe));
      resolved_profile_.reset();
    } catch (const std::exception &e) {
      LOG(DEBUG) << "Config can not be resolved. " << e.what();
      return false;
    } catch (...) {
      return false;
    }
    return true;
  }

  std::shared_ptr<PipelineProfilePrivate> getCachedResolvedProfile() {
    std::lock_guard<std::mutex> lock(mutex_);
    return resolved_profile_;
  }

  bool getRepeatPlayback() const {
    return playback_loop_;
  }

  PipelineConfigPrivate(const PipelineConfigPrivate &other) {
    device_request_ = other.device_request_;
    stream_requests_ = other.stream_requests_;
    enable_all_streams_ = other.enable_all_streams_;
    resolved_profile_ = nullptr;
    playback_loop_ = other.playback_loop_;
  }

 private:
  struct device_request {
    std::string serial;
    std::string filename;
    std::string record_output;
  };
  std::shared_ptr<DeviceInterface> get_or_add_playback_device(std::shared_ptr<ContextPrivate> ctx,
                                                              const std::string &file);
  std::shared_ptr<DeviceInterface> resolve_device_requests(std::shared_ptr<PipelinePrivate> pipe,
                                                           const std::chrono::milliseconds &timeout);
  StreamProfiles get_default_configuration(std::shared_ptr<DeviceInterface> dev);
  std::shared_ptr<PipelineProfilePrivate> resolve(std::shared_ptr<DeviceInterface> dev);

  device_request device_request_;
  std::map<std::pair<StreamType, int>, StreamProfileConfig> stream_requests_;
  std::mutex mutex_;
  bool enable_all_streams_ = false;
  std::shared_ptr<PipelineProfilePrivate> resolved_profile_;
  bool playback_loop_;
};

PipelineConfig::PipelineConfig() : config_(new PipelineConfigPrivate) {}

void PipelineConfig::enableStream(StreamType stream_type,
                                  int index,
                                  uint32_t width,
                                  uint32_t height,
                                  StreamFormat format,
                                  uint32_t framerate) {
  config_->enableStream(stream_type, index, width, height, format, framerate);
}

void PipelineConfig::enableStream(StreamType stream_type, int width, int height, StreamFormat format, int framerate) {
  enableStream(stream_type, -1, width, height, format, framerate);
}

void PipelineConfig::enableStream(StreamType stream_type, int stream_index) {
  enableStream(stream_type, stream_index, 0, 0, StreamFormat::FORMAT_CUSTOM, 0);
}

void PipelineConfig::enableStream(StreamType stream_type, StreamFormat format, int framerate) {
  enableStream(stream_type, -1, 0, 0, format, framerate);
}

void PipelineConfig::enableStream(StreamType stream_type, int stream_index, StreamFormat format, int framerate) {
  enableStream(stream_type, stream_index, 0, 0, format, framerate);
}

void PipelineConfig::enableAllStreams() {
  config_->enableAllStreams();
}

void PipelineConfig::enableDevice(const std::string &serial) {
  config_->enableDevice(serial);
}

void PipelineConfig::enableDeviceFromFile(const std::string &file, bool repeat_playback) {
  config_->enableDeviceFromFile(file, repeat_playback);
}

void PipelineConfig::disableStream(StreamType stream, int index) {
  config_->disableStream(stream, index);
}

void PipelineConfig::disableAllStreams() {
  config_->disableAllStreams();
}

PipelineProfile PipelineConfig::resolve(std::shared_ptr<PipelinePrivate> pipeline) const {
  auto profile = config_->resolve(std::move(pipeline));
  return PipelineProfile(profile);
}

bool PipelineConfig::canResolve(std::shared_ptr<PipelinePrivate> pipeline) const {
  return config_->canResolve(std::move(pipeline));
}

}  // namespace libsmartereye2
