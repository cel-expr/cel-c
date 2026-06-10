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

#ifndef THIRD_PARTY_CEL_C_DURATION_H_
#define THIRD_PARTY_CEL_C_DURATION_H_

#include <stdalign.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"

CEL_BEGIN_DECLS

#ifdef cel_Duration_kMinSeconds
#error cel_Duration_kMinSeconds cannot be directly set
#endif

#ifdef cel_Duration_kMaxSeconds
#error cel_Duration_kMaxSeconds cannot be directly set
#endif

#ifdef cel_Duration_kMinNanos
#error cel_Duration_kMinNanos cannot be directly set
#endif

#ifdef cel_Duration_kMaxNanos
#error cel_Duration_kMaxNanos cannot be directly set
#endif

#ifdef cel_Duration_kZero
#error cel_Duration_kZero cannot be directly set
#endif

#ifdef cel_Duration_kMin
#error cel_Duration_kMin cannot be directly set
#endif

#ifdef cel_Duration_kMax
#error cel_Duration_kMax cannot be directly set
#endif

#define cel_Duration_kMinSeconds INT64_C(-315576000000)
#define cel_Duration_kMaxSeconds INT64_C(315576000000)

#define cel_Duration_kMinNanos INT32_C(-999999999)
#define cel_Duration_kMaxNanos INT32_C(999999999)

#define cel_Duration_kZero ((cel_Duration){INT64_C(0), INT32_C(0)})
#define cel_Duration_kMin \
  ((cel_Duration){cel_Duration_kMinSeconds, cel_Duration_kMinNanos})
#define cel_Duration_kMax \
  ((cel_Duration){cel_Duration_kMaxSeconds, cel_Duration_kMaxNanos})

#ifdef _MSC_VER
#pragma pack(push, 4)
#endif

typedef struct CEL_ATTRIBUTE_PACKED(4) {
  // As this structure is packed, do not access these members directly.
  int64_t sec;
  int32_t nsec;
} cel_Duration;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

CEL_STATIC_ASSERT(sizeof(cel_Duration) == 12);
CEL_STATIC_ASSERT(alignof(cel_Duration) == 4);

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Duration_Valid(int64_t sec, int32_t nsec) {
  return sec >= cel_Duration_kMinSeconds && sec <= cel_Duration_kMaxSeconds &&
         nsec >= cel_Duration_kMinNanos && nsec <= cel_Duration_kMaxNanos &&
         !((sec < 0 && nsec > 0) || (sec > 0 && nsec < 0));
}

// cel_Duration_FromUnix
//
// Returns a cel_Duration using `sec` seconds and `nsec` nanoseconds from `sec`.
//
// Requires that `sec` is in the range [cel_Duration_kMinSeconds,
// cel_Duration_kMaxSeconds] and `nsec` is in the range
// [cel_Duration_kMinNanos, cel_Duration_kMaxNanos].
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Duration cel_Duration_FromUnix(int64_t sec,
                                                     int32_t nsec) {
  CEL_ASSERT(cel_Duration_Valid(sec, nsec));

  cel_Duration d;
  d.sec = sec;
  d.nsec = nsec;
  return d;
}

// cel_Duration_ToUnix
//
// Extracts the seconds and nanoseconds part from cel_Duration and stores them
// in `sec` and `nsec` respectively.
static CEL_INLINE void cel_Duration_ToUnix(cel_Duration d,
                                           int64_t* cel_nonnull sec,
                                           int32_t* cel_nullable nsec) {
  CEL_ASSERT(cel_Duration_Valid(d.sec, d.nsec));

  *sec = d.sec;
  if (nsec != cel_nullptr) {
    *nsec = d.nsec;
  }
}

// cel_Duration_ToUnixSeconds
//
// Returns the seconds part of the cel_Duration.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int64_t cel_Duration_ToUnixSeconds(cel_Duration d) {
  CEL_ASSERT(cel_Duration_Valid(d.sec, d.nsec));
  return d.sec;
}

// cel_Duration_Equals
//
// Tests the two durations for equality.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Duration_Equals(cel_Duration lhs, cel_Duration rhs) {
  CEL_ASSERT(cel_Duration_Valid(lhs.sec, lhs.nsec));
  CEL_ASSERT(cel_Duration_Valid(rhs.sec, rhs.nsec));

  return lhs.sec == rhs.sec && lhs.nsec == rhs.nsec;
}

// cel_Duration_Compare
//
// Compares the two durations.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int cel_Duration_Compare(cel_Duration lhs, cel_Duration rhs) {
  CEL_ASSERT(cel_Duration_Valid(lhs.sec, lhs.nsec));
  CEL_ASSERT(cel_Duration_Valid(rhs.sec, rhs.nsec));

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

// cel_Duration_Add
//
// Adds the duration `rhs` to the duration `lhs`. If the result of the
// calculation is out of the allowed range for durations, `false` is returned
// and `out` is unmodified. Otherwise the result is stored in `out` and `true`
// is returned.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Duration_Add(cel_Duration* cel_nonnull out,
                                 cel_Duration lhs, cel_Duration rhs);

// cel_Duration_Sub
//
// Subtracts the duration `rhs` from the duration `lhs`. If the result of the
// calculation is out of the allowed range for durations, `false` is returned
// and `out` is unmodified. Otherwise the result is stored in `out` and `true`
// is returned.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Duration_Sub(cel_Duration* cel_nonnull out,
                                 cel_Duration lhs, cel_Duration rhs);

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool cel_Duration_Normalize(int64_t* cel_nonnull sec,
                            int32_t* cel_nonnull nsec);

CEL_END_DECLS

#ifdef __cplusplus

inline bool operator==(cel_Duration lhs, cel_Duration rhs) {
  return cel_Duration_Equals(lhs, rhs);
}

inline bool operator!=(cel_Duration lhs, cel_Duration rhs) {
  return !operator==(lhs, rhs);
}

#endif

#endif  // THIRD_PARTY_CEL_C_DURATION_H_
