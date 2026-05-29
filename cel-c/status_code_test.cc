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

#include "cel-c/status_code.h"

#include <limits>

#include "gtest/gtest.h"

namespace {

TEST(StatusCode, Name) {
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kOk), "OK");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kCancelled), "CANCELLED");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kUnknown), "UNKNOWN");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kInvalidArgument),
               "INVALID_ARGUMENT");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kDeadlineExceeded),
               "DEADLINE_EXCEEDED");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kNotFound), "NOT_FOUND");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kAlreadyExists),
               "ALREADY_EXISTS");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kPermissionDenied),
               "PERMISSION_DENIED");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kResourceExhausted),
               "RESOURCE_EXHAUSTED");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kFailedPrecondition),
               "FAILED_PRECONDITION");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kAborted), "ABORTED");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kOutOfRange), "OUT_OF_RANGE");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kUnimplemented),
               "UNIMPLEMENTED");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kInternal), "INTERNAL");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kUnavailable), "UNAVAILABLE");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kDataLoss), "DATA_LOSS");
  EXPECT_STREQ(cel_StatusCode_Name(cel_StatusCode_kUnauthenticated),
               "UNAUTHENTICATED");

  EXPECT_STREQ(cel_StatusCode_Name(static_cast<cel_StatusCode>(
                   std::numeric_limits<int>::max())),
               "UNKNOWN");
}

}  // namespace
