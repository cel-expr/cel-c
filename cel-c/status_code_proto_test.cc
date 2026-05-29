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

#include "cel-c/status_code_proto.h"

#include "google/rpc/code.upb.h"
#include "gtest/gtest.h"
#include "cel-c/status_code.h"

namespace {

TEST(StatusCode, FromProto) {
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_OK), cel_StatusCode_kOk);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_CANCELLED),
            cel_StatusCode_kCancelled);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_UNKNOWN),
            cel_StatusCode_kUnknown);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_INVALID_ARGUMENT),
            cel_StatusCode_kInvalidArgument);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_DEADLINE_EXCEEDED),
            cel_StatusCode_kDeadlineExceeded);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_NOT_FOUND),
            cel_StatusCode_kNotFound);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_ALREADY_EXISTS),
            cel_StatusCode_kAlreadyExists);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_PERMISSION_DENIED),
            cel_StatusCode_kPermissionDenied);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_UNAUTHENTICATED),
            cel_StatusCode_kUnauthenticated);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_RESOURCE_EXHAUSTED),
            cel_StatusCode_kResourceExhausted);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_FAILED_PRECONDITION),
            cel_StatusCode_kFailedPrecondition);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_ABORTED),
            cel_StatusCode_kAborted);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_OUT_OF_RANGE),
            cel_StatusCode_kOutOfRange);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_UNIMPLEMENTED),
            cel_StatusCode_kUnimplemented);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_INTERNAL),
            cel_StatusCode_kInternal);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_UNAVAILABLE),
            cel_StatusCode_kUnavailable);
  EXPECT_EQ(cel_StatusCode_FromProto(google_rpc_DATA_LOSS),
            cel_StatusCode_kDataLoss);
}

TEST(StatusCode, ToProto) {
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kOk), google_rpc_OK);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kCancelled),
            google_rpc_CANCELLED);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kUnknown),
            google_rpc_UNKNOWN);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kInvalidArgument),
            google_rpc_INVALID_ARGUMENT);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kDeadlineExceeded),
            google_rpc_DEADLINE_EXCEEDED);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kNotFound),
            google_rpc_NOT_FOUND);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kAlreadyExists),
            google_rpc_ALREADY_EXISTS);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kPermissionDenied),
            google_rpc_PERMISSION_DENIED);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kUnauthenticated),
            google_rpc_UNAUTHENTICATED);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kResourceExhausted),
            google_rpc_RESOURCE_EXHAUSTED);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kFailedPrecondition),
            google_rpc_FAILED_PRECONDITION);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kAborted),
            google_rpc_ABORTED);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kOutOfRange),
            google_rpc_OUT_OF_RANGE);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kUnimplemented),
            google_rpc_UNIMPLEMENTED);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kInternal),
            google_rpc_INTERNAL);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kUnavailable),
            google_rpc_UNAVAILABLE);
  EXPECT_EQ(cel_StatusCode_ToProto(cel_StatusCode_kDataLoss),
            google_rpc_DATA_LOSS);
}

}  // namespace
