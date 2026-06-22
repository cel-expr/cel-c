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

#include "cel-c/internal/arena_deque.h"

#include <cstddef>
#include <initializer_list>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/internal/config.h"

namespace {

using ::testing::_;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Test;

class ArenaDequeTest : public Test {
 public:
  void SetUp() override {
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(cel_DefaultAllocator));
  }

  void TearDown() override {
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
  }

 protected:
  CEL_NONNULL(cel_Arena*) arena() { return ABSL_DIE_IF_NULL(arena_); }

 private:
  CEL_NULLABLE(cel_Arena*) arena_ = nullptr;
};

TEST_F(ArenaDequeTest, Empty) {
  _cel_ArenaDeque(int) deque;
  _cel_ArenaDeque_Construct(&deque);

  EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 0);
  EXPECT_TRUE(_cel_ArenaDeque_Empty(&deque));
}

template <typename T, typename U>
void CheckArenaDequeIndices(T* deque, std::initializer_list<U> il) {
  size_t n = _cel_ArenaDeque_Size(deque);
  ASSERT_EQ(n, il.size());
  auto it = il.begin();
  for (size_t i = 0; i < n; ++i, ++it) {
    ASSERT_EQ(*_cel_ArenaDeque_MutableAt(deque, i), *it);
  }
}

template <typename T, typename U, typename V>
void CheckArenaDequeIndices(T* deque, U value, V stride) {
  size_t n = _cel_ArenaDeque_Size(deque);
  for (size_t i = 0; i < n; ++i, value += stride) {
    ASSERT_EQ(*_cel_ArenaDeque_MutableAt(deque, i), value) << i;
  }
}

TEST_F(ArenaDequeTest, PushFrontPopFront) {
  _cel_ArenaDeque(int) deque;
  _cel_ArenaDeque_Construct(&deque);

  for (int i = 0; i < 65535; ++i) {
    int* front = _cel_ArenaDeque_PushFront(&deque, arena());
    ASSERT_THAT(front, NotNull());
    *front = i;

    EXPECT_EQ(_cel_ArenaDeque_Size(&deque), i + 1);
    EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));

    EXPECT_EQ(_cel_ArenaDeque_PeekFront(&deque), front);
    EXPECT_EQ(_cel_ArenaDeque_MutablePeekFront(&deque),
              _cel_ArenaDeque_PeekFront(&deque));
  }

  for (int i = 65535; i > 0; --i) {
    int* front = _cel_ArenaDeque_MutablePeekFront(&deque);
    EXPECT_EQ(*front, i - 1);

    EXPECT_EQ(_cel_ArenaDeque_Size(&deque), i);
    EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));

    _cel_ArenaDeque_PopFront(&deque);
  }

  EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 0);
  EXPECT_TRUE(_cel_ArenaDeque_Empty(&deque));
}

TEST_F(ArenaDequeTest, PushBackPopBack) {
  _cel_ArenaDeque(int) deque;
  _cel_ArenaDeque_Construct(&deque);

  for (int i = 0; i < 65535; ++i) {
    int* back = _cel_ArenaDeque_PushBack(&deque, arena());
    ASSERT_THAT(back, NotNull());
    *back = i;

    EXPECT_EQ(_cel_ArenaDeque_Size(&deque), i + 1);
    EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));

    EXPECT_EQ(_cel_ArenaDeque_PeekBack(&deque), back);
    EXPECT_EQ(_cel_ArenaDeque_MutablePeekBack(&deque),
              _cel_ArenaDeque_PeekBack(&deque));
  }

  for (int i = 65535; i > 0; --i) {
    int* back = _cel_ArenaDeque_MutablePeekBack(&deque);
    EXPECT_EQ(*back, i - 1);

    EXPECT_EQ(_cel_ArenaDeque_Size(&deque), i);
    EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));

    _cel_ArenaDeque_PopBack(&deque);
  }

  EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 0);
  EXPECT_TRUE(_cel_ArenaDeque_Empty(&deque));
}

TEST_F(ArenaDequeTest, PushFrontPopBack) {
  _cel_ArenaDeque(int) deque;
  _cel_ArenaDeque_Construct(&deque);

  {
    for (int i = 0; i < 65535; ++i) {
      int* front = _cel_ArenaDeque_PushFront(&deque, arena());
      ASSERT_THAT(front, NotNull());
      *front = i;

      EXPECT_EQ(_cel_ArenaDeque_Size(&deque), i + 1);
      EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));

      EXPECT_EQ(_cel_ArenaDeque_PeekFront(&deque), front);
      EXPECT_EQ(_cel_ArenaDeque_MutablePeekFront(&deque),
                _cel_ArenaDeque_PeekFront(&deque));
    }

    for (int i = 0; i < 65535; ++i) {
      int* back = _cel_ArenaDeque_MutablePeekBack(&deque);
      EXPECT_EQ(*back, i);

      EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 65535 - i);
      EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));

      _cel_ArenaDeque_PopBack(&deque);
    }

    EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 0);
    EXPECT_TRUE(_cel_ArenaDeque_Empty(&deque));
  }

  {
    for (int i = 0; i < 2048; i += 2) {
      int* front = _cel_ArenaDeque_PushFront(&deque, arena());
      ASSERT_THAT(front, NotNull());
      *front = i;

      front = _cel_ArenaDeque_PushFront(&deque, arena());
      ASSERT_THAT(front, NotNull());
      *front = i + 1;

      _cel_ArenaDeque_PopBack(&deque);
    }

    EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 1024);
    EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));
    CheckArenaDequeIndices(&deque, 2047, -1);

    for (int i = 1024; i < 2048; ++i) {
      int* back = _cel_ArenaDeque_MutablePeekBack(&deque);
      EXPECT_EQ(*back, i);

      _cel_ArenaDeque_PopBack(&deque);
    }
  }
}

