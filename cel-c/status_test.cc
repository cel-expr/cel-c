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

#include "cel-c/status.h"

#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/container/btree_map.h"
#include "cel-c/config.h"
#include "cel-c/error_space.h"
#include "cel-c/status_code.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"

namespace {

using ::testing::ElementsAre;
using ::testing::Pair;
using ::testing::Test;

class StatusTest : public Test {
 public:
  void SetUp() override { cel_Status_Construct(&status_); }

  void TearDown() override { cel_Status_Destruct(&status_); }

 protected:
  CEL_NONNULL(cel_Status*) status() { return &status_; }

 private:
  cel_Status status_;
};

TEST_F(StatusTest, Ok) {
  EXPECT_TRUE(cel_Status_Ok(status()));
  EXPECT_FALSE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kOk);
  EXPECT_TRUE(cel_StringView_Empty(cel_Status_Message(status())));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kOk);
}

TEST_F(StatusTest, Cancelled) {
  ASSERT_TRUE(
      cel_CancelledStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_TRUE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kCancelled);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kCancelled);
}

TEST_F(StatusTest, Unknown) {
  ASSERT_TRUE(cel_UnknownStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_TRUE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kUnknown);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kUnknown);
}

TEST_F(StatusTest, InvalidArgument) {
  ASSERT_TRUE(
      cel_InvalidArgumentStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_TRUE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kInvalidArgument);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kInvalidArgument);
}

TEST_F(StatusTest, DeadlineExceeded) {
  ASSERT_TRUE(cel_DeadlineExceededStatus(status(),
                                         cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_TRUE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kDeadlineExceeded);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kDeadlineExceeded);
}

TEST_F(StatusTest, NotFound) {
  ASSERT_TRUE(
      cel_NotFoundStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_TRUE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kNotFound);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kNotFound);
}

TEST_F(StatusTest, AlreadyExists) {
  ASSERT_TRUE(
      cel_AlreadyExistsStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_TRUE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kAlreadyExists);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kAlreadyExists);
}

TEST_F(StatusTest, PermissionDenied) {
  ASSERT_TRUE(cel_PermissionDeniedStatus(status(),
                                         cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_TRUE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kPermissionDenied);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kPermissionDenied);
}

TEST_F(StatusTest, ResourceExhausted) {
  ASSERT_TRUE(cel_ResourceExhaustedStatus(status(),
                                          cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_TRUE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kResourceExhausted);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kResourceExhausted);
}

TEST_F(StatusTest, FailedPrecondition) {
  ASSERT_TRUE(cel_FailedPreconditionStatus(
      status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_TRUE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kFailedPrecondition);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kFailedPrecondition);
}

TEST_F(StatusTest, Aborted) {
  ASSERT_TRUE(cel_AbortedStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_TRUE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kAborted);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kAborted);
}

TEST_F(StatusTest, OutOfRange) {
  ASSERT_TRUE(
      cel_OutOfRangeStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_TRUE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kOutOfRange);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kOutOfRange);
}

TEST_F(StatusTest, Unimplemented) {
  ASSERT_TRUE(
      cel_UnimplementedStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_TRUE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kUnimplemented);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kUnimplemented);
}

TEST_F(StatusTest, Internal) {
  ASSERT_TRUE(
      cel_InternalStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_TRUE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kInternal);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kInternal);
}

TEST_F(StatusTest, Unavailable) {
  ASSERT_TRUE(
      cel_UnavailableStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_TRUE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kUnavailable);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kUnavailable);
}

TEST_F(StatusTest, DataLoss) {
  ASSERT_TRUE(
      cel_DataLossStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_TRUE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()), cel_StatusCode_kDataLoss);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kDataLoss);
}

TEST_F(StatusTest, Unauthenticated) {
  ASSERT_TRUE(
      cel_UnauthenticatedStatus(status(), cel_StringView_From("Hello World!")));
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_FALSE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_TRUE(cel_Status_IsUnauthenticated(status()));
  EXPECT_FALSE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kUnauthenticated);
  EXPECT_TRUE(cel_StringView_Equals(cel_Status_Message(status()),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kUnauthenticated);
}

