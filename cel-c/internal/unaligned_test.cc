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

#include "cel-c/internal/unaligned.h"

#include <cstdint>
#include <cstring>

#include "gtest/gtest.h"

namespace {

TEST(Unaligned, RoundTrip8) {
  constexpr uint8_t kWant = (uint8_t{1} << 0);

  char unaligned[sizeof(uint8_t)];
  std::memset(unaligned, '\0', sizeof(unaligned));
  EXPECT_EQ(_cel_UnalignedLoad8(unaligned), uint8_t{0});
  _cel_UnalignedStore8(unaligned, kWant);
  EXPECT_EQ(_cel_UnalignedLoad8(unaligned), kWant);
}

TEST(Unaligned, RoundTrip16) {
  constexpr uint16_t kWant = (uint16_t{1} << 0) | (uint16_t{1} << 8);

  char unaligned[sizeof(uint16_t)];
  std::memset(unaligned, '\0', sizeof(unaligned));
  EXPECT_EQ(_cel_UnalignedLoad16(unaligned), uint16_t{0});
  _cel_UnalignedStore16(unaligned, kWant);
  EXPECT_EQ(_cel_UnalignedLoad16(unaligned), kWant);
}

TEST(Unaligned, RoundTrip32) {
  constexpr uint32_t kWant = (uint32_t{1} << 0) | (uint32_t{1} << 8) |
                             (uint32_t{1} << 16) | (uint32_t{1} << 24);

  char unaligned[sizeof(uint32_t)];
  std::memset(unaligned, '\0', sizeof(unaligned));
  EXPECT_EQ(_cel_UnalignedLoad32(unaligned), uint32_t{0});
  _cel_UnalignedStore32(unaligned, kWant);
  EXPECT_EQ(_cel_UnalignedLoad32(unaligned), kWant);
}

TEST(Unaligned, RoundTrip64) {
  constexpr uint64_t kWant = (uint64_t{1} << 0) | (uint64_t{1} << 8) |
                             (uint64_t{1} << 16) | (uint64_t{1} << 24) |
                             (uint64_t{1} << 32) | (uint64_t{1} << 40) |
                             (uint64_t{1} << 48) | (uint64_t{1} << 56);

  char unaligned[sizeof(uint64_t)];
  std::memset(unaligned, '\0', sizeof(unaligned));
  EXPECT_EQ(_cel_UnalignedLoad64(unaligned), uint64_t{0});
  _cel_UnalignedStore64(unaligned, kWant);
  EXPECT_EQ(_cel_UnalignedLoad64(unaligned), kWant);
}

}  // namespace
