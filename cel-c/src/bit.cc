// Copyright 2024 Google LLC
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

#include "cel-c/src/bit.h"

#include <limits.h>  // IWYU pragma: keep
#include <stdint.h>  // IWYU pragma: keep

#include "cel-c/config.h"

#if CEL_HAVE_INCLUDE(<stdbit.h>)
#include <stdbit.h>
#endif

#if !(defined(__STDC_VERSION_STDBIT_H__) && \
      __STDC_VERSION_STDBIT_H__ >= 202311L)

#if !(((defined(__GNUC__) && !defined(__clang__)) ||                 \
       CEL_HAVE_BUILTIN(__builtin_clz)) ||                           \
      (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                             defined(_M_X86) || defined(_M_ARM))))
static const unsigned char _cel_kLeadingZerosDeBruijn[32] = {
    0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31,
};

extern "C" int _cel_leading_zeros_ui(unsigned int x) {
  CEL_STATIC_ASSERT(sizeof(x) == 4);
  if (x == 0) {
    return sizeof(x) * CHAR_BIT;
  }
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return (
      int)_cel_kLeadingZerosDeBruijn[((unsigned int)(x * 0x07c4acddu)) >> 27];
}
#endif

#if !(((defined(__GNUC__) && !defined(__clang__)) ||                 \
       CEL_HAVE_BUILTIN(__builtin_clzl)) ||                          \
      (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                             defined(_M_X86) || defined(_M_ARM))))
extern "C" int _cel_leading_zeros_ul(unsigned long x) {
  CEL_STATIC_ASSERT(sizeof(x) == 4 || sizeof(x) == 8);
#if LONG_MAX == INT32_MAX
  return _cel_leading_zeros_ui((unsigned int)x);
#elif LONG_MAX == INT64_MAX
  return _cel_leading_zeros_ull((unsigned long long)x);
#else
#error Unreachable.
#endif
}
#endif

#if !(((defined(__GNUC__) && !defined(__clang__)) || \
       CEL_HAVE_BUILTIN(__builtin_clzll)) ||         \
      (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))))
extern "C" int _cel_leading_zeros_ull(unsigned long long x) {
  CEL_STATIC_ASSERT(sizeof(x) == 8);
  int count = _cel_leading_zeros_ui((unsigned int)(x >> 32));
  return count == sizeof(unsigned int) * CHAR_BIT
             ? count + _cel_leading_zeros_ui((unsigned int)x)
             : count;
}
#endif

#if !(((defined(__GNUC__) && !defined(__clang__)) ||                 \
       CEL_HAVE_BUILTIN(__builtin_ctz)) ||                           \
      (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                             defined(_M_X86) || defined(_M_ARM))))
static const unsigned char _cel_kTralingZerosDeBruijn[32] = {
    0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9,
};

extern "C" int _cel_trailing_zeros_ui(unsigned int x) {
  CEL_STATIC_ASSERT(sizeof(x) == 4);
  if (x == 0) {
    return sizeof(x) * CHAR_BIT;
  }
  return (
      int)_cel_kTralingZerosDeBruijn[((unsigned int)((v & -v) * 0x077cb531u)) >>
                                     27];
}
#endif

#if !(((defined(__GNUC__) && !defined(__clang__)) ||                 \
       CEL_HAVE_BUILTIN(__builtin_ctzl)) ||                          \
      (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                             defined(_M_X86) || defined(_M_ARM))))
extern "C" int _cel_trailing_zeros_ul(unsigned long x) {
  CEL_STATIC_ASSERT(sizeof(x) == 4 || sizeof(x) == 8);
#if LONG_MAX == INT32_MAX
  return _cel_trailing_zeros_ui((unsigned int)x);
#elif LONG_MAX == INT64_MAX
  return _cel_trailing_zeros_ull((unsigned long long)x);
#else
#error Unreachable.
#endif
}
#endif

#if !(((defined(__GNUC__) && !defined(__clang__)) || \
       CEL_HAVE_BUILTIN(__builtin_ctzll)) ||         \
      (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))))
extern "C" int _cel_trailing_zeros_ull(unsigned long long x) {
  CEL_STATIC_ASSERT(sizeof(x) == 8);
  int count = _cel_trailing_zeros_ui((unsigned int)x);
  return count == sizeof(unsigned int) * CHAR_BIT
             ? count + _cel_trailing_zeros_ui((unsigned int)(x >> 32))
             : count;
}
#endif

#endif
