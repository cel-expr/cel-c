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

#include "cel-c/internal/timestampconv.h"

#include <cstdint>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "cel-c/arena.h"
#include "cel-c/internal/config.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"
#include "cel-c/timestamp.h"

CEL_BEGIN_DECLS

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Timestamp_ToRFC3339(cel_Timestamp ts, cel_StringView* cel_nonnull out,
                              CEL_NONNULL(cel_Arena*) arena) {
  int64_t sec;
  int32_t nsec;
  cel_Timestamp_ToUnix(ts, &sec, &nsec);
  absl::Time time = absl::FromUnixSeconds(sec) + absl::Nanoseconds(nsec);
  absl::TimeZone utc = absl::UTCTimeZone();

  std::string rfc3339 = absl::FormatTime("%Y-%m-%d%ET%H:%M:%E*SZ", time, utc);

  cel_StringView temp_sv = cel_StringView_FromAbsl(rfc3339);
  if (CEL_UNLIKELY(!cel_Arena_StrDup(arena, out, temp_sv))) {
    return false;
  }
  return true;
}

CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Timestamp_FromRFC3339(cel_Timestamp* cel_nonnull out,
                                cel_StringView ts) {
  absl::string_view ts_str = absl::string_view(ts.data, ts.size);
  absl::Time time;
  std::string err;

  if (!absl::ParseTime(absl::RFC3339_full, ts_str, &time, &err)) {
    return false;
  }

  absl::Duration duration = time - absl::UnixEpoch();
  int64_t seconds = absl::IDivDuration(duration, absl::Seconds(1), &duration);
  int32_t nanos = static_cast<int32_t>(absl::ToInt64Nanoseconds(duration));

  if (!cel_Timestamp_Valid(seconds, nanos)) {
    return false;
  }
  *out = cel_Timestamp_FromUnix(seconds, nanos);
  return true;
}

CEL_END_DECLS
