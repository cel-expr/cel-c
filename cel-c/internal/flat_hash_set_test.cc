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

#include "cel-c/internal/flat_hash_set.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/hash.h"
#include "cel-c/internal/config.h"

namespace {

using ::testing::Test;

size_t IntHasher(CEL_NONNULL(const int*) ent) {
  cel_HashState state = cel_HashState_Initialize();
  state = cel_HashState_Combine(state, *ent);
  return cel_HashState_Finalize(state);
}

bool IntEqualer(CEL_NONNULL(const int*) lhs, CEL_NONNULL(const int*) rhs) {
  return *lhs == *rhs;
}

class FlatHashSetTest : public Test {
 public:
  void SetUp() override {
    _cel_FlatHashSet_Construct(&set_, IntHasher, IntEqualer);
  }

  void TearDown() override { _cel_FlatHashSet_Destruct(&set_, alloc()); }

 protected:
  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  auto* set() { return &set_; }

  _cel_FlatHashSet(int) set_;
};

TEST_F(FlatHashSetTest, Empty) {
  EXPECT_EQ(_cel_FlatHashSet_Size(set()), 0);
  EXPECT_TRUE(_cel_FlatHashSet_Empty(set()));
  EXPECT_EQ(_cel_FlatHashSet_Capacity(set()), 0);

  int key = 0;
  EXPECT_TRUE(_cel_FlatHashSet_Find(set(), &key) == nullptr);
  EXPECT_TRUE(_cel_FlatHashSet_MutableFind(set(), &key) == nullptr);

  {
    _cel_FlatHashSetNode(set_) ent =
        (_cel_FlatHashSetNode(set_))_cel_FlatHashSet_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashSet_Next(set(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }

  {
    _cel_FlatHashSetMutableNode(set_) ent =
        (_cel_FlatHashSetMutableNode(set_))_cel_FlatHashSet_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashSet_MutableNext(set(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }
}

TEST_F(FlatHashSetTest, InsertErase) {
  _cel_FlatHashSet_Construct(set(), IntHasher, IntEqualer);

  int key = 0;
  _cel_FlatHashSetMutableNode(set_) ent;
  ASSERT_TRUE(_cel_FlatHashSet_Insert(set(), alloc(), &key, &ent));

  EXPECT_EQ(_cel_FlatHashSet_Size(set()), 1);
  EXPECT_FALSE(_cel_FlatHashSet_Empty(set()));
  EXPECT_GE(_cel_FlatHashSet_Capacity(set()), _cel_FlatHashSet_Size(set()));

  EXPECT_EQ(_cel_FlatHashSet_Find(set(), &key), ent);
  EXPECT_EQ(_cel_FlatHashSet_MutableFind(set(), &key), ent);
  {
    _cel_FlatHashSetNode(set_) ent =
        (_cel_FlatHashSetNode(set_))_cel_FlatHashSet_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashSet_Next(set(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 1);
  }

  {
    _cel_FlatHashSetMutableNode(set_) ent =
        (_cel_FlatHashSetMutableNode(set_))_cel_FlatHashSet_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashSet_MutableNext(set(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 1);
  }

  _cel_FlatHashSet_Erase(set(), ent);
  EXPECT_EQ(_cel_FlatHashSet_Size(set()), 0);
  EXPECT_TRUE(_cel_FlatHashSet_Empty(set()));
  EXPECT_GE(_cel_FlatHashSet_Capacity(set()), 1);

  _cel_FlatHashSet_Clear(set());
  EXPECT_EQ(_cel_FlatHashSet_Size(set()), 0);
  EXPECT_TRUE(_cel_FlatHashSet_Empty(set()));
  EXPECT_GE(_cel_FlatHashSet_Capacity(set()), 1);

  {
    _cel_FlatHashSetNode(set_) ent =
        (_cel_FlatHashSetNode(set_))_cel_FlatHashSet_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashSet_Next(set(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }

  {
    _cel_FlatHashSetMutableNode(set_) ent =
        (_cel_FlatHashSetMutableNode(set_))_cel_FlatHashSet_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashSet_MutableNext(set(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }
}

TEST_F(FlatHashSetTest, InsertClear) {
  for (int key = 0; key < 65535; ++key) {
    _cel_FlatHashSetMutableNode(set_) ent;
    ASSERT_TRUE(_cel_FlatHashSet_Insert(set(), alloc(), &key, &ent));

    EXPECT_EQ(_cel_FlatHashSet_Size(set()), key + 1);
    EXPECT_FALSE(_cel_FlatHashSet_Empty(set()));
    EXPECT_GE(_cel_FlatHashSet_Capacity(set()), _cel_FlatHashSet_Size(set()));

    EXPECT_EQ(_cel_FlatHashSet_Find(set(), &key), ent) << key;
    EXPECT_EQ(_cel_FlatHashSet_MutableFind(set(), &key), ent) << key;
  }

  {
    _cel_FlatHashSetNode(set_) ent =
        (_cel_FlatHashSetNode(set_))_cel_FlatHashSet_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashSet_Next(set(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 65535);
  }

  {
    _cel_FlatHashSetMutableNode(set_) ent =
        (_cel_FlatHashSetMutableNode(set_))_cel_FlatHashSet_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashSet_MutableNext(set(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 65535);
  }

  _cel_FlatHashSet_Clear(set());
  EXPECT_EQ(_cel_FlatHashSet_Size(set()), 0);
  EXPECT_TRUE(_cel_FlatHashSet_Empty(set()));
  EXPECT_GE(_cel_FlatHashSet_Capacity(set()), 1);

  _cel_FlatHashSet_Clear(set());
  EXPECT_EQ(_cel_FlatHashSet_Size(set()), 0);
  EXPECT_TRUE(_cel_FlatHashSet_Empty(set()));
  EXPECT_GE(_cel_FlatHashSet_Capacity(set()), 1);
}

}  // namespace
