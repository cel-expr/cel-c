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

#ifndef THIRD_PARTY_CEL_C_STATUS_CODE_ABSL_H_
#define THIRD_PARTY_CEL_C_STATUS_CODE_ABSL_H_

#include "absl/status/status.h"
#include "cel-c/config.h"
#include "cel-c/status_code.h"

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE absl::StatusCode cel_StatusCode_ToAbsl(cel_StatusCode in) {
  return static_cast<absl::StatusCode>(in);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StatusCode cel_StatusCode_FromAbsl(absl::StatusCode in) {
  return static_cast<cel_StatusCode>(in);
}

#endif  // THIRD_PARTY_CEL_C_STATUS_CODE_ABSL_H_
