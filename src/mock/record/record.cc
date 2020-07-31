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

#include "record.h"

#include <utility>

namespace libsmartereye2 {
namespace platform {

Recording::Recording(std::shared_ptr<TimeService> ts, std::shared_ptr<PlaybackDeviceWatcher> watcher)
    : ts_(std::move(ts)), watcher_(std::move(watcher)) {

}

void Recording::save(const std::string &filename, const std::string &section, bool append) const {
  // TODO
}

std::shared_ptr<Recording> Recording::load(const std::string &filename,
                                           const std::string &section,
                                           std::shared_ptr<PlaybackDeviceWatcher> watcher) {
  if (!util::fileExists(filename)) {
    throw std::runtime_error("Recording file not found!");
  }

  auto result = std::make_shared<Recording>(nullptr, watcher);
  return result;
}

int Recording::saveBlob(const char *ptr, size_t size) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::vector<uint8_t> holder;
  holder.resize(size);
  std::copy(ptr, ptr + size, holder.data());
  return 0;
}

}  // namespace platform
}  // namespace libsmartereye2
