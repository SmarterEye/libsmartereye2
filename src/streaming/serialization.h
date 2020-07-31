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

#ifndef LIBSMARTEREYE2_SERIALIZATION_H
#define LIBSMARTEREYE2_SERIALIZATION_H

#include "streaming.h"
#include "core/frame.h"
#include "core/options.h"
#include "sensor/notification.h"
#include <chrono>

namespace libsmartereye2 {
namespace device_serializer {

struct sensor_identifier {
  uint32_t device_index;
  uint32_t sensor_index;
};

struct stream_identifier {
  uint32_t device_index;
  uint32_t sensor_index;
  FrameId frame_id;
  uint32_t stream_index;
};

inline bool operator==(const stream_identifier &lhs, const stream_identifier &rhs) {
  return lhs.device_index == rhs.device_index &&
      lhs.sensor_index == rhs.sensor_index &&
      lhs.frame_id == rhs.frame_id &&
      lhs.stream_index == rhs.stream_index;
}

inline bool operator<(const stream_identifier &lhs, const stream_identifier &rhs) {
  return std::make_tuple(lhs.device_index, lhs.sensor_index, lhs.frame_id, lhs.stream_index)
      < std::make_tuple(rhs.device_index, rhs.sensor_index, rhs.frame_id, rhs.stream_index);
}

inline std::ostream &operator<<(std::ostream &os, const stream_identifier &id) {
  os << id.device_index << "/" << id.sensor_index << "/" << static_cast<int>(id.frame_id) << "/" << id.stream_index;
  return os;
}

using nanoseconds = std::chrono::duration<uint64_t, std::nano>;

class serialized_data : public std::enable_shared_from_this<serialized_data> {
 protected:
  enum serialized_data_type {
    invalid,
    end_of_file,
    frame,
    option,
    invalid_frame,
    notificaion,
    max
  };

 public:
  explicit serialized_data(const device_serializer::nanoseconds &timestamp = device_serializer::nanoseconds()) :
      _timestamp(timestamp) {
  }
  virtual ~serialized_data() = default;

  template<typename T>
  bool is() const {
    return T::get_type() == type();
  }

  template<typename T>
  std::shared_ptr<T> as() const {
    if (!is<T>())
      return nullptr;

    switch (T::get_type()) {
      case end_of_file:
      case frame:
      case option:
      case notificaion:return std::static_pointer_cast<T>(std::const_pointer_cast<serialized_data>(shared_from_this()));
    }
    return nullptr;
  }

  virtual device_serializer::nanoseconds get_timestamp() const {
    return _timestamp;
  }

  virtual serialized_data_type type() const = 0;

 private:
  device_serializer::nanoseconds _timestamp;
};

class serialized_frame : public serialized_data {
 public:
  serialized_frame(device_serializer::nanoseconds time, stream_identifier id, FrameHolder f) :
      serialized_data(time),
      stream_id(id),
      frame(std::move(f)) {}

  stream_identifier stream_id;
  FrameHolder frame;

  static serialized_data_type get_type() {
    return serialized_data_type::frame;
  }
  serialized_data_type type() const override {
    return serialized_frame::get_type();
  }
};

class serialized_invalid_frame : public serialized_frame {
 public:
  serialized_invalid_frame(device_serializer::nanoseconds time, stream_identifier id)
      : serialized_frame(time, id, static_cast<FrameHolder>(nullptr)) {
    
  }
  
