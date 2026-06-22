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

#include "cel-c/internal/deque.h"

#include <cstddef>
#include <initializer_list>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/config.h"

namespace {

using ::testing::_;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Test;

class DequeTest : public Test {
 public:
  void SetUp() override { _cel_Deque_Construct(&deque_); }

  void TearDown() override { _cel_Deque_Destruct(&deque_, alloc()); }

 protected:
  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  auto* deque() { return &deque_; }

 private:
  _cel_Deque(int) deque_;
};

template <typename T, typename U>
void CheckDequeIndices(T* deque, std::initializer_list<U> il) {
  size_t n = _cel_Deque_Size(deque);
  ASSERT_EQ(n, il.size());
  auto it = il.begin();
  for (size_t i = 0; i < n; ++i, ++it) {
    ASSERT_EQ(*_cel_Deque_MutableAt(deque, i), *it);
  }
}

template <typename T, typename U, typename V>
void CheckDequeIndices(T* deque, U value, V stride) {
  size_t n = _cel_Deque_Size(deque);
  for (size_t i = 0; i < n; ++i, value += stride) {
    ASSERT_EQ(*_cel_Deque_MutableAt(deque, i), value) << i;
  }
}

TEST_F(DequeTest, Empty) {
  EXPECT_EQ(_cel_Deque_Size(deque()), 0);
  EXPECT_TRUE(_cel_Deque_Empty(deque()));
}

TEST_F(DequeTest, PushFrontPopFront) {
  for (int i = 0; i < 65535; ++i) {
    int* front = _cel_Deque_PushFront(deque(), alloc());
    ASSERT_THAT(front, NotNull());
    *front = i;

    EXPECT_EQ(_cel_Deque_Size(deque()), i + 1);
    EXPECT_FALSE(_cel_Deque_Empty(deque()));

    EXPECT_EQ(_cel_Deque_PeekFront(deque()), front);
    EXPECT_EQ(_cel_Deque_MutablePeekFront(deque()),
              _cel_Deque_PeekFront(deque()));
  }

  for (int i = 65535; i > 0; --i) {
    int* front = _cel_Deque_MutablePeekFront(deque());
    EXPECT_EQ(*front, i - 1);

    EXPECT_EQ(_cel_Deque_Size(deque()), i);
    EXPECT_FALSE(_cel_Deque_Empty(deque()));

    _cel_Deque_PopFront(deque(), alloc());
  }

  EXPECT_EQ(_cel_Deque_Size(deque()), 0);
  EXPECT_TRUE(_cel_Deque_Empty(deque()));
}

TEST_F(DequeTest, PushBackPopBack) {
  for (int i = 0; i < 65535; ++i) {
    int* back = _cel_Deque_PushBack(deque(), alloc());
    ASSERT_THAT(back, NotNull());
    *back = i;

    EXPECT_EQ(_cel_Deque_Size(deque()), i + 1);
    EXPECT_FALSE(_cel_Deque_Empty(deque()));

    EXPECT_EQ(_cel_Deque_PeekBack(deque()), back);
    EXPECT_EQ(_cel_Deque_MutablePeekBack(deque()),
              _cel_Deque_PeekBack(deque()));
  }

  for (int i = 65535; i > 0; --i) {
    int* back = _cel_Deque_MutablePeekBack(deque());
    EXPECT_EQ(*back, i - 1);

    EXPECT_EQ(_cel_Deque_Size(deque()), i);
    EXPECT_FALSE(_cel_Deque_Empty(deque()));

    _cel_Deque_PopBack(deque(), alloc());
  }

  EXPECT_EQ(_cel_Deque_Size(deque()), 0);
  EXPECT_TRUE(_cel_Deque_Empty(deque()));
}

TEST_F(DequeTest, PushFrontPopBack) {
  {
    for (int i = 0; i < 65535; ++i) {
      int* front = _cel_Deque_PushFront(deque(), alloc());
      ASSERT_THAT(front, NotNull());
      *front = i;

      EXPECT_EQ(_cel_Deque_Size(deque()), i + 1);
      EXPECT_FALSE(_cel_Deque_Empty(deque()));

      EXPECT_EQ(_cel_Deque_PeekFront(deque()), front);
      EXPECT_EQ(_cel_Deque_MutablePeekFront(deque()),
                _cel_Deque_PeekFront(deque()));
    }

    for (int i = 0; i < 65535; ++i) {
      int* back = _cel_Deque_MutablePeekBack(deque());
      EXPECT_EQ(*back, i);

      EXPECT_EQ(_cel_Deque_Size(deque()), 65535 - i);
      EXPECT_FALSE(_cel_Deque_Empty(deque()));

      _cel_Deque_PopBack(deque(), alloc());
    }

    EXPECT_EQ(_cel_Deque_Size(deque()), 0);
    EXPECT_TRUE(_cel_Deque_Empty(deque()));
  }

  {
    for (int i = 0; i < 2048; i += 2) {
      int* front = _cel_Deque_PushFront(deque(), alloc());
      ASSERT_THAT(front, NotNull());
      *front = i;

      front = _cel_Deque_PushFront(deque(), alloc());
      ASSERT_THAT(front, NotNull());
      *front = i + 1;

      _cel_Deque_PopBack(deque(), alloc());
    }

    EXPECT_EQ(_cel_Deque_Size(deque()), 1024);
    EXPECT_FALSE(_cel_Deque_Empty(deque()));
    CheckDequeIndices(deque(), 2047, -1);

    for (int i = 1024; i < 2048; ++i) {
      int* back = _cel_Deque_MutablePeekBack(deque());
      EXPECT_EQ(*back, i);

      _cel_Deque_PopBack(deque(), alloc());
    }
  }
}

TEST_F(DequeTest, PushBackPopFront) {
  {
    for (int i = 0; i < 65535; ++i) {
      int* back = _cel_Deque_PushBack(deque(), alloc());
      ASSERT_THAT(back, NotNull());
      *back = i;

      EXPECT_EQ(_cel_Deque_Size(deque()), i + 1);
      EXPECT_FALSE(_cel_Deque_Empty(deque()));

      EXPECT_EQ(_cel_Deque_PeekBack(deque()), back);
      EXPECT_EQ(_cel_Deque_MutablePeekBack(deque()),
                _cel_Deque_PeekBack(deque()));
    }

    for (int i = 0; i < 65535; ++i) {
      int* front = _cel_Deque_MutablePeekFront(deque());
      EXPECT_EQ(*front, i);

      EXPECT_EQ(_cel_Deque_Size(deque()), 65535 - i);
      EXPECT_FALSE(_cel_Deque_Empty(deque()));

      _cel_Deque_PopFront(deque(), alloc());
    }

    EXPECT_EQ(_cel_Deque_Size(deque()), 0);
    EXPECT_TRUE(_cel_Deque_Empty(deque()));
  }

  {
    for (int i = 0; i < 2048; i += 2) {
      int* back = _cel_Deque_PushBack(deque(), alloc());
      ASSERT_THAT(back, NotNull());
      *back = i;

      back = _cel_Deque_PushBack(deque(), alloc());
      ASSERT_THAT(back, NotNull());
      *back = i + 1;

      _cel_Deque_PopFront(deque(), alloc());
    }

    EXPECT_EQ(_cel_Deque_Size(deque()), 1024);
    EXPECT_FALSE(_cel_Deque_Empty(deque()));
    CheckDequeIndices(deque(), 1024, 1);

    for (int i = 1024; i < 2048; ++i) {
      int* front = _cel_Deque_MutablePeekFront(deque());
      EXPECT_EQ(*front, i);

      _cel_Deque_PopFront(deque(), alloc());
    }
  }
}

TEST_F(DequeTest, Indices) {
  int* next;

  next = _cel_Deque_PushBack(deque(), alloc());
  ASSERT_THAT(next, NotNull());
  *next = 0;
  CheckDequeIndices(deque(), {0});

  next = _cel_Deque_PushFront(deque(), alloc());
  ASSERT_THAT(next, NotNull());
  *next = 1;
  CheckDequeIndices(deque(), {1, 0});

  next = _cel_Deque_PushFront(deque(), alloc());
  ASSERT_THAT(next, NotNull());
  *next = 2;
  CheckDequeIndices(deque(), {2, 1, 0});

  next = _cel_Deque_PushBack(deque(), alloc());
  ASSERT_THAT(next, NotNull());
  *next = 3;
  CheckDequeIndices(deque(), {2, 1, 0, 3});
}

TEST_F(DequeTest, Clear) {
  int* back = _cel_Deque_PushBack(deque(), alloc());
  ASSERT_THAT(back, NotNull());
  *back = 0;

  EXPECT_THAT(deque()->v.g.cache, IsNull());

  _cel_Deque_Clear(deque(), alloc());
  _cel_Deque_Clear(deque(), alloc());

  EXPECT_THAT(deque()->v.g.cache, NotNull());

  int* front = _cel_Deque_PushBack(deque(), alloc());
  ASSERT_THAT(front, NotNull());
  *front = 0;

  EXPECT_THAT(deque()->v.g.cache, IsNull());

  _cel_Deque_Clear(deque(), alloc());
  _cel_Deque_Clear(deque(), alloc());

  EXPECT_THAT(deque()->v.g.cache, NotNull());
}

TEST_F(DequeTest, Reset) {
  int* back = _cel_Deque_PushBack(deque(), alloc());
  ASSERT_THAT(back, NotNull());
  *back = 0;

  EXPECT_THAT(deque()->v.g.cache, IsNull());

  _cel_Deque_Reset(deque(), alloc());

  EXPECT_THAT(deque()->v.g.cache, IsNull());

  int* front = _cel_Deque_PushBack(deque(), alloc());
  ASSERT_THAT(front, NotNull());
  *front = 0;

  EXPECT_THAT(deque()->v.g.cache, IsNull());

  _cel_Deque_Reset(deque(), alloc());

  EXPECT_THAT(deque()->v.g.cache, IsNull());
}

using DequeDeathTest = DequeTest;

TEST_F(DequeDeathTest, Empty) {
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Deque_PopFront(deque(), alloc())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Deque_PeekFront(deque())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Deque_MutablePeekFront(deque())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Deque_PopBack(deque(), alloc())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Deque_PeekBack(deque())), _);
  EXPECT_DEATH_IF_SUPPORTED(CEL_USED(_cel_Deque_MutablePeekBack(deque())), _);
#else
  GTEST_SKIP() << "optimized build or death test is unsupported";
#endif
}

}  // namespace
