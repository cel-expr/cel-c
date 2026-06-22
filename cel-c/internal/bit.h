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

// Internal header providing bit manipulation functions. Uses <stdbit.h> if
// available, otherwise falls back to compiler intrinsics or a pure
// implementation.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_BIT_H_
#define THIRD_PARTY_CEL_C_INTERNAL_BIT_H_

#include <limits.h>   // IWYU pragma: keep
#include <stdbool.h>  // IWYU pragma: keep

#include "cel-c/internal/config.h"

#if CEL_HAVE_INCLUDE(<stdbit.h>)
#include <stdbit.h>
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                            defined(_M_X86) || defined(_M_ARM))
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)
#if defined(_M_X64) || defined(_M_ARM64)
#pragma intrinsic(_BitScanReverse64)
#pragma intrinsic(_BitScanForward64)
#endif
#endif

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

#if defined(__STDC_VERSION_STDBIT_H__) && __STDC_VERSION_STDBIT_H__ >= 202311L
#define _cel_leading_zeros(x) stdc_leading_zeros(x)
#define _cel_trailing_zeros(x) stdc_trailing_zeros(x)
#define _cel_has_single_bit(x) stdc_has_single_bit(x)
#else
#if ((defined(__GNUC__) && !defined(__clang__)) ||                 \
     CEL_HAVE_BUILTIN(__builtin_clz)) ||                           \
    (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                           defined(_M_X86) || defined(_M_ARM)))
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_leading_zeros_ui(unsigned int x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_clz)
  return x == 0 ? sizeof(x) * CHAR_BIT : __builtin_clz(x);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                            defined(_M_X86) || defined(_M_ARM))
  CEL_STATIC_ASSERT(sizeof(x) == 4);
  unsigned long index;
  if (!_BitScanReverse(&index, x)) {
    index = sizeof(x) * CHAR_BIT;
  }
  return (int)(unsigned int)index;
#else
#error Unreachable.
#endif
}
#else
CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_PROTECTED int _cel_leading_zeros_ui(unsigned int x);
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_leading_zeros_uc(unsigned char x) {
  return _cel_leading_zeros_ui(x) -
         ((sizeof(unsigned int) * CHAR_BIT) - (sizeof(x) * CHAR_BIT));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_leading_zeros_us(unsigned short x) {
  return _cel_leading_zeros_ui(x) -
         ((sizeof(unsigned int) * CHAR_BIT) - (sizeof(x) * CHAR_BIT));
}

#if ((defined(__GNUC__) && !defined(__clang__)) ||                 \
     CEL_HAVE_BUILTIN(__builtin_clzl)) ||                          \
    (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                           defined(_M_X86) || defined(_M_ARM)))
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_leading_zeros_ul(unsigned long x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_clzl)
  return x == 0 ? sizeof(x) * CHAR_BIT : __builtin_clzl(x);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                            defined(_M_X86) || defined(_M_ARM))
  CEL_STATIC_ASSERT(sizeof(x) == 4);
  unsigned long index;
  if (!_BitScanReverse(&index, x)) {
    index = sizeof(x) * CHAR_BIT;
  }
  return (int)(unsigned int)index;
#else
#error Unreachable.
#endif
}
#else
CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
int _cel_leading_zeros_ul(unsigned long x);
#endif

#if ((defined(__GNUC__) && !defined(__clang__)) || \
     CEL_HAVE_BUILTIN(__builtin_clzll)) ||         \
    (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64)))
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_leading_zeros_ull(unsigned long long x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_clzll)
  return x == 0 ? sizeof(x) * CHAR_BIT : __builtin_clzll(x);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
  CEL_STATIC_ASSERT(sizeof(x) == 8);
  unsigned long index;
  if (!_BitScanReverse64(&index, x)) {
    index = sizeof(x) * CHAR_BIT;
  }
  return (int)(unsigned int)index;
#else
#error Unreachable.
#endif
}
#else
CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
int _cel_leading_zeros_ull(unsigned long long x);
#endif

CEL_END_DECLS

