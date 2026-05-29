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

#ifndef THIRD_PARTY_CEL_C_STRING_VIEW_H_
#define THIRD_PARTY_CEL_C_STRING_VIEW_H_

#include <limits.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/cstring_view.h"
#include "cel-c/hash.h"
#include "upb/base/string_view.h"

CEL_BEGIN_DECLS

#ifdef CEL_STRINGVIEW_INIT
#error CEL_STRINGVIEW_INIT cannot be directly set
#endif

#ifdef CEL_STRINGVIEW_C
#error CEL_STRINGVIEW_C cannot be directly set
#endif

#ifdef CEL_STRINGVIEW_FMT
#error CEL_STRINGVIEW_FMT cannot be directly set
#endif

#ifdef CEL_STRINGVIEW_ARGS
#error CEL_STRINGVIEW_ARGS cannot be directly set
#endif

#define CEL_STRINGVIEW_INIT(data, size) UPB_STRINGVIEW_INIT(data, size)

#define CEL_STRINGVIEW_C(x) CEL_STRINGVIEW_INIT((x), cel_arraysize(x) - 1)

#define CEL_STRINGVIEW_FMT "%.*s"

#define CEL_STRINGVIEW_ARGS(str) \
  cel_StringView_SizeInt(str), cel_StringView_Data(str)

