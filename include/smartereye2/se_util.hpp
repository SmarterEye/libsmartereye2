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
#include <chrono>
#include <sstream>
#include <vector>

class noncopyable {
 public:
  noncopyable(const noncopyable &) = delete;
  noncopyable &operator=(const noncopyable &) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

struct toString {
  std::ostringstream ss;
  template<class T>
  toString &operator<<(const T &val) {
    ss << val;
    return *this;
  }
  operator std::string() const { return ss.str(); }
};

template<class T>
std::vector<std::shared_ptr<T>> subtractSets(const std::vector<std::shared_ptr<T>> &first,
                                              const std::vector<std::shared_ptr<T>> &second) {
  std::vector<std::shared_ptr<T>> results;
  std::for_each(first.begin(), first.end(), [&](std::shared_ptr<T> data) {
    if (std::find_if(second.begin(), second.end(), [&](std::shared_ptr<T> new_dev) {
      return data == new_dev;
    }) == second.end()) {
      results.push_back(data);
    }
  });
  return results;
}

#define CHECK_PTR_NOT_NULL(ARG) if (!(ARG)) throw std::runtime_error("null pointer passed for argument \"" #ARG "\"");

#endif  // LIBSMARTEREYE2_SE_UTIL_H
