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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_STRING_POOL_H_
#define THIRD_PARTY_CEL_C_INTERNAL_STRING_POOL_H_

#include <cstddef>
#include <cstring>

#include "absl/hash/hash.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/flat_hash_set.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"

class cel_StringPool {
 public:
  explicit cel_StringPool(cel_Arena* cel_nonnull arena) : arena_(arena) {
    _cel_FlatHashSet_Construct(&strings_, &Hasher, &Equaler);
  }

  ~cel_StringPool() {
    _cel_FlatHashSet_Destruct(&strings_, cel_DefaultAllocator);
  }

  cel_StringPool(const cel_StringPool&) = delete;
  cel_StringPool(cel_StringPool&&) = delete;
  cel_StringPool& operator=(const cel_StringPool&) = delete;
  cel_StringPool& operator=(cel_StringPool&&) = delete;

  [[nodiscard]]
  bool Intern(cel_StringView in, cel_StringView* cel_nonnull out) {
    CEL_ASSERT_NOT_NULL(out);
    if (cel_StringView_Empty(in)) {
      *out = cel_StringView_From("");
      return true;
    }
    _cel_FlatHashSetMutableNode(strings_) ent = nullptr;
    if (_cel_FlatHashSet_Insert(&strings_, cel_DefaultAllocator, &in, &ent)) {
      const size_t len = cel_StringView_Size(in);
      *ent = in;
      char* str =
          reinterpret_cast<char*>(cel_Arena_Malloc(arena_, len, nullptr));
      if (CEL_UNLIKELY(str == nullptr)) {
        _cel_FlatHashSet_Erase(&strings_, ent);
        return false;
      }
      memcpy(str, cel_StringView_Data(in), len);
      *out = *ent = cel_StringView_FromArray(str, len);
    } else {
      if (CEL_UNLIKELY(ent == nullptr)) {
        return false;
      }
      *out = *ent;
    }
    return true;
  }

 private:
  static size_t Hasher(const cel_StringView* str) {
    return absl::HashOf(cel_StringView_ToAbsl(*str));
  }

  static bool Equaler(const cel_StringView* lhs, const cel_StringView* rhs) {
    return cel_StringView_Equals(*lhs, *rhs);
  }

  cel_Arena* cel_nonnull const arena_;
  _cel_FlatHashSet(cel_StringView) strings_;
};

#endif  // THIRD_PARTY_CEL_C_INTERNAL_STRING_POOL_H_
