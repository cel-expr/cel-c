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

#ifndef THIRD_PARTY_CEL_C_TYPE_KIND_H_
#define THIRD_PARTY_CEL_C_TYPE_KIND_H_

#include "cel-c/config.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_OPEN_ENUM {
  cel_TypeKind_kDyn = 0,
  cel_TypeKind_kNull,
  cel_TypeKind_kBool,
  cel_TypeKind_kInt,
  cel_TypeKind_kUint,
  cel_TypeKind_kDouble,
  cel_TypeKind_kString,
  cel_TypeKind_kBytes,
  cel_TypeKind_kStruct,
  cel_TypeKind_kDuration,
  cel_TypeKind_kTimestamp,
  cel_TypeKind_kList,
  cel_TypeKind_kMap,
  cel_TypeKind_kUnknown,
  cel_TypeKind_kType,
  cel_TypeKind_kError,
  cel_TypeKind_kAny,
  cel_TypeKind_kOpaque,

  cel_TypeKind_kBoolWrapper,
  cel_TypeKind_kIntWrapper,
  cel_TypeKind_kUintWrapper,
  cel_TypeKind_kDoubleWrapper,
  cel_TypeKind_kStringWrapper,
  cel_TypeKind_kBytesWrapper,

  cel_TypeKind_kTypeParam,
  cel_TypeKind_kFunction,
  cel_TypeKind_kEnum,
} cel_TypeKind;

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_TYPE_KIND_H_
