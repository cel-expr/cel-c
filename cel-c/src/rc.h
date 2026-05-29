// Copyright 2024 Google LLC
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

// Internal header providing thread-compatible reference counting.

#ifndef THIRD_PARTY_CEL_C_SRC_RC_H_
#define THIRD_PARTY_CEL_C_SRC_RC_H_

#include <limits.h>
#include <stdbool.h>  // IWYU pragma: keep

#include "cel-c/assert.h"
#include "cel-c/config.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

typedef int _cel_RefCount;

// _cel_RefCount_Initialize
//
// Initializes the reference count to `1`.
static CEL_INLINE void _cel_RefCount_Initialize(CEL_NONNULL(_cel_RefCount*)
                                                    refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
  *refcount = 1;
}

// _cel_RefCount_Increment
//
// Increments the reference count by `1`.
static CEL_INLINE void _cel_RefCount_Increment(CEL_NONNULL(_cel_RefCount*)
                                                   refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
  const int count = *refcount;
  ++(*refcount);
  CEL_ASSERT_GT(count, 0);
  CEL_ASSERT_LT(count, INT_MAX);
}

// _cel_RefCount_Decrement
//
// Decrements the reference count by `1` and returns whether the reference count
// has hit `0`, that is whether it expired.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_RefCount_Decrement(CEL_NONNULL(_cel_RefCount*)
                                                   refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
  const int count = *refcount;
  --(*refcount);
  CEL_ASSERT_GT(count, 0);
  CEL_ASSERT_LT(count, INT_MAX);
  return CEL_UNLIKELY(count == 1);
}

// _cel_RefCount_Unique
//
// Returns whether this reference count is unique, that is the count is `1`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_RefCount_Unique(CEL_NONNULL(const _cel_RefCount*)
                                                refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
  const int count = *refcount;
  CEL_ASSERT_GE(count, 0);
  CEL_ASSERT_LT(count, INT_MAX);
  return count == 1;
}

// _cel_RefCount_Expired
//
// Returns whether this reference count is expired, that is the count is `0`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_RefCount_Expired(CEL_NONNULL(const _cel_RefCount*)
                                                 refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
  const int count = *refcount;
  CEL_ASSERT_GE(count, 0);
  CEL_ASSERT_LT(count, INT_MAX);
  return count == 0;
}

CEL_END_DECLS

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_SRC_RC_H_
