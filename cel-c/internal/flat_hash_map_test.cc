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

#include "cel-c/internal/flat_hash_map.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/alloc.h"
#include "cel-c/config.h"
#include "cel-c/hash.h"

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

class FlatHashMapTest : public Test {
 public:
  void SetUp() override {
    _cel_FlatHashMap_Construct(&map_, IntHasher, IntEqualer);
  }

  void TearDown() override { _cel_FlatHashMap_Destruct(&map_, alloc()); }

 protected:
  CEL_NONNULL(cel_Allocator*) alloc() { return cel_DefaultAllocator; }

  auto* map() { return &map_; }

  _cel_FlatHashMap(int, int) map_;
};

TEST_F(FlatHashMapTest, Empty) {
  EXPECT_EQ(_cel_FlatHashMap_Size(map()), 0);
  EXPECT_TRUE(_cel_FlatHashMap_Empty(map()));
  EXPECT_EQ(_cel_FlatHashMap_Capacity(map()), 0);

  int key = 0;
  EXPECT_TRUE(_cel_FlatHashMap_Find(map(), &key) == nullptr);
  EXPECT_TRUE(_cel_FlatHashMap_MutableFind(map(), &key) == nullptr);

  {
    _cel_FlatHashMapNode(map_) ent =
        (_cel_FlatHashMapNode(map_))_cel_FlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashMap_Next(map(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }

  {
    _cel_FlatHashMapMutableNode(map_) ent =
        (_cel_FlatHashMapMutableNode(map_))_cel_FlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashMap_MutableNext(map(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }
}

TEST_F(FlatHashMapTest, InsertErase) {
  _cel_FlatHashMap_Construct(map(), IntHasher, IntEqualer);

  int key = 0;
  _cel_FlatHashMapMutableNode(map_) ent;
  ASSERT_TRUE(_cel_FlatHashMap_Insert(map(), alloc(), &key, &ent));
  ent->val = 0;

  EXPECT_EQ(_cel_FlatHashMap_Size(map()), 1);
  EXPECT_FALSE(_cel_FlatHashMap_Empty(map()));
  EXPECT_GE(_cel_FlatHashMap_Capacity(map()), _cel_FlatHashMap_Size(map()));

  EXPECT_EQ(_cel_FlatHashMap_Find(map(), &key), ent);
  EXPECT_EQ(_cel_FlatHashMap_MutableFind(map(), &key), ent);
  {
    _cel_FlatHashMapNode(map_) ent =
        (_cel_FlatHashMapNode(map_))_cel_FlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashMap_Next(map(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 1);
  }

  {
    _cel_FlatHashMapMutableNode(map_) ent =
        (_cel_FlatHashMapMutableNode(map_))_cel_FlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashMap_MutableNext(map(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 1);
  }

  _cel_FlatHashMap_Erase(map(), ent);
  EXPECT_EQ(_cel_FlatHashMap_Size(map()), 0);
  EXPECT_TRUE(_cel_FlatHashMap_Empty(map()));
  EXPECT_GE(_cel_FlatHashMap_Capacity(map()), 1);

  _cel_FlatHashMap_Clear(map());
  EXPECT_EQ(_cel_FlatHashMap_Size(map()), 0);
  EXPECT_TRUE(_cel_FlatHashMap_Empty(map()));
  EXPECT_GE(_cel_FlatHashMap_Capacity(map()), 1);

  {
    _cel_FlatHashMapNode(map_) ent =
        (_cel_FlatHashMapNode(map_))_cel_FlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashMap_Next(map(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }

  {
    _cel_FlatHashMapMutableNode(map_) ent =
        (_cel_FlatHashMapMutableNode(map_))_cel_FlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashMap_MutableNext(map(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }
}

TEST_F(FlatHashMapTest, InsertClear) {
  for (int key = 0; key < 65535; ++key) {
    _cel_FlatHashMapMutableNode(map_) ent;
    ASSERT_TRUE(_cel_FlatHashMap_Insert(map(), alloc(), &key, &ent));
    ent->val = key;

    EXPECT_EQ(_cel_FlatHashMap_Size(map()), key + 1);
    EXPECT_FALSE(_cel_FlatHashMap_Empty(map()));
    EXPECT_GE(_cel_FlatHashMap_Capacity(map()), _cel_FlatHashMap_Size(map()));

    EXPECT_EQ(_cel_FlatHashMap_Find(map(), &key), ent) << key;
    EXPECT_EQ(_cel_FlatHashMap_MutableFind(map(), &key), ent) << key;
  }

  {
    _cel_FlatHashMapNode(map_) ent =
        (_cel_FlatHashMapNode(map_))_cel_FlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashMap_Next(map(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 65535);
  }

  {
    _cel_FlatHashMapMutableNode(map_) ent =
        (_cel_FlatHashMapMutableNode(map_))_cel_FlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_FlatHashMap_MutableNext(map(), &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 65535);
  }

  _cel_FlatHashMap_Clear(map());
  EXPECT_EQ(_cel_FlatHashMap_Size(map()), 0);
  EXPECT_TRUE(_cel_FlatHashMap_Empty(map()));
  EXPECT_GE(_cel_FlatHashMap_Capacity(map()), 1);

  _cel_FlatHashMap_Clear(map());
  EXPECT_EQ(_cel_FlatHashMap_Size(map()), 0);
  EXPECT_TRUE(_cel_FlatHashMap_Empty(map()));
  EXPECT_GE(_cel_FlatHashMap_Capacity(map()), 1);
}

}  // namespace
