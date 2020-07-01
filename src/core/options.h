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

#ifndef LIBSMARTEREYE2_OPTION_H_
#define LIBSMARTEREYE2_OPTION_H_

#include <vector>

#include "se_common.h"
#include "se_util.h"

namespace libsmartereye2 {

enum class OptionKey {
  FRAMES_QUEUE_SIZE,
  STREAM_FILTER,
  STREAM_FORMAT_FILTER,
  STREAM_INDEX_FILTER,
  COLOR_SCHEME,
  MIN_DISTANCE,
  MAX_DISTANCE,
  MOTION_MODULE_TEMPERATURE,
  MOTION_RANGE,
  LASER_POWER,
  // TODO
};

struct OptionRange {
  float min;
  float max;
  float step;
  float def;
};

struct SeOptions;

class Option : public noncopyable {
 public:
  virtual void set(float value) = 0;
  virtual float query() const = 0;
  virtual OptionRange getRange() const = 0;
  virtual bool isEnabled() const = 0;
  virtual bool isReadonly() const { return false; }
  virtual std::string getDescription() const = 0;
  virtual std::string getValueDescription(float value) const { return std::string(); }
};

class OptionsInterface {
 public:
  virtual Option &getOption(OptionKey id) = 0;
  virtual const Option &getOption(OptionKey id) const = 0;
  virtual bool supportsOption(OptionKey id) const = 0;
  virtual std::vector<OptionKey> getSupportedOptions() const = 0;
  virtual std::string getOptionName(OptionKey id) const = 0;
};

class Options : public noncopyable {
 public:
  bool supports(OptionKey option) const;
  std::string getOptionDescription(OptionKey option) const;
  std::string getOptionName(OptionKey option) const;
  std::string getOptionValueDescription(OptionKey option, float val) const;
  float getOption(OptionKey option) const;
  OptionRange getOptionRange(OptionKey option) const;
  void setOption(OptionKey option, float value) const;
  bool isOptionReadonly(OptionKey option) const;
  std::vector<OptionKey> getSupportedOptions() const;

  Options &operator=(const Options &other);
  Options(const Options &other) : options_(other.options_) {}

  virtual ~Options() = default;

 protected:
  explicit Options(SeOptions *opt = nullptr) : options_(opt) {}

  template<typename T>
  Options &operator=(const T &dev) {
    options_ = reinterpret_cast<SeOptions *>(dev.get());
    return *this;
  }

 private:
  SeOptions *options_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_OPTION_H_
