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

// Internal header providing an unsigned 128 bit unsigned integral.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_UINT128_H_
#define THIRD_PARTY_CEL_C_INTERNAL_UINT128_H_

#include <stdalign.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stdint.h>

#if defined(_MSC_VER) && defined(_M_X64) && !defined(_M_ARM64EC)
#include <intrin.h>
#pragma intrinsic(_umul128)
#endif

#include "cel-c/assert.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/endian.h"  // IWYU pragma: keep

#ifdef __cplusplus
#include <type_traits>
#endif

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

#ifdef _CEL_HAVE_INTRINSIC_INT128
typedef unsigned __int128 _cel_Uint128;
#else
typedef struct {
  union {
    struct {
#ifdef _CEL_IS_BIG_ENDIAN
      uint64_t hi;
      uint64_t lo;
#else
      uint64_t lo;
      uint64_t hi;
#endif
    };
    alignas(16) uint64_t rep[2];
  };
} _cel_Uint128;
#endif

CEL_STATIC_ASSERT(sizeof(_cel_Uint128) == 16);
CEL_STATIC_ASSERT(alignof(_cel_Uint128) == 16);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Uint128 _cel_Uint128_Make(uint64_t high, uint64_t low) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return (((_cel_Uint128)high) << 64) | low;
#else
  _cel_Uint128 ret;
#ifdef _CEL_IS_BIG_ENDIAN
  ret.hi = high;
  ret.lo = low;
#else
  ret.lo = low;
  ret.hi = high;
#endif
  return ret;
#endif
}

CEL_END_DECLS

// _cel_Uint128_From
#ifndef __cplusplus
#ifdef _CEL_HAVE_INTRINSIC_INT128
#define _cel_Uint128_From(x)                                                  \
  (_Generic((x),                                                              \
       char: _cel_Uint128_Make(UINT64_C(0), (x)),                             \
       signed char: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))), \
       unsigned char: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),        \
       short: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))),       \
       unsigned short: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),       \
       int: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))),         \
       unsigned int: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),         \
       long: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))),        \
       unsigned long: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),        \
       long long: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))),   \
       unsigned long long: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),   \
       __int128: ((_cel_Uint128)(x)),                                         \
       unsigned __int128: ((_cel_Uint128)(x))))
#else
#define _cel_Uint128_From(x)                                                  \
  (_Generic((x),                                                              \
       char: _cel_Uint128_Make(UINT64_C(0), (x)),                             \
       signed char: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))), \
       unsigned char: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),        \
       short: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))),       \
       unsigned short: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),       \
       int: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))),         \
       unsigned int: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),         \
       long: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))),        \
       unsigned long: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x))),        \
       long long: _cel_Uint128_Make(UINT64_C(0), (uint64_t)((int64_t)(x))),   \
       unsigned long long: _cel_Uint128_Make(UINT64_C(0), ((uint64_t)(x)))))
