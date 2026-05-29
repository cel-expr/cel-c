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

#include "cel-c/src/stable_sort.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "absl/base/casts.h"
#include "absl/random/distributions.h"
#include "absl/random/random.h"
#include "absl/strings/str_join.h"

namespace {

TEST(StableSort, Sorted) {
  std::vector<int32_t> values;
  values.reserve(1024);

  {
    absl::BitGen rng;
    for (size_t i = 0; i < 1024; ++i) {
      values.push_back(absl::bit_cast<int32_t>(absl::Uniform<uint32_t>(rng)));
    }
  }

  _cel_StableSort(values.data(), values.size(), sizeof(int32_t),
                  [](const void* lhs, const void* rhs) -> int {
                    const int32_t l = *((const int32_t*)lhs);
                    const int32_t r = *((const int32_t*)rhs);
                    return l < r ? -1 : l > r ? 1 : 0;
                  });
  EXPECT_TRUE(std::is_sorted(values.begin(), values.end()))
      << "[" << absl::StrJoin(values, ", ") << "]";
}

}  // namespace
