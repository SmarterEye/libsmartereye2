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

#ifndef LIBSMARTEREYE2_DEVICE_LIST_H
#define LIBSMARTEREYE2_DEVICE_LIST_H

#include <memory>
#include <vector>

#include "device.h"
#include "device_info.h"
#include "se_types.h"

namespace libsmartereye2 {

class DeviceList {
 public:
  explicit DeviceList(std::shared_ptr<SeDeviceList> list)
      : list_(std::move(list)) {}

  DeviceList() : list_(nullptr) {}

  explicit operator std::vector<Device>() const {
    std::vector<Device> res;
    for (auto &&dev : *this) res.push_back(dev);
    return res;
  }

  DeviceList &operator=(std::shared_ptr<SeDeviceList> list) {
    list = std::move(list);
    return *this;
  }

  const Device &operator[](uint32_t index) const {
    std::shared_ptr<SeDevice> dev(new SeDevice{
        list_->ctx,
        list_->list[index].info,
        list_->list[index].info->createDevice()
    });
    return Device(dev);
  }

  bool contains(const Device &dev) const {
    bool found = false; // TODO
    return found;
  }

  int32_t size() const { return static_cast<int32_t>(list_->list.size()); }
  Device front() const { return (*this)[0]; }
  Device back() const { return (*this)[size() - 1]; }

  explicit operator std::shared_ptr<SeDeviceList>() { return list_; };

  class device_list_iterator {
    device_list_iterator(const DeviceList &device_list, int32_t int32)
        : list_(device_list), index_(int32) {}

   public:
    const Device &operator*() const {
      return list_[index_];
    }
    bool operator!=(const device_list_iterator &other) const {
      return other.index_ != index_ || &other.list_ != &list_;
    }
    bool operator==(const device_list_iterator &other) const {
      return !(*this != other);
    }
    device_list_iterator &operator++() {
      index_++;
      return *this;
    }
   private:
    friend DeviceList;
    const DeviceList &list_;
    int32_t index_;
  };

  device_list_iterator begin() const {
    return device_list_iterator(*this, 0);
  }
  device_list_iterator end() const {
    return device_list_iterator(*this, size());
  }
  const SeDeviceList *getList() const { return list_.get(); }

 private:
  std::shared_ptr<SeDeviceList> list_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_DEVICE_LIST_H
