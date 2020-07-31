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

#ifndef LIBSMARTEREYE2_USB_TYPES_H
#define LIBSMARTEREYE2_USB_TYPES_H

#include <cstdint>
#include <string>
#include <map>
#include <sstream>
#include <vector>

#define USB_DT_DEVICE 0x01
#define USB_DT_CONFIG 0x02
#define USB_DT_STRING 0x03
#define USB_DT_INTERFACE 0x04
#define USB_DT_ENDPOINT 0x05
#define USB_DT_DEVICE_QUALIFIER 0x06
#define USB_DT_OTHER_SPEED_CONFIG 0x07
#define USB_DT_INTERFACE_POWER 0x08
#define USB_DT_OTG 0x09
#define USB_DT_DEBUG 0x0a
#define USB_DT_INTERFACE_ASSOCIATION 0x0b
#define USB_DT_SECURITY 0x0c
#define USB_DT_KEY 0x0d
#define USB_DT_ENCRYPTION_TYPE 0x0e
#define USB_DT_BOS 0x0f
#define USB_DT_DEVICE_CAPABILITY 0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP 0x11
#define USB_DT_WIRE_ADAPTER 0x21
#define USB_DT_RPIPE 0x22
#define USB_DT_CS_RADIO_CONTROL 0x23
#define USB_DT_PIPE_USAGE 0x24
#define USB_DT_SS_ENDPOINT_COMP 0x30
#define USB_DT_SSP_ISOC_ENDPOINT_COMP 0x31
#define USB_DT_CS_DEVICE (USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG (USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING (USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE (USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT (USB_TYPE_CLASS | USB_DT_ENDPOINT)

namespace libsmartereye2 {

namespace platform {

enum UsbStatus {
  SE2_USB_STATUS_SUCCESS = 0,
  SE2_USB_STATUS_IO = -1,
  SE2_USB_STATUS_INVALID_PARAM = -2,
  SE2_USB_STATUS_ACCESS = -3,
  SE2_USB_STATUS_NO_DEVICE = -4,
  SE2_USB_STATUS_NOT_FOUND = -5,
  SE2_USB_STATUS_BUSY = -6,
  SE2_USB_STATUS_TIMEOUT = -7,
  SE2_USB_STATUS_OVERFLOW = -8,
  SE2_USB_STATUS_PIPE = -9,
  SE2_USB_STATUS_INTERRUPTED = -10,
  SE2_USB_STATUS_NO_MEM = -11,
  SE2_USB_STATUS_NOT_SUPPORTED = -12,
  SE2_USB_STATUS_OTHER = -13
};

enum EndpointDirection {
  SE2_USB_ENDPOINT_DIRECTION_WRITE = 0,
  SE2_USB_ENDPOINT_DIRECTION_READ = 0x80
};

enum EndpointType {
  SE2_USB_ENDPOINT_CONTROL,
  SE2_USB_ENDPOINT_ISOCHRONOUS,
  SE2_USB_ENDPOINT_BULK,
  SE2_USB_ENDPOINT_INTERRUPT
};

enum UsbClass {
  SE2_USB_CLASS_UNSPECIFIED = 0x00,
  SE2_USB_CLASS_AUDIO = 0x01,
  SE2_USB_CLASS_COM = 0x02,
  SE2_USB_CLASS_HID = 0x03,
  SE2_USB_CLASS_PID = 0x05,
  SE2_USB_CLASS_IMAGE = 0x06,
  SE2_USB_CLASS_PRINTER = 0x07,
  SE2_USB_CLASS_MASS_STORAGE = 0x08,
  SE2_USB_CLASS_HUB = 0x09,
  SE2_USB_CLASS_CDC_DATA = 0x0A,
  SE2_USB_CLASS_SMART_CARD = 0x0B,
  SE2_USB_CLASS_CONTENT_SECURITY = 0x0D,
  SE2_USB_CLASS_VIDEO = 0x0E,
  SE2_USB_CLASS_PHDC = 0x0F,
  SE2_USB_CLASS_AV = 0x10,
  SE2_USB_CLASS_BILLBOARD = 0x11,
  SE2_USB_CLASS_DIAGNOSTIC_DEVIE = 0xDC,
  SE2_USB_CLASS_WIRELESS_CONTROLLER = 0xE0,
  SE2_USB_CLASS_MISCELLANEOUS = 0xEF,
  SE2_USB_CLASS_APPLICATION_SPECIFIC = 0xFE,
  SE2_USB_CLASS_VENDOR_SPECIFIC = 0xFF
};

enum UsbSubclass {
  SE2_USB_SUBCLASS_VIDEO_CONTROL = 0x01,
  SE2_USB_SUBCLASS_VIDEO_STREAMING = 0x02
};

enum UsbSpec : uint16_t {
  USB_UNDEFINED = 0,
  USB1 = 0x0100,
  USB_1_1 = 0x0110,
  USB_2 = 0x0200,
  USB_2_1 = 0x0210,
  USB_3 = 0x0300,
  USB_3_1 = 0x0310,
  USB_3_2 = 0x0320,
};

static const std::map<UsbSpec, std::string> kUsbSpecNames = {
    {USB_UNDEFINED, "Undefined"},
    {USB1, "1.0"},
    {USB_1_1, "1.1"},
    {USB_2, "2.0"},
    {USB_2_1, "2.1"},
    {USB_3, "3.0"},
    {USB_3_1, "3.1"},
    {USB_3_2, "3.2"}
};

struct UsbDeviceInfo {
  std::string id;

