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

#ifndef LIBSMARTEREYE2_RECORD_H
#define LIBSMARTEREYE2_RECORD_H

#include <memory>
#include <vector>
#include <mutex>
#include <map>

#include "device/backend.h"

namespace libsmartereye2 {
namespace platform {

class TimeService;
class PlaybackDeviceWatcher;
struct UsbDeviceInfo;
struct BackendProfile;

class Recording {
 public:
  explicit Recording(std::shared_ptr<TimeService> ts = nullptr,
                     std::shared_ptr<PlaybackDeviceWatcher> watcher = nullptr);

  double getTime() const { return current_time_; }

  void save(const std::string &filename, const std::string &section, bool append = false) const;

  static std::shared_ptr<Recording> load(const std::string &filename,
                                         const std::string &section,
                                         std::shared_ptr<PlaybackDeviceWatcher> watcher = nullptr);

  int saveBlob(const char *ptr, size_t size);

  template<class T>
  std::pair<int, int> insertList(std::vector<T> list, std::vector<T> &target) {
    std::pair<int, int> range;

    range.first = static_cast<int>(target.size());
    for (auto &&i : list) target.push_back(i);
    range.second = static_cast<int>(target.size());

    return range;
  }

//  template<class T>
//  void saveList(std::vector<T> list, std::vector<T> &target, call_type type, int entity_id) {
//    std::lock_guard<std::recursive_mutex> lock(mutex_);
//    call c;
//    c.type = type;
//    c.entity_id = entity_id;
//    auto range = insertList(list, target);
//    c.param1 = range.first;
//    c.param2 = range.second;
//
//    c.timestamp = get_current_time();
//    calls.push_back(c);
//  }

//  call &add_call(lookup_key key) {
//    std::lock_guard<std::recursive_mutex> lock(mutex_);
//    call c;
//    c.type = key.type;
//    c.entity_id = key.entity_id;
//    c.timestamp = get_current_time();
//    calls.push_back(c);
//    return calls[calls.size() - 1];
//  }

//  template<class T>
//  std::vector<T> loadList(const std::vector<T> &source, const call &c) {
//    std::vector<T> results;
//    std::lock_guard<std::recursive_mutex> lock(mutex_);
//    for (auto i = c.param1; i < c.param2; i++) {
//      results.push_back(source[i]);
//    }
//    return results;
//  }

  template<class T>
  std::vector<T> loadList(const std::vector<T> &source, const int range_start, const int range_end) {
    std::vector<T> results;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto i = range_start; i < range_end; i++) {
      results.push_back(source[i]);
    }
    return results;
  }

//  void save_device_changed_data(BackendDeviceGroup old, BackendDeviceGroup curr, lookup_key k) {
//    std::lock_guard<std::recursive_mutex> lock(mutex_);
//    call c;
//    c.type = k.type;
//    c.entity_id = k.entity_id;
//
//    auto range = insertList(old.usb_devices, usb_device_infos_);
//    c.param3 = range.first;
//    c.param4 = range.second;
//
//    range = insertList(curr.usb_devices, usb_device_infos_);
//    c.param9 = range.first;
//    c.param10 = range.second;
//
//    c.timestamp = get_current_time();
//    calls.push_back(c);
//  }

//  void save_device_info_list(std::vector<UsbDeviceInfo> list, lookup_key k) {
//    saveList(list, usb_device_infos_, k.type, k.entity_id);
//  }
//
//  void save_stream_profiles(std::vector<BackendProfile> list, lookup_key key) {
//    saveList(list, backend_profiles_, key.type, key.entity_id);
//  }
//
//  std::vector<BackendProfile> load_stream_profiles(int id, call_type type) {
//    auto &&c = find_call(type, id);
//    return loadList(backend_profiles_, c);
//  }
//
//  void load_device_changed_data(BackendDeviceGroup &old, BackendDeviceGroup &curr, lookup_key k) {
//    auto &&c = find_call(k.type, k.entity_id);
//
//    old.usb_devices = loadList(usb_device_infos_, c.param3, c.param4);
//    curr.usb_devices = loadList(usb_device_infos_, c.param9, c.param10);
//  }
//
//  std::vector<UsbDeviceInfo> load_usb_device_info_list() {
//    auto &&c = find_call(call_type::query_usb_devices, 0);
//    return loadList(usb_device_infos_, c);
//  }

  std::vector<uint8_t> load_blob(int id) const {
    return blobs_[id];
  }

//  call &find_call(call_type t,
//                  int entity_id,
//                  std::function<bool(const call &c)> history_match_validation = [](const call &c) { return true; });
//  call *cycle_calls(call_type call_type, int id);
//  call *pick_next_call(int id = 0);
//  size_t size() const { return calls.size(); }

 private:
//  std::vector<call> calls;
  std::vector<std::vector<uint8_t>> blobs_;
  std::vector<UsbDeviceInfo> usb_device_infos_;
  std::vector<BackendProfile> backend_profiles_;
  std::shared_ptr<PlaybackDeviceWatcher> watcher_;

  std::recursive_mutex mutex_;
  std::shared_ptr<TimeService> ts_;

  std::map<size_t, size_t> cursors_;
  std::map<size_t, size_t> cycles_;

  double getCurrentTime() {return ts_->getTime();}

  void invoke_device_changed_event() {}

  double current_time_ = 0;
};

}  // namespace platform
}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_RECORD_H
