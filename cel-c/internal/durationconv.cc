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

#include "cel-c/internal/durationconv.h"

#include <cstdint>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

namespace {
constexpr int kNanosPerMillisecond = 1000000;
constexpr int kNanosPerMicrosecond = 1000;

std::string FormatNanos(int32_t nanos) {
  if (nanos % kNanosPerMillisecond == 0) {
    return absl::StrFormat("%03d", nanos / kNanosPerMillisecond);
  } else if (nanos % kNanosPerMicrosecond == 0) {
    return absl::StrFormat("%06d", nanos / kNanosPerMicrosecond);
  } else {
    return absl::StrFormat("%09d", nanos);
  }
}
}  // namespace

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Duration_ToStringView(cel_Duration d, cel_Arena* cel_nonnull arena,
                                cel_StringView* cel_nonnull out) {
  int64_t sec;
  int32_t nsec;
  cel_Duration_ToUnix(d, &sec, &nsec);
  CEL_ASSERT(cel_Duration_Valid(sec, nsec));
  std::string result;
  if (sec < 0 || nsec < 0) {
    result = "-";
    sec = -sec;
    nsec = -nsec;
  }
  absl::StrAppend(&result, sec);
  if (nsec != 0) {
    absl::StrAppend(&result, ".", FormatNanos(nsec));
  }
  absl::StrAppend(&result, "s");
  cel_StringView temp_sv = cel_StringView_FromAbsl(result);
  if (CEL_UNLIKELY(!cel_Arena_StrDup(arena, out, temp_sv))) {
    return false;
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Duration_FromStringView(cel_StringView in,
                                  cel_Duration* cel_nonnull out) {
  absl::string_view in_sv = absl::string_view(in.data, in.size);
  absl::Duration duration;
  if (!absl::ParseDuration(in_sv, &duration)) {
    return false;
  }

  int64_t sec = static_cast<int64_t>(
      absl::IDivDuration(duration, absl::Seconds(1), &duration));
  int32_t nsec = static_cast<int32_t>(
      absl::IDivDuration(duration, absl::Nanoseconds(1), &duration));

  if (!cel_Duration_Valid(sec, nsec)) {
    return false;
  }

  *out = cel_Duration_FromUnix(sec, nsec);
  return true;
}

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)
