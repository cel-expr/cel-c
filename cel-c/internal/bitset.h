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

// Internal header providing functions for working with bit sets.

#ifndef THIRD_PARTY_CEL_C_INTERNAL_BITSET_H_
#define THIRD_PARTY_CEL_C_INTERNAL_BITSET_H_

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"

CEL_BEGIN_DECLS

typedef size_t _cel_BitSetWord;

// _cel_BitSet_kBytesPerWord
//
// Number of bytes in `_cel_BitSetWord`.
#define _cel_BitSet_kBytesPerWord sizeof(_cel_BitSetWord)

// _cel_BitSet_kBitsPerWord
//
// Number of bits in `_cel_BitSetWord`.
#define _cel_BitSet_kBitsPerWord (_cel_BitSet_kBytesPerWord * 8)

// _cel_BitSet_WordsForBits
//
// Number of words needed to represent `count` bits.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_BitSet_WordsForBits(size_t count) {
  return count == 0 ? count : (count - 1) / _cel_BitSet_kBitsPerWord + 1;
}

// _cel_BitSet_WordsForBits
//
// Word index for the bit `bit`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE size_t _cel_BitSet_WordIndexForBit(size_t bit) {
  return bit / _cel_BitSet_kBitsPerWord;
}

// _cel_BitSet_WordsForBits
//
// Bit offset into a word for bit `bit`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE int _cel_BitSet_BitIndexForBit(size_t bit) {
  return (int)(bit % _cel_BitSet_kBitsPerWord);
}

// _cel_BitSet_WordMaskForBit
//
// Returns a bit mask for a word with a single bit set corresponding to `bit`.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE _cel_BitSetWord _cel_BitSet_WordMaskForBit(size_t bit) {
  return ((_cel_BitSetWord)1) << _cel_BitSet_BitIndexForBit(bit);
}

// _cel_BitSet_Set
//
// Sets the bit `bit`.
static CEL_INLINE void _cel_BitSet_Set(CEL_NONNULL(_cel_BitSetWord*) words,
                                       size_t bits, size_t bit) {
  CEL_ASSERT_NOT_NULL(words);
  CEL_ASSERT_LT(bit, bits);
  CEL_USED(bits);
  words[_cel_BitSet_WordIndexForBit(bit)] |= _cel_BitSet_WordMaskForBit(bit);
}

// _cel_BitSet_Clear
//
// Clears the bit `bit`.
static CEL_INLINE void _cel_BitSet_Clear(CEL_NONNULL(_cel_BitSetWord*) words,
                                         size_t bits, size_t bit) {
  CEL_ASSERT_NOT_NULL(words);
  CEL_ASSERT_LT(bit, bits);
  CEL_USED(bits);
  words[_cel_BitSet_WordIndexForBit(bit)] &= ~_cel_BitSet_WordMaskForBit(bit);
}

// _cel_BitSet_Test
//
// Tests whether the bit `bit` is set.
CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE bool _cel_BitSet_Test(CEL_NONNULL(const _cel_BitSetWord*)
                                            words,
                                        size_t bits, size_t bit) {
  CEL_ASSERT_NOT_NULL(words);
  CEL_ASSERT_LT(bit, bits);
  CEL_USED(bits);
  return (words[_cel_BitSet_WordIndexForBit(bit)] &
          _cel_BitSet_WordMaskForBit(bit)) != 0;
}

// _cel_BitSet_Reset
//
// Clears all bits.
static CEL_INLINE void _cel_BitSet_Reset(CEL_NULLABLE(_cel_BitSetWord*) words,
                                         size_t bits) {
  CEL_ASSERT(words != cel_nullptr || bits == 0);
  memset(words, '\0',
         _cel_BitSet_WordsForBits(bits) * _cel_BitSet_kBytesPerWord);
}

#define _cel_BitSet_kBegin ((size_t)-1)

// _cel_BitSet_Next
//
// Iterate over the set bits. On the first call, `bit` must point to the value
// `_cel_BitSet_kBegin`. Returns `true` if a set bit was found and stores the
// bit in `bit`. Otherwise `false` is returned and there are no more set bits.
CEL_ATTRIBUTE_NODISCARD
CEL_ATTRIBUTE_NOTHROW
bool _cel_BitSet_Next(CEL_NULLABLE(const _cel_BitSetWord*) words, size_t bits,
                      CEL_NONNULL(size_t*) bit);

CEL_END_DECLS

#endif  // THIRD_PARTY_CEL_C_INTERNAL_BITSET_H_
