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

#include "cel-c/hash.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "cel-c/assert.h"
#include "cel-c/src/bit.h"
#include "cel-c/src/config.h"
#include "cel-c/src/uint128.h"
#include "cel-c/src/unaligned.h"

static CEL_NONNULL(const void*) const _cel_kHashSeed = &_cel_kHashSeed;

CEL_ATTRIBUTE_NODISCARD
static uint64_t _cel_HashSeed() {
#if (!defined(__clang__) || __clang_major__ > 11) && \
    (!defined(__apple_build_version__) || __apple_build_version__ >= 19558921)
  return (uint64_t)((uintptr_t)&_cel_kHashSeed);
#else
  return (uint64_t)((uintptr_t)_cel_kHashSeed);
#endif
}

extern "C" cel_HashState cel_HashState_Initialize() {
  return ((cel_HashState){_cel_HashSeed()});
}

static void _cel_HashRead9To16(const uint8_t* cel_nonnull p, size_t n,
                               uint64_t* cel_nonnull low,
                               uint64_t* cel_nonnull high) {
  const uint64_t lo = _cel_UnalignedLoad64(p);
  const uint64_t hi = _cel_UnalignedLoad64(p + n - 8);
#ifdef _CEL_IS_LITTLE_ENDIAN
  const uint64_t most_significant = hi;
  const uint64_t least_significant = lo;
#else
  const uint64_t most_significant = lo;
  const uint64_t least_significant = hi;
#endif
  *low = least_significant;
  *high = most_significant;
}

CEL_ATTRIBUTE_NODISCARD
static uint64_t _cel_HashRead4To8(const uint8_t* cel_nonnull p, size_t n) {
  const uint32_t low = _cel_UnalignedLoad32(p);
  const uint32_t hi = _cel_UnalignedLoad32(p + n - 4);
#ifdef _CEL_IS_LITTLE_ENDIAN
  const uint32_t most_significant = hi;
  const uint32_t least_significant = low;
#else
  const uint32_t most_significant = low;
  const uint32_t least_significant = hi;
#endif
  return (((uint64_t)most_significant) << (n - 4) * 8) | least_significant;
}

CEL_ATTRIBUTE_NODISCARD
static uint32_t _cel_HashRead1To3(const uint8_t* cel_nonnull p, size_t n) {
  const uint8_t mem0 = p[0];
  const uint8_t mem1 = p[n / 2];
  const uint8_t mem2 = p[n - 1];
#ifdef _CEL_IS_LITTLE_ENDIAN
  const uint8_t significant2 = mem2;
  const uint8_t significant1 = mem1;
  const uint8_t significant0 = mem0;
#else
  const uint8_t significant2 = mem0;
  const uint8_t significant1 = n == 2 ? mem0 : mem1;
  const uint8_t significant0 = mem2;
#endif
  return (uint32_t)(significant0 | (significant1 << (n / 2 * 8)) |
                    (significant2 << ((n - 1) * 8)));
}

CEL_ATTRIBUTE_NODISCARD
static uint64_t _cel_HashMix(uint64_t a, uint64_t b) {
  _cel_Uint128 m = _cel_Uint128_From(a + b);
  m = _cel_Uint128_Mul(m, _cel_Uint128_From(UINT64_C(0x9ddfea08eb382d69)));
  m = _cel_Uint128_Xor(m, _cel_Uint128_ShiftRight(m, 64));
  return _cel_Uint128_To(uint64_t, m);
}

static const uint64_t _cel_kHashSalt[5] = {
    UINT64_C(0x243F6A8885A308D3), UINT64_C(0x13198A2E03707344),
    UINT64_C(0xA4093822299F31D0), UINT64_C(0x082EFA98EC4E6C89),
    UINT64_C(0x452821E638D01377),
};

CEL_ATTRIBUTE_NODISCARD
static uint64_t _cel_LowLevelHashMix(uint64_t a, uint64_t b) {
  _cel_Uint128 m = _cel_Uint128_From(a);
  m = _cel_Uint128_Mul(m, _cel_Uint128_From(b));
  return _cel_Uint128_To(uint64_t, m) ^
         _cel_Uint128_To(uint64_t, _cel_Uint128_ShiftRight(m, 64));
}

