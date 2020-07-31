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

#ifndef LIBSMARTEREYE2_SE_COMMON_H
#define LIBSMARTEREYE2_SE_COMMON_H

#define ENABLE_LOGGER

#include <atomic>
#include <memory>
#include <chrono>
#include <fstream>

namespace libsmartereye2 {

namespace util {

static bool fileExists(const std::string &filename) {
  std::ifstream f(filename);
  return f.good();
}

class UniqueId {
 public:
  static uint64_t generateId() {
    static std::atomic<uint64_t> id(0);
    return ++id;
  }

  UniqueId(const UniqueId &) = delete;
  UniqueId &operator=(const UniqueId &) = delete;
};

}  // namespace util

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

class Environment {
 public:
  static Environment &instance();

  int generateStreamId() { return _stream_id.fetch_add(1); }
  void setTimeService(std::shared_ptr<platform::TimeService> ts);
  std::shared_ptr<platform::TimeService> getTimeService();

  Environment(const Environment &) = delete;
  Environment(const Environment &&) = delete;
  Environment operator=(const Environment &) = delete;
  Environment operator=(const Environment &&) = delete;

 private:
  std::atomic<int> _stream_id{};
  std::shared_ptr<platform::TimeService> ts_;

  Environment() { _stream_id = 0; }
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_SE_COMMON_H
