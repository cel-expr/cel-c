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

#include "cel-c/internal/bitset.h"

#include <cstddef>

#include "gtest/gtest.h"

namespace {

TEST(BitSet, WordsForBits) {
  EXPECT_EQ(_cel_BitSet_WordsForBits(0), 0);
  EXPECT_EQ(_cel_BitSet_WordsForBits(1), 1);
}

TEST(BitSet, WordIndexForBit) {
  EXPECT_EQ(_cel_BitSet_WordIndexForBit(0), 0);
  EXPECT_EQ(_cel_BitSet_WordIndexForBit(_cel_BitSet_kBitsPerWord), 1);
}

TEST(BitSet, BitIndexForBit) {
  EXPECT_EQ(_cel_BitSet_BitIndexForBit(0), 0);
  EXPECT_EQ(_cel_BitSet_BitIndexForBit(1), 1);
  EXPECT_EQ(_cel_BitSet_BitIndexForBit(_cel_BitSet_kBitsPerWord), 0);
  EXPECT_EQ(_cel_BitSet_BitIndexForBit(_cel_BitSet_kBitsPerWord + 1), 1);
}

TEST(BitSet, WordMaskForBit) {
  EXPECT_EQ(_cel_BitSet_WordMaskForBit(0), 1);
  EXPECT_EQ(_cel_BitSet_WordMaskForBit(1), 2);
  EXPECT_EQ(_cel_BitSet_WordMaskForBit(_cel_BitSet_kBitsPerWord), 1);
  EXPECT_EQ(_cel_BitSet_WordMaskForBit(_cel_BitSet_kBitsPerWord + 1), 2);
}

TEST(BitSet, Correct) {
  _cel_BitSetWord words[3];
  size_t bits = _cel_BitSet_kBitsPerWord * 3;

  _cel_BitSet_Reset(words, bits);
  EXPECT_EQ(words[0], 0);
  EXPECT_EQ(words[1], 0);

  EXPECT_FALSE(_cel_BitSet_Test(words, bits, 0));
  EXPECT_FALSE(_cel_BitSet_Test(words, bits, bits - 1));
  EXPECT_FALSE(_cel_BitSet_Test(words, bits, bits / 2));

  _cel_BitSet_Set(words, bits, 0);
  _cel_BitSet_Set(words, bits, bits - 1);
  _cel_BitSet_Set(words, bits, bits / 2);

  EXPECT_TRUE(_cel_BitSet_Test(words, bits, 0));
  EXPECT_TRUE(_cel_BitSet_Test(words, bits, bits - 1));
  EXPECT_TRUE(_cel_BitSet_Test(words, bits, bits / 2));
  EXPECT_FALSE(_cel_BitSet_Test(words, bits, 1));

  _cel_BitSet_Clear(words, bits, bits / 2);

  size_t bit = _cel_BitSet_kBegin;
  ASSERT_TRUE(_cel_BitSet_Next(words, bits, &bit));
  EXPECT_EQ(bit, 0);
  ASSERT_TRUE(_cel_BitSet_Next(words, bits, &bit));
  EXPECT_EQ(bit, bits - 1);
  EXPECT_FALSE(_cel_BitSet_Next(words, bits, &bit));
  EXPECT_EQ(bit, bits);

  _cel_BitSet_Clear(words, bits, 0);
  _cel_BitSet_Clear(words, bits, bits - 1);

  EXPECT_FALSE(_cel_BitSet_Test(words, bits, 0));
  EXPECT_FALSE(_cel_BitSet_Test(words, bits, bits - 1));
  EXPECT_FALSE(_cel_BitSet_Test(words, bits, bits / 2));
}

}  // namespace
