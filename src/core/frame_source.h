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

#ifndef LIBSMARTEREYE2_FRAME_SOURCE_H
#define LIBSMARTEREYE2_FRAME_SOURCE_H

#include <cstdint>
#include <memory>
#include <map>
#include <mutex>
#include <atomic>

#include "se_callbacks.hpp"
#include "se_common.h"
#include "frame.h"
#include "frame_archive.h"
#include "metadata_parser.h"

namespace libsmartereye2 {

class Option;

class FrameSource {
 public:
  explicit FrameSource(uint32_t max_publish_list_size = 256);

  void init(std::shared_ptr<MetadataParserMap> metadata_parsers);

  CallbackInvocationHolder begin_callback();

  void reset();

  std::shared_ptr<Option> get_published_size_option();

  FrameInterface *alloc_frame(SeExtension type,
                              size_t size,
                              const FrameExtension& additional_data,
                              bool requires_memory) const;

  void set_callback(FrameCallbackPtr callback);
  FrameCallbackPtr get_callback() const;

  void invoke_callback(FrameHolder frame) const;

  void flush() const;

  virtual ~FrameSource() { flush(); }

  double get_time() const { return time_service_ ? time_service_->getTime() : 0; }

  void set_sensor(const std::shared_ptr<SensorInterface> &s);

  template<class T>
  void add_extension(SeExtension ex) {
    archives_[ex] = std::make_shared<FrameArchive<T >>
    (&max_publish_list_size_, time_service_, metadata_parsers_);
  }

  void set_max_publish_list_size(int qsize) { max_publish_list_size_ = qsize; }

 private:
//  friend class syncer_process_unit;

  std::map<SeExtension, std::shared_ptr<ArchiveInterface>> archives_;
  FrameCallbackPtr frame_callback_;
  mutable std::mutex callback_mutex_;

  std::atomic<uint32_t> max_publish_list_size_;
  std::shared_ptr<platform::TimeService> time_service_;
  std::shared_ptr<MetadataParserMap> metadata_parsers_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_SOURCE_H
