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

#ifndef THIRD_PARTY_CEL_C_TRILEAN_H_
#define THIRD_PARTY_CEL_C_TRILEAN_H_

#include <stdbool.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"

CEL_BEGIN_DECLS

typedef enum CEL_ATTRIBUTE_CLOSED_ENUM {
  cel_Trilean_kFalse = 0,
  cel_Trilean_kTrue = 1,
  cel_Trilean_kError = 2,

  cel_Trilean_kUnknown = cel_Trilean_kError,
} cel_Trilean;

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Trilean cel_Trilean_FromBool(bool value) {
  return value ? cel_Trilean_kTrue : cel_Trilean_kFalse;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool cel_Trilean_ToBool(cel_Trilean trilean) {
  CEL_ASSERT_NE(trilean, cel_Trilean_kUnknown);

  switch (trilean) {
    case cel_Trilean_kFalse:
      return false;
    case cel_Trilean_kTrue:
      return true;
    default:
      CEL_UNREACHABLE();
  }
}

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_TRILEAN_H_
