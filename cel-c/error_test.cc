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

#include "cel-c/error.h"

#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/container/btree_map.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/error_code.h"
#include "cel-c/error_space.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"

namespace {

using ::testing::ElementsAre;
using ::testing::Pair;
using ::testing::Test;

class ErrorTest : public Test {
 public:
  void SetUp() override { arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc())); }

  void TearDown() override {
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  cel_Allocator* cel_nonnull alloc() { return cel_DefaultAllocator; }

  cel_Arena* cel_nonnull arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  cel_Arena* cel_nullability_unknown arena_ = nullptr;
};

TEST_F(ErrorTest, Cancelled) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_CancelledError(error, cel_StringView_From("Hello World!"));
  EXPECT_TRUE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kCancelled);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kCancelled);
}

TEST_F(ErrorTest, Unknown) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_UnknownError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_TRUE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kUnknown);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kUnknown);
}

TEST_F(ErrorTest, InvalidArgument) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_InvalidArgumentError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_TRUE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kInvalidArgument);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kInvalidArgument);
}

TEST_F(ErrorTest, DeadlineExceeded) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_DeadlineExceededError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_TRUE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kDeadlineExceeded);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kDeadlineExceeded);
}

TEST_F(ErrorTest, NotFound) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_NotFoundError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_TRUE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kNotFound);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kNotFound);
}

TEST_F(ErrorTest, AlreadyExists) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_AlreadyExistsError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_TRUE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kAlreadyExists);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kAlreadyExists);
}

TEST_F(ErrorTest, PermissionDenied) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_PermissionDeniedError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_TRUE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kPermissionDenied);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kPermissionDenied);
}

TEST_F(ErrorTest, ResourceExhausted) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_ResourceExhaustedError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_TRUE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kResourceExhausted);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kResourceExhausted);
}

TEST_F(ErrorTest, FailedPrecondition) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_FailedPreconditionError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_TRUE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kFailedPrecondition);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kFailedPrecondition);
}

TEST_F(ErrorTest, Aborted) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_AbortedError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_TRUE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kAborted);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kAborted);
}

TEST_F(ErrorTest, OutOfRange) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_OutOfRangeError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_TRUE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kOutOfRange);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kOutOfRange);
}

TEST_F(ErrorTest, Unimplemented) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_UnimplementedError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_TRUE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kUnimplemented);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kUnimplemented);
}

TEST_F(ErrorTest, Internal) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_InternalError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_TRUE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kInternal);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kInternal);
}

TEST_F(ErrorTest, Unavailable) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_UnavailableError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_TRUE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kUnavailable);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kUnavailable);
}

TEST_F(ErrorTest, DataLoss) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  cel_DataLossError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_TRUE(cel_Error_IsDataLoss(error));
  EXPECT_FALSE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kDataLoss);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kDataLoss);
}

TEST_F(ErrorTest, Unauthenticated) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));

  cel_UnauthenticatedError(error, cel_StringView_From("Hello World!"));
  EXPECT_FALSE(cel_Error_IsCancelled(error));
  EXPECT_FALSE(cel_Error_IsUnknown(error));
  EXPECT_FALSE(cel_Error_IsInvalidArgument(error));
  EXPECT_FALSE(cel_Error_IsDeadlineExceeded(error));
  EXPECT_FALSE(cel_Error_IsNotFound(error));
  EXPECT_FALSE(cel_Error_IsAlreadyExists(error));
  EXPECT_FALSE(cel_Error_IsPermissionDenied(error));
  EXPECT_FALSE(cel_Error_IsResourceExhausted(error));
  EXPECT_FALSE(cel_Error_IsFailedPrecondition(error));
  EXPECT_FALSE(cel_Error_IsAborted(error));
  EXPECT_FALSE(cel_Error_IsOutOfRange(error));
  EXPECT_FALSE(cel_Error_IsUnimplemented(error));
  EXPECT_FALSE(cel_Error_IsInternal(error));
  EXPECT_FALSE(cel_Error_IsUnavailable(error));
  EXPECT_FALSE(cel_Error_IsDataLoss(error));
  EXPECT_TRUE(cel_Error_IsUnauthenticated(error));
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kUnauthenticated);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_From("Hello World!")));
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kUnauthenticated);
}

