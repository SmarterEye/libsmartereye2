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

#include "pipeline_config.h"
#include "pipeline/pipeline_config.hpp"
#include "pipeline/pipeline_profile.hpp"
#include "device/device_info.h"
#include "easylogging++.h"

namespace libsmartereye2 {

void PipelineConfigPrivate::enableStream(FrameId frame_id,
                                         int index,
                                         uint32_t width,
                                         uint32_t height,
                                         FrameFormat format,
                                         uint32_t framerate) {
  std::lock_guard<std::mutex> lock(mutex_);
  resolved_profile_.reset();
  stream_requests_[{frame_id, index}] = StreamProfileConfig{format, frame_id, index, width, height, framerate};
}

void PipelineConfigPrivate::enableAllStreams() {
  std::lock_guard<std::mutex> lock(mutex_);
  resolved_profile_.reset();
  stream_requests_.clear();
  enable_all_streams_ = true;
}

void PipelineConfigPrivate::enableDevice(const std::string &serial) {
  std::lock_guard<std::mutex> lock(mutex_);
  resolved_profile_.reset();
  device_request_.serial = serial;
}

void PipelineConfigPrivate::enableDeviceFromFile(const std::string &file, bool repeat_playback) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (device_request_.record_output.empty()) {
    throw std::runtime_error("Configuring both device from file, and record to file is unsupported");
  }
  resolved_profile_.reset();
  device_request_.filename = file;
  playback_loop_ = repeat_playback;
}

void PipelineConfigPrivate::enableRecordToFile(const std::string &file) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (device_request_.record_output.empty()) {
    throw std::runtime_error("Configuring both device from file, and record to file is unsupported");
  }
  resolved_profile_.reset();
  device_request_.filename = file;
}

void PipelineConfigPrivate::disableStream(FrameId frame_id, int index) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = stream_requests_.begin();
  while (iter != stream_requests_.end()) {
    if (iter->first.first == frame_id
        && (index == -1 || iter->first.second == index)) {
      iter = stream_requests_.erase(iter);
    } else {
      ++iter;
    }
  }
  resolved_profile_.reset();
}

void PipelineConfigPrivate::disableAllStreams() {
  std::lock_guard<std::mutex> lock(mutex_);
  stream_requests_.clear();
  enable_all_streams_ = false;
  resolved_profile_.reset();
}

std::shared_ptr<PipelineProfilePrivate> PipelineConfigPrivate::resolve(std::shared_ptr<PipelinePrivate> pipe,
                                                                       const std::chrono::milliseconds &timeout) {
  std::lock_guard<std::mutex> lock(mutex_);
  resolved_profile_.reset();

  auto requested_device = resolveDeviceRequests(pipe, timeout);
  if (requested_device) {
    resolved_profile_ = resolve(requested_device);
    return resolved_profile_;
  }

  auto devs = pipe->getContext()->queryDevices(ProductCode::SE_PRODUCT_ANY);
  for (const auto &dev_info : devs) {
    try {
      auto dev = dev_info->createDevice(true);
      resolved_profile_ = resolve(dev);
      return resolved_profile_;
    } catch (const std::exception &e) {
      LOG(DEBUG) << "Iterate available devices - config can not be resolved. " << e.what();
    }
  }

  auto dev = pipe->waitForDevice(timeout);
  if (dev) {
    resolved_profile_ = resolve(dev);
    return resolved_profile_;
  }

  throw std::runtime_error("Failed to resolve request. No device found that satisfies all requirements");
}

