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

#include "cel-c/error_absl.h"

#include "absl/status/status.h"
#include "absl/strings/cord.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code_absl.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"

absl::Status cel_Error_ToAbsl(const cel_Error* cel_nonnull in) noexcept {
  CEL_ASSERT_NOT_NULL(in);

  absl::Status out(cel_ErrorCode_ToAbsl(cel_Error_CanonicalCode(in)),
                   cel_StringView_ToAbsl(cel_Error_Message(in)));
  cel_StringView type_url;
  cel_StringView value;
  cel_ErrorPayloadIterator iter = cel_Error_BeginPayloads(in);
  while (cel_Error_NextPayload(in, &type_url, &value, &iter)) {
    out.SetPayload(cel_StringView_ToAbsl(type_url),
                   absl::Cord(cel_StringView_ToAbsl(value)));
  }
  return out;
}
