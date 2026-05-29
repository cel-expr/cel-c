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

// Internal header providing address alignment functions. Uses compiler
// intrinsics if available, otherwise falls back to a pure implementation.

#ifndef THIRD_PARTY_CEL_C_SRC_ALIGN_H_
#define THIRD_PARTY_CEL_C_SRC_ALIGN_H_

#include <limits.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/bit.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

#if CEL_HAVE_BUILTIN(__builtin_align_up)
#define _CEL_ALIGN_UP(val, align, suffix) \
  ((val) != 0##suffix) ? __builtin_align_up((val), (align)) : 0##suffix
#else
#define _CEL_ALIGN_UP(val, align, suffix)                                      \
  ((val) != 0##suffix) ? (((val) + (((cel_typeof(val))(align)) - 1##suffix)) & \
                          ~(((cel_typeof(val))(align)) - 1##suffix))           \
                       : 0##suffix
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned int _cel_align_up_ui(unsigned int val,
                                                size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, UINT_MAX);

  return _CEL_ALIGN_UP(val, align, u);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned char _cel_align_up_uc(unsigned char val,
                                                 size_t align) {
  CEL_ASSERT_LE(align, UCHAR_MAX);

  return (unsigned char)_cel_align_up_ui(val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned short _cel_align_up_us(unsigned short val,
                                                  size_t align) {
  CEL_ASSERT_LE(align, USHRT_MAX);

  return (unsigned short)_cel_align_up_ui(val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long _cel_align_up_ul(unsigned long val,
                                                 size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, ULONG_MAX);

  return _CEL_ALIGN_UP(val, align, ul);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long long _cel_align_up_ull(unsigned long long val,
                                                       size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, ULLONG_MAX);

  return _CEL_ALIGN_UP(val, align, ull);
}

#undef _CEL_ALIGN_UP

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uintptr_t _cel_align_up_up(uintptr_t val, size_t align) {
#if UINTPTR_MAX == ULLONG_MAX
  return (uintptr_t)_cel_align_up_ull((unsigned long long)val, align);
#elif UINTPTR_MAX == ULONG_MAX
  return (uintptr_t)_cel_align_up_ul((unsigned long)val, align);
#elif UINTPTR_MAX == UINT_MAX
  return (uintptr_t)_cel_align_up_ui((unsigned int)val, align);
#else
#error Unsupported configuration.
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE void* cel_nullability_unknown
_cel_align_up_vp(void* cel_nullability_unknown val, size_t align) {
  return (void*)_cel_align_up_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const void* cel_nullability_unknown
_cel_align_up_cvp(const void* cel_nullability_unknown val, size_t align) {
  return (const void*)_cel_align_up_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE char* cel_nullability_unknown
_cel_align_up_cp(char* cel_nullability_unknown val, size_t align) {
  return (char*)_cel_align_up_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const char* cel_nullability_unknown
_cel_align_up_ccp(const char* cel_nullability_unknown val, size_t align) {
  return (const char*)_cel_align_up_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned char* cel_nullability_unknown
_cel_align_up_ucp(unsigned char* cel_nullability_unknown val, size_t align) {
  return (unsigned char*)_cel_align_up_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const unsigned char* cel_nullability_unknown
_cel_align_up_uccp(const unsigned char* cel_nullability_unknown val,
                   size_t align) {
  return (const unsigned char*)_cel_align_up_up((uintptr_t)val, align);
}

CEL_END_DECLS

// _cel_align_up
//
// Aligns the value upward. if the value is already aligned or `0`, it is
// returned as is.
#ifndef __cplusplus
#define _cel_align_up(val, align)             \
  (_Generic((val),                            \
       unsigned char: _cel_align_up_uc,       \
       unsigned short: _cel_align_up_us,      \
       unsigned int: _cel_align_up_ui,        \
       unsigned long: _cel_align_up_ul,       \
       unsigned long long: _cel_align_up_ull, \
       void*: _cel_align_up_vp,               \
       const void*: _cel_align_up_cvp,        \
       char*: _cel_align_up_cp,               \
       const char*: _cel_align_up_ccp,        \
       unsigned char*: _cel_align_up_ucp,     \
       const unsigned char*: _cel_align_up_uccp)((val), (align)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned char _cel_align_up(unsigned char val, size_t align) {
  return _cel_align_up_uc(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned short _cel_align_up(unsigned short val, size_t align) {
  return _cel_align_up_us(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned int _cel_align_up(unsigned int val, size_t align) {
  return _cel_align_up_ui(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long _cel_align_up(unsigned long val, size_t align) {
  return _cel_align_up_ul(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long long _cel_align_up(unsigned long long val,
                                            size_t align) {
  return _cel_align_up_ull(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE void* cel_nullability_unknown
_cel_align_up(void* cel_nullability_unknown val, size_t align) {
  return _cel_align_up_vp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const void* cel_nullability_unknown
_cel_align_up(const void* cel_nullability_unknown val, size_t align) {
  return _cel_align_up_cvp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE char* cel_nullability_unknown
_cel_align_up(char* cel_nullability_unknown val, size_t align) {
  return _cel_align_up_cp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const char* cel_nullability_unknown
_cel_align_up(const char* cel_nullability_unknown val, size_t align) {
  return _cel_align_up_ccp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned char* cel_nullability_unknown
_cel_align_up(unsigned char* cel_nullability_unknown val, size_t align) {
  return _cel_align_up_ucp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const unsigned char* cel_nullability_unknown
_cel_align_up(const unsigned char* cel_nullability_unknown val, size_t align) {
  return _cel_align_up_uccp(val, align);
}
#endif

CEL_BEGIN_DECLS

#if CEL_HAVE_BUILTIN(__builtin_align_down)
#define _CEL_ALIGN_DOWN(val, align, suffix) \
  ((val) != 0##suffix) ? __builtin_align_down((val), (align)) : 0##suffix
#else
#define _CEL_ALIGN_DOWN(val, align, suffix)                                  \
  ((val) != 0##suffix) ? ((val) & ~(((cel_typeof(val))(align)) - 1##suffix)) \
                       : 0##suffix
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned int _cel_align_down_ui(unsigned int val,
                                                  size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, UINT_MAX);
  return _CEL_ALIGN_DOWN(val, align, u);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned char _cel_align_down_uc(unsigned char val,
                                                   size_t align) {
  CEL_ASSERT_LE(align, UCHAR_MAX);
  return (unsigned char)_cel_align_down_ui(val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned short _cel_align_down_us(unsigned short val,
                                                    size_t align) {
  CEL_ASSERT_LE(align, USHRT_MAX);
  return (unsigned short)_cel_align_down_ui(val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long _cel_align_down_ul(unsigned long val,
                                                   size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, ULONG_MAX);
  return _CEL_ALIGN_DOWN(val, align, ul);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned long long _cel_align_down_ull(unsigned long long val,
                                                         size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, ULLONG_MAX);
  return _CEL_ALIGN_DOWN(val, align, ull);
}

#undef _CEL_ALIGN_DOWN

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uintptr_t _cel_align_down_up(uintptr_t val, size_t align) {
#if UINTPTR_MAX == ULLONG_MAX
  return (uintptr_t)_cel_align_down_ull((unsigned long long)val, align);
#elif UINTPTR_MAX == ULONG_MAX
  return (uintptr_t)_cel_align_down_ul((unsigned long)val, align);
#elif UINTPTR_MAX == UINT_MAX
  return (uintptr_t)_cel_align_down_ui((unsigned int)val, align);
#else
#error Unsupported configuration.
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE void* cel_nullability_unknown
_cel_align_down_vp(void* cel_nullability_unknown val, size_t align) {
  return (void*)_cel_align_down_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const void* cel_nullability_unknown
_cel_align_down_cvp(const void* cel_nullability_unknown val, size_t align) {
  return (const void*)_cel_align_down_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE char* cel_nullability_unknown
_cel_align_down_cp(char* cel_nullability_unknown val, size_t align) {
  return (char*)_cel_align_down_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const char* cel_nullability_unknown
_cel_align_down_ccp(const char* cel_nullability_unknown val, size_t align) {
  return (const char*)_cel_align_down_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE unsigned char* cel_nullability_unknown
_cel_align_down_ucp(unsigned char* cel_nullability_unknown val, size_t align) {
  return (unsigned char*)_cel_align_down_up((uintptr_t)val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const unsigned char* cel_nullability_unknown
_cel_align_down_uccp(const unsigned char* cel_nullability_unknown val,
                     size_t align) {
  return (const unsigned char*)_cel_align_down_up((uintptr_t)val, align);
}

CEL_END_DECLS

// _cel_align_down
//
// Aligns the value downward. If the value is already aligned or `0`, it is
// returned as is.
#ifndef __cplusplus
#define _cel_align_down(val, align)             \
  (_Generic((val),                              \
       unsigned char: _cel_align_down_uc,       \
       unsigned short: _cel_align_down_us,      \
       unsigned int: _cel_align_down_ui,        \
       unsigned long: _cel_align_down_ul,       \
       unsigned long long: _cel_align_down_ull, \
       void*: _cel_align_down_vp,               \
       const void*: _cel_align_down_cvp,        \
       char*: _cel_align_down_cp,               \
       const char*: _cel_align_down_ccp,        \
       unsigned char*: _cel_align_down_ucp,     \
       const unsigned char*: _cel_align_down_uccp)((val), (align)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned char _cel_align_down(unsigned char val, size_t align) {
  return _cel_align_down_uc(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned short _cel_align_down(unsigned short val, size_t align) {
  return _cel_align_down_us(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned int _cel_align_down(unsigned int val, size_t align) {
  return _cel_align_down_ui(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long _cel_align_down(unsigned long val, size_t align) {
  return _cel_align_down_ul(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long long _cel_align_down(unsigned long long val,
                                              size_t align) {
  return _cel_align_down_ull(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE void* cel_nullability_unknown
_cel_align_down(void* cel_nullability_unknown val, size_t align) {
  return _cel_align_down_vp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const void* cel_nullability_unknown
_cel_align_down(const void* cel_nullability_unknown val, size_t align) {
  return _cel_align_down_cvp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE char* cel_nullability_unknown
_cel_align_down(char* cel_nullability_unknown val, size_t align) {
  return _cel_align_down_cp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const char* cel_nullability_unknown
_cel_align_down(const char* cel_nullability_unknown val, size_t align) {
  return _cel_align_down_ccp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned char* cel_nullability_unknown
_cel_align_down(unsigned char* cel_nullability_unknown val, size_t align) {
  return _cel_align_down_ucp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE const unsigned char* cel_nullability_unknown _cel_align_down(
    const unsigned char* cel_nullability_unknown val, size_t align) {
  return _cel_align_down_uccp(val, align);
}
#endif

CEL_BEGIN_DECLS

#if CEL_HAVE_BUILTIN(__builtin_is_aligned)
#define _CEL_IS_ALIGNED(val, align, suffix) __builtin_is_aligned((val), (align))
#else
#define _CEL_IS_ALIGNED(val, align, suffix) \
  ((val) & (((cel_typeof(val))(align)) - 1##suffix)) == 0##suffix
#endif

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_is_aligned_ui(unsigned int val, size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, UINT_MAX);
  return _CEL_IS_ALIGNED(val, align, u);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_is_aligned_uc(unsigned char val, size_t align) {
  CEL_ASSERT_LE(align, UCHAR_MAX);
  return _cel_is_aligned_ui(val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_is_aligned_us(unsigned short val, size_t align) {
  CEL_ASSERT_LE(align, USHRT_MAX);
  return _cel_is_aligned_ui(val, align);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_is_aligned_ul(unsigned long val, size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, ULONG_MAX);
  return _CEL_IS_ALIGNED(val, align, ul);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_is_aligned_ull(unsigned long long val,
                                           size_t align) {
  CEL_ASSERT(_cel_has_single_bit(align));
  CEL_ASSERT_LE(align, ULLONG_MAX);
  return _CEL_IS_ALIGNED(val, align, ull);
}

#undef _CEL_IS_ALIGNED

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_is_aligned_up(uintptr_t val, size_t align) {
#if UINTPTR_MAX == ULLONG_MAX
  return _cel_is_aligned_ull((unsigned long long)val, align);
#elif UINTPTR_MAX == ULONG_MAX
  return _cel_is_aligned_ul((unsigned long)val, align);
#elif UINTPTR_MAX == UINT_MAX
  return _cel_is_aligned_ui((unsigned int)val, align);
#else
#error Unsupported configuration.
#endif
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_is_aligned_cvp(
    const void* cel_nullability_unknown val, size_t align) {
  return _cel_is_aligned_up((uintptr_t)val, align);
}

CEL_END_DECLS

// _cel_is_aligned
//
// Tests whether the value is aligned. If the value is `0`, the result is always
// `true`.
#ifndef __cplusplus
#define _cel_is_aligned(val, align)             \
  (_Generic((val),                              \
       unsigned char: _cel_is_aligned_uc,       \
       unsigned short: _cel_is_aligned_us,      \
       unsigned int: _cel_is_aligned_ui,        \
       unsigned long: _cel_is_aligned_ul,       \
       unsigned long long: _cel_is_aligned_ull, \
       void*: _cel_is_aligned_cvp,              \
       const void*: _cel_is_aligned_cvp,        \
       char*: _cel_is_aligned_cvp,              \
       const char*: _cel_is_aligned_cvp,        \
       unsigned char*: _cel_is_aligned_cvp,     \
       const unsigned char*: _cel_is_aligned_cvp)((val), (align)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned char _cel_is_aligned(unsigned char val, size_t align) {
  return _cel_is_aligned_uc(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned short _cel_is_aligned(unsigned short val, size_t align) {
  return _cel_is_aligned_us(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned int _cel_is_aligned(unsigned int val, size_t align) {
  return _cel_is_aligned_ui(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long _cel_is_aligned(unsigned long val, size_t align) {
  return _cel_is_aligned_ul(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE unsigned long long _cel_is_aligned(unsigned long long val,
                                              size_t align) {
  return _cel_is_aligned_ull(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_is_aligned(void* cel_nullability_unknown val,
                                size_t align) {
  return _cel_is_aligned_cvp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_is_aligned(const void* cel_nullability_unknown val,
                                size_t align) {
  return _cel_is_aligned_cvp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_is_aligned(char* cel_nullability_unknown val,
                                size_t align) {
  return _cel_is_aligned_cvp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_is_aligned(const char* cel_nullability_unknown val,
                                size_t align) {
  return _cel_is_aligned_cvp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_is_aligned(unsigned char* cel_nullability_unknown val,
                                size_t align) {
  return _cel_is_aligned_cvp(val, align);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE bool _cel_is_aligned(
    const unsigned char* cel_nullability_unknown val, size_t align) {
  return _cel_is_aligned_cvp(val, align);
}
#endif

CEL_BEGIN_DECLS

CEL_END_DECLS

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_SRC_ALIGN_H_
