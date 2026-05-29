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

#include "cel-c/error_code_proto.h"

#include "google/rpc/code.upb.h"
#include "gtest/gtest.h"
#include "cel-c/error_code.h"

namespace {

TEST(ErrorCode, ToProto) {
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kCancelled),
            google_rpc_CANCELLED);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kUnknown), google_rpc_UNKNOWN);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kInvalidArgument),
            google_rpc_INVALID_ARGUMENT);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kDeadlineExceeded),
            google_rpc_DEADLINE_EXCEEDED);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kNotFound),
            google_rpc_NOT_FOUND);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kAlreadyExists),
            google_rpc_ALREADY_EXISTS);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kPermissionDenied),
            google_rpc_PERMISSION_DENIED);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kUnauthenticated),
            google_rpc_UNAUTHENTICATED);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kResourceExhausted),
            google_rpc_RESOURCE_EXHAUSTED);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kFailedPrecondition),
            google_rpc_FAILED_PRECONDITION);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kAborted), google_rpc_ABORTED);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kOutOfRange),
            google_rpc_OUT_OF_RANGE);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kUnimplemented),
            google_rpc_UNIMPLEMENTED);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kInternal),
            google_rpc_INTERNAL);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kUnavailable),
            google_rpc_UNAVAILABLE);
  EXPECT_EQ(cel_ErrorCode_ToProto(cel_ErrorCode_kDataLoss),
            google_rpc_DATA_LOSS);
}

}  // namespace
