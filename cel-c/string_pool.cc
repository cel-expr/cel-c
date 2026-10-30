// Copyright 2026 Google LLC
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

#include "cel-c/string_pool.h"

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/malloc.h"
#include "cel-c/internal/string_pool.h"
#include "cel-c/string_view.h"

extern "C" cel_StringPool* cel_nullable
cel_StringPool_New(cel_Arena* cel_nonnull arena) {
  CEL_ASSERT_NOT_NULL(arena);
  cel_StringPool* string_pool = reinterpret_cast<cel_StringPool*>(
      _cel_Malloc(sizeof(cel_StringPool), nullptr));
  if (CEL_LIKELY(string_pool != nullptr)) {
    ::new (static_cast<void*>(string_pool)) cel_StringPool(arena);
  }
  return string_pool;
}

extern "C" void cel_StringPool_Delete(
    cel_StringPool* cel_nullable string_pool) {
  if (string_pool == nullptr) {
    return;
  }
  string_pool->~cel_StringPool();
  _cel_FreeSized(string_pool, sizeof(*string_pool));
}

extern "C" bool cel_StringPool_Intern(cel_StringPool* cel_nonnull string_pool,
                                      cel_StringView in,
                                      cel_StringView* cel_nonnull out) {
  CEL_ASSERT_NOT_NULL(string_pool);
  CEL_ASSERT_NOT_NULL(out);
  return string_pool->Intern(in, out);
}
