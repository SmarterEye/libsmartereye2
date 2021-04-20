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

#ifndef LIBSMARTEREYE2_OPTION_H
#define LIBSMARTEREYE2_OPTION_H

#include <vector>
#include <map>
#include <memory>

#include "se_util.hpp"
#include "core/core_types.hpp"

namespace libsmartereye2 {
class OptionsInterface;
}

struct SeOptions {
  explicit SeOptions(libsmartereye2::OptionsInterface *options) : options(options) {}
  libsmartereye2::OptionsInterface *options;
};

namespace libsmartereye2 {

using namespace se2;

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

class OptionsContainer : public virtual OptionsInterface {
 public:
  Option &getOption(OptionKey id) override;
  const Option &getOption(OptionKey id) const override;
  bool supportsOption(OptionKey id) const override;
  std::vector<OptionKey> getSupportedOptions() const override;
  std::string getOptionName(OptionKey id) const override;

  void registerOption(OptionKey id, std::shared_ptr<Option> option) {
    _options[id] = std::move(option);
  }

  void unregisterOption(OptionKey id) {
    _options.erase(id);
  }

 protected:
  std::map<OptionKey, std::shared_ptr<Option>> _options;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_OPTION_H
