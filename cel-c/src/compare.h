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

#ifndef THIRD_PARTY_CEL_C_SRC_COMPARE_H_
#define THIRD_PARTY_CEL_C_SRC_COMPARE_H_

#include "cel-c/config.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  _cel_PartialOrdering_kEquivalent = 0,
  _cel_PartialOrdering_kLess = -1,
  _cel_PartialOrdering_kGreater = 1,
  _cel_PartialOrdering_kUnordered = -127,
} _cel_PartialOrdering;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_PartialOrdering _cel_PartialOrdering_FromInt(int value) {
  return value < 0   ? _cel_PartialOrdering_kLess
         : value > 0 ? _cel_PartialOrdering_kGreater
                     : _cel_PartialOrdering_kEquivalent;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_COMPARE_H_