  uint16_t vid;
  uint16_t pid;
  uint16_t mi;
  std::string unique_id;
  std::string serial;
  UsbSpec conn_spec;
  UsbClass usb_class;

  operator std::string() const {
    std::stringstream s;

    s << "vid- " << std::hex << vid <<
      "\npid- " << std::hex << pid <<
      "\nmi- " << mi <<
      "\nsusb specification- " << std::hex << (uint16_t) conn_spec << std::dec <<
      "\nunique_id- " << unique_id;

    return s.str();
  }

  bool operator==(const UsbDeviceInfo &other) const {
    return id == other.id
        && vid == other.vid
        && pid == other.pid
        && mi == other.mi
        && unique_id == other.unique_id;
  }
};

static std::map<int, std::string> kUsbStatus2String =
    {
        {SE2_USB_STATUS_SUCCESS, "SE2_USB_STATUS_SUCCESS"},
        {SE2_USB_STATUS_IO, "SE2_USB_STATUS_IO"},
        {SE2_USB_STATUS_INVALID_PARAM, "SE2_USB_STATUS_INVALID_PARAM"},
        {SE2_USB_STATUS_ACCESS, "SE2_USB_STATUS_ACCESS"},
        {SE2_USB_STATUS_NO_DEVICE, "SE2_USB_STATUS_NO_DEVICE"},
        {SE2_USB_STATUS_NOT_FOUND, "SE2_USB_STATUS_NOT_FOUND"},
        {SE2_USB_STATUS_BUSY, "SE2_USB_STATUS_BUSY"},
        {SE2_USB_STATUS_TIMEOUT, "SE2_USB_STATUS_TIMEOUT"},
        {SE2_USB_STATUS_OVERFLOW, "SE2_USB_STATUS_OVERFLOW"},
        {SE2_USB_STATUS_PIPE, "SE2_USB_STATUS_PIPE"},
        {SE2_USB_STATUS_INTERRUPTED, "SE2_USB_STATUS_INTERRUPTED"},
        {SE2_USB_STATUS_NO_MEM, "SE2_USB_STATUS_NO_MEM"},
        {SE2_USB_STATUS_NOT_SUPPORTED, "SE2_USB_STATUS_NOT_SUPPORTED"},
        {SE2_USB_STATUS_OTHER, "SE2_USB_STATUS_OTHER"}
    };

struct UsbConfigDescriptor {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t wTotalLength;
  uint8_t bNumInterfaces;
  uint8_t bConfigurationValue;
  uint8_t iConfiguration;
  uint8_t bmAttributes;
  uint8_t bMaxPower;
};

struct UsbInterfaceDescriptor {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
};

struct UsbDescriptor {
  uint8_t length;
  uint8_t type;
  std::vector<uint8_t> data;
};

struct UsbFrameInfo {
  uint16_t frame_id;
  uint16_t frame_format;
  uint32_t data_size;
  uint16_t width;
  uint16_t height;
  uint32_t frame_index;
};

struct UsbFrameCapacity {
  int32_t supported_frame_count;
  UsbFrameInfo frame_infos[0];
};

enum UsbCommand {
  NO_CMD = 0,
  OPEN_CAM = 1 << 0,
  CLOSE_CAM = 1 << 1,
  QUERY_FRAME_CAP = 1 << 2,
  SET_FRAME_IDS = 1 << 3,
  GET_FRAME = 1 << 4,
  RELEASE_FRAME = 1 << 5,
  RESET_USB_EDP = 1 << 6,
};

enum UsbMessageError {
  MESSAGE_SUCCESS,
  MESSAGE_ERR_DEV_NOT_FOUND,
  MESSAGE_ERR_DEV_OPEN,
  MESSAGE_ERR_DEV_CLAIM_ITF,
  MESSAGE_ERR_DEV_GET_EP,
  MESSAGE_ERR_PACK_HEAD,
  MESSAGE_ERR_PACK_LEN,
  MESSAGE_ERR_PACK_MAGIC,
  MESSAGE_ERR_USB_RW,
  MESSAGE_ERR_NOT_SP_FRAMEIDS,
  MESSAGE_ERR_RESET_EDP,
  MESSAGE_ERR_EP0_TIMEOUT,
  MESSAGE_ERR_USB_DEV_NOT_OPEN,
  MESSAGE_ERR_SET_FRAMEIDS,
};

struct UsbCommonPack {
  uint32_t magic;
  uint32_t pack_length;
  uint16_t cmd;
  union {
    int16_t state;
    int16_t reserve;
    int16_t raw_image_frame_count;
    int16_t frame_info_count;
  };

  union {
    struct UsbFrameInfo frame_info[0];
    uint8_t raw_image_frame[0];
    int8_t timestamp[0];
  };
};

}  // namespace platform

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USB_TYPES_H