#endif
#else
#ifdef _CEL_HAVE_INTRINSIC_INT128
CEL_ATTRIBUTE_NODISCARD CEL_INLINE _cel_Uint128 _cel_Uint128_From(__int128 x) {
  return (_cel_Uint128)x;
}
CEL_ATTRIBUTE_NODISCARD CEL_INLINE _cel_Uint128
_cel_Uint128_From(unsigned __int128 x) {
  return (_cel_Uint128)x;
}
#endif
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_INLINE
    std::enable_if_t<(std::is_integral_v<T> && std::is_signed_v<T> &&
                      sizeof(T) <= sizeof(int64_t)),
                     _cel_Uint128>
    _cel_Uint128_From(T x) {
  return _cel_Uint128_Make(UINT64_C(0), (uint64_t)(int64_t)x);
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_INLINE
    std::enable_if_t<(std::is_integral_v<T> && std::is_unsigned_v<T> &&
                      sizeof(T) <= sizeof(uint64_t)),
                     _cel_Uint128>
    _cel_Uint128_From(T x) {
  return _cel_Uint128_Make(UINT64_C(0), (uint64_t)x);
}
#endif

CEL_BEGIN_DECLS

CEL_END_DECLS

// _cel_Uint128_To
#ifndef __cplusplus
#ifdef _CEL_HAVE_INTRINSIC_INT128
#define _cel_Uint128_To(to, x)                        \
  (_Generic(((to)0),                                  \
       char: ((char)(x)),                             \
       signed char: ((signed char)(x)),               \
       unsigned char: ((unsigned char)(x)),           \
       short: ((short)(x)),                           \
       unsigned short: ((unsigned short)(x)),         \
       int: ((int)(x)),                               \
       unsigned int: ((unsigned int)(x)),             \
       long: ((long)(x)),                             \
       unsigned long: ((unsigned long)(x)),           \
       long long: ((long long)(x)),                   \
       unsigned long long: ((unsigned long long)(x)), \
       __int128: ((__int128)(x)),                     \
       unsigned __int128: ((unsigned __int128)(x))))
#else
#define _cel_Uint128_To(to, x)                   \
  (_Generic(((to)0),                             \
       char: ((char)(x).lo),                     \
       signed char: ((signed char)(x).lo),       \
       unsigned char: ((unsigned char)(x).lo),   \
       short: ((short)(x).lo),                   \
       unsigned short: ((unsigned short)(x).lo), \
       int: ((int)(x).lo),                       \
       unsigned int: ((unsigned int)(x).lo),     \
       long: ((long)(x).lo),                     \
       unsigned long: ((unsigned long)(x).lo),   \
       long long: ((long long)(x).lo),           \
       unsigned long long: ((unsigned long long)(x).lo)))
#endif
#else
#ifdef _CEL_HAVE_INTRINSIC_INT128
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_INLINE
    std::enable_if_t<std::is_same_v<T, __int128>, T>
    _cel_Uint128_To(_cel_Uint128 x) {
  return (__int128)x;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_INLINE
    std::enable_if_t<std::is_same_v<T, unsigned __int128>, T>
    _cel_Uint128_To(_cel_Uint128 x) {
  return (unsigned __int128)x;
}
#endif
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_INLINE
    std::enable_if_t<(std::is_integral_v<T> && std::is_signed_v<T> &&
                      sizeof(T) <= sizeof(int64_t)),
                     T>
    _cel_Uint128_To(_cel_Uint128 x) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return (T)x;
#else
  return (T)(int64_t)x.lo;
#endif
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_INLINE
    std::enable_if_t<(std::is_integral_v<T> && std::is_unsigned_v<T> &&
                      sizeof(T) <= sizeof(uint64_t)),
                     T>
    _cel_Uint128_To(_cel_Uint128 x) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return (T)x;
#else
  return (T)x.lo;
#endif
}
#define _cel_Uint128_To(to, x) _cel_Uint128_To<to>(x)
#endif

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Uint128_Less(_cel_Uint128 lhs, _cel_Uint128 rhs) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs < rhs;
#else
  return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi;
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Uint128 _cel_Uint128_ShiftLeft(_cel_Uint128 lhs,
                                                      int rhs) {
  CEL_ASSERT_GE(rhs, 0);
  CEL_ASSERT_LT(rhs, 128);
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs << rhs;
#else
  _cel_Uint128 ret;
  if (rhs >= 64) {
    ret.hi = lhs.lo << (rhs - 64);
    ret.lo = 0;
  } else if (rhs == 0) {
    ret = lhs;
  } else {
    ret.hi = (lhs.hi << rhs) | (lhs.lo >> (64 - rhs));
    ret.lo = lhs.lo << rhs;
  }
  return ret;
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Uint128 _cel_Uint128_ShiftRight(_cel_Uint128 lhs,
                                                       int rhs) {
  CEL_ASSERT_GE(rhs, 0);
  CEL_ASSERT_LT(rhs, 128);
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs >> rhs;
#else
  _cel_Uint128 ret;
  if (rhs >= 64) {
    ret.hi = 0;
    ret.lo = lhs.hi >> (rhs - 64);
  } else if (rhs == 0) {
    ret = lhs;
  } else {
    ret.hi = lhs.hi >> rhs;
    ret.lo = (lhs.lo >> rhs) | (lhs.hi << (64 - rhs));
  }
  return ret;
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Uint128 _cel_Uint128_Add(_cel_Uint128 lhs,
                                                _cel_Uint128 rhs) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs + rhs;
#else
  _cel_Uint128 ret;
  ret.hi = lhs.hi + rhs.hi;
  ret.lo = lhs.lo + rhs.lo;
  if (ret.lo < lhs.lo) {
    ret.hi += 1;
  }
  return ret;
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Uint128 _cel_Uint128_Mul(_cel_Uint128 lhs,
                                                _cel_Uint128 rhs) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs * rhs;
#elif defined(_MSC_VER) && defined(_M_X64) && !defined(_M_ARM64EC)
  _cel_Uint128 ret;
  ret.lo = _umul128(lhs.lo, rhs.lo, &ret.hi);
  ret.hi += lhs.lo * rhs.hi + lhs.hi * rhs.lo;
  return ret;
#else
  const uint64_t a32 = lhs.lo >> 32;
  const uint64_t a00 = lhs.lo & UINT64_C(0xffffffff);
  const uint64_t b32 = rhs.lo >> 32;
  const uint64_t b00 = rhs.lo & UINT64_C(0xffffffff);
  _cel_Uint128 ret;
  ret.hi = lhs.hi * rhs.lo + lhs.lo * rhs.hi + a32 * b32;
  ret.lo = a00 * b00;
  ret = _cel_Uint128_Add(
      ret, _cel_Uint128_LeftShift(_CEL_UINT128(UINT64_C(0), a32 * b00), 32));
  ret = _cel_Uint128_Add(
      ret, _cel_Uint128_LeftShift(_CEL_UINT128(UINT64_C(0), a00 * b32), 32));
  return ret;
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Uint128 _cel_Uint128_Or(_cel_Uint128 lhs,
                                               _cel_Uint128 rhs) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs | rhs;
#else
  _cel_Uint128 ret;
  ret.lo = lhs.lo | rhs.lo;
  ret.hi = lhs.hi | rhs.hi;
  return ret;
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_Uint128 _cel_Uint128_Xor(_cel_Uint128 lhs,
                                                _cel_Uint128 rhs) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs ^ rhs;
#else
  _cel_Uint128 ret;
  ret.lo = lhs.lo ^ rhs.lo;
  ret.hi = lhs.hi ^ rhs.hi;
  return ret;
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_Uint128_Compare(_cel_Uint128 lhs, _cel_Uint128 rhs) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
#else
  return (lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi)   ? -1
         : (lhs.hi == rhs.hi ? lhs.lo > rhs.lo : lhs.hi > rhs.hi) ? 1
                                                                  : 0;
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_Uint128_Equals(_cel_Uint128 lhs, _cel_Uint128 rhs) {
#ifdef _CEL_HAVE_INTRINSIC_INT128
  return lhs == rhs;
#else
  return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
#endif
}

CEL_END_DECLS

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_INTERNAL_UINT128_H_
