// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef THIRD_PARTY_CEL_C_VALUE_KIND_H_
#define THIRD_PARTY_CEL_C_VALUE_KIND_H_

#include "cel-c/config.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_ValueKind_kNull = 0,
  cel_ValueKind_kBool,
  cel_ValueKind_kInt,
  cel_ValueKind_kUint,
  cel_ValueKind_kDouble,
  cel_ValueKind_kString,
  cel_ValueKind_kBytes,
  cel_ValueKind_kStruct,
  cel_ValueKind_kDuration,
  cel_ValueKind_kTimestamp,
  cel_ValueKind_kList,
  cel_ValueKind_kMap,
  cel_ValueKind_kUnknown,
  cel_ValueKind_kType,
  cel_ValueKind_kError,
  cel_ValueKind_kOpaque,
} cel_ValueKind;

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_VALUE_KIND_H_
