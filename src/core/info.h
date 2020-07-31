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

#ifndef LIBSMARTEREYE2_INFO_H
#define LIBSMARTEREYE2_INFO_H

#include <map>
#include "se_types.hpp"

namespace libsmartereye2 {

class InfoInterface {
 public:
  virtual std::string getInfo(CameraInfo info) const = 0;
  virtual bool supportsInfo(CameraInfo info) const = 0;

  virtual ~InfoInterface() = default;
};

class InfoContainer : public virtual InfoInterface {
 public:
  std::string getInfo(CameraInfo info) const override { return std::string(); }
  bool supportsInfo(CameraInfo info) const override {return false;}

  void registerInfo(CameraInfo info, const std::string &val) {}
  void updateInfo(CameraInfo info, const std::string &val) {}

 private:
  std::map<CameraInfo, std::string> _camera_info;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_INFO_H