// _cel_leading_zeros
//
// Returns the number of consecutive 0 bits, starting from the most significant
// bit.
#ifndef __cplusplus
#define _cel_leading_zeros(x)                 \
  (_Generic((x),                              \
       unsigned char: _cel_leading_zeros_uc,  \
       unsigned short: _cel_leading_zeros_us, \
       unsigned int: _cel_leading_zeros_ui,   \
       unsigned long: _cel_leading_zeros_ul,  \
       unsigned long long: _cel_leading_zeros_ull)((x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_leading_zeros(unsigned char x) {
  return _cel_leading_zeros_uc(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_leading_zeros(unsigned short x) {
  return _cel_leading_zeros_us(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_leading_zeros(unsigned int x) {
  return _cel_leading_zeros_ui(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_leading_zeros(unsigned long x) {
  return _cel_leading_zeros_ul(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_leading_zeros(unsigned long long x) {
  return _cel_leading_zeros_ull(x);
}
#endif

CEL_BEGIN_DECLS

#if ((defined(__GNUC__) && !defined(__clang__)) ||                 \
     CEL_HAVE_BUILTIN(__builtin_ctz)) ||                           \
    (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                           defined(_M_X86) || defined(_M_ARM)))
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_trailing_zeros_ui(unsigned int x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_ctz)
  return x == 0 ? sizeof(x) * CHAR_BIT : __builtin_ctz(x);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                            defined(_M_X86) || defined(_M_ARM))
  CEL_STATIC_ASSERT(sizeof(x) == 4);
  unsigned long index;
  if (!_BitScanForward(&index, x)) {
    index = sizeof(x) * CHAR_BIT;
  }
  return (int)(unsigned int)index;
#else
#error Unreachable.
#endif
}
#else
CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_PROTECTED int _cel_trailing_zeros_ui(unsigned int x);
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_trailing_zeros_uc(unsigned char x) {
  return x == 0 ? sizeof(x) * CHAR_BIT : _cel_trailing_zeros_ui(x);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_trailing_zeros_us(unsigned short x) {
  return x == 0 ? sizeof(x) * CHAR_BIT : _cel_trailing_zeros_ui(x);
}

#if ((defined(__GNUC__) && !defined(__clang__)) ||                 \
     CEL_HAVE_BUILTIN(__builtin_ctzl)) ||                          \
    (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                           defined(_M_X86) || defined(_M_ARM)))
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_trailing_zeros_ul(unsigned long x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_ctzl)
  return x == 0 ? sizeof(x) * CHAR_BIT : __builtin_ctzl(x);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64) || \
                            defined(_M_X86) || defined(_M_ARM))
  CEL_STATIC_ASSERT(sizeof(x) == 4);
  unsigned long index;
  if (!_BitScanForward(&index, x)) {
    index = sizeof(x) * CHAR_BIT;
  }
  return (int)(unsigned int)index;
#else
#error Unreachable.
#endif
}
#else
CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
int _cel_trailing_zeros_ul(unsigned long x);
#endif

#if ((defined(__GNUC__) && !defined(__clang__)) || \
     CEL_HAVE_BUILTIN(__builtin_ctzll)) ||         \
    (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64)))
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_trailing_zeros_ull(unsigned long long x) {
#if (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_ctzll)
  return x == 0 ? sizeof(x) * CHAR_BIT : __builtin_ctzll(x);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
  CEL_STATIC_ASSERT(sizeof(x) == 8);
  unsigned long index;
  if (!_BitScanForward64(&index, x)) {
    index = sizeof(x) * CHAR_BIT;
  }
  return (int)(unsigned int)index;
#else
#error Unreachable.
#endif
}
#else
CEL_ATTRIBUTE_PURE
CEL_ATTRIBUTE_NODISCARD
int _cel_trailing_zeros_ull(unsigned long long x);
#endif

CEL_END_DECLS

