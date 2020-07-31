//
// Created by xtp on 2020/8/24.
//

#ifndef LIBSMARTEREYE2_USB_FOO_H
#define LIBSMARTEREYE2_USB_FOO_H

#include "usb_device.h"

namespace libsmartereye2 {

namespace platform {

class UsbFoo {
 public:
  static SeUsbDevice createUsbDevcie(const UsbDeviceInfo &info);
  static std::vector<UsbDeviceInfo> queryDevicesInfo();
};

}  // namespace platform

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USB_FOO_H
