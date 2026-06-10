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

#include "cel-c/cstring_view.h"

#include <stddef.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"

extern "C" size_t cel_CStringView_Count(cel_CStringView haystack,
                                        cel_CStringView needle) {
  CEL_ASSERT_NOT(cel_CStringView_Empty(needle));

  size_t count = 0;
  const char* pos;
  size_t needle_len = 0;
  while ((pos = cel_CStringView_FindFirst(haystack, needle)) != cel_nullptr) {
    if (needle_len == 0) {
      needle_len = cel_CStringView_Size(needle);
    }
    cel_CStringView_RemovePrefix(
        &haystack, (pos - cel_CStringView_Data(haystack)) + needle_len);
    ++count;
  }
  return count;
}

extern "C" bool cel_CStringViewTokenizer_Next(
    cel_CStringViewTokenizer* cel_nonnull tokenizer,
    cel_CStringView* cel_nonnull token, size_t* cel_nonnull token_len) {
  CEL_ASSERT_NOT_NULL(tokenizer);
  CEL_ASSERT_NOT_NULL(token);
  CEL_ASSERT_NOT_NULL(token_len);

  if (tokenizer->done) {
    return false;
  }
  const char* next =
      cel_CStringView_FindFirst(tokenizer->subject, tokenizer->delim);
  if (next == cel_nullptr) {
    *token = tokenizer->subject;
    *token_len = cel_CStringView_Size(tokenizer->subject);
    tokenizer->done = true;
    return true;
  }
  if (tokenizer->delim_len == 0) {
    tokenizer->delim_len = cel_CStringView_Size(tokenizer->delim);
  }
  const char* data = cel_CStringView_Data(tokenizer->subject);
  *token = cel_CStringView_FromString(data);
  *token_len = next - data;
  cel_CStringView_RemovePrefix(&tokenizer->subject,
                               (next - data) + tokenizer->delim_len);
  return true;
}

extern "C" bool cel_CStringView_EqualsIgnoreCase(cel_CStringView lhs,
                                                 cel_CStringView rhs) {
  const char* lhs_data = cel_CStringView_Data(lhs);
  const char* rhs_data = cel_CStringView_Data(rhs);
  if (lhs_data == rhs_data) {
    return true;
  }
  while (true) {
    uint8_t lhs_c = (uint8_t)(*lhs_data++);
    uint8_t rhs_c = (uint8_t)(*rhs_data++);
    if (lhs_c == '\0' || rhs_c == '\0') {
      return lhs_c == rhs_c;
    }
    if (lhs_c != rhs_c) {
      lhs_c = lhs_c >= 'A' && lhs_c <= 'Z' ? lhs_c - 'A' + 'a' : lhs_c;
      rhs_c = rhs_c >= 'A' && rhs_c <= 'Z' ? rhs_c - 'A' + 'a' : rhs_c;
      if (lhs_c != rhs_c) {
        return false;
      }
    }
  }
}
