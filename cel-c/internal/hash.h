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

#ifndef THIRD_PARTY_CEL_C_HASH_H_
#define THIRD_PARTY_CEL_C_HASH_H_

#include <limits.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/config.h"
#include "cel-c/cstring_view.h"
#include "cel-c/string_view.h"
#include "upb/base/string_view.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

typedef struct {
  uint64_t val;
} _cel_HashState;

// _cel_HashState_Initialize
//
// Returns an initial `_cel_HashState`.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_PURE
_cel_HashState _cel_HashState_Initialize();

// _cel_HashState_Finalize
//
// Computes the final hash code and returns it.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_HashState_Finalize(_cel_HashState state) {
  return (size_t)state.val;
}

// _cel_HashState_CombineN
//
// Combines the contiguous block of memory starting at `data` and extending
// `size` bytes into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_HashState _cel_HashState_CombineN(_cel_HashState state,
                                       const void* cel_nullable data,
                                       size_t size);

// _cel_HashState_Combine64
//
// Combines the 64-bit value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_HashState _cel_HashState_Combine64(_cel_HashState state, uint64_t data);

// _cel_HashState_Combine8
//
// Combines the 8-bit value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_Combine8(_cel_HashState state,
                                                         uint8_t data) {
  return _cel_HashState_Combine64(state, data);
}

// _cel_HashState_Combine16
//
// Combines the 16-bit value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_Combine16(_cel_HashState state,
                                                          uint16_t data) {
  return _cel_HashState_Combine64(state, data);
}

// _cel_HashState_Combine32
//
// Combines the 32-bit value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_Combine32(_cel_HashState state,
                                                          uint32_t data) {
  return _cel_HashState_Combine64(state, data);
}

// _cel_HashState_CombineB
//
// Combines the bool value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineB(_cel_HashState state,
                                                         bool value) {
  CEL_STATIC_ASSERT(sizeof(value) == 1);
  return _cel_HashState_Combine8(state, (uint8_t)(value ? 1 : 0));
}

// _cel_HashState_CombineUC
//
// Combines the unsigned char value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineUC(_cel_HashState state,
                                                          unsigned char value) {
  CEL_STATIC_ASSERT(sizeof(value) == 1);
  return _cel_HashState_Combine8(state, (uint8_t)value);
}

// _cel_HashState_CombineC
//
// Combines the char value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineC(_cel_HashState state,
                                                         char value) {
  return _cel_HashState_CombineUC(state, (unsigned char)value);
}

// _cel_HashState_CombineSC
//
// Combines the signed char value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineSC(_cel_HashState state,
                                                          signed char value) {
  return _cel_HashState_CombineUC(state, (unsigned char)value);
}

// _cel_HashState_CombineUS
//
// Combines the unsigned short value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState
_cel_HashState_CombineUS(_cel_HashState state, unsigned short value) {
  CEL_STATIC_ASSERT(sizeof(value) == 2);
  return _cel_HashState_Combine16(state, (uint16_t)value);
}

// _cel_HashState_CombineS
//
// Combines the short value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineS(_cel_HashState state,
                                                         short value) {
  return _cel_HashState_CombineUS(state, (unsigned short)value);
}

// _cel_HashState_CombineUI
//
// Combines the unsigned int value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineUI(_cel_HashState state,
                                                          unsigned int value) {
  CEL_STATIC_ASSERT(sizeof(value) == 4);
  return _cel_HashState_Combine32(state, (uint32_t)value);
}

// _cel_HashState_CombineI
//
// Combines the int value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineI(_cel_HashState state,
                                                         int value) {
  return _cel_HashState_CombineUI(state, (unsigned int)value);
}

// _cel_HashState_CombineUL
//
// Combines the unsigned long value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineUL(_cel_HashState state,
                                                          unsigned long value) {
  CEL_STATIC_ASSERT(sizeof(value) == 4 || sizeof(value) == 8);
#if LONG_MAX == INT32_MAX
  return _cel_HashState_Combine32(state, (uint32_t)value);
#elif LONG_MAX == INT64_MAX
  return _cel_HashState_Combine64(state, (uint64_t)value);
#else
#error Unreachable.
#endif
}

// _cel_HashState_CombineL
//
// Combines the long value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineL(_cel_HashState state,
                                                         long value) {
  return _cel_HashState_CombineUL(state, (unsigned long)value);
}

// _cel_HashState_CombineULL
//
// Combines the unsigned long long value into the hash state, returning the
// updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState
_cel_HashState_CombineULL(_cel_HashState state, unsigned long long value) {
  CEL_STATIC_ASSERT(sizeof(value) == 8);
  return _cel_HashState_Combine64(state, (uint64_t)value);
}

// _cel_HashState_CombineLL
//
// Combines the long long value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineLL(_cel_HashState state,
                                                          long long value) {
  return _cel_HashState_CombineULL(state, (unsigned long long)value);
}

// _cel_HashState_CombineF
//
// Combines the float value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineF(_cel_HashState state,
                                                         float value) {
  CEL_STATIC_ASSERT(sizeof(value) == sizeof(uint32_t));
  union {
    float f;
    uint32_t u;
  } data = {.f = (value != 0.0f ? value : 0.0f)};
  return _cel_HashState_Combine32(state, data.u);
}

