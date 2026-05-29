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

#include "cel-c/src/utf8.h"

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/uchar.h"  // IWYU pragma: keep
#include "cel-c/src/unicode.h"
#include "upb/base/string_view.h"

#define _cel_kUtf8RuneSelf 0x80

#define _cel_kUtf8Low 0x80
#define _cel_kUtf8High 0xbf

#define _cel_kUtf8MaskX 0x3f
#define _cel_kUtf8Mask2 0x1f
#define _cel_kUtf8Mask3 0xf
#define _cel_kUtf8Mask4 0x7

#define _cel_kUtf8TX 0x80
#define _cel_kUtf8T2 0xc0
#define _cel_kUtf8T3 0xe0
#define _cel_kUtf8T4 0xf0

#define _cel_kUtf8XX 0xf1
#define _cel_kUtf8AS 0xf0
#define _cel_kUtf8S1 0x02
#define _cel_kUtf8S2 0x13
#define _cel_kUtf8S3 0x03
#define _cel_kUtf8S4 0x23
#define _cel_kUtf8S5 0x34
#define _cel_kUtf8S6 0x04
#define _cel_kUtf8S7 0x44

// NOLINTBEGIN
// clang-format off
static const uint8_t _cel_kUtf8Leading[256] = {
  _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, // 0x00-0x0F
  _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, // 0x10-0x1F
  _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, // 0x20-0x2F
  _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, // 0x30-0x3F
  _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, // 0x40-0x4F
  _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, // 0x50-0x5F
  _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, // 0x60-0x6F
  _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, _cel_kUtf8AS, // 0x70-0x7F
  _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, // 0x80-0x8F
  _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, // 0x90-0x9F
  _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, // 0xA0-0xAF
  _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, // 0xB0-0xBF
  _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, // 0xC0-0xCF
  _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, _cel_kUtf8S1, // 0xD0-0xDF
  _cel_kUtf8S2, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S3, _cel_kUtf8S4, _cel_kUtf8S3, _cel_kUtf8S3, // 0xE0-0xEF
  _cel_kUtf8S5, _cel_kUtf8S6, _cel_kUtf8S6, _cel_kUtf8S6, _cel_kUtf8S7, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, _cel_kUtf8XX, // 0xF0-0xFF
};
// clang-format on
// NOLINTEND

static const uint8_t _cel_kUtf8Accept[32] = {
    _cel_kUtf8Low, _cel_kUtf8High,
    0xa0,          _cel_kUtf8High,
    _cel_kUtf8Low, 0x9f,
    0x90,          _cel_kUtf8High,
    _cel_kUtf8Low, 0x8f,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
    0x0,           0x0,
};

static void _cel_Utf8_DecodeResult(char32_t pnt, size_t units,
                                   CEL_NULLABLE(char32_t*) out_pnt,
                                   CEL_NULLABLE(size_t*) out_units) {
  if (out_pnt != cel_nullptr) {
    *out_pnt = pnt;
  }
  if (out_units != cel_nullptr) {
    *out_units = units;
  }
}

static void _cel_Utf8_DecodeFailed(CEL_NULLABLE(char32_t*) pnt,
                                   CEL_NULLABLE(size_t*) units) {
  _cel_Utf8_DecodeResult(_cel_Unicode_kReplacementChar, 1, pnt, units);
}

