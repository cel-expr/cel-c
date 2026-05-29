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

#ifndef THIRD_PARTY_CEL_C_DURATION_ABSL_H_
#define THIRD_PARTY_CEL_C_DURATION_ABSL_H_

#include <cstdint>

#include "absl/time/time.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE absl::Duration cel_Duration_ToAbsl(cel_Duration in) {
  int64_t sec;
  int32_t nsec;
  cel_Duration_ToUnix(in, &sec, &nsec);
  return absl::Seconds(sec) + absl::Nanoseconds(nsec);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Duration cel_Duration_FromAbsl(absl::Duration in) {
  int64_t sec;
  int32_t nsec;
  if (in == absl::InfiniteDuration()) {
    sec = cel_Duration_kMaxSeconds;
    nsec = cel_Duration_kMaxNanos;
  } else if (in == -absl::InfiniteDuration()) {
    sec = cel_Duration_kMinSeconds;
    nsec = cel_Duration_kMinNanos;
  } else {
    sec = absl::IDivDuration(in, absl::Seconds(1), &in);
    nsec =
        static_cast<int32_t>(absl::IDivDuration(in, absl::Nanoseconds(1), &in));
    if (sec < cel_Duration_kMinSeconds) {
      sec = cel_Duration_kMinSeconds;
      nsec = cel_Duration_kMinNanos;
    } else if (sec > cel_Duration_kMaxSeconds) {
      sec = cel_Duration_kMaxSeconds;
      nsec = cel_Duration_kMaxNanos;
    }
  }
  return cel_Duration_FromUnix(sec, nsec);
}

#endif  // THIRD_PARTY_CEL_C_DURATION_ABSL_H_
