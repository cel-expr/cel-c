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
#include "upb/base/string_view.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

typedef struct {
  uint64_t val;
} cel_HashState;

// cel_HashState_Initialize
//
// Returns an initial `cel_HashState`.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_ATTRIBUTE_PURE
CEL_EXTERN cel_HashState cel_HashState_Initialize();

// cel_HashState_Finalize
//
// Computes the final hash code and returns it.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t cel_HashState_Finalize(cel_HashState state) {
  return (size_t)state.val;
}

// cel_HashState_CombineN
//
// Combines the contiguous block of memory starting at `data` and extending
// `size` bytes into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_HashState cel_HashState_CombineN(cel_HashState state,
                                                const void* cel_nullable data,
                                                size_t size);

// cel_HashState_Combine64
//
// Combines the 64-bit value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_HashState cel_HashState_Combine64(cel_HashState state,
                                                 uint64_t data);

// cel_HashState_Combine8
//
// Combines the 8-bit value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_Combine8(cel_HashState state,
                                                       uint8_t data) {
  return cel_HashState_Combine64(state, data);
}

// cel_HashState_Combine16
//
// Combines the 16-bit value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_Combine16(cel_HashState state,
                                                        uint16_t data) {
  return cel_HashState_Combine64(state, data);
}

// cel_HashState_Combine32
//
// Combines the 32-bit value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_Combine32(cel_HashState state,
                                                        uint32_t data) {
  return cel_HashState_Combine64(state, data);
}

// cel_HashState_CombineB
//
// Combines the bool value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineB(cel_HashState state,
                                                       bool value) {
  CEL_STATIC_ASSERT(sizeof(value) == 1);
  return cel_HashState_Combine8(state, (uint8_t)(value ? 1 : 0));
}

// cel_HashState_CombineUC
//
// Combines the unsigned char value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineUC(cel_HashState state,
                                                        unsigned char value) {
  CEL_STATIC_ASSERT(sizeof(value) == 1);
  return cel_HashState_Combine8(state, (uint8_t)value);
}

// cel_HashState_CombineC
//
// Combines the char value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineC(cel_HashState state,
                                                       char value) {
  return cel_HashState_CombineUC(state, (unsigned char)value);
}

// cel_HashState_CombineSC
//
// Combines the signed char value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineSC(cel_HashState state,
                                                        signed char value) {
  return cel_HashState_CombineUC(state, (unsigned char)value);
}

// cel_HashState_CombineUS
//
// Combines the unsigned short value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineUS(cel_HashState state,
                                                        unsigned short value) {
  CEL_STATIC_ASSERT(sizeof(value) == 2);
  return cel_HashState_Combine16(state, (uint16_t)value);
}

// cel_HashState_CombineS
//
// Combines the short value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineS(cel_HashState state,
                                                       short value) {
  return cel_HashState_CombineUS(state, (unsigned short)value);
}

// cel_HashState_CombineUI
//
// Combines the unsigned int value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineUI(cel_HashState state,
                                                        unsigned int value) {
  CEL_STATIC_ASSERT(sizeof(value) == 4);
  return cel_HashState_Combine32(state, (uint32_t)value);
}

// cel_HashState_CombineI
//
// Combines the int value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineI(cel_HashState state,
                                                       int value) {
  return cel_HashState_CombineUI(state, (unsigned int)value);
}

// cel_HashState_CombineUL
//
// Combines the unsigned long value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineUL(cel_HashState state,
                                                        unsigned long value) {
  CEL_STATIC_ASSERT(sizeof(value) == 4 || sizeof(value) == 8);
#if LONG_MAX == INT32_MAX
  return cel_HashState_Combine32(state, (uint32_t)value);
#elif LONG_MAX == INT64_MAX
  return cel_HashState_Combine64(state, (uint64_t)value);
#else
#error Unreachable.
#endif
}

// cel_HashState_CombineL
//
// Combines the long value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineL(cel_HashState state,
                                                       long value) {
  return cel_HashState_CombineUL(state, (unsigned long)value);
}

// cel_HashState_CombineULL
//
// Combines the unsigned long long value into the hash state, returning the
// updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState
cel_HashState_CombineULL(cel_HashState state, unsigned long long value) {
  CEL_STATIC_ASSERT(sizeof(value) == 8);
  return cel_HashState_Combine64(state, (uint64_t)value);
}

// cel_HashState_CombineLL
//
// Combines the long long value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineLL(cel_HashState state,
                                                        long long value) {
  return cel_HashState_CombineULL(state, (unsigned long long)value);
}

// cel_HashState_CombineF
//
// Combines the float value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineF(cel_HashState state,
                                                       float value) {
  CEL_STATIC_ASSERT(sizeof(value) == sizeof(uint32_t));
  union {
    float f;
    uint32_t u;
  } data = {.f = (value != 0.0f ? value : 0.0f)};
  return cel_HashState_Combine32(state, data.u);
}

