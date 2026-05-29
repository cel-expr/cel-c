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

#ifndef THIRD_PARTY_CEL_C_STATUS_ABSL_H_
#define THIRD_PARTY_CEL_C_STATUS_ABSL_H_

#include <stdbool.h>  // IWYU pragma: keep

#include "absl/status/status.h"
#include "cel-c/config.h"
#include "cel-c/status.h"

CEL_EXTERN absl::Status cel_Status_ToAbsl(CEL_NONNULL(const cel_Status*)
                                              in) noexcept;

CEL_ATTRIBUTE_NOTHROW
CEL_EXTERN bool cel_Status_FromAbsl(CEL_NONNULL(cel_Status*) out,
                                    const absl::Status& in) noexcept;

#endif  // THIRD_PARTY_CEL_C_STATUS_ABSL_H_
