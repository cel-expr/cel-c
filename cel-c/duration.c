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

#include "cel-c/duration.h"

#include <stdbool.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/ckdint.h"

bool cel_Duration_Normalize(int64_t* cel_nonnull sec,
                             int32_t* cel_nonnull nsec) {
  int32_t rem;

  // Normalize nsec.
  rem = *nsec / INT32_C(1000000000);
  if (rem != 0) {
    if (CEL_UNLIKELY(_cel_ckd_add(sec, *sec, (int64_t)rem))) {
      return false;
    }
    *nsec %= INT32_C(1000000000);
  }

  // Normalize sign.
  if (*sec < 0 && *nsec > 0) {
    if (CEL_UNLIKELY(_cel_ckd_add(sec, *sec, INT64_C(1)))) {
      return false;
    }
    *nsec -= INT32_C(1000000000);
  } else if (*sec > 0 && *nsec < 0) {
    if (CEL_UNLIKELY(_cel_ckd_sub(sec, *sec, INT64_C(1)))) {
      return false;
    }
    *nsec += INT32_C(1000000000);
  }

  return cel_Duration_Valid(*sec, *nsec);
}

bool cel_Duration_Add(cel_Duration* cel_nonnull out, cel_Duration lhs,
                      cel_Duration rhs) {
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT(cel_Duration_Valid(lhs.sec, lhs.nsec));
  CEL_ASSERT(cel_Duration_Valid(rhs.sec, rhs.nsec));

  int64_t lhs_sec = lhs.sec;
  int64_t rhs_sec = rhs.sec;
  int64_t sec;
  int32_t nsec;
  int32_t lhs_nsec = lhs.nsec;
  int32_t rhs_nsec = rhs.nsec;

  if (CEL_UNLIKELY(_cel_ckd_add(&sec, lhs_sec, rhs_sec))) {
    return false;
  }

  if (CEL_UNLIKELY(_cel_ckd_add(&nsec, lhs_nsec, rhs_nsec))) {
    return false;
  }

  if (CEL_UNLIKELY(!cel_Duration_Normalize(&sec, &nsec))) {
    return false;
  }

  *out = cel_Duration_FromUnix(sec, nsec);
  return true;
}

bool cel_Duration_Sub(cel_Duration* cel_nonnull out, cel_Duration lhs,
                      cel_Duration rhs) {
  CEL_ASSERT_NOT_NULL(out);
  CEL_ASSERT(cel_Duration_Valid(lhs.sec, lhs.nsec));
  CEL_ASSERT(cel_Duration_Valid(rhs.sec, rhs.nsec));

  int64_t lhs_sec = lhs.sec;
  int64_t rhs_sec = rhs.sec;
  int64_t sec;
  int32_t nsec;
  int32_t lhs_nsec = lhs.nsec;
  int32_t rhs_nsec = rhs.nsec;

  if (CEL_UNLIKELY(_cel_ckd_sub(&sec, lhs_sec, rhs_sec))) {
    return false;
  }

  if (CEL_UNLIKELY(_cel_ckd_sub(&nsec, lhs_nsec, rhs_nsec))) {
    return false;
  }

  if (CEL_UNLIKELY(!cel_Duration_Normalize(&sec, &nsec))) {
    return false;
  }

  *out = cel_Duration_FromUnix(sec, nsec);
  return true;
}
