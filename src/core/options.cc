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

#include "options.h"
#include "se_types.h"

#include <vector>

namespace libsmartereye2 {

bool Options::supports(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options_interface->supportsOption(option);
}

std::string Options::getOptionDescription(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options_interface->getOption(option).getDescription();
}

std::string Options::getOptionName(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options_interface->getOptionName(option);
}

std::string Options::getOptionValueDescription(OptionKey option, float val) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options_interface->getOption(option).getValueDescription(val);
}

float Options::getOption(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options_interface->getOption(option).query();
}

OptionRange Options::getOptionRange(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options_interface->getOption(option).getRange();
}

void Options::setOption(OptionKey option, float value) const {
  CHECK_PTR_NOT_NULL(options_);
  options_->options_interface->getOption(option).set(value);
}

bool Options::isOptionReadonly(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options_interface->getOption(option).isReadonly();
}

std::vector<OptionKey> Options::getSupportedOptions() const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options_interface->getSupportedOptions();
}

Options &Options::operator=(const Options &other) {
  CHECK_PTR_NOT_NULL(options_);
  options_ = other.options_;
  return *this;
}

}  // namespace libsmartereye2