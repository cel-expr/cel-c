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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_VALUE_TESTING_H_
#define THIRD_PARTY_CEL_C_INTERNAL_VALUE_TESTING_H_

#include "gtest/gtest.h"
#include "absl/log/die_if_null.h"
#include "absl/strings/string_view.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/internal/config.h"
#include "cel-c/status.h"
#include "cel-c/value.h"
#include "cel-c/well_known_types.h"
#include "upb/message/array.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"

class ValueTest : public ::testing::Test {
 public:
  void SetUp() override;

  void TearDown() override;

  upb_Message* ParseProto(absl::string_view name, absl::string_view text,
                          absl::string_view binary);

  cel_Allocator* cel_nonnull alloc() { return cel_DefaultAllocator; }

  cel_Arena* cel_nonnull arena() { return ABSL_DIE_IF_NULL(arena_); }

  cel_Status* cel_nonnull status() { return &status_; }

  upb_DefPool* cel_nonnull def_pool() { return ABSL_DIE_IF_NULL(def_pool_); }

  cel_WellKnownTypes* cel_nonnull wkts() { return &wkts_; }

  cel_ValueContext* cel_nonnull ctx() { return &ctx_; }

  const upb_MessageDef* cel_nonnull MessageDef(absl::string_view name);

  const upb_MessageDef* cel_nonnull TestAllTypesDef() {
    return ABSL_DIE_IF_NULL(test_all_types_def_);
  }

  const upb_FieldDef* cel_nonnull TestAllTypesFieldDef(absl::string_view name);

  upb_MessageValue TestAllTypesField(const upb_Message* cel_nonnull msg,
                                     const upb_FieldDef* cel_nonnull field);

  upb_MessageValue TestAllTypesField(const upb_Message* cel_nonnull msg,
                                     absl::string_view field);

  const upb_EnumDef* cel_nonnull EnumDef(absl::string_view name);

  const upb_EnumValueDef* cel_nonnull EnumValueDef(const upb_EnumDef* enm,
                                                   absl::string_view value);

  const upb_EnumValueDef* cel_nonnull EnumValueDef(absl::string_view name,
                                                   absl::string_view value);

 private:
  cel_Arena* cel_nullable arena_ = nullptr;
  upb_DefPool* cel_nullable def_pool_ = nullptr;
  const upb_MessageDef* cel_nullable test_all_types_def_ = nullptr;
  cel_Status status_ = {};
  cel_WellKnownTypes wkts_ = {};
  cel_ValueContext ctx_ = {};
};

#endif  // THIRD_PARTY_CEL_C_INTERNAL_VALUE_TESTING_H_
