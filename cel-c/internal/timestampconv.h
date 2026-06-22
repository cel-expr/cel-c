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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_TIMESTAMPCONV_H_
#define THIRD_PARTY_CEL_C_INTERNAL_TIMESTAMPCONV_H_

#include <stddef.h>

#include "cel-c/arena.h"
#include "cel-c/internal/config.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"

CEL_BEGIN_DECLS

// _cel_Timestamp_ToRFC3339
//
// Formats a `cel_Timestamp` to an RFC3339 string. Returns true on success,
// with the result stored in `out`, and false otherwise.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Timestamp_ToRFC3339(cel_Timestamp ts, cel_StringView* cel_nonnull out,
                              CEL_NONNULL(cel_Arena*) arena);

// _cel_Timestamp_FromRFC3339
//
// Parses a `cel_Timestamp` from an RFC3339 string. Returns true on success,
// otherwise returns false and `out` is unmodified.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_Timestamp_FromRFC3339(cel_Timestamp* cel_nonnull out,
                                cel_StringView ts);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_TIMESTAMPCONV_H_