TEST_F(ArenaDequeTest, PushBackPopFront) {
  _cel_ArenaDeque(int) deque;
  _cel_ArenaDeque_Construct(&deque);

  {
    for (int i = 0; i < 65535; ++i) {
      int* back = _cel_ArenaDeque_PushBack(&deque, arena());
      ASSERT_THAT(back, NotNull());
      *back = i;

      EXPECT_EQ(_cel_ArenaDeque_Size(&deque), i + 1);
      EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));

      EXPECT_EQ(_cel_ArenaDeque_PeekBack(&deque), back);
      EXPECT_EQ(_cel_ArenaDeque_MutablePeekBack(&deque),
                _cel_ArenaDeque_PeekBack(&deque));
    }

    for (int i = 0; i < 65535; ++i) {
      int* front = _cel_ArenaDeque_MutablePeekFront(&deque);
      EXPECT_EQ(*front, i);

      EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 65535 - i);
      EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));

      _cel_ArenaDeque_PopFront(&deque);
    }

    EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 0);
    EXPECT_TRUE(_cel_ArenaDeque_Empty(&deque));
  }

  {
    for (int i = 0; i < 2048; i += 2) {
      int* back = _cel_ArenaDeque_PushBack(&deque, arena());
      ASSERT_THAT(back, NotNull());
      *back = i;

      back = _cel_ArenaDeque_PushBack(&deque, arena());
      ASSERT_THAT(back, NotNull());
      *back = i + 1;

      _cel_ArenaDeque_PopFront(&deque);
    }

    EXPECT_EQ(_cel_ArenaDeque_Size(&deque), 1024);
    EXPECT_FALSE(_cel_ArenaDeque_Empty(&deque));
    CheckArenaDequeIndices(&deque, 1024, 1);

    for (int i = 1024; i < 2048; ++i) {
      int* front = _cel_ArenaDeque_MutablePeekFront(&deque);
      EXPECT_EQ(*front, i);

      _cel_ArenaDeque_PopFront(&deque);
    }
  }
}

TEST_F(ArenaDequeTest, Indices) {
  _cel_ArenaDeque(int) deque;
  _cel_ArenaDeque_Construct(&deque);

  int* next;

  next = _cel_ArenaDeque_PushBack(&deque, arena());
  ASSERT_THAT(next, NotNull());
  *next = 0;
  CheckArenaDequeIndices(&deque, {0});

  next = _cel_ArenaDeque_PushFront(&deque, arena());
  ASSERT_THAT(next, NotNull());
  *next = 1;
  CheckArenaDequeIndices(&deque, {1, 0});

  next = _cel_ArenaDeque_PushFront(&deque, arena());
  ASSERT_THAT(next, NotNull());
  *next = 2;
  CheckArenaDequeIndices(&deque, {2, 1, 0});

  next = _cel_ArenaDeque_PushBack(&deque, arena());
  ASSERT_THAT(next, NotNull());
  *next = 3;
  CheckArenaDequeIndices(&deque, {2, 1, 0, 3});
}

TEST_F(ArenaDequeTest, Clear) {
  _cel_ArenaDeque(int) deque;
  _cel_ArenaDeque_Construct(&deque);

  int* back = _cel_ArenaDeque_PushBack(&deque, arena());
  ASSERT_THAT(back, NotNull());
  *back = 0;

  EXPECT_THAT(deque.v.g.cache, IsNull());

  _cel_ArenaDeque_Clear(&deque);
  _cel_ArenaDeque_Clear(&deque);

  EXPECT_THAT(deque.v.g.cache, NotNull());

  int* front = _cel_ArenaDeque_PushBack(&deque, arena());
  ASSERT_THAT(front, NotNull());
  *front = 0;

  EXPECT_THAT(deque.v.g.cache, IsNull());

  _cel_ArenaDeque_Clear(&deque);
  _cel_ArenaDeque_Clear(&deque);

  EXPECT_THAT(deque.v.g.cache, NotNull());
}

using ArenaDequeDeathTest = ArenaDequeTest;

TEST_F(ArenaDequeDeathTest, Empty) {
  _cel_ArenaDeque(int) deque;
  _cel_ArenaDeque_Construct(&deque);

#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaDeque_PopFront(&deque)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaDeque_PeekFront(&deque)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaDeque_MutablePeekFront(&deque)),
                            _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaDeque_PopBack(&deque)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaDeque_PeekBack(&deque)), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_ArenaDeque_MutablePeekBack(&deque)),
                            _);
#else
  GTEST_SKIP() << "optimized build or death test is unsupported";
#endif
}

}  // namespace
