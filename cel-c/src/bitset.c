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

#include "cel-c/src/bitset.h"

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/src/bit.h"

bool _cel_BitSet_Next(CEL_NULLABLE(const _cel_BitSetWord*) words, size_t bits,
                      CEL_NONNULL(size_t*) bit) {
  CEL_ASSERT(words != cel_nullptr || bits == 0);
  CEL_ASSERT_NOT_NULL(bit);
  size_t i = *bit;
  // If `i` is `_cel_BitSet_kBegin`, `++i` will wrap back around to 0.
  ++i;
  while (i < bits) {
    const _cel_BitSetWord word_index = _cel_BitSet_WordIndexForBit(i);
    const _cel_BitSetWord word = words[word_index];
    // Check if any bits are set in word. If they are not, skip to the next
    // word.
    if (word != 0) {
      const int bit_index = _cel_BitSet_BitIndexForBit(i);
      const _cel_BitSetWord word_mask =
          ~((((_cel_BitSetWord)1) << bit_index) - 1);
      const _cel_BitSetWord masked_word = word & word_mask;
      // If the masked word does not have any bits set, we can skip to the next
      // word.
      if (masked_word != 0) {
        const int n = _cel_trailing_zeros(masked_word);
        CEL_ASSERT(n >= ((size_t)bit_index) && n < _cel_BitSet_kBitsPerWord);
        *bit = (word_index * _cel_BitSet_kBitsPerWord) + n;
        return true;
      }
    }
    i = ((word_index + 1) * _cel_BitSet_kBitsPerWord);
  }
  *bit = bits;
  return false;
}
