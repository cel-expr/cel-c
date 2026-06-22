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

#include "cel-c/internal/endian.h"

#include <cstdint>
#include <cstring>

#include "gtest/gtest.h"

namespace {

TEST(Endian, ReverseBytes8) {
  EXPECT_EQ(_cel_ReverseBytes8(uint8_t{1}), uint8_t{1});
}

TEST(Endian, ReverseBytes16) {
  EXPECT_EQ(_cel_ReverseBytes16(uint16_t{1}), uint16_t{1} << 8);
  EXPECT_EQ(_cel_ReverseBytes16(_cel_ReverseBytes16(uint16_t{1})), uint16_t{1});
}

TEST(Endian, ReverseBytes32) {
  EXPECT_EQ(_cel_ReverseBytes32(uint32_t{1}), uint32_t{1} << 24);
  EXPECT_EQ(_cel_ReverseBytes32(_cel_ReverseBytes32(uint32_t{1})), uint32_t{1});
}

TEST(Endian, ReverseBytes64) {
  EXPECT_EQ(_cel_ReverseBytes64(uint64_t{1}), uint64_t{1} << 56);
  EXPECT_EQ(_cel_ReverseBytes64(_cel_ReverseBytes64(uint64_t{1})), uint64_t{1});
}

TEST(Endian, LittleUnalignedLoad8) {
  char unaligned[sizeof(uint8_t)];
  std::memset(unaligned, '\0', sizeof(unaligned));
  EXPECT_EQ(_cel_LittleEndianUnalignedLoad8(unaligned), uint8_t{0});
}

TEST(Endian, LittleUnalignedLoad16) {
  char unaligned[sizeof(uint16_t)];
  std::memset(unaligned, '\0', sizeof(unaligned));
  EXPECT_EQ(_cel_LittleEndianUnalignedLoad16(unaligned), uint16_t{0});
}

TEST(Endian, LittleUnalignedLoad32) {
  char unaligned[sizeof(uint32_t)];
  std::memset(unaligned, '\0', sizeof(unaligned));
  EXPECT_EQ(_cel_LittleEndianUnalignedLoad32(unaligned), uint32_t{0});
}

TEST(Endian, LittleUnalignedLoad64) {
  char unaligned[sizeof(uint64_t)];
  std::memset(unaligned, '\0', sizeof(unaligned));
  EXPECT_EQ(_cel_LittleEndianUnalignedLoad64(unaligned), uint64_t{0});
}

}  // namespace