void _cel_Utf8_Decode(upb_StringView str, CEL_NULLABLE(char32_t*) pnt,
                      CEL_NULLABLE(size_t*) units) {
  CEL_ASSERT_NOT_NULL(str.data);
  CEL_ASSERT_GT(str.size, 0);
  CEL_ASSERT(pnt != cel_nullptr || units != cel_nullptr);
  CEL_NONNULL(const char*) data = str.data;
  size_t len = str.size;
  const uint8_t b = (uint8_t)(*data++);
  --len;
  if (b < _cel_kUtf8RuneSelf) {
    _cel_Utf8_DecodeResult(b, 1, pnt, units);
    return;
  }
  const uint8_t leading = _cel_kUtf8Leading[b];
  if (CEL_UNLIKELY(leading == _cel_kUtf8XX)) {
    _cel_Utf8_DecodeFailed(pnt, units);
    return;
  }
  const size_t size = ((size_t)(leading & 7)) - 1;
  CEL_ASSERT(size >= 1 && size <= 3);
  if (CEL_UNLIKELY(size > len)) {
    _cel_Utf8_DecodeFailed(pnt, units);
    return;
  }
  const uint8_t* const accept = &_cel_kUtf8Accept[(leading >> 4) * 2];
  const uint8_t b1 = (uint8_t)(*data++);
  if (CEL_UNLIKELY(b1 < accept[0] || b1 > accept[1])) {
    _cel_Utf8_DecodeFailed(pnt, units);
    return;
  }
  if (size == 1) {
    _cel_Utf8_DecodeResult((((char32_t)(b & _cel_kUtf8Mask2)) << 6) |
                               ((char32_t)(b1 & _cel_kUtf8MaskX)),
                           2, pnt, units);
    return;
  }
  const uint8_t b2 = (uint8_t)(*data++);
  if (CEL_UNLIKELY(b2 < _cel_kUtf8Low || b2 > _cel_kUtf8High)) {
    _cel_Utf8_DecodeFailed(pnt, units);
    return;
  }
  if (size == 2) {
    _cel_Utf8_DecodeResult((((char32_t)(b & _cel_kUtf8Mask3)) << 12) |
                               (((char32_t)(b1 & _cel_kUtf8MaskX)) << 6) |
                               ((char32_t)(b2 & _cel_kUtf8MaskX)),
                           3, pnt, units);
    return;
  }
  const uint8_t b3 = (uint8_t)(*data++);
  if (CEL_UNLIKELY(b3 < _cel_kUtf8Low || b3 > _cel_kUtf8High)) {
    _cel_Utf8_DecodeFailed(pnt, units);
    return;
  }
  CEL_ASSERT_EQ(size, 3);
  _cel_Utf8_DecodeResult((((char32_t)(b & _cel_kUtf8Mask4)) << 18) |
                             (((char32_t)(b1 & _cel_kUtf8MaskX)) << 12) |
                             (((char32_t)(b2 & _cel_kUtf8MaskX)) << 6) |
                             ((char32_t)(b3 & _cel_kUtf8MaskX)),
                         4, pnt, units);
}

size_t _cel_Utf8_Encode(char32_t pnt, CEL_NONNULL(char*) str, size_t len) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_GT(len, 0);
  if (CEL_UNLIKELY(!_cel_Unicode_IsValid(pnt))) {
    pnt = _cel_Unicode_kReplacementChar;
  }
  size_t units;
  if (pnt <= 0x7f) {
    CEL_ASSERT_GE(len, 1);
    *str = (char)((uint8_t)pnt);
    units = 1;
  } else if (pnt <= 0x7ff) {
    CEL_ASSERT_GE(len, 2);
    *str++ = (char)(_cel_kUtf8T2 | ((uint8_t)(pnt >> 6)));
    *str = (char)(_cel_kUtf8TX | (((uint8_t)pnt) & _cel_kUtf8MaskX));
    units = 2;
  } else if (pnt <= 0xffff) {
    CEL_ASSERT_GE(len, 3);
    *str++ = (char)(_cel_kUtf8T3 | ((uint8_t)(pnt >> 12)));
    *str++ = (char)(_cel_kUtf8TX | (((uint8_t)(pnt >> 6)) & _cel_kUtf8MaskX));
    *str = (char)(_cel_kUtf8TX | (((uint8_t)pnt) & _cel_kUtf8MaskX));
    units = 3;
  } else {
    CEL_ASSERT_GE(len, 4);
    *str++ = (char)(_cel_kUtf8T4 | ((uint8_t)(pnt >> 18)));
    *str++ = (char)(_cel_kUtf8TX | (((uint8_t)(pnt >> 12)) & _cel_kUtf8MaskX));
    *str++ = (char)(_cel_kUtf8TX | (((uint8_t)(pnt >> 6)) & _cel_kUtf8MaskX));
    *str = (char)(_cel_kUtf8TX | (((uint8_t)pnt) & _cel_kUtf8MaskX));
    units = 4;
  }
  return units;
}

size_t _cel_Utf8_DecodedSize(upb_StringView str) {
  size_t pnts = 0;
  size_t units;
  while (str.size > 0) {
    _cel_Utf8_Decode(str, cel_nullptr, &units);
    ++pnts;
    str.data += units;
    str.size -= units;
  }
  return pnts;
}

bool _cel_Utf8_IsValid(upb_StringView str) {
  char32_t pnt;
  size_t units;
  while (str.size > 0) {
    _cel_Utf8_Decode(str, &pnt, &units);
    if (pnt == _cel_Unicode_kReplacementChar && units == 1) {
      return false;
    }
    str.data += units;
    str.size -= units;
  }
  return true;
}
