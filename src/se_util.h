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

#ifndef LIBSMARTEREYE2_SE_UTIL_H
#define LIBSMARTEREYE2_SE_UTIL_H

#include <stdexcept>

namespace libsmartereye2 {

class noncopyable {
 public:
  noncopyable(const noncopyable &) = delete;
  noncopyable &operator=(const noncopyable &) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

namespace platform {

class TimeService {
 public:
  virtual double getTime() const = 0;
};

class OsTimeService : public TimeService {
 public:
  double getTime() const override {
    return std::chrono::duration<double, std::milli>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
  }
};

}  // namespace platform

#define CHECK_PTR_NOT_NULL(ARG) if (!(ARG)) throw std::runtime_error("null pointer passed for argument \"" #ARG "\"");
#define CHECK_RANGE(ARG, MIN, MAX) \
  if((ARG) < (MIN) || (ARG) > (MAX)) { \
    std::ostringstream ss; ss << "out of range value for argument \"" #ARG "\""; \
    throw librealsense::invalid_value_exception(ss.str()); \
  }

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_SE_UTIL_H