// cel_HashState_CombineD
//
// Combines the double value into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineD(cel_HashState state,
                                                       double value) {
  CEL_STATIC_ASSERT(sizeof(value) == sizeof(uint64_t));
  union {
    double d;
    uint64_t u;
  } data = {.d = (value != 0.0 ? value : 0.0)};
  return cel_HashState_Combine64(state, data.u);
}

// cel_HashState_CombineLD
//
// Combines the long double value into the hash state, returning the updated
// state.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_HashState cel_HashState_CombineLD(cel_HashState state,
                                                 long double value);

// cel_HashState_CombineStr
//
// Combines the C string into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState
cel_HashState_CombineStr(cel_HashState state, const char* cel_nullable str) {
  return cel_HashState_CombineN(state, str,
                                str != cel_nullptr ? strlen(str) : 0);
}

// cel_HashState_CombineMem
//
// Combines the upb string into the hash state, returning the updated state.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_HashState_CombineMem(cel_HashState state,
                                                         upb_StringView mem) {
  return cel_HashState_CombineN(state, mem.data, mem.size);
}

CEL_END_DECLS

// cel_HashState_Combine
//
// Combines the value into the hash state, returning the updated state.
#ifndef __cplusplus
#define cel_HashState_Combine(state, x)              \
  (_Generic((x),                                     \
       bool: cel_HashState_CombineB,                 \
       char: cel_HashState_CombineC,                 \
       signed char: cel_HashState_CombineSC,         \
       unsigned char: cel_HashState_CombineUC,       \
       short: cel_HashState_CombineS,                \
       unsigned short: cel_HashState_CombineUS,      \
       int: cel_HashState_CombineI,                  \
       unsigned int: cel_HashState_CombineUI,        \
       long: cel_HashState_CombineL,                 \
       unsigned long: cel_HashState_CombineUL,       \
       long long: cel_HashState_CombineLL,           \
       unsigned long long: cel_HashState_CombineULL, \
       float: cel_HashState_CombineF,                \
       double: cel_HashState_CombineD,               \
       long double: cel_HashState_CombineLD,         \
       char*: cel_HashState_CombineStr,              \
       const char*: cel_HashState_CombineStr,        \
       upb_StringView: cel_HashState_CombineMem)((state), (x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state, bool val) {
  return cel_HashState_CombineB(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state, char val) {
  return cel_HashState_CombineC(state, val);
}
#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               char8_t val) {
  CEL_STATIC_ASSERT(sizeof(val) == sizeof(unsigned char));
  return cel_HashState_CombineUC(state, (unsigned char)val);
}
#endif
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               char16_t val) {
  CEL_STATIC_ASSERT(sizeof(val) == sizeof(unsigned short));
  return cel_HashState_CombineUS(state, (unsigned short)val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               char32_t val) {
  CEL_STATIC_ASSERT(sizeof(val) == sizeof(unsigned int));
  return cel_HashState_CombineUI(state, (unsigned int)val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               signed char val) {
  return cel_HashState_CombineSC(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               unsigned char val) {
  return cel_HashState_CombineUC(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state, short val) {
  return cel_HashState_CombineS(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               unsigned short val) {
  return cel_HashState_CombineUS(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state, int val) {
  return cel_HashState_CombineI(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               unsigned int val) {
  return cel_HashState_CombineUI(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state, long val) {
  return cel_HashState_CombineL(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               unsigned long val) {
  return cel_HashState_CombineUL(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               long long val) {
  return cel_HashState_CombineLL(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               unsigned long long val) {
  return cel_HashState_CombineULL(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state, float val) {
  return cel_HashState_CombineF(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               double val) {
  return cel_HashState_CombineD(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               long double val) {
  return cel_HashState_CombineLD(state, val);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               const char* cel_nullable str) {
  return cel_HashState_CombineStr(state, str);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               upb_StringView mem) {
  return cel_HashState_CombineMem(state, mem);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_HashState cel_HashState_Combine(cel_HashState state,
                                               wchar_t val) {
  CEL_STATIC_ASSERT(sizeof(wchar_t) == 1 || sizeof(wchar_t) == 2 ||
                    sizeof(wchar_t) == 4 || sizeof(wchar_t) == 8);
  if constexpr (sizeof(wchar_t) == 1) {
    return cel_HashState_Combine(state, (uint8_t)val);
  } else if constexpr (sizeof(wchar_t) == 2) {
    return cel_HashState_Combine(state, (uint16_t)val);
  } else if constexpr (sizeof(wchar_t) == 4) {
    return cel_HashState_Combine(state, (uint32_t)val);
  } else {
    return cel_HashState_Combine(state, (uint64_t)val);
  }
}
#endif

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_HASH_H_
