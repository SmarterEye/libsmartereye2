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

#include "context.h"

#include "device/device.h"
#include "device/backend.h"
#include "device/device_info.h"

#include "mock/playback/playback.h"
#include "mock/record/record.h"

#include "easylogging++.h"

#include "smartereye2/device/context.hpp"
#include "device/device_list.hpp"
#include "se_types.hpp"

#include "gemini/gemini_info.h"

#include <memory>
#include <utility>

namespace libsmartereye2 {

using platform::BackendType;

ContextPrivate::ContextPrivate(platform::BackendType type, const std::string &filename, const std::string &section)
    : backend_(nullptr),
      devices_changed_callback_(nullptr, [](SeDevicesChangedCallback *) {}) {
  switch (type) {
    case BackendType::STANDARD: {
      backend_ = std::make_shared<platform::StandardBackend>();
    }
      break;
    case BackendType::PLAYBACK: {
      backend_ = std::make_shared<platform::PlaybackBackend>(filename, section);
    }
      break;
    case BackendType::RECORD: {
      backend_ = nullptr; // TODO
    }
      break;
  }

  CHECK_PTR_NOT_NULL(backend_);

  Environment::instance().setTimeService(backend_->createTimeService());
  device_watcher_ = backend_->createDeviceWatcher();
}

void ContextPrivate::stop() {
  device_watcher_->stop();
}

std::vector<std::shared_ptr<DeviceInfo>> ContextPrivate::queryDevices(int mask) const {
  platform::BackendDeviceGroup devices(backend_->queryUsbDevices());
  return createDevices(devices, playback_devices_, mask);
}

int64_t ContextPrivate::registerInternalDeviceCallback(DevicesChangedCallbackPtr callback) {
  std::lock_guard<std::mutex> lock(_devices_changed_callbacks_mtx);
  auto callback_id = util::UniqueId::generateId();
  devices_changed_callbacks_.insert(std::make_pair(callback_id, std::move(callback)));

  return callback_id;
}

void ContextPrivate::unregisterInternalDeviceCallback(int64_t cb_id) {
  std::lock_guard<std::mutex> lock(_devices_changed_callbacks_mtx);
  devices_changed_callbacks_.erase(cb_id);
}

void ContextPrivate::setDevicesChangedCallback(DevicesChangedCallbackPtr callback) {
  device_watcher_->stop();

  devices_changed_callback_ = std::move(callback);
  device_watcher_->start([this](platform::BackendDeviceGroup old, platform::BackendDeviceGroup curr) {
    onDeviceChanged(std::move(old), std::move(curr), playback_devices_, playback_devices_);
  });
}

std::vector<std::shared_ptr<DeviceInfo>>
ContextPrivate::createDevices(platform::BackendDeviceGroup devices,
                              const std::map<std::string, std::weak_ptr<DeviceInfo>> &playback_devices,
                              int mask) const {
  std::vector<std::shared_ptr<DeviceInfo>> result_list, matched_list;

  auto t = const_cast<ContextPrivate *>(this);
  auto ctx = t->shared_from_this();

  if (mask & ProductCode::SE_PRODUCT_GEMINI) {
    auto gemini_devices = GeminiInfo::pikachu(ctx, devices.usb_devices);
    std::copy(gemini_devices.begin(), gemini_devices.end(), std::back_inserter(matched_list));
  }

  std::copy(matched_list.begin(), matched_list.end(), std::back_inserter(result_list));

  for (auto &&item : playback_devices) {
    if (auto dev = item.second.lock()) result_list.push_back(dev);
  }

  LOG(INFO) << "Found " << result_list.size() << " SmarterEye devices (mask " << mask << ")";
  return result_list;
}

std::shared_ptr<PlaybackDeviceInfo> ContextPrivate::addDevice(const std::string &file) {
  auto it = playback_devices_.find(file);
  if (it != playback_devices_.end() && it->second.lock()) {
    //Already exists
    LOG(WARNING) << "File \"" << file << "\" already loaded to context";
    return nullptr;
  }

  auto playback_device = std::make_shared<PlaybackDevice>();
  auto new_info = std::make_shared<PlaybackDeviceInfo>(playback_device);
  auto prev_devs = playback_devices_;
  playback_devices_[file] = new_info;
  onDeviceChanged({}, {}, prev_devs, playback_devices_);
  return std::move(new_info);
}

void ContextPrivate::onDeviceChanged(platform::BackendDeviceGroup old,
                                     platform::BackendDeviceGroup current,
                                     const std::map<std::string, std::weak_ptr<DeviceInfo>> &old_playback_devices,
                                     const std::map<std::string, std::weak_ptr<DeviceInfo>> &new_playback_devices) {
  auto old_list = createDevices(std::move(old), old_playback_devices, ProductCode::SE_PRODUCT_ANY);
  auto new_list = createDevices(std::move(current), new_playback_devices, ProductCode::SE_PRODUCT_ANY);

  bool changed_flag = platform::listChanged<std::shared_ptr<DeviceInfo>>(old_list, new_list,
                                                                         [](std::shared_ptr<DeviceInfo> first,
                                                                            std::shared_ptr<DeviceInfo> second) {
                                                                           return *first == *second;
                                                                         });
  if (changed_flag) {
    std::vector<SeDeviceInfo> devices_info_added, devices_info_removed;
    // TODO
    raiseDevicesChanged(devices_info_removed, devices_info_added);
  }
}

void ContextPrivate::raiseDevicesChanged(const std::vector<SeDeviceInfo> &removed,
                                         const std::vector<SeDeviceInfo> &added) {
  if (devices_changed_callback_) {
    try {
      devices_changed_callback_->onDevicesChanged(new SeDeviceList{shared_from_this(), removed},
                                                  new SeDeviceList{shared_from_this(), removed});
    } catch (...) {
      LOG(ERROR) << "Exception thrown from user callback handler";
    }
  }
}

}  // namespace libsmartereye2

