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

// Internal header providing thread-safe reference counting.
//
// NOTE: We do not use <stdatomic.h> due to build issues across multiple
// toolchains that occur when mixing `<atomic>` and `<stdatomic.h>`. Instead we
// use compiler intrinsics across the board.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_ARC_H_
#define THIRD_PARTY_CEL_C_INTERNAL_ARC_H_

#include <limits.h>
#include <stdbool.h>  // IWYU pragma: keep

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedCompareExchange)
#endif

#include "cel-c/assert.h"
#include "cel-c/internal/config.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

#ifdef _MSC_VER
typedef volatile long _cel_AtomicRefCount;
#else
typedef volatile int _cel_AtomicRefCount;
#endif

// _cel_AtomicRefCount_Initialize
//
// Initializes the reference count to `1`.
static CEL_INLINE void _cel_AtomicRefCount_Initialize(
    CEL_NONNULL(_cel_AtomicRefCount*) refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
#ifdef _MSC_VER
  *refcount = 1;
#else
  __atomic_store_n(refcount, 1, __ATOMIC_RELAXED);
#endif
}

// _cel_AtomicRefCount_Increment
//
// Increments the reference count by `1`.
static CEL_INLINE void _cel_AtomicRefCount_Increment(
    CEL_NONNULL(_cel_AtomicRefCount*) refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
#ifdef _MSC_VER
  const long count = _InterlockedIncrement(refcount);
  CEL_ASSERT_GT(count, 1);
#else
  const int count = __atomic_fetch_add(refcount, 1, __ATOMIC_RELAXED);
  CEL_ASSERT_GT(count, 0);
  CEL_ASSERT_LT(count, INT_MAX);
#endif
}

// _cel_AtomicRefCount_Decrement
//
// Decrements the reference count by `1` and returns whether the reference count
// has hit `0`, that is whether it expired.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_AtomicRefCount_Decrement(
    CEL_NONNULL(_cel_AtomicRefCount*) refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
#ifdef _MSC_VER
  const long count = _InterlockedDecrement(refcount);
  CEL_ASSERT_GE(count, 0);
  return CEL_UNLIKELY(count == 0);
#else
  const int count = __atomic_fetch_sub(refcount, 1, __ATOMIC_ACQ_REL);
  CEL_ASSERT_GT(count, 0);
  CEL_ASSERT_LT(count, INT_MAX);
  return CEL_UNLIKELY(count == 1);
#endif
}

// _cel_AtomicRefCount_Unique
//
// Returns whether this reference count is unique, that is the count is `1`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_AtomicRefCount_Unique(
    CEL_NONNULL(const _cel_AtomicRefCount*) refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
#ifdef _MSC_VER
  const long count =
      _InterlockedCompareExchange((CEL_NONNULL(long volatile*))refcount, 1, 1);
#else
  const int count = __atomic_load_n(refcount, __ATOMIC_ACQUIRE);
#endif
  CEL_ASSERT_GE(count, 0);
  CEL_ASSERT_LT(count, INT_MAX);
  return count == 1;
}

// _cel_AtomicRefCount_Expired
//
// Returns whether this reference count is expired, that is the count is `0`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_AtomicRefCount_Expired(
    CEL_NONNULL(const _cel_AtomicRefCount*) refcount) {
  CEL_ASSERT_NOT_NULL(refcount);
#ifdef _MSC_VER
  const long count =
      _InterlockedCompareExchange((CEL_NONNULL(long volatile*))refcount, 1, 1);
#else
  const int count = __atomic_load_n(refcount, __ATOMIC_ACQUIRE);
#endif
  CEL_ASSERT_GE(count, 0);
  CEL_ASSERT_LT(count, INT_MAX);
  return count == 0;
}

CEL_END_DECLS

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_INTERNAL_ARC_H_
