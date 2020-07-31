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

#ifndef LIBSMARTEREYE2_USBREQUEST_H
#define LIBSMARTEREYE2_USBREQUEST_H

#include "usb_endpoint.h"

#include <memory>
#include <utility>
#include <vector>
#include <functional>
#include <mutex>
#include <queue>

namespace libsmartereye2 {
namespace platform {

class UsbRequestCallback;
class UsbRequest;

using SeUsbRequestCallback = std::shared_ptr<UsbRequestCallback>;
using SeUsbRequest = std::shared_ptr<UsbRequest>;

class UsbRequest {
 public:
  UsbRequest(libusb_device_handle *dev_handle, SeUsbEndpoint endpoint);
  ~UsbRequest();

  SeUsbEndpoint get_endpoint() const { return endpoint_; }
  int get_actual_length() const { return transfer_->actual_length; }
  void set_callback(const SeUsbRequestCallback &callback) { callback_ = callback; }
  SeUsbRequestCallback get_callback() const { return callback_; }
  void set_client_data(void *data) { client_data_ = data; }
  void *get_client_data() const { return client_data_; }
  void *get_native_request() const { return transfer_.get(); }
  const std::vector<uint8_t> &get_buffer() const { return buffer_; }
  void set_buffer(const std::vector<uint8_t> &buffer);

  std::shared_ptr<UsbRequest> get_shared() const { return shared_.lock(); }
  void set_shared(const std::shared_ptr<UsbRequest> &shared) { shared_ = shared; }
  void set_active(bool state) { active_ = state; }

 protected:
  void set_native_buffer_length(int length) { transfer_->length = length; }
  int get_native_buffer_length() const { return transfer_->length; }
  void set_native_buffer(uint8_t *buffer) { transfer_->buffer = buffer; }
  uint8_t *get_native_buffer() const { return transfer_->buffer; }

 protected:
  void *client_data_{};
  SeUsbRequest request_;
  SeUsbEndpoint endpoint_;
  std::vector<uint8_t> buffer_;
  SeUsbRequestCallback callback_;

 private:
  bool active_ = false;
  std::weak_ptr<UsbRequest> shared_;
  std::shared_ptr<libusb_transfer> transfer_;
};

typedef std::shared_ptr<UsbRequest> SeUsbRequest;

class UsbRequestCallback {
  std::function<void(SeUsbRequest)> callback_;
  std::mutex mutex_;

 public:
  explicit UsbRequestCallback(std::function<void(SeUsbRequest)> callback) {
    callback_ = std::move(callback);
  }

  ~UsbRequestCallback() {
    cancel();
  }

  void cancel() {
    std::lock_guard<std::mutex> lk(mutex_);
    callback_ = nullptr;
  }

  void callback(SeUsbRequest response) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (callback_)
      callback_(std::move(response));
  }
};

}  // namespace platform
}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USBREQUEST_H
