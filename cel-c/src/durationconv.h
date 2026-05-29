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

#ifndef THIRD_PARTY_CEL_C_SRC_DURATIONCONV_H_
#define THIRD_PARTY_CEL_C_SRC_DURATIONCONV_H_

#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/string_view.h"

// NOLINTBEGIN(runtime/int)
// NOLINTBEGIN(google-runtime-int)

CEL_BEGIN_DECLS

// _cel_Duration_ToStringView
//
// Converts a duration value to seconds and fractional seconds with an 's'
// suffix.
// for example if sec = 60 and nsec = 100000000, then the string is 60.001s
// The memory for the string is allocated from the arena.
// Returns true on success, with result in `out`, and false otherwise.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool _cel_Duration_ToStringView(cel_Duration d,
                                           cel_Arena* cel_nonnull arena,
                                           cel_StringView* cel_nonnull out);

// _cel_Duration_FromStringView
//
// Converts a string value to a cel_Duration. The format is a sequence of
// decimal numbers, each with an optional fractional part and a unit suffix.
// The supported suffixes are:
// "ns" (nanoseconds)
// "us" (microseconds)
// "ms" (milliseconds)
// "s" (seconds)
// "m" (minutes)
// "h" (hours)
//
// Examples of valid duration strings:
// "1h30m" (1 hour and 30 minutes)
// "0" (zero duration)
// "-1.5h" (minus 90 minutes)
// "300ms"
//
// Durations greater than the hour granularity, such as days or weeks, are
// not supported. Returns true on success, with the result stored in `out`,
// and false otherwise.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool _cel_Duration_FromStringView(cel_StringView in,
                                             cel_Duration* cel_nonnull out);

CEL_END_DECLS

// NOLINTEND(google-runtime-int)
// NOLINTEND(runtime/int)

#endif  // THIRD_PARTY_CEL_C_SRC_DURATIONCONV_H_
