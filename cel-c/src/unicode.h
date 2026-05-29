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

// Internal header providing utilities for working with unicode.

#ifndef THIRD_PARTY_CEL_C_SRC_UNICODE_H_
#define THIRD_PARTY_CEL_C_SRC_UNICODE_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stdint.h>

#include "cel-c/config.h"
#include "cel-c/src/uchar.h"  // IWYU pragma: keep

CEL_BEGIN_DECLS

// _cel_Unicode_kReplacementChar
//
// Unicode code point used as a replacement when invalid code units or code
// points are encountered.
#define _cel_Unicode_kReplacementChar ((char32_t)0xfffd)

// _cel_Unicode_IsValid
//
// Tests whether the Unicode code point is within the valid range. It does not
// test whether it is actually defined.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Unicode_IsValid(char32_t pnt) {
  return pnt < UINT32_C(0xd800) ||
         (pnt >= UINT32_C(0xe000) && pnt < UINT32_C(0x110000));
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_SRC_UNICODE_H_