CEL_ATTRIBUTE_NODISCARD
static uint64_t _cel_LowLevelHashN(const uint8_t* cel_nonnull p, size_t n) {
  CEL_ASSERT_GT(n, 16);
  const size_t starting_n = n;
  const uint8_t* const last_16_p = p + starting_n - 16;
  uint64_t state = _cel_HashSeed() ^ _cel_kHashSalt[0];

  if (n > 64) {
    uint64_t dup_state[3];
    dup_state[0] = state;
    dup_state[1] = state;
    dup_state[2] = state;

    do {
      const uint64_t a = _cel_UnalignedLoad64(p);
      const uint64_t b = _cel_UnalignedLoad64(p + 8);
      const uint64_t c = _cel_UnalignedLoad64(p + 16);
      const uint64_t d = _cel_UnalignedLoad64(p + 24);
      const uint64_t e = _cel_UnalignedLoad64(p + 32);
      const uint64_t f = _cel_UnalignedLoad64(p + 40);
      const uint64_t g = _cel_UnalignedLoad64(p + 48);
      const uint64_t h = _cel_UnalignedLoad64(p + 56);

      state = _cel_LowLevelHashMix(a ^ _cel_kHashSalt[1], b ^ state);
      dup_state[0] =
          _cel_LowLevelHashMix(c ^ _cel_kHashSalt[2], d ^ dup_state[0]);
      dup_state[1] =
          _cel_LowLevelHashMix(e ^ _cel_kHashSalt[3], f ^ dup_state[1]);
      dup_state[2] =
          _cel_LowLevelHashMix(g ^ _cel_kHashSalt[4], h ^ dup_state[2]);

      p += 64;
      n -= 64;
    } while (n > 64);

    state = (state ^ dup_state[0]) ^ (dup_state[1] + dup_state[2]);
  }

  if (n > 32) {
    uint64_t dup_state[2];

    const uint64_t a = _cel_UnalignedLoad64(p);
    const uint64_t b = _cel_UnalignedLoad64(p + 8);
    const uint64_t c = _cel_UnalignedLoad64(p + 16);
    const uint64_t d = _cel_UnalignedLoad64(p + 24);

    dup_state[0] = _cel_LowLevelHashMix(a ^ _cel_kHashSalt[1], b ^ state);
    dup_state[1] = _cel_LowLevelHashMix(c ^ _cel_kHashSalt[2], d ^ state);
    state = dup_state[0] ^ dup_state[1];

    p += 32;
    n -= 32;
  }

  if (n > 16) {
    const uint64_t a = _cel_UnalignedLoad64(p);
    const uint64_t b = _cel_UnalignedLoad64(p + 8);

    state = _cel_LowLevelHashMix(a ^ _cel_kHashSalt[1], b ^ state);
  }

  const uint64_t a = _cel_UnalignedLoad64(last_16_p);
  const uint64_t b = _cel_UnalignedLoad64(last_16_p + 8);

  return _cel_LowLevelHashMix(a ^ _cel_kHashSalt[1] ^ starting_n, b ^ state);
}

extern "C" cel_HashState cel_HashState_CombineN(cel_HashState state,
                                                const void* cel_nullable data,
                                                size_t size) {
  uint64_t v;
  if (size > 16) {
    if (CEL_UNLIKELY(size > 1024)) {
      do {
        state.val = _cel_HashMix(
            state.val,
            _cel_LowLevelHashN(reinterpret_cast<const uint8_t*>(data), 1024));
        size -= 1024;
        data = ((const uint8_t*)data) + 1024;
      } while (size > 1024);
      return cel_HashState_CombineN(state, data, size);
    }
    v = _cel_LowLevelHashN((const uint8_t*)data, size);
  } else if (size > 8) {
    uint64_t lo;
    uint64_t hi;
    _cel_HashRead9To16(reinterpret_cast<const uint8_t*>(data), size, &lo, &hi);
    lo = _cel_rotr(lo, 53);
    state.val *= UINT64_C(0x9ddfea08eb382d69);
    lo += state.val;
    state.val ^= hi;
    _cel_Uint128 m = _cel_Uint128_From(state.val);
    m = _cel_Uint128_Mul(m, _cel_Uint128_From(lo));
    m = _cel_Uint128_Xor(m, _cel_Uint128_ShiftRight(m, 64));
    return ((cel_HashState){_cel_Uint128_To(uint64_t, m)});
  } else if (size > 3) {
    v = _cel_HashRead4To8(reinterpret_cast<const uint8_t*>(data), size);
  } else if (size > 0) {
    v = _cel_HashRead1To3(reinterpret_cast<const uint8_t*>(data), size);
  } else {
    return state;
  }
  return ((cel_HashState){_cel_HashMix(state.val, v)});
}

extern "C" cel_HashState cel_HashState_Combine64(cel_HashState state,
                                                 uint64_t data) {
  return ((cel_HashState){_cel_HashMix(state.val, data)});
}

extern "C" cel_HashState cel_HashState_CombineLD(cel_HashState state,
                                                 long double value) {
  const int cat = fpclassify(value);
  switch (cat) {
    case FP_INFINITE:
      // Add the sign bit to differentiate between +Inf and -Inf
      state = cel_HashState_CombineB(state, signbit(value));
      break;
    case FP_NAN:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case FP_ZERO:
      CEL_ATTRIBUTE_FALLTHROUGH;
    default:
      // Category is enough for these.
      break;
    case FP_NORMAL:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case FP_SUBNORMAL: {
      // We can't convert `value` directly to double because this would have
      // undefined behavior if the value is out of range.
      // std::frexp gives us a value in the range (-1, -.5] or [.5, 1) that is
      // guaranteed to be in range for `double`. The truncation is
      // implementation defined, but that works as long as it is deterministic.
      int exp;
      double mant = (double)frexpl(value, &exp);
      state = cel_HashState_CombineD(state, mant);
      state = cel_HashState_CombineI(state, exp);
    } break;
  }
  return cel_HashState_CombineI(state, cat);
}
