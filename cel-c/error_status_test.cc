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

#include "cel-c/error_status.h"

#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/error_space.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"

namespace {

class ErrorStatusTest : public ::testing::Test {
 public:
  void SetUp() override {
    cel_Status_Construct(&status_);
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc()));
  }

  void TearDown() override {
    cel_Status_Destruct(&status_);
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  cel_Allocator* cel_nonnull alloc() { return cel_DefaultAllocator; }

  cel_Status* cel_nonnull status() { return &status_; }

  cel_Arena* cel_nonnull arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  cel_Status status_;
  cel_Arena* cel_nullability_unknown arena_ = nullptr;
};

TEST_F(ErrorStatusTest, FromStatus) {
  cel_Error* error = ABSL_DIE_IF_NULL(cel_Error_New(arena()));

  ASSERT_TRUE(
      cel_InvalidArgumentStatus(status(), cel_StringView_From("Hello World!")));
  ASSERT_TRUE(cel_Status_SetPayload(status(), cel_StringView_From("type_url"),
                                    cel_StringView_From("value")));

  ASSERT_TRUE(cel_Error_FromStatus(error, status(), arena()));
  EXPECT_EQ(cel_Error_CanonicalCode(error), cel_ErrorCode_kInvalidArgument);
  EXPECT_EQ(cel_Error_Code(error), cel_ErrorCode_kInvalidArgument);
  EXPECT_EQ(cel_Error_Space(error), cel_CanonicalErrorSpace);
  EXPECT_TRUE(cel_StringView_Equals(cel_Error_Message(error),
                                    cel_StringView_FromString("Hello World!")));
  EXPECT_EQ(cel_Error_Payloads(error), 1);
  cel_StringView value;
  ASSERT_TRUE(cel_Error_GetPayload(error, cel_StringView_FromString("type_url"),
                                   &value));
  EXPECT_TRUE(cel_StringView_Equals(value, cel_StringView_FromString("value")));
}

}  // namespace
