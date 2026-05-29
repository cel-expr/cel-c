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

#ifndef THIRD_PARTY_CEL_C_TIMESTAMP_ABSL_H_
#define THIRD_PARTY_CEL_C_TIMESTAMP_ABSL_H_

#include <cstdint>

#include "absl/time/time.h"
#include "cel-c/config.h"
#include "cel-c/timestamp.h"

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE absl::Time cel_Timestamp_ToAbsl(cel_Timestamp in) {
  int64_t sec;
  int32_t nsec;
  cel_Timestamp_ToUnix(in, &sec, &nsec);
  return absl::UnixEpoch() + absl::Seconds(sec) + absl::Nanoseconds(nsec);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Timestamp cel_Timestamp_FromAbsl(absl::Time in) {
  int64_t sec;
  int32_t nsec;
  if (in == absl::InfiniteFuture()) {
    sec = cel_Timestamp_kMaxSeconds;
    nsec = cel_Timestamp_kMaxNanos;
  } else if (in == absl::InfinitePast()) {
    sec = cel_Timestamp_kMinSeconds;
    nsec = cel_Timestamp_kMinNanos;
  } else {
    sec = absl::ToUnixSeconds(in);
    nsec = static_cast<int32_t>((in - absl::FromUnixSeconds(sec)) /
                                absl::Nanoseconds(1));
    if (sec < cel_Timestamp_kMinSeconds) {
      sec = cel_Timestamp_kMinSeconds;
      nsec = cel_Timestamp_kMinNanos;
    } else if (sec > cel_Timestamp_kMaxSeconds) {
      sec = cel_Timestamp_kMaxSeconds;
      nsec = cel_Timestamp_kMaxNanos;
    }
  }
  return cel_Timestamp_FromUnix(sec, nsec);
}

#endif  // THIRD_PARTY_CEL_C_TIMESTAMP_ABSL_H_