// _cel_trailing_zeros
//
// Returns the number of consecutive 0 bits, starting from the least significant
// bit.
#ifndef __cplusplus
#define _cel_trailing_zeros(x)                 \
  (_Generic((x),                               \
       unsigned char: _cel_trailing_zeros_uc,  \
       unsigned short: _cel_trailing_zeros_us, \
       unsigned int: _cel_trailing_zeros_ui,   \
       unsigned long: _cel_trailing_zeros_ul,  \
       unsigned long long: _cel_trailing_zeros_ull)((x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_trailing_zeros(unsigned char x) {
  return _cel_trailing_zeros_uc(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_trailing_zeros(unsigned short x) {
  return _cel_trailing_zeros_us(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_trailing_zeros(unsigned int x) {
  return _cel_trailing_zeros_ui(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_trailing_zeros(unsigned long x) {
  return _cel_trailing_zeros_ul(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_trailing_zeros(unsigned long long x) {
  return _cel_trailing_zeros_ull(x);
}
#endif

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_has_single_bit_ui(unsigned int x) {
  return x && !(x & (x - 1u));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_has_single_bit_uc(unsigned char x) {
  return _cel_has_single_bit_ui(x);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_has_single_bit_us(unsigned short x) {
  return _cel_has_single_bit_ui(x);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_has_single_bit_ul(unsigned long x) {
  return x && !(x & (x - 1ul));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_has_single_bit_ull(unsigned long long x) {
  return x && !(x & (x - 1ull));
}

CEL_END_DECLS

// _cel_has_single_bit
//
// Returns whether there is a single 1 bit.
#ifndef __cplusplus
#define _cel_has_single_bit(x)                 \
  (_Generic((x),                               \
       unsigned char: _cel_has_single_bit_uc,  \
       unsigned short: _cel_has_single_bit_us, \
       unsigned int: _cel_has_single_bit_ui,   \
       unsigned long: _cel_has_single_bit_ul,  \
       unsigned long long: _cel_has_single_bit_ull)((x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_has_single_bit(unsigned char x) {
  return _cel_has_single_bit_uc(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_has_single_bit(unsigned short x) {
  return _cel_has_single_bit_us(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_has_single_bit(unsigned int x) {
  return _cel_has_single_bit_ui(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_has_single_bit(unsigned long x) {
  return _cel_has_single_bit_ul(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_has_single_bit(unsigned long long x) {
  return _cel_has_single_bit_ull(x);
}
#endif

CEL_BEGIN_DECLS
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_bit_width_uc(unsigned char x) {
  return (sizeof(x) * CHAR_BIT) - _cel_leading_zeros(x);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_bit_width_us(unsigned short x) {
  return (sizeof(x) * CHAR_BIT) - _cel_leading_zeros(x);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_bit_width_ui(unsigned int x) {
  return (sizeof(x) * CHAR_BIT) - _cel_leading_zeros(x);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_bit_width_ul(unsigned long x) {
  return (sizeof(x) * CHAR_BIT) - _cel_leading_zeros(x);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_bit_width_ull(unsigned long long x) {
  return (sizeof(x) * CHAR_BIT) - _cel_leading_zeros(x);
}

CEL_END_DECLS

// _cel_bit_width
//
// Calculates the number of bits needed to store the value.
#ifndef __cplusplus
#define _cel_bit_width(x)                 \
  (_Generic((x),                          \
       unsigned char: _cel_bit_width_uc,  \
       unsigned short: _cel_bit_width_us, \
       unsigned int: _cel_bit_width_ui,   \
       unsigned long: _cel_bit_width_ul,  \
       unsigned long long: _cel_bit_width_ull)((x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_bit_width(unsigned char x) { return _cel_bit_width_uc(x); }
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_bit_width(unsigned short x) { return _cel_bit_width_us(x); }
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_bit_width(unsigned int x) { return _cel_bit_width_ui(x); }
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_bit_width(unsigned long x) { return _cel_bit_width_ul(x); }
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_bit_width(unsigned long long x) {
  return _cel_bit_width_ull(x);
}
#endif

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned char _cel_bit_ceil_uc(unsigned char x) {
  if (x <= 1) {
    return 1;
  }
  return ((unsigned char)1) << _cel_bit_width((unsigned char)(x - 1));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned short _cel_bit_ceil_us(unsigned short x) {
  if (x <= 1) {
    return 1;
  }
  return ((unsigned short)1) << _cel_bit_width((unsigned short)(x - 1));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned int _cel_bit_ceil_ui(unsigned int x) {
  if (x <= 1) {
    return 1;
  }
  return ((unsigned int)1) << _cel_bit_width((unsigned int)(x - 1));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long _cel_bit_ceil_ul(unsigned long x) {
  if (x <= 1) {
    return 1;
  }
  return ((unsigned long)1) << _cel_bit_width((unsigned long)(x - 1));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long long _cel_bit_ceil_ull(unsigned long long x) {
  if (x <= 1) {
    return 1;
  }
  return ((unsigned long long)1) << _cel_bit_width((unsigned long long)(x - 1));
}

CEL_END_DECLS

// _cel_bit_ceil
//
// Calculates the smallest integral power of two that is not smaller.
#ifndef __cplusplus
#define _cel_bit_ceil(x)                 \
  (_Generic((x),                         \
       unsigned char: _cel_bit_ceil_uc,  \
       unsigned short: _cel_bit_ceil_us, \
       unsigned int: _cel_bit_ceil_ui,   \
       unsigned long: _cel_bit_ceil_ul,  \
       unsigned long long: _cel_bit_ceil_ull)((x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned char _cel_bit_ceil(unsigned char x) {
  return _cel_bit_ceil_uc(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned short _cel_bit_ceil(unsigned short x) {
  return _cel_bit_ceil_us(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned int _cel_bit_ceil(unsigned int x) {
  return _cel_bit_ceil_ui(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long _cel_bit_ceil(unsigned long x) {
  return _cel_bit_ceil_ul(x);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long long _cel_bit_ceil(unsigned long long x) {
  return _cel_bit_ceil_ull(x);
}
#endif

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned char _cel_rotr_uc(unsigned char x, int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x << r) | (x >> (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x >> r) | (x << (N - r));
  }
  return x;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned short _cel_rotr_us(unsigned short x, int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x << r) | (x >> (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x >> r) | (x << (N - r));
  }
  return x;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned int _cel_rotr_ui(unsigned int x, int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x << r) | (x >> (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x >> r) | (x << (N - r));
  }
  return x;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long _cel_rotr_ul(unsigned long x, int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x << r) | (x >> (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x >> r) | (x << (N - r));
  }
  return x;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long long _cel_rotr_ull(unsigned long long x,
                                                   int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x << r) | (x >> (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x >> r) | (x << (N - r));
  }
  return x;
}

CEL_END_DECLS

// _cel_rotr
//
// Rotates the bits right.
#ifndef __cplusplus
#define _cel_rotr(x, s)              \
  (_Generic((x),                     \
       unsigned char: _cel_rotr_uc,  \
       unsigned short: _cel_rotr_us, \
       unsigned int: _cel_rotr_ui,   \
       unsigned long: _cel_rotr_ul,  \
       unsigned long long: _cel_rotr_ull)((x), (s)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned char _cel_rotr(unsigned char x, int s) {
  return _cel_rotr_uc(x, s);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned short _cel_rotr(unsigned short x, int s) {
  return _cel_rotr_us(x, s);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned int _cel_rotr(unsigned int x, int s) {
  return _cel_rotr_ui(x, s);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long _cel_rotr(unsigned long x, int s) {
  return _cel_rotr_ul(x, s);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long long _cel_rotr(unsigned long long x, int s) {
  return _cel_rotr_ull(x, s);
}
#endif

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned char _cel_rotl_uc(unsigned char x, int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x >> r) | (x << (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x << r) | (x >> (N - r));
  }
  return x;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned short _cel_rotl_us(unsigned short x, int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x >> r) | (x << (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x << r) | (x >> (N - r));
  }
  return x;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned int _cel_rotl_ui(unsigned int x, int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x >> r) | (x << (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x << r) | (x >> (N - r));
  }
  return x;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long _cel_rotl_ul(unsigned long x, int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x >> r) | (x << (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x << r) | (x >> (N - r));
  }
  return x;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long long _cel_rotl_ull(unsigned long long x,
                                                   int s) {
  const int N = sizeof(x) * 8;
  int r = s % N;
  if (r < 0) {
    r = -r;
    return (x >> r) | (x << (N - r));
  }
  if (CEL_LIKELY(r > 0)) {
    return (x << r) | (x >> (N - r));
  }
  return x;
}

CEL_END_DECLS

// _cel_rotl
//
// Rotates the bits left.
#ifndef __cplusplus
#define _cel_rotl(x, s)              \
  (_Generic((x),                     \
       unsigned char: _cel_rotl_uc,  \
       unsigned short: _cel_rotl_us, \
       unsigned int: _cel_rotl_ui,   \
       unsigned long: _cel_rotl_ul,  \
       unsigned long long: _cel_rotl_ull)((x), (s)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned char _cel_rotl(unsigned char x, int s) {
  return _cel_rotl_uc(x, s);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned short _cel_rotl(unsigned short x, int s) {
  return _cel_rotl_us(x, s);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned int _cel_rotl(unsigned int x, int s) {
  return _cel_rotl_ui(x, s);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long _cel_rotl(unsigned long x, int s) {
  return _cel_rotl_ul(x, s);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long long _cel_rotl(unsigned long long x, int s) {
  return _cel_rotl_ull(x, s);
}
#endif

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_ffs_uc(unsigned char x) {
  return x != 0 ? _cel_trailing_zeros(x) + 1 : 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_ffs_us(unsigned short x) {
  return x != 0 ? _cel_trailing_zeros(x) + 1 : 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_ffs_ui(unsigned int x) {
  return x != 0 ? _cel_trailing_zeros(x) + 1 : 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_ffs_ul(unsigned long x) {
  return x != 0 ? _cel_trailing_zeros(x) + 1 : 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_ffs_ull(unsigned long long x) {
  return x != 0 ? _cel_trailing_zeros(x) + 1 : 0;
}

CEL_END_DECLS

// _cel_ffs
//
// Find the first set bit.
#ifndef __cplusplus
#define _cel_ffs(x)                 \
  (_Generic((x),                    \
       unsigned char: _cel_ffs_uc,  \
       unsigned short: _cel_ffs_us, \
       unsigned int: _cel_ffs_ui,   \
       unsigned long: _cel_ffs_ul,  \
       unsigned long long: _cel_ffs_ull)((x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_ffs(unsigned char x) { return _cel_ffs_uc(x); }
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_ffs(unsigned short x) { return _cel_ffs_us(x); }
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_ffs(unsigned int x) { return _cel_ffs_ui(x); }
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_ffs(unsigned long x) { return _cel_ffs_ul(x); }
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE int _cel_ffs(unsigned long long x) { return _cel_ffs_ull(x); }
#endif

CEL_BEGIN_DECLS

CEL_END_DECLS

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_INTERNAL_BIT_H_
