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

#include "cel-c/error_absl.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/cord.h"
#include "absl/strings/string_view.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/string_view.h"

namespace {

using ::absl_testing::StatusIs;
using ::testing::Optional;
using ::testing::Test;

class ErrorAbslTest : public Test {
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

TEST_F(ErrorAbslTest, ToAbsl) {
  cel_Error* in = ABSL_DIE_IF_NULL(cel_Error_New(arena()));
  absl::Status out = cel_Error_ToAbsl(in);
  EXPECT_THAT(out, StatusIs(absl::StatusCode::kUnknown));
  size_t payload_count = 0;
  out.ForEachPayload([&payload_count](absl::string_view, const absl::Cord&) {
    ++payload_count;
  });
  EXPECT_EQ(payload_count, 0);

  cel_Error_SetCanonicalCode(in, cel_ErrorCode_kInvalidArgument);
  cel_Error_SetMessage(in, cel_StringView_From("Hello World!"));
  ASSERT_TRUE(cel_Error_SetPayload(in, cel_StringView_From("type_url"),
                                   cel_StringView_From("value"), arena()));
  out = cel_Error_ToAbsl(in);
  EXPECT_THAT(out,
              StatusIs(absl::StatusCode::kInvalidArgument, "Hello World!"));
  payload_count = 0;
  out.ForEachPayload([&payload_count](absl::string_view, const absl::Cord&) {
    ++payload_count;
  });
  EXPECT_EQ(payload_count, 1);
  EXPECT_THAT(out.GetPayload(absl::string_view("type_url")),
              Optional(absl::string_view("value")));
}

}  // namespace
