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

#include "cel-c/status_code_absl.h"

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "cel-c/status_code.h"

namespace {

TEST(StatusCode, FromAbsl) {
  EXPECT_EQ(cel_StatusCode_FromAbsl(absl::StatusCode::kOk), cel_StatusCode_kOk);
  EXPECT_EQ(cel_StatusCode_FromAbsl(absl::StatusCode::kUnknown),
            cel_StatusCode_kUnknown);
}

TEST(StatusCode, ToAbsl) {
  EXPECT_EQ(cel_StatusCode_ToAbsl(cel_StatusCode_kOk), absl::StatusCode::kOk);
  EXPECT_EQ(cel_StatusCode_ToAbsl(cel_StatusCode_kUnknown),
            absl::StatusCode::kUnknown);
}

}  // namespace
