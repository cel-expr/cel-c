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

#include "cel-c/src/unicode.h"

#include "gtest/gtest.h"

namespace {

TEST(Unicode, IsValid) {
  EXPECT_TRUE(_cel_Unicode_IsValid(0));
  EXPECT_TRUE(_cel_Unicode_IsValid(_cel_Unicode_kReplacementChar));
  EXPECT_FALSE(_cel_Unicode_IsValid(0xd900));
  EXPECT_FALSE(_cel_Unicode_IsValid(0x120000));
}

}  // namespace
