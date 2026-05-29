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

#include "cel-c/src/arena_flat_hash_map.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/config.h"
#include "cel-c/hash.h"

namespace {

using ::testing::Test;

class ArenaFlatHashMapTest : public Test {
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

size_t IntHasher(CEL_NONNULL(const int*) ent) {
  cel_HashState state = cel_HashState_Initialize();
  state = cel_HashState_Combine(state, *ent);
  return cel_HashState_Finalize(state);
}

bool IntEqualer(CEL_NONNULL(const int*) lhs, CEL_NONNULL(const int*) rhs) {
  return *lhs == *rhs;
}

TEST_F(ArenaFlatHashMapTest, Empty) {
  _cel_ArenaFlatHashMap(int, int) map;
  _cel_ArenaFlatHashMap_Construct(&map, IntHasher, IntEqualer);

  EXPECT_EQ(_cel_ArenaFlatHashMap_Size(&map), 0);
  EXPECT_TRUE(_cel_ArenaFlatHashMap_Empty(&map));
  EXPECT_EQ(_cel_ArenaFlatHashMap_Capacity(&map), 0);

  int key = 0;
  EXPECT_TRUE(_cel_ArenaFlatHashMap_Find(&map, &key) == nullptr);
  EXPECT_TRUE(_cel_ArenaFlatHashMap_MutableFind(&map, &key) == nullptr);

  {
    _cel_ArenaFlatHashMapNode(map) ent =
        (_cel_ArenaFlatHashMapNode(map))_cel_ArenaFlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_ArenaFlatHashMap_Next(&map, &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }

  {
    _cel_ArenaFlatHashMapMutableNode(map) ent =
        (_cel_ArenaFlatHashMapMutableNode(map))_cel_ArenaFlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_ArenaFlatHashMap_MutableNext(&map, &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }
}

TEST_F(ArenaFlatHashMapTest, InsertErase) {
  _cel_ArenaFlatHashMap(int, int) map;
  _cel_ArenaFlatHashMap_Construct(&map, IntHasher, IntEqualer);

  int key = 0;
  _cel_ArenaFlatHashMapMutableNode(map) ent;
  ASSERT_TRUE(_cel_ArenaFlatHashMap_Insert(&map, arena(), &key, &ent));
  ent->val = 0;

  EXPECT_EQ(_cel_ArenaFlatHashMap_Size(&map), 1);
  EXPECT_FALSE(_cel_ArenaFlatHashMap_Empty(&map));
  EXPECT_GE(_cel_ArenaFlatHashMap_Capacity(&map),
            _cel_ArenaFlatHashMap_Size(&map));

  EXPECT_EQ(_cel_ArenaFlatHashMap_Find(&map, &key), ent);
  EXPECT_EQ(_cel_ArenaFlatHashMap_MutableFind(&map, &key), ent);
  {
    _cel_ArenaFlatHashMapNode(map) ent =
        (_cel_ArenaFlatHashMapNode(map))_cel_ArenaFlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_ArenaFlatHashMap_Next(&map, &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 1);
  }

  {
    _cel_ArenaFlatHashMapMutableNode(map) ent =
        (_cel_ArenaFlatHashMapMutableNode(map))_cel_ArenaFlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_ArenaFlatHashMap_MutableNext(&map, &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 1);
  }

  _cel_ArenaFlatHashMap_Erase(&map, ent);
  EXPECT_EQ(_cel_ArenaFlatHashMap_Size(&map), 0);
  EXPECT_TRUE(_cel_ArenaFlatHashMap_Empty(&map));
  EXPECT_GE(_cel_ArenaFlatHashMap_Capacity(&map), 1);

  _cel_ArenaFlatHashMap_Clear(&map);
  EXPECT_EQ(_cel_ArenaFlatHashMap_Size(&map), 0);
  EXPECT_TRUE(_cel_ArenaFlatHashMap_Empty(&map));
  EXPECT_GE(_cel_ArenaFlatHashMap_Capacity(&map), 1);

  {
    _cel_ArenaFlatHashMapNode(map) ent =
        (_cel_ArenaFlatHashMapNode(map))_cel_ArenaFlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_ArenaFlatHashMap_Next(&map, &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }

  {
    _cel_ArenaFlatHashMapMutableNode(map) ent =
        (_cel_ArenaFlatHashMapMutableNode(map))_cel_ArenaFlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_ArenaFlatHashMap_MutableNext(&map, &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 0);
  }
}

TEST_F(ArenaFlatHashMapTest, InsertClear) {
  _cel_ArenaFlatHashMap(int, int) map;
  _cel_ArenaFlatHashMap_Construct(&map, IntHasher, IntEqualer);

  for (int key = 0; key < 65535; ++key) {
    _cel_ArenaFlatHashMapMutableNode(map) ent;
    ASSERT_TRUE(_cel_ArenaFlatHashMap_Insert(&map, arena(), &key, &ent));
    ent->val = key;

    EXPECT_EQ(_cel_ArenaFlatHashMap_Size(&map), key + 1);
    EXPECT_FALSE(_cel_ArenaFlatHashMap_Empty(&map));
    EXPECT_GE(_cel_ArenaFlatHashMap_Capacity(&map),
              _cel_ArenaFlatHashMap_Size(&map));

    EXPECT_EQ(_cel_ArenaFlatHashMap_Find(&map, &key), ent) << key;
    EXPECT_EQ(_cel_ArenaFlatHashMap_MutableFind(&map, &key), ent) << key;
  }

  {
    _cel_ArenaFlatHashMapNode(map) ent =
        (_cel_ArenaFlatHashMapNode(map))_cel_ArenaFlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_ArenaFlatHashMap_Next(&map, &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 65535);
  }

  {
    _cel_ArenaFlatHashMapMutableNode(map) ent =
        (_cel_ArenaFlatHashMapMutableNode(map))_cel_ArenaFlatHashMap_kBegin;
    size_t nr_ents = 0;
    while (_cel_ArenaFlatHashMap_MutableNext(&map, &ent)) {
      ++nr_ents;
    }
    EXPECT_EQ(nr_ents, 65535);
  }

  _cel_ArenaFlatHashMap_Clear(&map);
  EXPECT_EQ(_cel_ArenaFlatHashMap_Size(&map), 0);
  EXPECT_TRUE(_cel_ArenaFlatHashMap_Empty(&map));
  EXPECT_GE(_cel_ArenaFlatHashMap_Capacity(&map), 1);

  _cel_ArenaFlatHashMap_Clear(&map);
  EXPECT_EQ(_cel_ArenaFlatHashMap_Size(&map), 0);
  EXPECT_TRUE(_cel_ArenaFlatHashMap_Empty(&map));
  EXPECT_GE(_cel_ArenaFlatHashMap_Capacity(&map), 1);
}

}  // namespace