namespace se2 {

bool EventInfomation::wasRemoved(const Device &dev) const {
  if (!dev) return false;

  bool ok = false;
  for (const auto &info : removed_.getList()->list) {
    if (dev.get()->info && dev.get()->info->getDeviceData() == info.info->getDeviceData()) {
      ok = true;
      break;
    }
  }
  return ok;
}

bool EventInfomation::wasAdded(const Device &dev) const {
  if (!dev) return false;

  bool ok = false;
  for (const auto &info : added_.getList()->list) {
    if (dev.get()->info && dev.get()->info->getDeviceData() == info.info->getDeviceData()) {
      ok = true;
      break;
    }
  }
  return ok;
}

Context::Context() {
  auto xx = std::make_shared<libsmartereye2::ContextPrivate>(libsmartereye2::platform::BackendType::STANDARD);
  context_ = std::make_shared<SeContext>(SeContext{xx});
}

Context::Context(std::shared_ptr<SeContext> context)
    : context_(std::move(context)) {
}

Context::operator std::shared_ptr<SeContext>() const {
  return context_;
}

DeviceList Context::queryDevices() const {
  return queryDevices(ProductCode::SE_PRODUCT_ANY);
}

DeviceList Context::queryDevices(int mask) const {
  std::vector<SeDeviceInfo> infos;
  for (auto &&dev_info : context_->context->queryDevices(mask)) {
    try {
      SeDeviceInfo curr_info{context_->context, dev_info};
      infos.push_back(curr_info);
    } catch (...) {
      LOG(WARNING) << "Could not open device!";
    }
  }
  std::shared_ptr<SeDeviceList> list(new SeDeviceList{context_->context, infos});
  return DeviceList(list);
}

std::vector<Sensor> Context::queryAllSensors() const {
  std::vector<Sensor> results;
  for (auto &&dev : queryDevices()) {
    auto sensors = dev.querySensors();
    std::copy(sensors.begin(), sensors.end(), std::back_inserter(results));
  }
  return results;
}

Device Context::getSensorParent(const Sensor &sensor) const {
  std::shared_ptr<SeDevice> dev(new SeDevice(*sensor.get()->parent));
  return Device{dev};
}

}  // namespace se2

