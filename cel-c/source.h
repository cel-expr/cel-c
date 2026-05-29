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

#ifndef THIRD_PARTY_CEL_C_SOURCE_H_
#define THIRD_PARTY_CEL_C_SOURCE_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stdint.h>

#include "cel-c/config.h"

CEL_BEGIN_DECLS

// cel_SourceRange represents a range of positions, where `begin` is inclusive
// and `end` is exclusive. If `begin` is not `-1` and `end` is `-1`, then the
// end position is not known.
typedef struct {
  int32_t begin;
  int32_t end;
} cel_SourceRange;

#define cel_SourceRange(begin, end) ((cel_SourceRange){(begin), (end)})

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_SourceRange_Equals(cel_SourceRange lhs,
                                              cel_SourceRange rhs) {
  return lhs.begin == rhs.begin && lhs.end == rhs.end;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SOURCE_H_
