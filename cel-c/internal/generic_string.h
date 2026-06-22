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

// Internal header providing a generic string implementation.

// IWYU pragma: private
// IWYU pragma: friend "cel-c/internal/(?:(?:arena|generic)_)?string\.[hc]"

#ifndef THIRD_PARTY_CEL_C_INTERNAL_GENERIC_STRING_H_
#define THIRD_PARTY_CEL_C_INTERNAL_GENERIC_STRING_H_

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/cstring_view.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef struct {
  CEL_NONNULL(char*) data;
  size_t cap;
  size_t len : sizeof(size_t) * CHAR_BIT - 1;
  size_t is_large : 1;
} _cel_GenericStringLarge;

typedef struct {
  char data[sizeof(_cel_GenericStringLarge) - 1];
  uint8_t len : 7;
  uint8_t is_large : 1;
} _cel_GenericStringSmall;

#define _cel_GenericStringSmall_kCapacity \
  (sizeof(((_cel_GenericStringSmall*)cel_nullptr)->data) - 1)

#define _cel_GenericString_kMaxSize (((size_t)PTRDIFF_MAX) - 1)

CEL_STATIC_ASSERT(sizeof(_cel_GenericStringSmall) ==
                  sizeof(_cel_GenericStringLarge));

typedef struct {
  union {
    // We implement SSO similar to the alternative layout of `std::string` in
    // libcxx. For strings with a length less than or equal to
    // `_cel_GenericStringSmall_kCapacity` we store the data inline. Otherwise
    // we store the data on the heap.
    _cel_GenericStringLarge large;
    _cel_GenericStringSmall small;
  };
} _cel_GenericString;

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericString_Construct(CEL_NONNULL(_cel_GenericString*) str);

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericString_DestructAllocator(CEL_NONNULL(_cel_GenericString*) str,
                                          CEL_NONNULL(cel_Allocator*) alloc);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_GenericString_Size(CEL_NONNULL(const _cel_GenericString*) str) {
  CEL_ASSERT_NOT_NULL(str);

  return str->large.is_large ? str->large.len : str->small.len;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_GenericString_SizeInt(
    CEL_NONNULL(const _cel_GenericString*) str) {
  CEL_ASSERT_NOT_NULL(str);

  size_t size = str->large.is_large ? str->large.len : str->small.len;
  return size <= (size_t)INT_MAX ? (int)size : INT_MAX;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_GenericString_Empty(
    CEL_NONNULL(const _cel_GenericString*) str) {
  return _cel_GenericString_Size(str) == 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t
_cel_GenericString_Capacity(CEL_NONNULL(const _cel_GenericString*) str) {
  CEL_ASSERT_NOT_NULL(str);

  return str->large.is_large ? str->large.cap
                             : _cel_GenericStringSmall_kCapacity;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const char*)
    _cel_GenericString_Data(CEL_NONNULL(const _cel_GenericString*) str) {
  CEL_ASSERT_NOT_NULL(str);

  return str->large.is_large ? str->large.data : str->small.data;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(char*)
    _cel_GenericString_MutableData(CEL_NONNULL(_cel_GenericString*) str) {
  CEL_ASSERT_NOT_NULL(str);

  return str->large.is_large ? str->large.data : str->small.data;
}

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericString_Clear(CEL_NONNULL(_cel_GenericString*) str);

CEL_ATTRIBUTE_NOTHROW
void _cel_GenericString_ResetAllocator(CEL_NONNULL(_cel_GenericString*) str,
                                       CEL_NONNULL(cel_Allocator*) alloc);

CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_ShrinkToFitAllocator(CEL_NONNULL(_cel_GenericString*)
                                                 str,
                                             CEL_NONNULL(cel_Allocator*) alloc);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_AssignAllocator(CEL_NONNULL(_cel_GenericString*) str,
                                        CEL_NONNULL(cel_Allocator*) alloc,
                                        cel_StringView val);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_AssignArena(CEL_NONNULL(_cel_GenericString*) str,
                                    CEL_NONNULL(cel_Arena*) arena,
                                    cel_StringView val);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_AppendAllocator(CEL_NONNULL(_cel_GenericString*) str,
                                        CEL_NONNULL(cel_Allocator*) alloc,
                                        cel_StringView val);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_AppendArena(CEL_NONNULL(_cel_GenericString*) str,
                                    CEL_NONNULL(cel_Arena*) arena,
                                    cel_StringView val);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_PushBackAllocator(CEL_NONNULL(_cel_GenericString*) str,
                                          CEL_NONNULL(cel_Allocator*) alloc,
                                          char val);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_PushBackArena(CEL_NONNULL(_cel_GenericString*) str,
                                      CEL_NONNULL(cel_Arena*) arena, char val);

CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_ReserveAllocator(CEL_NONNULL(_cel_GenericString*) str,
                                         CEL_NONNULL(cel_Allocator*) alloc,
                                         size_t capacity);

CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_ReserveArena(CEL_NONNULL(_cel_GenericString*) str,
                                     CEL_NONNULL(cel_Arena*) arena,
                                     size_t capacity);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_VFORMAT(3)
CEL_ATTRIBUTE_NOTHROW
ptrdiff_t _cel_GenericString_VAppendFAllocator(
    CEL_NONNULL(_cel_GenericString*) str, CEL_NONNULL(cel_Allocator*) alloc,
    CEL_NONNULL(const char*) fmt, va_list args);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_VFORMAT(3)
CEL_ATTRIBUTE_NOTHROW
ptrdiff_t _cel_GenericString_VAppendFArena(CEL_NONNULL(_cel_GenericString*) str,
                                           CEL_NONNULL(cel_Arena*) arena,
                                           CEL_NONNULL(const char*) fmt,
                                           va_list args);

// _cel_GenericString_StabilizeAllocator
//
// Stabilizes the string such that even if `str` is relocated in memory, the
// result from `_cel_GenericString_Data`/`_cel_GenericString_MutableData` will
// be the same.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_StabilizeAllocator(CEL_NONNULL(_cel_GenericString*) str,
                                           CEL_NONNULL(cel_Allocator*) alloc);

// _cel_GenericString_StabilizeArena
//
// Stabilizes the string such that even if `str` is relocated in memory, the
// result from `_cel_GenericString_Data`/`_cel_GenericString_MutableData` will
// be the same.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_GenericString_StabilizeArena(CEL_NONNULL(_cel_GenericString*) str,
                                       CEL_NONNULL(cel_Arena*) arena);

// _cel_GenericString_DestabilizeAllocator
//
// Potentially undo the stabilization done by
// `_cel_GenericString_StabilizeAllocator`. If the string is small enough such
// that SSO can be applied, it will be moved inline.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericString_DestabilizeAllocator(CEL_NONNULL(_cel_GenericString*)
                                                 str,
                                             CEL_NONNULL(cel_Allocator*) alloc);

// _cel_GenericString_DestabilizeArena
//
// Potentially undo the stabilization done by
// `_cel_GenericString_StabilizeArena`. If the string is small enough such
// that SSO can be applied, it will be moved inline.
CEL_ATTRIBUTE_NOTHROW
void _cel_GenericString_DestabilizeArena(CEL_NONNULL(_cel_GenericString*) str,
                                         CEL_NONNULL(cel_Arena*) arena);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
_cel_GenericString_ToStringView(CEL_NONNULL(const _cel_GenericString*) str) {
  CEL_ASSERT_NOT_NULL(str);

  if (str->large.is_large) {
    return cel_StringView_FromArray(str->large.data, str->large.len);
  }
  return cel_StringView_FromArray(str->small.data, str->small.len);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_CStringView
_cel_GenericString_ToCStringView(CEL_NONNULL(const _cel_GenericString*) str) {
  CEL_ASSERT_NOT_NULL(str);

  // We cheat and create cel_CStringView directly rather than using factory
  // functions. We know _cel_GenericString backing data conforms to the
  // requirements for cel_CStringView.
  cel_CStringView out;
  if (str->large.is_large) {
    out.data = str->large.data;
  } else {
    out.data = str->small.data;
  }
  return out;
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_GENERIC_STRING_H_
