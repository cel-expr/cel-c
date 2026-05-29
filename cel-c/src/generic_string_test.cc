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

#include "cel-c/src/generic_string.h"

#include <cstring>

#include "gtest/gtest.h"

namespace {

TEST(GenericString, Layout) {
  // There is no nice way at compile time to assert that the `is_large` bit
  // field is at the same offset in both structs. So we do it at runtime.
  _cel_GenericString str;

  std::memset(&str, 0, sizeof(str));

  ASSERT_EQ(str.small.is_large, 0);
  ASSERT_EQ(str.large.is_large, 0);

  str.small.is_large = 1;

  ASSERT_EQ(str.small.is_large, 1);
  ASSERT_EQ(str.large.is_large, 1);
}

}  // namespace
