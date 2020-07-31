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

#include "se_common.h"

#include <utility>
#include "easylogging++.h"

#ifdef ENABLE_LOGGER
#undef ENABLE_LOGGER
INITIALIZE_EASYLOGGINGPP
#endif

namespace libsmartereye2 {

Environment &libsmartereye2::Environment::instance() {
  static Environment env;
  return env;
}

void Environment::setTimeService(std::shared_ptr<platform::TimeService> ts) {
  ts_ = std::move(ts);
}

std::shared_ptr<platform::TimeService> Environment::getTimeService() {
  return ts_;
}

}
