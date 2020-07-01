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

#ifndef LIBSMARTEREYE2_SENSOR_H
#define LIBSMARTEREYE2_SENSOR_H

#include <memory>
#include <vector>

#include "se_types.h"
#include "core/options.h"

namespace libsmartereye2 {

enum class CameraInfo {
  CAMERA_INFO_NAME, /**< Friendly name */
  CAMERA_INFO_SERIAL_NUMBER, /**< Device serial number */
  CAMERA_INFO_FIRMWARE_VERSION, /**< Primary firmware version */
  CAMERA_INFO_PHYSICAL_PORT, /**< Unique identifier of the port the device is connected to (platform specific) */
  CAMERA_INFO_DEBUG_OP_CODE, /**< If device supports firmware logging, this is the command to send to get logs from firmware */
  CAMERA_INFO_PRODUCT_ID, /**< Product ID as reported in the USB descriptor */
  CAMERA_INFO_CAMERA_LOCKED, /**< True iff EEPROM is locked */
  CAMERA_INFO_USB_TYPE_DESCRIPTOR, /**< Designated USB specification: USB2/USB3 */
  CAMERA_INFO_PRODUCT_LINE, /**< Device product line D400/SR300/L500/T200 */
  CAMERA_INFO_ASIC_SERIAL_NUMBER, /**< ASIC serial number */
  CAMERA_INFO_IP_ADDRESS, /**< IP address for remote camera. */
  CAMERA_INFO_COUNT                            /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
};

// Stereo camera’s intrinsic and extrinsic params.
struct Intrinsics {
  int width;     /**< Width of the image in pixels */
  int height;    /**< Height of the image in pixels */
  float ppx;     /**< Horizontal coordinate of the principal point of the image, as a pixel offset from the left edge */
  float ppy;     /**< Vertical coordinate of the principal point of the image, as a pixel offset from the top edge */
  float fx;      /**< Focal length of the image plane, as a multiple of pixel width */
  float fy;      /**< Focal length of the image plane, as a multiple of pixel height */
  float coeffs[5]; /**< Distortion coefficients */
};

struct Extrinsics {
  float rotation[9];
  float translation[3];
};

struct StreamBackendProfile;
class DeviceInterface;
struct SeOptions;

class SensorInterface {
 public:
  virtual StreamBackendProfile getStrreamProfile(int tag) const = 0;
  virtual StreamBackendProfile getActiveStreams() const = 0;
  virtual void open(const StreamBackendProfile &requests) = 0;
  virtual void close() = 0;

  virtual DeviceInterface &getDevice() = 0;
  // TODO
};

struct SeDevice;

struct SeSensor : public SeOptions, public noncopyable {
  SeSensor(SeDevice parent, SensorInterface *sensor)
      : SeOptions((OptionsInterface*)sensor), parent(parent), sensor(sensor) {}

  SeDevice parent;
  SensorInterface *sensor;
};

class Sensor : public Options {
 public:
  Sensor() : sensor_(nullptr) {}
  explicit Sensor(std::shared_ptr<SeSensor> dev);
  explicit operator std::shared_ptr<SeSensor>();
  Sensor &operator=(const std::shared_ptr<SeSensor> &other);
  Sensor &operator=(const Sensor &other);

  explicit operator bool() const {
    return sensor_ != nullptr;
  }

  const std::shared_ptr<SeSensor> &get() const {
    return sensor_;
  }

  template<typename T>
  bool is() const {
    T extension(*this);
    return extension;
  }

  template<typename T>
  T as() const {
    T extension(*this);
    return extension;
  }

  bool supports(CameraInfo info) const;
  std::string getInfo(CameraInfo info) const;

  void open(const StreamBackendProfile &profile) const;
  void open(const std::vector<StreamBackendProfile> &profiles) const;
  void close() const;

  template<typename T>
  void setNotificationCallback(T callback) const;

  template<typename T>
  void start(T callback) const;
  void stop() const;

  std::vector<StreamBackendProfile> getStreamProfiles() const;
  std::vector<StreamBackendProfile> getActiveStreams() const;
//  std::vecotr<Filter> getRecommendedFilters() const;

 private:
  std::shared_ptr<SeSensor> sensor_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_SENSOR_H
