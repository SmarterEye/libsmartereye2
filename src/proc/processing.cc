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

#include "processing.h"
#include "core/frame.h"
#include "se_types.h"

namespace libsmartereye2 {

struct SeProcessingBlock : public SeOptions, public noncopyable {
  explicit SeProcessingBlock(const std::shared_ptr<ProcessingBlockInterface> &block)
      : SeOptions(block.get()), block_(block) {}

  std::shared_ptr<ProcessingBlockInterface> block_;
};

void ProcessingBlock::invoke(Frame frame) const {

}

bool ProcessingBlock::support(CameraInfo info) const {
  return false;
}

std::string ProcessingBlock::getInfo(CameraInfo info) const {
  return std::string();
}

void ProcessingBlock::registerSimpleOption(OptionKey option_key, OptionRange range) {

}

}  // namespace libsmartereye2