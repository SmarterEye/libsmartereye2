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

#ifndef LIBSMARTEREYE2_OPTION_HPP
#define LIBSMARTEREYE2_OPTION_HPP

#include "smartereye2/se_global.hpp"
#include "smartereye2/se_types.hpp"
#include "core_types.hpp"

namespace se2 {

class SMARTEREYE2_API Options {
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

}  // namespace se2

#endif  // LIBSMARTEREYE2_OPTION_HPP
