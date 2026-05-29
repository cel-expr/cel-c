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

#ifndef THIRD_PARTY_CEL_C_STRING_VIEW_ABSL_H_
#define THIRD_PARTY_CEL_C_STRING_VIEW_ABSL_H_

#include "absl/strings/string_view.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE absl::string_view cel_StringView_ToAbsl(cel_StringView in) {
  return absl::string_view(cel_StringView_Data(in), cel_StringView_Size(in));
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView cel_StringView_FromAbsl(absl::string_view in) {
  return cel_StringView_FromArray(in.data(), in.size());
}

#endif  // THIRD_PARTY_CEL_C_STRING_VIEW_ABSL_H_
