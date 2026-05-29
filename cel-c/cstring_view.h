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

#ifndef THIRD_PARTY_CEL_C_CSTRING_VIEW_H_
#define THIRD_PARTY_CEL_C_CSTRING_VIEW_H_

#include <limits.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/hash.h"

CEL_BEGIN_DECLS

#ifdef CEL_CSTRINGVIEW_INIT
#error CEL_CSTRINGVIEW_INIT cannot be directly set
#endif

#ifdef CEL_CSTRINGVIEW_C
#error CEL_CSTRINGVIEW_C cannot be directly set
#endif

#ifdef CEL_CSTRINGVIEW_FMT
#error CEL_CSTRINGVIEW_FMT cannot be directly set
#endif

#ifdef CEL_CSTRINGVIEW_ARGS
#error CEL_CSTRINGVIEW_ARGS cannot be directly set
#endif

#define CEL_CSTRINGVIEW_INIT(data) {(data)}

#define CEL_CSTRINGVIEW_C(x) CEL_CSTRINGVIEW_INIT(x)

#define CEL_CSTRINGVIEW_FMT "%s"

#define CEL_CSTRINGVIEW_ARGS(str) cel_CStringView_Data(str)

typedef struct {
  const char* cel_nullability_unknown data;
} cel_CStringView;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_CStringView
cel_CStringView_FromString(const char* cel_nullable str) {
  cel_CStringView result;
  result.data = str;
  return result;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const char* cel_nonnull
cel_CStringView_Data(cel_CStringView str) {
  return str.data != cel_nullptr ? str.data : "";
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t cel_CStringView_Size(cel_CStringView str) {
  return strlen(cel_CStringView_Data(str));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE uint32_t cel_CStringView_Size32(cel_CStringView str) {
  CEL_ASSERT_LE(cel_CStringView_Size(str), UINT32_MAX);

  return CEL_LIKELY(cel_CStringView_Size(str) <= (size_t)UINT32_MAX)
             ? (uint32_t)cel_CStringView_Size(str)
             : UINT32_MAX;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int cel_CStringView_SizeInt(cel_CStringView str) {
  CEL_ASSERT_LE(cel_CStringView_Size(str), (size_t)INT_MAX);

  return CEL_LIKELY(cel_CStringView_Size(str) <= (size_t)INT_MAX)
             ? (int)cel_CStringView_Size(str)
             : INT_MAX;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_CStringView_Empty(cel_CStringView str) {
  return str.data == cel_nullptr || str.data[0] == '\0';
}

static CEL_INLINE void cel_CStringView_RemovePrefix(
    cel_CStringView* cel_nonnull str, size_t n) {
  CEL_ASSERT_NOT_NULL(str);
  CEL_ASSERT_LE(n, cel_CStringView_Size(*str));

  str->data += n;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_CStringView_StartsWith(cel_CStringView lhs,
                                                  cel_CStringView rhs) {
  const char* lhs_data = cel_CStringView_Data(lhs);
  const char* rhs_data = cel_CStringView_Data(rhs);
  while (true) {
    char rhs_chr = *rhs_data++;
    if (rhs_chr == '\0') {
      return true;
    }
    char lhs_chr = *lhs_data++;
    if (lhs_chr == '\0' || lhs_chr != rhs_chr) {
      return false;
    }
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_CStringView_ConsumePrefix(
    cel_CStringView* cel_nonnull lhs, cel_CStringView rhs) {
  CEL_ASSERT_NOT_NULL(lhs);

  const char* lhs_data = cel_CStringView_Data(*lhs);
  const char* rhs_data = cel_CStringView_Data(rhs);
  while (true) {
    char rhs_chr = *rhs_data++;
    if (rhs_chr == '\0') {
      lhs->data = lhs_data;
      return true;
    }
    char lhs_chr = *lhs_data++;
    if (lhs_chr == '\0' || lhs_chr != rhs_chr) {
      return false;
    }
  }
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const char* cel_nullable
cel_CStringView_FindFirst(cel_CStringView haystack, cel_CStringView needle) {
  if (haystack.data == cel_nullptr || needle.data == cel_nullptr) {
    return haystack.data;
  }
  return strstr(haystack.data, needle.data);
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN size_t cel_CStringView_Count(cel_CStringView haystack,
                                        cel_CStringView needle);

typedef struct {
  cel_CStringView subject;
  cel_CStringView delim;
  size_t delim_len;
  bool done;
} cel_CStringViewTokenizer;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_CStringViewTokenizer
cel_CStringView_Tokenize(cel_CStringView subject, cel_CStringView delim) {
  CEL_ASSERT_NOT(cel_CStringView_Empty(delim));

  cel_CStringViewTokenizer tokenizer;
  tokenizer.subject = subject;
  tokenizer.delim = delim;
  tokenizer.delim_len = 0;
  tokenizer.done = false;
  return tokenizer;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_HashState cel_CStringView_Hash(cel_CStringView str,
                                                     cel_HashState state) {
  return cel_HashState_CombineN(state, cel_CStringView_Data(str),
                                cel_CStringView_Size(str));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_CStringView_Equals(cel_CStringView lhs,
                                              cel_CStringView rhs) {
  const char* lhs_data = cel_CStringView_Data(lhs);
  const char* rhs_data = cel_CStringView_Data(rhs);
  return lhs_data == rhs_data || strcmp(lhs_data, rhs_data) == 0;
}

// cel_CStringView_EqualsIgnoreCase
//
// Tests for equality of two strings, but ignores case. When folding case, only
// ASCII is handled.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_CStringView_EqualsIgnoreCase(cel_CStringView lhs,
                                                 cel_CStringView rhs);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int cel_CStringView_Compare(cel_CStringView lhs,
                                              cel_CStringView rhs) {
  const char* lhs_data = cel_CStringView_Data(lhs);
  const char* rhs_data = cel_CStringView_Data(rhs);
  return lhs_data != rhs_data ? strcmp(lhs_data, rhs_data) : 0;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_CStringViewTokenizer_Next(
    cel_CStringViewTokenizer* cel_nonnull tokenizer,
    cel_CStringView* cel_nonnull token, size_t* cel_nonnull token_len);

CEL_END_DECLS

#ifndef __cplusplus
#define cel_CStringView_From(x)           \
  (_Generic((x),                          \
       char*: cel_CStringView_FromString, \
       const char*: cel_CStringView_FromString)((x)))
#else
CEL_ATTRIBUTE_NODISCARD
CEL_INLINE cel_CStringView cel_CStringView_From(const char* cel_nullable str) {
  return cel_CStringView_FromString(str);
}
#endif

#endif  // THIRD_PARTY_CEL_C_CSTRING_VIEW_H_
