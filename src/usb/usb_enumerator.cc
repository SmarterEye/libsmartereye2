
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

#include "usb_enumerator.h"

#include "usb/usb_context.h"
#include "usb/usb_device.h"
#include "easylogging++.h"

namespace libsmartereye2 {

namespace platform {

std::string getDevicePath(libusb_device *usb_device) {
  auto usb_bus = std::to_string(libusb_get_bus_number(usb_device));

  // As per the USB 3.0 specs, the current maximum limit for the depth is 7.
  const auto max_usb_depth = 8;
  uint8_t usb_ports[max_usb_depth] = {};
  std::stringstream port_path;
  auto port_count = libusb_get_port_numbers(usb_device, usb_ports, max_usb_depth);
  auto usb_dev = std::to_string(libusb_get_device_address(usb_device));
  libusb_device_descriptor dev_desc{};
  auto r = libusb_get_device_descriptor(usb_device, &dev_desc);

  for (size_t i = 0; i < port_count; ++i) {
    port_path << std::to_string(usb_ports[i]) << (((i + 1) < port_count) ? "." : "");
  }

  return usb_bus + "-" + port_path.str() + "-" + usb_dev;
}

std::vector<UsbDeviceInfo> getSubdevices(libusb_device *device, libusb_device_descriptor desc) {
  std::vector<UsbDeviceInfo> rv;
  for (uint8_t c = 0; c < desc.bNumConfigurations; ++c) {
    libusb_config_descriptor *config = nullptr;
    auto ret = libusb_get_config_descriptor(device, c, &config);
    if (LIBUSB_SUCCESS != ret) {
//      LOG(WARNING) << "failed to read USB config descriptor while getSubdevices(): error = " << std::dec << ret;
      continue;
    }

    for (uint8_t i = 0; i < config->bNumInterfaces; ++i) {
      auto inf = config->interface[i];

      //avoid publish streaming interfaces TODO:MK
      if (inf.altsetting->bInterfaceSubClass == 2)
        continue;
      // when device is in DFU state, two USB devices are detected, one of RS2_USB_CLASS_VENDOR_SPECIFIC (255) class
      // and the other of RS2_USB_CLASS_APPLICATION_SPECIFIC (254) class.
      // in order to avoid listing two usb devices for a single physical device we ignore the application specific class
      // https://www.usb.org/defined-class-codes#anchor_BaseClassFEh
      if (inf.altsetting->bInterfaceClass == SE2_USB_CLASS_APPLICATION_SPECIFIC)
        continue;

      UsbDeviceInfo info{};
      auto path = getDevicePath(device);
      info.id = path;
      info.unique_id = path;
      info.conn_spec = UsbSpec(desc.bcdUSB);
      info.vid = desc.idVendor;
      info.pid = desc.idProduct;
      info.mi = i;
      info.usb_class = UsbClass(inf.altsetting->bInterfaceClass);
      rv.push_back(info);
    }

    libusb_free_config_descriptor(config);
  }
  return rv;
}

SeUsbDevice libsmartereye2::platform::UsbEnumerator::createUsbDevcie(const libsmartereye2::platform::UsbDeviceInfo &info) {
  auto usb_context = std::make_shared<UsbContext>();

  for (auto index = 0; index < usb_context->deviceCount(); ++index) {
    auto device = usb_context->getDevice(index);
    if (device == nullptr || getDevicePath(device) != info.id) {
      continue;
    }

    libusb_device_descriptor desc{};
    int ret = libusb_get_device_descriptor(device, &desc);
    if (LIBUSB_SUCCESS == ret) {
      try {
        return std::make_shared<UsbDevice>(device, desc, info, usb_context);
      } catch (std::exception &e) {
        LOG(WARNING) << "failed to create usb device at index: %d" << index;
      }
    } else
      LOG(WARNING) << "failed to read USB device descriptor while createUsbDevcie(): error = " << std::dec << ret;
  }

  return nullptr;
}

std::vector<UsbDeviceInfo> UsbEnumerator::queryDevicesInfo() {
  std::vector<UsbDeviceInfo> rv;
  auto ctx = std::make_shared<UsbContext>();

  for (uint8_t idx = 0; idx < ctx->deviceCount(); ++idx) {
    auto device = ctx->getDevice(idx);
    if (device == nullptr)
      continue;
    libusb_device_descriptor desc{};

    auto ret = libusb_get_device_descriptor(device, &desc);
    if (LIBUSB_SUCCESS == ret) {
      auto sd = getSubdevices(device, desc);
      rv.insert(rv.end(), sd.begin(), sd.end());
    } else
      LOG(WARNING) << "failed to read USB device descriptor: error = " << std::dec << ret;
  }
  return rv;
}

}  // namespace platform

}  // namespace libsmartereye2