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

// Internal header providing a generic string implementation which uses
// allocator-based memory management.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_STRING_H_
#define THIRD_PARTY_CEL_C_INTERNAL_STRING_H_

#include <stdarg.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "cel-c/cstring_view.h"
#include "cel-c/internal/generic_string.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef struct {
  _cel_GenericString g;
} _cel_String;

static CEL_INLINE void _cel_String_Construct(CEL_NONNULL(_cel_String*) str) {
  _cel_GenericString_Construct(&str->g);
}

static CEL_INLINE void _cel_String_Destruct(CEL_NONNULL(_cel_String*) str,
                                            CEL_NONNULL(cel_Allocator*) alloc) {
  _cel_GenericString_DestructAllocator(&str->g, alloc);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_String_Size(CEL_NONNULL(const _cel_String*) str) {
  return _cel_GenericString_Size(&str->g);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_String_SizeInt(CEL_NONNULL(const _cel_String*) str) {
  return _cel_GenericString_SizeInt(&str->g);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_String_Empty(CEL_NONNULL(const _cel_String*) str) {
  return _cel_GenericString_Empty(&str->g);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_String_Capacity(CEL_NONNULL(const _cel_String*)
                                                  str) {
  return _cel_GenericString_Capacity(&str->g);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(const char*)
    _cel_String_Data(CEL_NONNULL(const _cel_String*) str) {
  return _cel_GenericString_Data(&str->g);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE CEL_NONNULL(char*)
    _cel_String_MutableData(CEL_NONNULL(_cel_String*) str) {
  return _cel_GenericString_MutableData(&str->g);
}

static CEL_INLINE void _cel_String_Clear(CEL_NONNULL(_cel_String*) str) {
  _cel_GenericString_Clear(&str->g);
}

static CEL_INLINE void _cel_String_Reset(CEL_NONNULL(_cel_String*) str,
                                         CEL_NONNULL(cel_Allocator*) alloc) {
  _cel_GenericString_ResetAllocator(&str->g, alloc);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_String_ShrinkToFit(CEL_NONNULL(_cel_String*) str,
                                               CEL_NONNULL(cel_Allocator*)
                                                   alloc) {
  return _cel_GenericString_ShrinkToFitAllocator(&str->g, alloc);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_String_Assign(CEL_NONNULL(_cel_String*) str,
                                          CEL_NONNULL(cel_Allocator*) alloc,
                                          cel_StringView val) {
  return _cel_GenericString_AssignAllocator(&str->g, alloc, val);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_VFORMAT(3)
static CEL_INLINE ptrdiff_t _cel_String_VAppendF(CEL_NONNULL(_cel_String*) str,
                                                 CEL_NONNULL(cel_Allocator*)
                                                     alloc,
                                                 CEL_NONNULL(const char*) fmt,
                                                 va_list args) {
  return _cel_GenericString_VAppendFAllocator(&str->g, alloc, fmt, args);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_FORMAT(3, 4)
static CEL_INLINE ptrdiff_t _cel_String_AppendF(CEL_NONNULL(_cel_String*) str,
                                                CEL_NONNULL(cel_Allocator*)
                                                    alloc,
                                                CEL_NONNULL(const char*) fmt,
                                                ...) {
  va_list args;
  va_start(args, fmt);
  const ptrdiff_t ret = _cel_String_VAppendF(str, alloc, fmt, args);
  va_end(args);
  return ret;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_String_Append(CEL_NONNULL(_cel_String*) str,
                                          CEL_NONNULL(cel_Allocator*) alloc,
                                          cel_StringView val) {
  return _cel_GenericString_AppendAllocator(&str->g, alloc, val);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_String_PushBack(CEL_NONNULL(_cel_String*) str,
                                            CEL_NONNULL(cel_Allocator*) alloc,
                                            char val) {
  return _cel_GenericString_PushBackAllocator(&str->g, alloc, val);
}

static CEL_INLINE bool _cel_String_Reserve(CEL_NONNULL(_cel_String*) str,
                                           CEL_NONNULL(cel_Allocator*) alloc,
                                           size_t capacity) {
  return _cel_GenericString_ReserveAllocator(&str->g, alloc, capacity);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_String_Stabilize(CEL_NONNULL(_cel_String*) str,
                                             CEL_NONNULL(cel_Allocator*)
                                                 alloc) {
  return _cel_GenericString_StabilizeAllocator(&str->g, alloc);
}

static CEL_INLINE void _cel_String_Destabilize(CEL_NONNULL(_cel_String*) str,
                                               CEL_NONNULL(cel_Allocator*)
                                                   alloc) {
  _cel_GenericString_DestabilizeAllocator(&str->g, alloc);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
_cel_String_ToStringView(CEL_NONNULL(const _cel_String*) str) {
  return _cel_GenericString_ToStringView(&str->g);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_CStringView
_cel_String_ToCStringView(CEL_NONNULL(const _cel_String*) str) {
  return _cel_GenericString_ToCStringView(&str->g);
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_STRING_H_
