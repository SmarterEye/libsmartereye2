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

#ifndef LIBSMARTEREYE2_METADATA_PARSER_H
#define LIBSMARTEREYE2_METADATA_PARSER_H

#include <cstdint>
#include <memory>
#include <map>

namespace libsmartereye2 {

class FrameData;

class MetadataParserBase {
 public:
  virtual int64_t get(const FrameData &frame_data) const = 0;
  virtual bool supports(const FrameData &frame_data) const = 0;
};

using MetadataParserMap = std::map<int64_t, std::shared_ptr<MetadataParserBase>>;

class ConstantParser : public MetadataParserBase {

};



}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_METADATA_PARSER_H
