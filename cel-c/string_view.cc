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

#include "cel-c/string_view.h"

#include <stddef.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/memory.h"

extern "C" const char* cel_nullable
cel_StringView_FindFirst(cel_StringView haystack, cel_StringView needle) {
  return reinterpret_cast<const char*>(_cel_Memory_FindFirst(
      cel_StringView_Data(haystack), cel_StringView_Size(haystack),
      cel_StringView_Data(needle), cel_StringView_Size(needle)));
}

extern "C" const char* cel_nullable
cel_StringView_FindLast(cel_StringView haystack, cel_StringView needle) {
  return reinterpret_cast<const char*>(_cel_Memory_FindLast(
      cel_StringView_Data(haystack), cel_StringView_Size(haystack),
      cel_StringView_Data(needle), cel_StringView_Size(needle)));
}

extern "C" size_t cel_StringView_Count(cel_StringView haystack,
                                       cel_StringView needle) {
  CEL_ASSERT_NOT(cel_StringView_Empty(needle));

  size_t count = 0;
  const char* pos;
  while ((pos = cel_StringView_FindFirst(haystack, needle)) != cel_nullptr) {
    cel_StringView_RemovePrefix(
        &haystack,
        (pos - cel_StringView_Data(haystack)) + cel_StringView_Size(needle));
    ++count;
  }
  return count;
}

extern "C" bool cel_StringViewTokenizer_Next(
    cel_StringViewTokenizer* cel_nonnull tokenizer,
    cel_StringView* cel_nonnull token) {
  CEL_ASSERT_NOT_NULL(tokenizer);
  CEL_ASSERT_NOT_NULL(token);

  if (tokenizer->done) {
    return false;
  }
  const char* next =
      cel_StringView_FindFirst(tokenizer->subject, tokenizer->delim);
  if (next == cel_nullptr) {
    *token = tokenizer->subject;
    tokenizer->done = true;
    return true;
  }
  const char* data = cel_StringView_Data(tokenizer->subject);
  *token = cel_StringView_FromArray(data, next - data);
  cel_StringView_RemovePrefix(
      &tokenizer->subject,
      (next - data) + cel_StringView_Size(tokenizer->delim));
  return true;
}

extern "C" bool cel_StringView_EqualsIgnoreCase(cel_StringView lhs,
                                                cel_StringView rhs) {
  const char* const lhs_data = cel_StringView_Data(lhs);
  const size_t lhs_size = cel_StringView_Size(lhs);
  const char* const rhs_data = cel_StringView_Data(rhs);
  if (lhs_size != cel_StringView_Size(rhs)) {
    return false;
  }
  if (lhs_data == rhs_data) {
    return true;
  }
  for (size_t i = 0; i < lhs_size; ++i) {
    uint8_t lhs_c = (uint8_t)lhs_data[i];
    uint8_t rhs_c = (uint8_t)rhs_data[i];
    if (lhs_c != rhs_c) {
      lhs_c = lhs_c >= 'A' && lhs_c <= 'Z' ? lhs_c - 'A' + 'a' : lhs_c;
      rhs_c = rhs_c >= 'A' && rhs_c <= 'Z' ? rhs_c - 'A' + 'a' : rhs_c;
      if (lhs_c != rhs_c) {
        return false;
      }
    }
  }
  return true;
}