// _cel_HashState_CombineD
//
// Combines the double value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineD(_cel_HashState state,
                                                         double value) {
  CEL_STATIC_ASSERT(sizeof(value) == sizeof(uint64_t));
  union {
    double d;
    uint64_t u;
  } data = {.d = (value != 0.0 ? value : 0.0)};
  return _cel_HashState_Combine64(state, data.u);
}

// _cel_HashState_CombineLD
//
// Combines the long double value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
_cel_HashState _cel_HashState_CombineLD(_cel_HashState state,
                                        long double value);

// _cel_HashState_CombineStr
//
// Combines the C string into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState
_cel_HashState_CombineStr(_cel_HashState state, const char* cel_nullable str) {
  return _cel_HashState_CombineN(state, str,
                                 str != cel_nullptr ? strlen(str) : 0);
}

// _cel_HashState_CombineMem
//
// Combines the upb string into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState _cel_HashState_CombineMem(_cel_HashState state,
                                                           upb_StringView mem) {
  return _cel_HashState_CombineN(state, mem.data, mem.size);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState cel_StringView_Hash(cel_StringView str,
                                                     _cel_HashState state) {
  return _cel_HashState_CombineN(state, cel_StringView_Data(str),
                                 cel_StringView_Size(str));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_HashState cel_CStringView_Hash(cel_CStringView str,
                                                      _cel_HashState state) {
  return _cel_HashState_CombineN(state, cel_CStringView_Data(str),
                                 cel_CStringView_Size(str));
}

CEL_END_DECLS

// _cel_HashState_Combine
//
// Combines the value into the hash state, returning the updated state.
#ifndef __cplusplus
#define _cel_HashState_Combine(state, x)              \
  (_Generic((x),                                      \
       bool: _cel_HashState_CombineB,                 \
       char: _cel_HashState_CombineC,                 \
       signed char: _cel_HashState_CombineSC,         \
       unsigned char: _cel_HashState_CombineUC,       \
       short: _cel_HashState_CombineS,                \
       unsigned short: _cel_HashState_CombineUS,      \
       int: _cel_HashState_CombineI,                  \
       unsigned int: _cel_HashState_CombineUI,        \
       long: _cel_HashState_CombineL,                 \
       unsigned long: _cel_HashState_CombineUL,       \
       long long: _cel_HashState_CombineLL,           \
       unsigned long long: _cel_HashState_CombineULL, \
       float: _cel_HashState_CombineF,                \
       double: _cel_HashState_CombineD,               \
       long double: _cel_HashState_CombineLD,         \
       char*: _cel_HashState_CombineStr,              \
       const char*: _cel_HashState_CombineStr,        \
       upb_StringView: _cel_HashState_CombineMem)((state), (x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 bool val) {
  return _cel_HashState_CombineB(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 char val) {
  return _cel_HashState_CombineC(state, val);
}
#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 char8_t val) {
  CEL_STATIC_ASSERT(sizeof(val) == sizeof(unsigned char));
  return _cel_HashState_CombineUC(state, (unsigned char)val);
}
#endif
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 char16_t val) {
  CEL_STATIC_ASSERT(sizeof(val) == sizeof(unsigned short));
  return _cel_HashState_CombineUS(state, (unsigned short)val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 char32_t val) {
  CEL_STATIC_ASSERT(sizeof(val) == sizeof(unsigned int));
  return _cel_HashState_CombineUI(state, (unsigned int)val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 signed char val) {
  return _cel_HashState_CombineSC(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 unsigned char val) {
  return _cel_HashState_CombineUC(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 short val) {
  return _cel_HashState_CombineS(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 unsigned short val) {
  return _cel_HashState_CombineUS(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 int val) {
  return _cel_HashState_CombineI(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 unsigned int val) {
  return _cel_HashState_CombineUI(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 long val) {
  return _cel_HashState_CombineL(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 unsigned long val) {
  return _cel_HashState_CombineUL(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 long long val) {
  return _cel_HashState_CombineLL(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 unsigned long long val) {
  return _cel_HashState_CombineULL(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 float val) {
  return _cel_HashState_CombineF(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 double val) {
  return _cel_HashState_CombineD(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 long double val) {
  return _cel_HashState_CombineLD(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 const char* cel_nullable str) {
  return _cel_HashState_CombineStr(state, str);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 upb_StringView mem) {
  return _cel_HashState_CombineMem(state, mem);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE _cel_HashState _cel_HashState_Combine(_cel_HashState state,
                                                 wchar_t val) {
  CEL_STATIC_ASSERT(sizeof(wchar_t) == 1 || sizeof(wchar_t) == 2 ||
                    sizeof(wchar_t) == 4 || sizeof(wchar_t) == 8);
  if constexpr (sizeof(wchar_t) == 1) {
    return _cel_HashState_Combine(state, (uint8_t)val);
  } else if constexpr (sizeof(wchar_t) == 2) {
    return _cel_HashState_Combine(state, (uint16_t)val);
  } else if constexpr (sizeof(wchar_t) == 4) {
    return _cel_HashState_Combine(state, (uint32_t)val);
  } else {
    return _cel_HashState_Combine(state, (uint64_t)val);
  }
}
#endif

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_HASH_H_
