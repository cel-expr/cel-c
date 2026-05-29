// Copyright 2024 Google LLC
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

#include "cel-c/src/rc.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/config.h"

namespace {

using ::testing::_;

TEST(RefCount, RefCount) {
  _cel_RefCount rc;
  _cel_RefCount_Initialize(&rc);
  EXPECT_TRUE(_cel_RefCount_Unique(&rc));
  EXPECT_FALSE(_cel_RefCount_Expired(&rc));
  _cel_RefCount_Increment(&rc);
  EXPECT_FALSE(_cel_RefCount_Unique(&rc));
  EXPECT_FALSE(_cel_RefCount_Expired(&rc));
  ASSERT_FALSE(_cel_RefCount_Decrement(&rc));
  EXPECT_TRUE(_cel_RefCount_Unique(&rc));
  EXPECT_FALSE(_cel_RefCount_Expired(&rc));
  ASSERT_TRUE(_cel_RefCount_Decrement(&rc));
  EXPECT_FALSE(_cel_RefCount_Unique(&rc));
  EXPECT_TRUE(_cel_RefCount_Expired(&rc));
  EXPECT_DEBUG_DEATH(CEL_USED(_cel_RefCount_Decrement(&rc)), _);
  EXPECT_DEBUG_DEATH(CEL_USED(_cel_RefCount_Increment(&rc)), _);
}

}  // namespace
