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
#include "core/options.hpp"

#include <vector>

namespace libsmartereye2 {

Option &OptionsContainer::getOption(OptionKey id) {
  return const_cast<Option &>(const_cast<const OptionsContainer *>(this)->getOption(id));
}

const Option &OptionsContainer::getOption(OptionKey id) const {
  auto it = _options.find(id);
  if (it == _options.end()) {
    throw std::runtime_error(toString()
                                 << "Device does not support option "
                                 << getOptionName(id) << "!");
  }
  return *it->second;
}

bool OptionsContainer::supportsOption(OptionKey id) const {
  auto it = _options.find(id);
  if (it == _options.end()) return false;
  return it->second->isEnabled();
}

std::vector<OptionKey> OptionsContainer::getSupportedOptions() const {
  std::vector<OptionKey> options;
  for (const auto &option : _options)
    options.push_back(option.first);

  return options;
}

std::string OptionsContainer::getOptionName(OptionKey id) const {
  // TODO
  return std::string();
}

}  // namespace libsmartereye2

namespace se2 {

bool Options::supports(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options->supportsOption(option);
}

std::string Options::getOptionDescription(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options->getOption(option).getDescription();
}

std::string Options::getOptionName(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options->getOptionName(option);
}

std::string Options::getOptionValueDescription(OptionKey option, float val) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options->getOption(option).getValueDescription(val);
}

float Options::getOption(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options->getOption(option).query();
}

OptionRange Options::getOptionRange(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options->getOption(option).getRange();
}

void Options::setOption(OptionKey option, float value) const {
  CHECK_PTR_NOT_NULL(options_);
  options_->options->getOption(option).set(value);
}

bool Options::isOptionReadonly(OptionKey option) const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options->getOption(option).isReadonly();
}

std::vector<OptionKey> Options::getSupportedOptions() const {
  CHECK_PTR_NOT_NULL(options_);
  return options_->options->getSupportedOptions();
}

Options &Options::operator=(const Options &other) {
  CHECK_PTR_NOT_NULL(options_);
  options_ = other.options_;
  return *this;
}

}  // namespace se2
