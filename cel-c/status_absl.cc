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

#include "cel-c/status_absl.h"

#include <string>

#include "absl/status/status.h"
#include "absl/strings/cord.h"
#include "absl/strings/string_view.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/status_code_absl.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"

absl::Status cel_Status_ToAbsl(CEL_NONNULL(const cel_Status*) in) noexcept {
  CEL_ASSERT_NOT_NULL(in);

  if (cel_Status_Ok(in)) {
    return absl::OkStatus();
  }

  absl::Status out(cel_StatusCode_ToAbsl(cel_Status_CanonicalCode(in)),
                   cel_StringView_ToAbsl(cel_Status_Message(in)));
  cel_StringView type_url;
  cel_StringView value;
  cel_StatusPayloadIterator iter = cel_Status_BeginPayloads(in);
  while (cel_Status_NextPayload(in, &type_url, &value, &iter)) {
    out.SetPayload(cel_StringView_ToAbsl(type_url),
                   absl::Cord(cel_StringView_ToAbsl(value)));
  }
  return out;
}

bool cel_Status_FromAbsl(CEL_NONNULL(cel_Status*) out,
                         const absl::Status& in) noexcept {
  CEL_ASSERT_NOT_NULL(out);

  if (in.ok()) {
    cel_Status_Reset(out);
    return true;
  }

  cel_Status_SetCanonicalCode(out, cel_StatusCode_FromAbsl(in.code()));
  if (!cel_Status_SetMessage(out, cel_StringView_FromAbsl(in.message()))) {
    return false;
  }
  std::string value_scratch;
  absl::string_view value_view;
  bool oom = false;
  in.ForEachPayload(
      [&](absl::string_view type_url, const absl::Cord& value) -> void {
        if (oom || type_url.empty()) {
          return;
        }
        if (auto flat = value.TryFlat(); flat) {
          value_view = *flat;
        } else {
          absl::CopyCordToString(value, &value_scratch);
          value_view = absl::string_view(value_scratch);
        }
        if (!cel_Status_SetPayload(out, cel_StringView_FromAbsl(type_url),
                                   cel_StringView_FromAbsl(value_view))) {
          oom = true;
        }
      });
  return !oom;
}
