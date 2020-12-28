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


#include "usb_device.h"
#include "usb_handle.h"

#include <utility>
#include "easylogging++.h"

namespace libsmartereye2 {

namespace platform {

auto getDescFromInterface = [](const libusb_interface &inf, std::vector<UsbDescriptor> descriptors) {
  for (int j = 0; j < inf.num_altsetting; j++) {
    auto d = inf.altsetting[j];
    UsbDescriptor ud = {d.bLength, d.bDescriptorType, std::vector<uint8_t>(d.bLength)};
    memcpy(ud.data.data(), &d, d.bLength);
    descriptors.push_back(ud);
    for (int k = 0; k < d.extra_length;) {
      auto l = d.extra[k];
      auto dt = d.extra[k + 1];
      UsbDescriptor ud = {l, dt, std::vector<uint8_t>(l)};
      memcpy(ud.data.data(), &d.extra[k], l);
      descriptors.push_back(ud);
      k += l;
    }
  }
};

UsbDevice::UsbDevice(libusb_device *device,
                     const libusb_device_descriptor &desc,
                     UsbDeviceInfo info,
                     std::shared_ptr<UsbContext> context)
    : device_(device),
      usb_device_descriptor_(desc),
      info_(std::move(info)),
      context_(std::move(context)) {
  UsbDescriptor ud = {desc.bLength, desc.bDescriptorType, std::vector<uint8_t>(desc.bLength)};
  memcpy(ud.data.data(), &desc, desc.bLength);
  descriptors_.push_back(ud);

  for (auto idx_config = 0; idx_config < desc.bNumConfigurations; ++idx_config) {
    libusb_config_descriptor *config{};
    auto ret = libusb_get_config_descriptor(device, idx_config, &config);
    if (ret != LIBUSB_SUCCESS) {
      LOG(WARNING) << "failed to read USB config descriptor: error = " << std::dec << ret;
      continue;
    }

    std::shared_ptr<UsbInterface> current_interface;
    for (auto idx_interface = 0; idx_interface < config->bNumInterfaces; ++idx_interface) {
      auto inf = config->interface[idx_interface];
      auto usb_interface = std::make_shared<UsbInterface>(&inf);
      interfaces_.push_back(usb_interface);

      uint8_t usb_class = inf.altsetting->bInterfaceClass;
      uint8_t usb_sub_class = inf.altsetting->bInterfaceSubClass;
      if (usb_class == SE2_USB_CLASS_VIDEO) {
        if (usb_sub_class == SE2_USB_SUBCLASS_VIDEO_CONTROL) {
          current_interface = usb_interface;
        }
        if (usb_sub_class == SE2_USB_SUBCLASS_VIDEO_STREAMING) {
          current_interface->addAssociatedInterface(usb_interface);
        }
      }

      getDescFromInterface(inf, descriptors_);
    }
    libusb_free_config_descriptor(config);
  }

  libusb_hotplug_callback_fn cb = [](libusb_context *ctx,
                                     libusb_device *device,
                                     libusb_hotplug_event event,
                                     void *user_data) -> int {
    if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
      LOG(INFO) << "usb disconnected~~~";
    }
    return 0;
  };
  // not supported on Windows
  libusb_hotplug_register_callback(context_->get(), LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_ENUMERATE,
                                   info_.vid, info_.pid, LIBUSB_HOTPLUG_MATCH_ANY, cb, nullptr, &hotplug_cb_handle_);
  libusb_ref_device(device_);
}

UsbDevice::~UsbDevice() {
  libusb_hotplug_deregister_callback(context_->get(), hotplug_cb_handle_);
  libusb_unref_device(device_);
}

SeUsbInterface UsbDevice::getInterface(uint8_t interface_number) const {
  auto it = std::find_if(interfaces_.begin(), interfaces_.end(),
                         [interface_number](const SeUsbInterface &i) { return interface_number == i->getNumber(); });

  return it == interfaces_.end() ? nullptr : *it;
}

SeUsbMessenger UsbDevice::open(uint8_t interface_number) {
  auto handle = get_handle(interface_number);
  return handle == nullptr ? nullptr : std::make_shared<UsbMessenger>(shared_from_this(), handle);
}

std::shared_ptr<UsbHandle> UsbDevice::get_handle(uint8_t interface_number) const {
  try {
    auto interface = getInterface(interface_number);
    if (!interface) {
      return nullptr;
    }
    auto usb_interface = std::dynamic_pointer_cast<UsbInterface>(interface);
    return std::make_shared<UsbHandle>(context_, device_, usb_interface);
  } catch (const std::exception &e) {
    return nullptr;
  }
}

}  // namespace platform

}  // namespace libsmartereye2
