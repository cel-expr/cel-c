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

#ifndef THIRD_PARTY_CEL_C_TIMESTAMP_H_
#define THIRD_PARTY_CEL_C_TIMESTAMP_H_

#include <stdalign.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"

CEL_BEGIN_DECLS

#ifdef cel_Timestamp_kMinSeconds
#error cel_Timestamp_kMinSeconds cannot be directly set
#endif

#ifdef cel_Timestamp_kMaxSeconds
#error cel_Timestamp_kMaxSeconds cannot be directly set
#endif

#ifdef cel_Timestamp_kMinNanos
#error cel_Timestamp_kMinNanos cannot be directly set
#endif

#ifdef cel_Timestamp_kMaxNanos
#error cel_Timestamp_kMaxNanos cannot be directly set
#endif

#ifdef cel_Timestamp_kUnixEpoch
#error cel_Timestamp_kUnixEpoch cannot be directly set
#endif

#ifdef cel_Timestamp_kMin
#error cel_Timestamp_kMin cannot be directly set
#endif

#ifdef cel_Timestamp_kMax
#error cel_Timestamp_kMax cannot be directly set
#endif

#define cel_Timestamp_kMinSeconds INT64_C(-62135596800)
#define cel_Timestamp_kMaxSeconds INT64_C(253402300799)

#define cel_Timestamp_kMinNanos INT32_C(0)
#define cel_Timestamp_kMaxNanos INT32_C(999999999)

#define cel_Timestamp_kUnixEpoch ((cel_Timestamp){INT64_C(0), INT32_C(0)})
#define cel_Timestamp_kMin \
  ((cel_Timestamp){cel_Timestamp_kMinSeconds, cel_Timestamp_kMinNanos})
#define cel_Timestamp_kMax \
  ((cel_Timestamp){cel_Timestamp_kMaxSeconds, cel_Timestamp_kMaxNanos})

#ifdef _MSC_VER
#pragma pack(push, 4)
#endif

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  // As this structure is packed, do not access these members directly.
  int64_t sec;
  int32_t nsec;
} cel_Timestamp;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

CEL_STATIC_ASSERT(sizeof(cel_Timestamp) == 12);
CEL_STATIC_ASSERT(alignof(cel_Timestamp) == 4);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Timestamp_Valid(int64_t sec, int32_t nsec) {
  return sec >= cel_Timestamp_kMinSeconds && sec <= cel_Timestamp_kMaxSeconds &&
         nsec >= cel_Timestamp_kMinNanos && nsec <= cel_Timestamp_kMaxNanos;
}

// cel_Timestamp_FromUnix
//
// Returns a cel_Timestamp using `sec` seconds from the Unix epoch and `nsec`
// nanoseconds from `sec`.
//
// Requires that `sec` is in the range [cel_Timestamp_kMinSeconds,
// cel_Timestamp_kMaxSeconds] and `nsec` is in the range
// [cel_Timestamp_kMinNanos, cel_Timestamp_kMaxNanos].
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Timestamp cel_Timestamp_FromUnix(int64_t sec,
                                                       int32_t nsec) {
  CEL_ASSERT(cel_Timestamp_Valid(sec, nsec));

  cel_Timestamp t;
  t.sec = sec;
  t.nsec = nsec;
  return t;
}

// cel_Timestamp_ToUnix
//
// Extracts the seconds and nanoseconds part from cel_Timestamp and stores them
// in `sec` and `nsec` respectively.
static CEL_INLINE void cel_Timestamp_ToUnix(cel_Timestamp t,
                                            int64_t* cel_nonnull sec,
                                            int32_t* cel_nullable nsec) {
  CEL_ASSERT(cel_Timestamp_Valid(t.sec, t.nsec));

  *sec = t.sec;
  if (nsec != cel_nullptr) {
    *nsec = t.nsec;
  }
}

// cel_Timestamp_ToUnixSeconds
//
// Returns the seconds part of the cel_Timestamp.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int64_t cel_Timestamp_ToUnixSeconds(cel_Timestamp t) {
  CEL_ASSERT(cel_Timestamp_Valid(t.sec, t.nsec));
  return t.sec;
}

// cel_Timestamp_Equals
//
// Tests the two timestamps for equality.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Timestamp_Equals(cel_Timestamp lhs,
                                            cel_Timestamp rhs) {
  CEL_ASSERT(cel_Timestamp_Valid(lhs.sec, lhs.nsec));
  CEL_ASSERT(cel_Timestamp_Valid(rhs.sec, rhs.nsec));

  return lhs.sec == rhs.sec && lhs.nsec == rhs.nsec;
}

// cel_Timestamp_Compare
//
// Compares the two timestamps.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int cel_Timestamp_Compare(cel_Timestamp lhs,
                                            cel_Timestamp rhs) {
  CEL_ASSERT(cel_Timestamp_Valid(lhs.sec, lhs.nsec));
  CEL_ASSERT(cel_Timestamp_Valid(rhs.sec, rhs.nsec));

  if (lhs.sec < rhs.sec) {
    return -1;
  }
  if (lhs.sec > rhs.sec) {
    return 1;
  }
  if (lhs.nsec < rhs.nsec) {
    return -1;
  }
  if (lhs.nsec > rhs.nsec) {
    return 1;
  }
  return 0;
}

// cel_Timestamp_Add
//
// Adds the duration `rhs` to the timestamp `lhs`. If the result of the
// calculation is out of the allowed range for timestamps, `false` is returned
// and `out` is unmodified. Otherwise the result is stored in `out` and `true`
// is returned.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Timestamp_Add(cel_Timestamp* cel_nonnull out,
                                  cel_Timestamp lhs, cel_Duration rhs);

// cel_Timestamp_Sub
//
// Subtracts the duration `rhs` from the timestamp `lhs`. If the result of the
// calculation is out of the allowed range for timestamps, `false` is returned
// and `out` is unmodified. Otherwise the result is stored in `out` and `true`
// is returned.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Timestamp_Sub(cel_Timestamp* cel_nonnull out,
                                  cel_Timestamp lhs, cel_Duration rhs);

// cel_Timestamp_Diff
//
// Subtracts the timestamp `rhs` from the timestamp `lhs` and returns it as a
// duration. If the result of the calculation is out of the allowed range for
// durations, `false` is returned and `out` is unmodified. Otherwise the result
// is stored in `out` and `true` is returned.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Timestamp_Diff(cel_Duration* cel_nonnull out,
                                   cel_Timestamp lhs, cel_Timestamp rhs);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool cel_Timestamp_Normalize(int64_t* cel_nonnull sec,
                             int32_t* cel_nonnull nsec);

CEL_END_DECLS

#ifdef __cplusplus

inline bool operator==(cel_Timestamp lhs, cel_Timestamp rhs) {
  return cel_Timestamp_Equals(lhs, rhs);
}

inline bool operator!=(cel_Timestamp lhs, cel_Timestamp rhs) {
  return !operator==(lhs, rhs);
}

#endif

#endif  // THIRD_PARTY_CEL_C_TIMESTAMP_H_
