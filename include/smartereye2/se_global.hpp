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

#ifndef LIBSMARTEREYE2_SE_GLOBAL_HPP
#define LIBSMARTEREYE2_SE_GLOBAL_HPP

#pragma warning(disable : 4275)        /* disable: C4275: non dll-interface class used as base for dll-interface class */
#pragma warning(disable : 4251)        /* disable: C4251: class needs to have dll-interface to be used by clients of class */
#ifdef WIN32
#define SMARTEREYE2_API __declspec(dllexport)
#else
#define SMARTEREYE2_API
#endif

#endif //LIBSMARTEREYE2_SE_GLOBAL_HPP