absl::btree_map<std::string, std::string> ErrorPayloadsToMap(
    CEL_NONNULL(const cel_Error*) status) {
  absl::btree_map<std::string, std::string> out;
  cel_StringView type_url;
  cel_StringView value;
  cel_ErrorPayloadIterator iter = cel_Error_BeginPayloads(status);
  while (cel_Error_NextPayload(status, &type_url, &value, &iter)) {
    out.insert_or_assign(cel_StringView_ToAbsl(type_url),
                         cel_StringView_ToAbsl(value));
  }
  return out;
}

TEST_F(ErrorTest, Payloads) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));

  cel_StringView value;
  std::vector<std::pair<std::string, std::string>> payloads;

  EXPECT_THAT(ErrorPayloadsToMap(error), ElementsAre());

  cel_Error_SetCode(error, cel_CanonicalErrorSpace, cel_ErrorCode_kCancelled);

  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_FALSE(
      cel_Error_GetPayload(error, cel_StringView_From("type_url1"), &value));
  EXPECT_THAT(ErrorPayloadsToMap(error), ElementsAre());

  EXPECT_TRUE(cel_Error_SetPayload(error, cel_StringView_From("type_url1"),
                                   cel_StringView_From("value1"), arena()));
  EXPECT_EQ(cel_Error_Payloads(error), 1);
  EXPECT_TRUE(
      cel_Error_GetPayload(error, cel_StringView_From("type_url1"), &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value1")));
  EXPECT_THAT(ErrorPayloadsToMap(error),
              ElementsAre(Pair("type_url1", "value1")));

  EXPECT_TRUE(cel_Error_SetPayload(error, cel_StringView_From("type_url2"),
                                   cel_StringView_From("value2"), arena()));
  EXPECT_EQ(cel_Error_Payloads(error), 2);
  EXPECT_TRUE(
      cel_Error_GetPayload(error, cel_StringView_From("type_url1"), &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value1")));
  EXPECT_TRUE(
      cel_Error_GetPayload(error, cel_StringView_From("type_url2"), &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value2")));
  EXPECT_THAT(
      ErrorPayloadsToMap(error),
      ElementsAre(Pair("type_url1", "value1"), Pair("type_url2", "value2")));

  EXPECT_TRUE(cel_Error_SetPayload(error, cel_StringView_From("type_url2"),
                                   cel_StringView_From("value3"), arena()));
  EXPECT_EQ(cel_Error_Payloads(error), 2);
  EXPECT_TRUE(
      cel_Error_GetPayload(error, cel_StringView_From("type_url1"), &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value1")));
  EXPECT_TRUE(
      cel_Error_GetPayload(error, cel_StringView_From("type_url2"), &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value3")));
  EXPECT_THAT(
      ErrorPayloadsToMap(error),
      ElementsAre(Pair("type_url1", "value1"), Pair("type_url2", "value3")));

  EXPECT_TRUE(
      cel_Error_DeletePayload(error, cel_StringView_From("type_url1"), &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value1")));
  EXPECT_FALSE(
      cel_Error_GetPayload(error, cel_StringView_From("type_url1"), &value));
  EXPECT_TRUE(
      cel_Error_GetPayload(error, cel_StringView_From("type_url2"), &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_From("value3")));
  EXPECT_THAT(ErrorPayloadsToMap(error),
              ElementsAre(Pair("type_url2", "value3")));

  cel_Error_ClearPayloads(error);
  EXPECT_EQ(cel_Error_Payloads(error), 0);
  EXPECT_THAT(ErrorPayloadsToMap(error), ElementsAre());
}

}  // namespace