typedef upb_StringView cel_StringView;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_StringView_FromArray(const char* cel_nullable data, size_t size) {
  CEL_ASSERT(data != cel_nullptr || size == 0);

  return upb_StringView_FromDataAndSize(data, size);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_StringView_FromString(const char* cel_nullable str) {
  return cel_StringView_FromArray(str, str != cel_nullptr ? strlen(str) : 0);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView
cel_StringView_FromCString(cel_CStringView str) {
  return cel_StringView_FromString(cel_CStringView_Data(str));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const char* cel_nullability_unknown
cel_StringView_Data(cel_StringView str) {
  return str.data;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t cel_StringView_Size(cel_StringView str) {
  return str.size;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint32_t cel_StringView_Size32(cel_StringView str) {
  CEL_ASSERT_LE(cel_StringView_Size(str), UINT32_MAX);

  return CEL_LIKELY(cel_StringView_Size(str) <= (size_t)UINT32_MAX)
             ? (uint32_t)cel_StringView_Size(str)
             : UINT32_MAX;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int cel_StringView_SizeInt(cel_StringView str) {
  CEL_ASSERT_LE(cel_StringView_Size(str), (size_t)INT_MAX);

  return CEL_LIKELY(cel_StringView_Size(str) <= (size_t)INT_MAX)
             ? (int)cel_StringView_Size(str)
             : INT_MAX;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StringView_Empty(cel_StringView str) {
  return cel_StringView_Size(str) == 0;
}

static CEL_INLINE void cel_StringView_RemovePrefix(
    cel_StringView* cel_nonnull str, size_t n) {
  CEL_ASSERT_LE(n, cel_StringView_Size(*str));

  str->data += n;
  str->size -= n;
}

static CEL_INLINE void cel_StringView_RemoveSuffix(
    cel_StringView* cel_nonnull str, size_t n) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_LE(n, cel_StringView_Size(*str));

  str->size -= n;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StringView_StartsWith(cel_StringView lhs,
                                                 cel_StringView rhs) {
  const size_t rhs_size = cel_StringView_Size(rhs);
  return cel_StringView_Size(lhs) >= rhs_size &&
         memcmp(cel_StringView_Data(lhs), cel_StringView_Data(rhs), rhs_size) ==
             0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StringView_EndsWith(cel_StringView lhs,
                                               cel_StringView rhs) {
  const size_t lhs_size = cel_StringView_Size(lhs);
  const size_t rhs_size = cel_StringView_Size(rhs);
  return lhs_size >= rhs_size &&
         memcmp(cel_StringView_Data(lhs) + (lhs_size - rhs_size),
                cel_StringView_Data(rhs), rhs_size) == 0;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StringView_ConsumePrefix(
    cel_StringView* cel_nonnull lhs, cel_StringView rhs) {
  CEL_ASSERT_NOT_NULL(lhs);

  if (cel_StringView_StartsWith(*lhs, rhs)) {
    cel_StringView_RemovePrefix(lhs, cel_StringView_Size(rhs));
    return true;
  }
  return false;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StringView_ConsumeSuffix(
    cel_StringView* cel_nonnull lhs, cel_StringView rhs) {
  CEL_ASSERT_NOT_NULL(lhs);

  if (cel_StringView_EndsWith(*lhs, rhs)) {
    cel_StringView_RemoveSuffix(lhs, cel_StringView_Size(rhs));
    return true;
  }
  return false;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const char* cel_nullable
cel_StringView_FindFirst(cel_StringView haystack, cel_StringView needle);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN const char* cel_nullable
cel_StringView_FindLast(cel_StringView haystack, cel_StringView needle);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_StringView_Count(cel_StringView haystack,
                                       cel_StringView needle);

typedef struct {
  cel_StringView subject;
  cel_StringView delim;
  bool done;
} cel_StringViewTokenizer;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringViewTokenizer
cel_StringView_Tokenize(cel_StringView subject, cel_StringView delim) {
  CEL_ASSERT_NOT(cel_StringView_Empty(delim));

  cel_StringViewTokenizer tokenizer;
  tokenizer.subject = subject;
  tokenizer.delim = delim;
  tokenizer.done = false;
  return tokenizer;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_StringView_Hash(cel_StringView str,
                                                    cel_HashState state) {
  return cel_HashState_CombineN(state, cel_StringView_Data(str),
                                cel_StringView_Size(str));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_StringView_Equals(cel_StringView lhs,
                                             cel_StringView rhs) {
  const char* const lhs_data = cel_StringView_Data(lhs);
  const size_t lhs_size = cel_StringView_Size(lhs);
  const char* const rhs_data = cel_StringView_Data(rhs);
  return lhs_size == cel_StringView_Size(rhs) &&
         (lhs_data == rhs_data || memcmp(lhs_data, rhs_data, lhs_size) == 0);
}

// cel_StringView_EqualsIgnoreCase
//
// Tests for equality of two strings, but ignores case. When folding case, only
// ASCII is handled.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_StringView_EqualsIgnoreCase(cel_StringView lhs,
                                                cel_StringView rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int cel_StringView_Compare(cel_StringView lhs,
                                             cel_StringView rhs) {
  const char* const lhs_data = cel_StringView_Data(lhs);
  const size_t lhs_size = cel_StringView_Size(lhs);
  const char* const rhs_data = cel_StringView_Data(rhs);
  const size_t rhs_size = cel_StringView_Size(rhs);
  int cmp = lhs_data != rhs_data
                ? memcmp(lhs_data, rhs_data,
                         lhs_size < rhs_size ? lhs_size : rhs_size)
                : 0;
  if (cmp == 0) {
    cmp = lhs_size < rhs_size ? -1 : lhs_size > rhs_size ? 1 : 0;
  }
  return cmp;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_StringViewTokenizer_Next(
    cel_StringViewTokenizer* cel_nonnull tokenizer,
    cel_StringView* cel_nonnull token);

CEL_END_DECLS

#ifndef __cplusplus
#define cel_StringView_From(x)                      \
  (_Generic((x),                                    \
       cel_CStringView: cel_StringView_FromCString, \
       char*: cel_StringView_FromString,            \
       const char*: cel_StringView_FromString)((x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_StringView cel_StringView_From(cel_CStringView str) {
  return cel_StringView_FromCString(str);
}
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_StringView cel_StringView_From(const char* cel_nullable str) {
  return cel_StringView_FromString(str);
}
#endif

#ifdef __cplusplus

inline bool operator==(cel_StringView lhs, cel_StringView rhs) {
  return cel_StringView_Equals(lhs, rhs);
}

inline bool operator!=(cel_StringView lhs, cel_StringView rhs) {
  return !operator==(lhs, rhs);
}

inline bool operator==(cel_StringView lhs, cel_CStringView rhs) {
  return cel_StringView_Equals(lhs, cel_StringView_FromCString(rhs));
}

inline bool operator==(cel_CStringView lhs, cel_StringView rhs) {
  return cel_StringView_Equals(cel_StringView_FromCString(lhs), rhs);
}

inline bool operator!=(cel_StringView lhs, cel_CStringView rhs) {
  return !operator==(lhs, rhs);
}

inline bool operator!=(cel_CStringView lhs, cel_StringView rhs) {
  return !operator==(lhs, rhs);
}

#endif

#endif  // THIRD_PARTY_CEL_C_STRING_VIEW_H_