TEST_F(StatusTest, OutOfMemory) {
  cel_OutOfMemoryStatus(status());
  EXPECT_FALSE(cel_Status_Ok(status()));
  EXPECT_TRUE(!cel_Status_Ok(status()));
  EXPECT_FALSE(cel_Status_IsCancelled(status()));
  EXPECT_FALSE(cel_Status_IsUnknown(status()));
  EXPECT_FALSE(cel_Status_IsInvalidArgument(status()));
  EXPECT_FALSE(cel_Status_IsDeadlineExceeded(status()));
  EXPECT_FALSE(cel_Status_IsNotFound(status()));
  EXPECT_FALSE(cel_Status_IsAlreadyExists(status()));
  EXPECT_FALSE(cel_Status_IsPermissionDenied(status()));
  EXPECT_TRUE(cel_Status_IsResourceExhausted(status()));
  EXPECT_FALSE(cel_Status_IsFailedPrecondition(status()));
  EXPECT_FALSE(cel_Status_IsAborted(status()));
  EXPECT_FALSE(cel_Status_IsOutOfRange(status()));
  EXPECT_FALSE(cel_Status_IsUnimplemented(status()));
  EXPECT_FALSE(cel_Status_IsInternal(status()));
  EXPECT_FALSE(cel_Status_IsUnavailable(status()));
  EXPECT_FALSE(cel_Status_IsDataLoss(status()));
  EXPECT_FALSE(cel_Status_IsUnauthenticated(status()));
  EXPECT_TRUE(cel_Status_IsOutOfMemory(status()));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_EQ(cel_Status_CanonicalCode(status()),
            cel_StatusCode_kResourceExhausted);
  EXPECT_EQ(cel_Status_Space(status()), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Status_Code(status()), cel_StatusCode_kResourceExhausted);
}

absl::btree_map<std::string, std::string> StatusPayloadsToMap(
    CEL_NONNULL(const cel_Status*) status) {
  absl::btree_map<std::string, std::string> out;
  cel_StringView type_url;
  cel_StringView value;
  cel_StatusPayloadIterator iter = cel_Status_BeginPayloads(status);
  while (cel_Status_NextPayload(status, &type_url, &value, &iter)) {
    out.insert_or_assign(cel_StringView_ToAbsl(type_url),
                         cel_StringView_ToAbsl(value));
  }
  return out;
}

TEST_F(StatusTest, Payloads) {
  cel_StringView value;
  std::vector<std::pair<std::string, std::string>> payloads;

  EXPECT_TRUE(cel_Status_SetPayload(status(), cel_StringView_From("type_url1"),
                                    cel_StringView_From("value1")));
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_FALSE(cel_Status_GetPayload(status(), cel_StringView_From("type_url1"),
                                     &value));
  EXPECT_FALSE(
      cel_Status_DeletePayload(status(), cel_StringView_From("type_url1")));
  EXPECT_THAT(StatusPayloadsToMap(status()), ElementsAre());

  cel_Status_SetCode(status(), cel_CanonicalErrorSpace,
                     cel_StatusCode_kCancelled);

  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_FALSE(cel_Status_GetPayload(status(), cel_StringView_From("type_url1"),
                                     &value));
  EXPECT_THAT(StatusPayloadsToMap(status()), ElementsAre());

  EXPECT_TRUE(cel_Status_SetPayload(status(), cel_StringView_From("type_url1"),
                                    cel_StringView_From("value1")));
  EXPECT_EQ(cel_Status_Payloads(status()), 1);
  EXPECT_TRUE(cel_Status_GetPayload(status(), cel_StringView_From("type_url1"),
                                    &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value1")));
  EXPECT_THAT(StatusPayloadsToMap(status()),
              ElementsAre(Pair("type_url1", "value1")));

  EXPECT_TRUE(cel_Status_SetPayload(status(), cel_StringView_From("type_url2"),
                                    cel_StringView_From("value2")));
  EXPECT_EQ(cel_Status_Payloads(status()), 2);
  EXPECT_TRUE(cel_Status_GetPayload(status(), cel_StringView_From("type_url1"),
                                    &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value1")));
  EXPECT_TRUE(cel_Status_GetPayload(status(), cel_StringView_From("type_url2"),
                                    &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value2")));
  EXPECT_THAT(
      StatusPayloadsToMap(status()),
      ElementsAre(Pair("type_url1", "value1"), Pair("type_url2", "value2")));

  EXPECT_TRUE(cel_Status_SetPayload(status(), cel_StringView_From("type_url2"),
                                    cel_StringView_From("value3")));
  EXPECT_EQ(cel_Status_Payloads(status()), 2);
  EXPECT_TRUE(cel_Status_GetPayload(status(), cel_StringView_From("type_url1"),
                                    &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value1")));
  EXPECT_TRUE(cel_Status_GetPayload(status(), cel_StringView_From("type_url2"),
                                    &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value3")));
  EXPECT_THAT(
      StatusPayloadsToMap(status()),
      ElementsAre(Pair("type_url1", "value1"), Pair("type_url2", "value3")));

  EXPECT_TRUE(
      cel_Status_DeletePayload(status(), cel_StringView_From("type_url1")));
  EXPECT_FALSE(cel_Status_GetPayload(status(), cel_StringView_From("type_url1"),
                                     &value));
  EXPECT_TRUE(cel_Status_GetPayload(status(), cel_StringView_From("type_url2"),
                                    &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value3")));
  EXPECT_THAT(StatusPayloadsToMap(status()),
              ElementsAre(Pair("type_url2", "value3")));

  cel_Status_ClearPayloads(status());
  EXPECT_EQ(cel_Status_Payloads(status()), 0);
  EXPECT_THAT(StatusPayloadsToMap(status()), ElementsAre());
}

}  // namespace
