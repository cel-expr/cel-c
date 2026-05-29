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

// Internal header providing checked arithmetic functions. Uses <stdckdint.h> if
// available, otherwise falls back to compiler intrinsics or a pure
// implementation.

#ifndef THIRD_PARTY_CEL_C_SRC_CKDINT_H_
#define THIRD_PARTY_CEL_C_SRC_CKDINT_H_

#include <limits.h>
#include <stdbool.h>  // IWYU pragma: keep

#include "cel-c/config.h"

#if CEL_HAVE_INCLUDE(<stdckdint.h>)
#include <stdckdint.h>
#endif

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

#if defined(__STDC_VERSION_STDCKDINT_H__) && \
    __STDC_VERSION_STDCKDINT_H__ >= 202311L
#define _CEL_CKD_ADD(out, x, y, min, max) \
  CEL_USED(min);                          \
  CEL_USED(max);                          \
  return ckd_add((out), (x), (y))
#elif (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_add_overflow)
#define _CEL_CKD_ADD(out, x, y, min, max) \
  CEL_USED(min);                          \
  CEL_USED(max);                          \
  return __builtin_add_overflow((x), (y), (out))
#else
#define _CEL_CKD_ADD(out, x, y, min, max)                 \
  if ((y) >= 0 ? (x) > (max) - (y) : (x) < (min) - (y)) { \
    return true;                                          \
  }                                                       \
  *(out) = (x) + (y);                                     \
  return false
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_sc(signed char* cel_nonnull out,
                                       signed char x, signed char y) {
  _CEL_CKD_ADD(out, x, y, SCHAR_MIN, SCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_uc(unsigned char* cel_nonnull out,
                                       unsigned char x, unsigned char y) {
  _CEL_CKD_ADD(out, x, y, 0, UCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_s(short* cel_nonnull out, short x,
                                      short y) {
  _CEL_CKD_ADD(out, x, y, SHRT_MIN, SHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_us(unsigned short* cel_nonnull out,
                                       unsigned short x, unsigned short y) {
  _CEL_CKD_ADD(out, x, y, 0, USHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_i(int* cel_nonnull out, int x, int y) {
  _CEL_CKD_ADD(out, x, y, INT_MIN, INT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_ui(unsigned int* cel_nonnull out,
                                       unsigned int x, unsigned int y) {
  _CEL_CKD_ADD(out, x, y, 0u, UINT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_l(long* cel_nonnull out, long x, long y) {
  _CEL_CKD_ADD(out, x, y, LONG_MIN, LONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_ul(unsigned long* cel_nonnull out,
                                       unsigned long x, unsigned long y) {
  _CEL_CKD_ADD(out, x, y, 0ul, ULONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_ll(long long* cel_nonnull out, long long x,
                                       long long y) {
  _CEL_CKD_ADD(out, x, y, LLONG_MIN, LLONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_add_ull(unsigned long long* cel_nonnull out,
                                        unsigned long long x,
                                        unsigned long long y) {
  _CEL_CKD_ADD(out, x, y, 0ull, ULLONG_MAX);
}

#undef _CEL_CKD_ADD

CEL_END_DECLS

// _cel_ckd_add(out, lhs, rhs)
//
// Performs checked arithmetic of `lhs + rhs`, returning `true` if an
// overflow/underflow occurred. Otherwise `false` is returned and the result is
// stored in the address pointed to by `out`.
#ifndef __cplusplus
#define _cel_ckd_add(out, x, y)         \
  (_Generic(*(out),                     \
       signed char: _cel_ckd_add_sc,    \
       unsigned char: _cel_ckd_add_uc,  \
       short: _cel_ckd_add_s,           \
       unsigned short: _cel_ckd_add_us, \
       int: _cel_ckd_add_i,             \
       unsigned int: _cel_ckd_add_ui,   \
       long: _cel_ckd_add_l,            \
       unsigned long: _cel_ckd_add_ul,  \
       long long: _cel_ckd_add_ll,      \
       unsigned long long: _cel_ckd_add_ull)((out), (x), (y)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(signed char* cel_nonnull out, signed char x,
                             signed char y) {
  return _cel_ckd_add_sc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(unsigned char* cel_nonnull out, unsigned char x,
                             unsigned char y) {
  return _cel_ckd_add_uc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(short* cel_nonnull out, short x, short y) {
  return _cel_ckd_add_s(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(unsigned short* cel_nonnull out, unsigned short x,
                             unsigned short y) {
  return _cel_ckd_add_us(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(int* cel_nonnull out, int x, int y) {
  return _cel_ckd_add_i(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(unsigned int* cel_nonnull out, unsigned int x,
                             unsigned int y) {
  return _cel_ckd_add_ui(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(long* cel_nonnull out, long x, long y) {
  return _cel_ckd_add_l(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(unsigned long* cel_nonnull out, unsigned long x,
                             unsigned long y) {
  return _cel_ckd_add_ul(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(long long* cel_nonnull out, long long x,
                             long long y) {
  return _cel_ckd_add_ll(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_add(unsigned long long* cel_nonnull out,
                             unsigned long long x, unsigned long long y) {
  return _cel_ckd_add_ull(out, x, y);
}
#endif

CEL_BEGIN_DECLS

#if defined(__STDC_VERSION_STDCKDINT_H__) && \
    __STDC_VERSION_STDCKDINT_H__ >= 202311L
#define _CEL_CKD_SUB(out, x, y, min, max) \
  CEL_USED(min);                          \
  CEL_USED(max);                          \
  return ckd_sub((out), (x), (y))
#elif (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_sub_overflow)
#define _CEL_CKD_SUB(out, x, y, min, max) \
  CEL_USED(min);                          \
  CEL_USED(max);                          \
  return __builtin_sub_overflow((x), (y), (out))
#else
#define _CEL_CKD_SUB(out, x, y, min, max)                 \
  if ((y) >= 0 ? (x) < (min) + (y) : (x) > (max) + (y)) { \
    return true;                                          \
  }                                                       \
  *(out) = (x) - (y);                                     \
  return false
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_sc(signed char* cel_nonnull out,
                                       signed char x, signed char y) {
  _CEL_CKD_SUB(out, x, y, SCHAR_MIN, SCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_uc(unsigned char* cel_nonnull out,
                                       unsigned char x, unsigned char y) {
  _CEL_CKD_SUB(out, x, y, 0, UCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_s(short* cel_nonnull out, short x,
                                      short y) {
  _CEL_CKD_SUB(out, x, y, SHRT_MIN, SHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_us(unsigned short* cel_nonnull out,
                                       unsigned short x, unsigned short y) {
  _CEL_CKD_SUB(out, x, y, 0, USHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_i(int* cel_nonnull out, int x, int y) {
  _CEL_CKD_SUB(out, x, y, INT_MIN, INT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_ui(unsigned int* cel_nonnull out,
                                       unsigned int x, unsigned int y) {
  _CEL_CKD_SUB(out, x, y, 0u, UINT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_l(long* cel_nonnull out, long x, long y) {
  _CEL_CKD_SUB(out, x, y, LONG_MIN, LONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_ul(unsigned long* cel_nonnull out,
                                       unsigned long x, unsigned long y) {
  _CEL_CKD_SUB(out, x, y, 0ul, ULONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_ll(long long* cel_nonnull out, long long x,
                                       long long y) {
  _CEL_CKD_SUB(out, x, y, LLONG_MIN, LLONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_sub_ull(unsigned long long* cel_nonnull out,
                                        unsigned long long x,
                                        unsigned long long y) {
  _CEL_CKD_SUB(out, x, y, 0ull, ULLONG_MAX);
}

#undef _CEL_CKD_SUB

CEL_END_DECLS

// _cel_ckd_sub(out, lhs, rhs)
//
// Performs checked arithmetic of `lhs - rhs`, returning `true` if an
// overflow/underflow occurred. Otherwise `false` is returned and the result is
// stored in the address pointed to by `out`.
#ifndef __cplusplus
#define _cel_ckd_sub(out, x, y)         \
  (_Generic(*(out),                     \
       signed char: _cel_ckd_sub_sc,    \
       unsigned char: _cel_ckd_sub_uc,  \
       short: _cel_ckd_sub_s,           \
       unsigned short: _cel_ckd_sub_us, \
       int: _cel_ckd_sub_i,             \
       unsigned int: _cel_ckd_sub_ui,   \
       long: _cel_ckd_sub_l,            \
       unsigned long: _cel_ckd_sub_ul,  \
       long long: _cel_ckd_sub_ll,      \
       unsigned long long: _cel_ckd_sub_ull)((out), (x), (y)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(signed char* cel_nonnull out, signed char x,
                             signed char y) {
  return _cel_ckd_sub_sc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(unsigned char* cel_nonnull out, unsigned char x,
                             unsigned char y) {
  return _cel_ckd_sub_uc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(short* cel_nonnull out, short x, short y) {
  return _cel_ckd_sub_s(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(unsigned short* cel_nonnull out, unsigned short x,
                             unsigned short y) {
  return _cel_ckd_sub_us(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(int* cel_nonnull out, int x, int y) {
  return _cel_ckd_sub_i(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(unsigned int* cel_nonnull out, unsigned int x,
                             unsigned int y) {
  return _cel_ckd_sub_ui(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(long* cel_nonnull out, long x, long y) {
  return _cel_ckd_sub_l(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(unsigned long* cel_nonnull out, unsigned long x,
                             unsigned long y) {
  return _cel_ckd_sub_ul(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(long long* cel_nonnull out, long long x,
                             long long y) {
  return _cel_ckd_sub_ll(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_sub(unsigned long long* cel_nonnull out,
                             unsigned long long x, unsigned long long y) {
  return _cel_ckd_sub_ull(out, x, y);
}
#endif

CEL_BEGIN_DECLS

#if defined(__STDC_VERSION_STDCKDINT_H__) && \
    __STDC_VERSION_STDCKDINT_H__ >= 202311L
#define _CEL_CKD_MUL(out, x, y, min, max) \
  CEL_USED(min);                          \
  CEL_USED(max);                          \
  return ckd_mul((out), (x), (y))
#elif (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_mul_overflow)
#define _CEL_CKD_MUL(out, x, y, min, max) \
  CEL_USED(min);                          \
  CEL_USED(max);                          \
  return __builtin_mul_overflow((x), (y), (out))
#else
#define _CEL_CKD_MUL(out, x, y, min, max)                           \
  if (((x) == -1 && (y) == (min)) || ((y) == -1 && (x) == (min)) || \
      ((x) > 0 && (y) > 0 && (x) > (max) / (y)) ||                  \
      ((x) < 0 && (y) < 0 && (x) < (max) / (y)) ||                  \
      ((x) > 0 && (y) < 0 && (y) < (min) / (x)) ||                  \
      ((x) < 0 && (y) > 0 && (x) < (min) / (y))) {                  \
    return true;                                                    \
  }                                                                 \
  *(out) = (x) * (y);                                               \
  return false
#endif

#if defined(__STDC_VERSION_STDCKDINT_H__) && \
    __STDC_VERSION_STDCKDINT_H__ >= 202311L
#define _CEL_CKD_UMUL(out, x, y, min, max) \
  CEL_USED(min);                           \
  CEL_USED(max);                           \
  return ckd_mul((out), (x), (y))
#elif (defined(__GNUC__) && !defined(__clang__)) || \
    CEL_HAVE_BUILTIN(__builtin_mul_overflow)
#define _CEL_CKD_UMUL(out, x, y, min, max) \
  CEL_USED(min);                           \
  CEL_USED(max);                           \
  return __builtin_mul_overflow((x), (y), (out))
#else
#define _CEL_CKD_UMUL(out, x, y, min, max) \
  ((void)(min));                           \
  if ((y) != 0 && (x) > (max) / (y)) {     \
    return true;                           \
  }                                        \
  *(out) = (x) * (y);                      \
  return false
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_sc(signed char* cel_nonnull out,
                                       signed char x, signed char y) {
  _CEL_CKD_MUL(out, x, y, SCHAR_MIN, SCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_uc(unsigned char* cel_nonnull out,
                                       unsigned char x, unsigned char y) {
  _CEL_CKD_UMUL(out, x, y, 0, UCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_s(short* cel_nonnull out, short x,
                                      short y) {
  _CEL_CKD_MUL(out, x, y, SHRT_MIN, SHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_us(unsigned short* cel_nonnull out,
                                       unsigned short x, unsigned short y) {
  _CEL_CKD_UMUL(out, x, y, 0, USHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_i(int* cel_nonnull out, int x, int y) {
  _CEL_CKD_MUL(out, x, y, INT_MIN, INT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_ui(unsigned int* cel_nonnull out,
                                       unsigned int x, unsigned int y) {
  _CEL_CKD_UMUL(out, x, y, 0u, UINT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_l(long* cel_nonnull out, long x, long y) {
  _CEL_CKD_MUL(out, x, y, LONG_MIN, LONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_ul(unsigned long* cel_nonnull out,
                                       unsigned long x, unsigned long y) {
  _CEL_CKD_UMUL(out, x, y, 0ul, ULONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_ll(long long* cel_nonnull out, long long x,
                                       long long y) {
  _CEL_CKD_MUL(out, x, y, LLONG_MIN, LLONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mul_ull(unsigned long long* cel_nonnull out,
                                        unsigned long long x,
                                        unsigned long long y) {
  _CEL_CKD_UMUL(out, x, y, 0ull, ULLONG_MAX);
}

#undef _CEL_CKD_MUL
#undef _CEL_CKD_UMUL

CEL_END_DECLS

// _cel_ckd_mul(out, lhs, rhs)
//
// Performs checked arithmetic of `lhs * rhs`, returning `true` if an
// overflow/underflow occurred. Otherwise `false` is returned and the result is
// stored in the address pointed to by `out`.
#ifndef __cplusplus
#define _cel_ckd_mul(out, x, y)         \
  (_Generic(*(out),                     \
       signed char: _cel_ckd_mul_sc,    \
       unsigned char: _cel_ckd_mul_uc,  \
       short: _cel_ckd_mul_s,           \
       unsigned short: _cel_ckd_mul_us, \
       int: _cel_ckd_mul_i,             \
       unsigned int: _cel_ckd_mul_ui,   \
       long: _cel_ckd_mul_l,            \
       unsigned long: _cel_ckd_mul_ul,  \
       long long: _cel_ckd_mul_ll,      \
       unsigned long long: _cel_ckd_mul_ull)((out), (x), (y)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(signed char* cel_nonnull out, signed char x,
                             signed char y) {
  return _cel_ckd_mul_sc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(unsigned char* cel_nonnull out, unsigned char x,
                             unsigned char y) {
  return _cel_ckd_mul_uc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(short* cel_nonnull out, short x, short y) {
  return _cel_ckd_mul_s(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(unsigned short* cel_nonnull out, unsigned short x,
                             unsigned short y) {
  return _cel_ckd_mul_us(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(int* cel_nonnull out, int x, int y) {
  return _cel_ckd_mul_i(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(unsigned int* cel_nonnull out, unsigned int x,
                             unsigned int y) {
  return _cel_ckd_mul_ui(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(long* cel_nonnull out, long x, long y) {
  return _cel_ckd_mul_l(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(unsigned long* cel_nonnull out, unsigned long x,
                             unsigned long y) {
  return _cel_ckd_mul_ul(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(long long* cel_nonnull out, long long x,
                             long long y) {
  return _cel_ckd_mul_ll(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mul(unsigned long long* cel_nonnull out,
                             unsigned long long x, unsigned long long y) {
  return _cel_ckd_mul_ull(out, x, y);
}
#endif

CEL_BEGIN_DECLS

#define _CEL_CKD_DIV(out, x, y, min, max)          \
  CEL_USED(min);                                   \
  CEL_USED(max);                                   \
  if (((y) == 0) || ((x) == (min) && (y) == -1)) { \
    return true;                                   \
  }                                                \
  *(out) = (x) / (y);                              \
  return false

#define _CEL_CKD_UDIV(out, x, y, min, max) \
  CEL_USED(min);                           \
  CEL_USED(max);                           \
  ((void)(min));                           \
  if ((y) == 0) {                          \
    return true;                           \
  }                                        \
  *(out) = (x) / (y);                      \
  return false

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_sc(signed char* cel_nonnull out,
                                       signed char x, signed char y) {
  _CEL_CKD_DIV(out, x, y, SCHAR_MIN, SCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_uc(unsigned char* cel_nonnull out,
                                       unsigned char x, unsigned char y) {
  _CEL_CKD_UDIV(out, x, y, 0, UCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_s(short* cel_nonnull out, short x,
                                      short y) {
  _CEL_CKD_DIV(out, x, y, SHRT_MIN, SHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_us(unsigned short* cel_nonnull out,
                                       unsigned short x, unsigned short y) {
  _CEL_CKD_UDIV(out, x, y, 0, USHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_i(int* cel_nonnull out, int x, int y) {
  _CEL_CKD_DIV(out, x, y, INT_MIN, INT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_ui(unsigned int* cel_nonnull out,
                                       unsigned int x, unsigned int y) {
  _CEL_CKD_UDIV(out, x, y, 0u, UINT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_l(long* cel_nonnull out, long x, long y) {
  _CEL_CKD_DIV(out, x, y, LONG_MIN, LONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_ul(unsigned long* cel_nonnull out,
                                       unsigned long x, unsigned long y) {
  _CEL_CKD_UDIV(out, x, y, 0ul, ULONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_ll(long long* cel_nonnull out, long long x,
                                       long long y) {
  _CEL_CKD_DIV(out, x, y, LLONG_MIN, LLONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_div_ull(unsigned long long* cel_nonnull out,
                                        unsigned long long x,
                                        unsigned long long y) {
  _CEL_CKD_UDIV(out, x, y, 0ull, ULLONG_MAX);
}

#undef _CEL_CKD_DIV
#undef _CEL_CKD_UDIV

CEL_END_DECLS

// _cel_ckd_div(out, lhs, rhs)
//
// Performs checked arithmetic of `lhs / rhs`, returning `true` if an
// overflow/underflow occurred. Otherwise `false` is returned and the result is
// stored in the address pointed to by `out`.
#ifndef __cplusplus
#define _cel_ckd_div(out, x, y)         \
  (_Generic(*(out),                     \
       signed char: _cel_ckd_div_sc,    \
       unsigned char: _cel_ckd_div_uc,  \
       short: _cel_ckd_div_s,           \
       unsigned short: _cel_ckd_div_us, \
       int: _cel_ckd_div_i,             \
       unsigned int: _cel_ckd_div_ui,   \
       long: _cel_ckd_div_l,            \
       unsigned long: _cel_ckd_div_ul,  \
       long long: _cel_ckd_div_ll,      \
       unsigned long long: _cel_ckd_div_ull)((out), (x), (y)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(signed char* cel_nonnull out, signed char x,
                             signed char y) {
  return _cel_ckd_div_sc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(unsigned char* cel_nonnull out, unsigned char x,
                             unsigned char y) {
  return _cel_ckd_div_uc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(short* cel_nonnull out, short x, short y) {
  return _cel_ckd_div_s(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(unsigned short* cel_nonnull out, unsigned short x,
                             unsigned short y) {
  return _cel_ckd_div_us(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(int* cel_nonnull out, int x, int y) {
  return _cel_ckd_div_i(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(unsigned int* cel_nonnull out, unsigned int x,
                             unsigned int y) {
  return _cel_ckd_div_ui(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(long* cel_nonnull out, long x, long y) {
  return _cel_ckd_div_l(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(unsigned long* cel_nonnull out, unsigned long x,
                             unsigned long y) {
  return _cel_ckd_div_ul(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(long long* cel_nonnull out, long long x,
                             long long y) {
  return _cel_ckd_div_ll(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_div(unsigned long long* cel_nonnull out,
                             unsigned long long x, unsigned long long y) {
  return _cel_ckd_div_ull(out, x, y);
}
#endif

CEL_BEGIN_DECLS

#define _CEL_CKD_MOD(out, x, y, min, max)  \
  CEL_USED(min);                           \
  CEL_USED(max);                           \
  if ((y) == 0 || (x == min && y == -1)) { \
    return true;                           \
  }                                        \
  *(out) = (x) % (y);                      \
  return false;

#define _CEL_CKD_UMOD(out, x, y, min, max) \
  CEL_USED(min);                           \
  CEL_USED(max);                           \
  ((void)(min));                           \
  if ((y) == 0) {                          \
    return true;                           \
  }                                        \
  *(out) = (x) % (y);                      \
  return false

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_sc(signed char* cel_nonnull out,
                                       signed char x, signed char y) {
  _CEL_CKD_MOD(out, x, y, SCHAR_MIN, SCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_uc(unsigned char* cel_nonnull out,
                                       unsigned char x, unsigned char y) {
  _CEL_CKD_UMOD(out, x, y, 0, UCHAR_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_s(short* cel_nonnull out, short x,
                                      short y) {
  _CEL_CKD_MOD(out, x, y, SHRT_MIN, SHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_us(unsigned short* cel_nonnull out,
                                       unsigned short x, unsigned short y) {
  _CEL_CKD_UMOD(out, x, y, 0, USHRT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_i(int* cel_nonnull out, int x, int y) {
  _CEL_CKD_MOD(out, x, y, INT_MIN, INT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_ui(unsigned int* cel_nonnull out,
                                       unsigned int x, unsigned int y) {
  _CEL_CKD_UMOD(out, x, y, 0u, UINT_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_l(long* cel_nonnull out, long x, long y) {
  _CEL_CKD_MOD(out, x, y, LONG_MIN, LONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_ul(unsigned long* cel_nonnull out,
                                       unsigned long x, unsigned long y) {
  _CEL_CKD_UMOD(out, x, y, 0ul, ULONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_ll(long long* cel_nonnull out, long long x,
                                       long long y) {
  _CEL_CKD_MOD(out, x, y, LLONG_MIN, LLONG_MAX);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_ckd_mod_ull(unsigned long long* cel_nonnull out,
                                        unsigned long long x,
                                        unsigned long long y) {
  _CEL_CKD_UMOD(out, x, y, 0ull, ULLONG_MAX);
}

#undef _CEL_CKD_MOD
#undef _CEL_CKD_UMOD

CEL_END_DECLS

// _cel_ckd_mod(out, lhs, rhs)
//
// Performs checked arithmetic of `lhs % rhs`, returning `true` if an
// overflow/underflow occurred. Otherwise `false` is returned and the result is
// stored in the address pointed to by `out`.
#ifndef __cplusplus
#define _cel_ckd_mod(out, x, y)         \
  (_Generic(*(out),                     \
       signed char: _cel_ckd_mod_sc,    \
       unsigned char: _cel_ckd_mod_uc,  \
       short: _cel_ckd_mod_s,           \
       unsigned short: _cel_ckd_mod_us, \
       int: _cel_ckd_mod_i,             \
       unsigned int: _cel_ckd_mod_ui,   \
       long: _cel_ckd_mod_l,            \
       unsigned long: _cel_ckd_mod_ul,  \
       long long: _cel_ckd_mod_ll,      \
       unsigned long long: _cel_ckd_mod_ull)((out), (x), (y)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(signed char* cel_nonnull out, signed char x,
                             signed char y) {
  return _cel_ckd_mod_sc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(unsigned char* cel_nonnull out, unsigned char x,
                             unsigned char y) {
  return _cel_ckd_mod_uc(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(short* cel_nonnull out, short x, short y) {
  return _cel_ckd_mod_s(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(unsigned short* cel_nonnull out, unsigned short x,
                             unsigned short y) {
  return _cel_ckd_mod_us(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(int* cel_nonnull out, int x, int y) {
  return _cel_ckd_mod_i(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(unsigned int* cel_nonnull out, unsigned int x,
                             unsigned int y) {
  return _cel_ckd_mod_ui(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(long* cel_nonnull out, long x, long y) {
  return _cel_ckd_mod_l(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(unsigned long* cel_nonnull out, unsigned long x,
                             unsigned long y) {
  return _cel_ckd_mod_ul(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(long long* cel_nonnull out, long long x,
                             long long y) {
  return _cel_ckd_mod_ll(out, x, y);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_ckd_mod(unsigned long long* cel_nonnull out,
                             unsigned long long x, unsigned long long y) {
  return _cel_ckd_mod_ull(out, x, y);
}
#endif

CEL_BEGIN_DECLS

CEL_END_DECLS

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_SRC_CKDINT_H_