bool PipelineConfigPrivate::canResolve(std::shared_ptr<PipelinePrivate> pipe) {
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

std::shared_ptr<PipelineProfilePrivate> PipelineConfigPrivate::getCachedResolvedProfile() {
  std::lock_guard<std::mutex> lock(mutex_);
  return resolved_profile_;
}

PipelineConfigPrivate::PipelineConfigPrivate(const PipelineConfigPrivate &other) {
  device_request_ = other.device_request_;
  stream_requests_ = other.stream_requests_;
  enable_all_streams_ = other.enable_all_streams_;
  resolved_profile_ = nullptr;
  playback_loop_ = other.playback_loop_;
}

std::shared_ptr<DeviceInterface> PipelineConfigPrivate::getOrAddPlaybackDevice(std::shared_ptr<ContextPrivate> ctx,
                                                                               const std::string &file) {
  // TODO
  return nullptr;
}

std::shared_ptr<DeviceInterface> PipelineConfigPrivate::resolveDeviceRequests(std::shared_ptr<PipelinePrivate> pipe,
                                                                              const std::chrono::milliseconds &timeout) {
  //Prefer filename over serial
  if (!device_request_.filename.empty()) {
    std::shared_ptr<DeviceInterface> dev;
    try {
      dev = getOrAddPlaybackDevice(pipe->getContext(), device_request_.filename);
    }
    catch (const std::exception &e) {
      throw std::runtime_error(
          toString() << "Failed to resolve request. Request to enable_device_from_file(\"" << device_request_.filename
                     << "\") was invalid, Reason: " << e.what());
    }

    //check if a serial number was also requested, and check again the device
    if (!device_request_.serial.empty()) {
      if (!dev->supportsInfo(CAMERA_INFO_SERIAL_NUMBER)) {
        throw std::runtime_error(toString() << "Failed to resolve request. "
                                               "Conflic between enable_device_from_file(\"" << device_request_.filename
                                            << "\") and enable_device(\"" << device_request_.serial << "\"), "
                                                                                                       "File does not contain a device with such serial");
      } else {
        std::string s = dev->getInfo(CAMERA_INFO_SERIAL_NUMBER);
        if (s != device_request_.serial) {
          throw std::runtime_error(toString() << "Failed to resolve request. "
                                                 "Conflic between enable_device_from_file(\""
                                              << device_request_.filename
                                              << "\") and enable_device(\"" << device_request_.serial << "\"), "
                                                                                                         "File contains device with different serial number ("
                                              << s << "\")");
        }
      }
    }
    return dev;
  }

  if (!device_request_.serial.empty()) {
    return pipe->waitForDevice(timeout, device_request_.serial);
  }

  return nullptr;
}

StreamProfiles PipelineConfigPrivate::getDefaultConfiguration(std::shared_ptr<DeviceInterface> dev) {
  StreamProfiles default_profiles;

  for (unsigned int i = 0; i < dev->getSensorCount(); i++) {
    auto &&sensor = dev->getSensor(i);
    auto profiles = sensor.getStreamProfiles(ProfileTag::PROFILE_TAG_DEFAULT);
    default_profiles.insert(std::end(default_profiles), std::begin(profiles), std::end(profiles));
  }

  return default_profiles;
}

std::shared_ptr<PipelineProfilePrivate> PipelineConfigPrivate::resolve(std::shared_ptr<DeviceInterface> dev) {
  //if the user requested all streams
  if (enable_all_streams_) {
    for (size_t i = 0; i < dev->getSensorCount(); ++i) {
      auto &&sub = dev->getSensor(i);
      auto profiles = sub.getStreamProfiles(PROFILE_TAG_SUPERSET);
    }
    return std::make_shared<PipelineProfilePrivate>(dev, device_request_.record_output);
  }

  //If the user did not request anything, give it the default, on playback all recorded streams are marked as default.
  if (stream_requests_.empty()) {
    auto default_profiles = getDefaultConfiguration(dev);
    return std::make_shared<PipelineProfilePrivate>(dev, device_request_.record_output);
  }

  //Enabled requested streams
  for (auto &&req : stream_requests_) {
    auto r = req.second;
  }
  return std::make_shared<PipelineProfilePrivate>(dev, device_request_.record_output);
}

}  // namespace libsmartereye2

namespace se2 {

PipelineConfig::PipelineConfig()
    : config_(new SePipelineConfig{std::make_shared<PipelineConfigPrivate>()}) {
}

void PipelineConfig::enableStream(FrameId frame_id,
                                  int index,
                                  uint32_t width,
                                  uint32_t height,
                                  FrameFormat format,
                                  uint32_t framerate) {
  config_->config->enableStream(frame_id, index, width, height, format, framerate);
}

void PipelineConfig::enableStream(FrameId frame_id, int width, int height, FrameFormat format, int framerate) {
  enableStream(frame_id, -1, width, height, format, framerate);
}

void PipelineConfig::enableStream(FrameId frame_id, int stream_index) {
  enableStream(frame_id, stream_index, 0, 0, FrameFormat::Any, 0);
}

void PipelineConfig::enableStream(FrameId frame_id, FrameFormat format, int framerate) {
  enableStream(frame_id, -1, 0, 0, format, framerate);
}

void PipelineConfig::enableStream(FrameId frame_id, int stream_index, FrameFormat format, int framerate) {
  enableStream(frame_id, stream_index, 0, 0, format, framerate);
}

void PipelineConfig::enableAllStreams() {
  config_->config->enableAllStreams();
}

void PipelineConfig::enableDevice(const std::string &serial) {
  config_->config->enableDevice(serial);
}

void PipelineConfig::enableDeviceFromFile(const std::string &file, bool repeat_playback) {
  config_->config->enableDeviceFromFile(file, repeat_playback);
}

void PipelineConfig::disableStream(FrameId frame_id, int index) {
  config_->config->disableStream(frame_id, index);
}

void PipelineConfig::disableAllStreams() {
  config_->config->disableAllStreams();
}

PipelineProfile PipelineConfig::resolve(std::shared_ptr<SePipeline> pipeline) const {
  auto profile_private = config_->config->resolve(std::move(pipeline->pipeline));
  std::shared_ptr<SePipelineProfile> pipe(new SePipelineProfile{profile_private});
  return PipelineProfile(pipe);
}

bool PipelineConfig::canResolve(std::shared_ptr<SePipeline> pipeline) const {
  return config_->config->canResolve(std::move(pipeline->pipeline));
}

}  // namespace se2
