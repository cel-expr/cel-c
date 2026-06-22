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

#include "cel-c/internal/container.h"

#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "absl/strings/string_view.h"
#include "cel-c/alloc.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"

namespace {

class ContainerTest : public ::testing::Test {
 public:
  void SetUp() override { _cel_Container_Construct(&cont_); }

  void TearDown() override { _cel_Container_Destruct(cont(), alloc()); }

  cel_Allocator* alloc() { return cel_DefaultAllocator; }

  _cel_Container* cont() { return &cont_; }

 private:
  _cel_Container cont_;
};

TEST_F(ContainerTest, Empty) {
  EXPECT_TRUE(_cel_Container_Empty(cont()));
  EXPECT_EQ(_cel_Container_Count(cont()), 0);
  _cel_ContainerIterator cont_it = _cel_Container_Iterate(cont());
  EXPECT_FALSE(_cel_ContainerIterator_HasNext(&cont_it));

  ASSERT_TRUE(
      _cel_Container_Update(cont(), cel_StringView_FromString(""), alloc()));

  EXPECT_TRUE(_cel_Container_Empty(cont()));
  EXPECT_EQ(_cel_Container_Count(cont()), 0);
  cont_it = _cel_Container_Iterate(cont());
  EXPECT_FALSE(_cel_ContainerIterator_HasNext(&cont_it));
}

struct ContainerTestParam {
  std::string subject;
  std::vector<std::string> results;
};

class ContainerTestWithParam
    : public ContainerTest,
      public ::testing::WithParamInterface<ContainerTestParam> {};

TEST_P(ContainerTestWithParam, Match) {
  const auto& param = GetParam();
  ASSERT_TRUE(_cel_Container_Update(
      cont(), cel_StringView_FromAbsl(absl::string_view(param.subject)),
      alloc()));
  EXPECT_EQ(_cel_Container_Count(cont()), param.results.size());
  _cel_ContainerIterator cont_it = _cel_Container_Iterate(cont());
  for (const auto& part : param.results) {
    ASSERT_TRUE(_cel_ContainerIterator_HasNext(&cont_it));
    EXPECT_EQ(_cel_ContainerIterator_Next(&cont_it),
              cel_StringView_FromAbsl(absl::string_view(part)));
  }
  EXPECT_FALSE(_cel_ContainerIterator_HasNext(&cont_it));
}

INSTANTIATE_TEST_SUITE_P(
    ContainerTestWithParam, ContainerTestWithParam,
    ::testing::ValuesIn<ContainerTestParam>(
        {{.subject = "foo", .results = {"foo"}},
         {.subject = ".foo", .results = {"foo"}},
         {.subject = "foo.", .results = {"foo"}},
         {.subject = ".foo.", .results = {"foo"}},
         {.subject = "..foo", .results = {"foo"}},
         {.subject = "foo..", .results = {"foo"}},
         {.subject = "..foo..", .results = {"foo"}},
         {.subject = "foo.bar", .results = {"foo.bar", "foo"}},
         {.subject = "foo..bar", .results = {"foo.bar", "foo"}},
         {.subject = ".foo.bar", .results = {"foo.bar", "foo"}},
         {.subject = "foo.bar.", .results = {"foo.bar", "foo"}},
         {.subject = "..foo.bar", .results = {"foo.bar", "foo"}},
         {.subject = "foo.bar..", .results = {"foo.bar", "foo"}},
         {.subject = "..foo.bar..", .results = {"foo.bar", "foo"}}}));

}  // namespace