  static serialized_data_type get_type() {
    return serialized_data_type::invalid_frame;
  }
  serialized_data_type type() const override {
    return serialized_invalid_frame::get_type();
  }
};

class serialized_option : public serialized_data {
 public:
  serialized_option(device_serializer::nanoseconds time,
                    sensor_identifier id,
                    OptionKey opt_id,
                    std::shared_ptr<libsmartereye2::Option> o) :
      serialized_data(time),
      sensor_id(id), option_id(opt_id), option(o) {}
  sensor_identifier sensor_id;
  std::shared_ptr<libsmartereye2::Option> option;
  OptionKey option_id;
  static serialized_data_type get_type() {
    return serialized_data_type::option;
  }
  serialized_data_type type() const override {
    return serialized_option::get_type();
  }
};

class serialized_end_of_file : public serialized_data {
 public:
  serialized_end_of_file() = default;
  static serialized_data_type get_type() {
    return serialized_data_type::end_of_file;
  }
  serialized_data_type type() const override {
    return serialized_end_of_file::get_type();
  }
};

class serialized_notification : public serialized_data {
 public:
  serialized_notification(device_serializer::nanoseconds time, sensor_identifier id, const NotificationPrivate &n) :
      serialized_data(time),
      sensor_id(id), notif(n) {

  }

  sensor_identifier sensor_id;
  NotificationPrivate notif;

  static serialized_data_type get_type() {
    return serialized_data_type::notificaion;
  }

  serialized_data_type type() const override {
    return serialized_notification::get_type();
  }
};

//using device_extrinsics = std::map<std::tuple<size_t, FrameId, size_t, FrameId>, rs2_extrinsics>;

/** @brief
*  Defines return codes that SDK interfaces
*  use.  Negative values indicate errors, a zero value indicates success,
*  and positive values indicate warnings.
*/
enum status {
  /* success */
  status_no_error = 0,                /**< Operation succeeded without any warning */

  /* errors */
  status_feature_unsupported = -1,    /**< Unsupported feature */
  status_param_unsupported = -2,      /**< Unsupported parameter(s) */
  status_item_unavailable = -3,       /**< Item not found/not available */
  status_key_already_exists = -4,     /**< Key already exists in the data structure */
  status_invalid_argument = -5,       /**< Argument passed to the method is invalid */
  status_allocation_failled = -6,     /**< Failure in allocation */

  status_file_write_failed = -401,    /**< Failure in open file in WRITE mode */
  status_file_read_failed = -402,     /**< Failure in open file in READ mode */
  status_file_close_failed = -403,    /**< Failure in close a file handle */
  status_file_eof = -404,             /**< EOF */
};

class writer {
 public:
//  virtual void write_device_description(const device_snapshot &device_description) = 0;
  virtual void write_frame(const stream_identifier &stream_id, const nanoseconds &timestamp, FrameHolder &&frame) = 0;
//  virtual void write_snapshot(uint32_t device_index,
//                              const nanoseconds &timestamp,
//                              rs2_extension type,
//                              const std::shared_ptr<extension_snapshot> &snapshot) = 0;
//  virtual void write_snapshot(const sensor_identifier &sensor_id,
//                              const nanoseconds &timestamp,
//                              rs2_extension type,
//                              const std::shared_ptr<extension_snapshot> &snapshot) = 0;
//  virtual void write_notification(const sensor_identifier &stream_id,
//                                  const nanoseconds &timestamp,
//                                  const notification &n) = 0;
  virtual const std::string &get_file_name() const = 0;
  virtual ~writer() = default;
};

class reader {
 public:
  virtual ~reader() = default;
//  virtual device_snapshot query_device_description(const nanoseconds &time) = 0;
  virtual std::shared_ptr<serialized_data> read_next_data() = 0;
  virtual void seek_to_time(const nanoseconds &time) = 0;
  virtual nanoseconds query_duration() const = 0;
  virtual void reset() = 0;
  virtual void enable_stream(const std::vector<device_serializer::stream_identifier> &stream_ids) = 0;
  virtual void disable_stream(const std::vector<device_serializer::stream_identifier> &stream_ids) = 0;
  virtual const std::string &get_file_name() const = 0;
  virtual std::vector<std::shared_ptr<serialized_data>> fetch_last_frames(const nanoseconds &seek_time) = 0;
};

}  // namespace serializer
}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_SERIALIZATION_H
