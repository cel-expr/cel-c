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

#ifndef THIRD_PARTY_CEL_C_STRING_POOL_H_
#define THIRD_PARTY_CEL_C_STRING_POOL_H_

#include <stdbool.h>  // IWYU pragma: keep

#ifdef __cplusplus
#include <memory>
#endif

#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"

CEL_BEGIN_DECLS

typedef struct cel_StringPool cel_StringPool;

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN cel_StringPool* cel_nullable
cel_StringPool_New(cel_Arena* cel_nonnull arena);

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN void cel_StringPool_Delete(cel_StringPool* cel_nullable string_pool);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_StringPool_Intern(cel_StringPool* cel_nonnull string_pool,
                                      cel_StringView in,
                                      cel_StringView* cel_nonnull out);

CEL_END_DECLS

#ifdef __cplusplus
struct cel_StringPoolDeleter {
  CEL_INLINE void operator()(cel_StringPool* cel_nullable pool) const noexcept {
    cel_StringPool_Delete(pool);
  }
  CEL_INLINE void operator()(
      const cel_StringPool* cel_nullable pool) const noexcept {
    (*this)(const_cast<cel_StringPool*>(pool));
  }
};
using cel_StringPoolPtr =
    std::unique_ptr<cel_StringPool, cel_StringPoolDeleter>;
#endif

#endif  // THIRD_PARTY_CEL_C_STRING_POOL_H_
