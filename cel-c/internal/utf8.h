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

// Internal header providing facilities for working with UTF-8.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_UTF8_H_
#define THIRD_PARTY_CEL_C_INTERNAL_UTF8_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#include "cel-c/internal/config.h"
#include "cel-c/internal/uchar.h"  // IWYU pragma: keep
#include "upb/base/string_view.h"

CEL_BEGIN_DECLS

// _cel_Utf8_kMaxEncodedSize
//
// Maximum number of code units (bytes) used to encode a single code point in
// UTF-8.
#define _cel_Utf8_kMaxEncodedSize 4

// _cel_Utf8_EncodedSize
//
// Returns the number of code units needed to represent the code point when
// encoded for UTF-8.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
static CEL_INLINE size_t _cel_Utf8_EncodedSize(char32_t pnt) {
  return pnt < UINT32_C(0x80)                                   ? 1
         : pnt < UINT32_C(0x800)                                ? 2
         : pnt < UINT32_C(0x10000) || pnt >= UINT32_C(0x110000) ? 3
                                                                : 4;
}

// _cel_Utf8_Decode
//
// Decodes the next code point from the UTF-8 encoded string `str`. The string
// must not be empty. The decoded code point is stored in `pnt` and the number
// of code units consumed is stored in `units`. If an invalid sequence is
// encountered `pnt` will be `_cel_Unicode_kReplacementChar` and `units` will be
// `1`.
CEL_ATTRIBUTE_NOTHROW
void _cel_Utf8_Decode(upb_StringView str, CEL_NULLABLE(char32_t*) pnt,
                      CEL_NULLABLE(size_t*) units);

// _cel_Utf8_Encode
//
// Encodes the code point `pnt` into the buffer `str` which is `len` code units
// in size. Returns the number of code units actually used.
CEL_ATTRIBUTE_NOTHROW
size_t _cel_Utf8_Encode(char32_t pnt, CEL_NONNULL(char*) str, size_t len);

// _cel_Utf8_DecodedSize
//
// Returns the number of code points in the UTF-8 encode string `str`.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
size_t _cel_Utf8_DecodedSize(upb_StringView str);

// _cel_Utf8_IsValid
//
// Tests whether the UTF-8 encoded string `str` is valid UTF-8.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Utf8_IsValid(upb_StringView str);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_UTF8_H_
