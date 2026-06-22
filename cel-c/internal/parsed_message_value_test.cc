#include "cel-c/internal/parsed_message_value.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "cel-c/internal/value_testing.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"
#include "cel-c/value.h"

namespace {

using ::testing::NotNull;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

using ParsedMessageValueTest = ValueTest;

struct StructValueIteratorDeleter {
  void operator()(cel_StructValueIterator* iterator) const {
    cel_StructValueIterator_Delete(iterator);
  }
};

using StructValueIteratorPtr =
    std::unique_ptr<cel_StructValueIterator, StructValueIteratorDeleter>;

TEST_F(ParsedMessageValueTest, Equals) {
  cel_StructValue struct_value;
  _cel_ParsedMessageValue_Set(
      &struct_value,
      ParseProto("cel.expr.conformance.proto3.TestAllTypes",
                 R"pb(single_int32: 1)pb", ""),
      TestAllTypesDef());
  cel_StructValue other_struct_value;
  _cel_ParsedMessageValue_Set(
      &other_struct_value,
      ParseProto("cel.expr.conformance.proto3.TestAllTypes",
                 R"pb(single_int32: 1)pb", ""),
      TestAllTypesDef());

  cel_Value result;

  ASSERT_TRUE(cel_StructValue_Equals(&struct_value, ctx(), &other_struct_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));

  ASSERT_TRUE(cel_StructValue_Equals(&other_struct_value, ctx(), &struct_value,
                                     &result, status()));
  EXPECT_TRUE(cel_Value_IsTrue(&result));
}

TEST_F(ParsedMessageValueTest, TypeName) {
  cel_StructValue struct_value;
  _cel_ParsedMessageValue_Set(
      &struct_value,
      ParseProto("cel.expr.conformance.proto3.TestAllTypes", R"pb()pb", ""),
      TestAllTypesDef());

  EXPECT_EQ(
      cel_StructValue_TypeName(&struct_value),
      cel_StringView_FromString("cel.expr.conformance.proto3.TestAllTypes"));
}

TEST_F(ParsedMessageValueTest, Get) {
  cel_StructValue struct_value;
  _cel_ParsedMessageValue_Set(
      &struct_value,
      ParseProto("cel.expr.conformance.proto3.TestAllTypes", R"pb()pb", ""),
      TestAllTypesDef());

  cel_StructValueKey key;
  cel_Value result;

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_int32"));
  ASSERT_TRUE(
      cel_StructValue_Get(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_THAT(cel_Value_GetInt(&result), 0);

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_int32"));
  ASSERT_TRUE(
      cel_StructValue_Get(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsInt(&result));
  EXPECT_THAT(cel_Value_GetInt(&result), 0);

  cel_StructValueKey_SetName(
      &key, cel_StringView_FromString("field_that_does_not_exist"));
  ASSERT_TRUE(
      cel_StructValue_Get(&struct_value, ctx(), &key, &result, status()));
  EXPECT_TRUE(cel_Value_IsError(&result));
}

TEST_F(ParsedMessageValueTest, Has) {
  cel_StructValue struct_value;
  _cel_ParsedMessageValue_Set(
      &struct_value,
      ParseProto("cel.expr.conformance.proto3.TestAllTypes",
                 R"pb(single_int32: 1
                      single_int64: 1
                      single_uint32: 1
                      single_uint64: 1
                      single_float: nan
                      single_double: nan
                      single_bytes: "foo"
                      single_string: "foo"
                      optional_bool: false
                      empty: {}
                      repeated_int32: 1
                      map_int32_string: { key: 1 value: "foo" })pb",
                 ""),
      TestAllTypesDef());

  cel_StructValueKey key;
  cel_Value result;

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_int32"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_int32"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_int64"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_int64"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_uint32"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_uint32"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_uint64"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_uint64"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_float"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_float"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_double"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_double"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_bytes"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_bytes"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("single_string"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("single_string"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("optional_bool"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("optional_bool"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("empty"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("empty"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key, cel_StringView_FromString("repeated_int32"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("repeated_int32"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(&key,
                             cel_StringView_FromString("map_int32_string"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetDef(&key, TestAllTypesFieldDef("map_int32_string"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  ASSERT_TRUE(cel_Value_IsTrue(&result));

  cel_StructValueKey_SetName(
      &key, cel_StringView_FromString("field_that_does_not_exist"));
  ASSERT_TRUE(
      cel_StructValue_Has(&struct_value, ctx(), &key, &result, status()));
  EXPECT_TRUE(cel_Value_IsError(&result));
}

TEST_F(ParsedMessageValueTest, NewIterator) {
  cel_StructValue struct_value;
  _cel_ParsedMessageValue_Set(
      &struct_value,
      ParseProto("cel.expr.conformance.proto3.TestAllTypes",
                 R"pb(single_int32: 2 single_int64: 1)pb", ""),
      TestAllTypesDef());

  {
    StructValueIteratorPtr iter(
        cel_StructValue_NewIterator(&struct_value, ctx(), status()));
    ASSERT_THAT(iter, NotNull());

    size_t remaining;
    ASSERT_FALSE(cel_StructValueIterator_Remaining(iter.get(), &remaining));

    cel_StructValueKey key;
    cel_Value value;

    std::vector<std::pair<std::string, int64_t>> entries;

    ASSERT_TRUE(cel_StructValueIterator_Next(iter.get(), ctx(), &key, &value,
                                             status()));
    ASSERT_TRUE(cel_StructValueKey_IsDef(&key));
    ASSERT_TRUE(cel_Value_IsInt(&value));
    entries.emplace_back(
        cel_StringView_ToAbsl(cel_StructValueKey_GetName(&key)),
        cel_Value_GetInt(&value));

    ASSERT_TRUE(cel_StructValueIterator_Next(iter.get(), ctx(), &key, &value,
                                             status()));
    ASSERT_TRUE(cel_StructValueKey_IsDef(&key));
    ASSERT_TRUE(cel_Value_IsInt(&value));
    entries.emplace_back(
        cel_StringView_ToAbsl(cel_StructValueKey_GetName(&key)),
        cel_Value_GetInt(&value));

    ASSERT_FALSE(cel_StructValueIterator_Next(iter.get(), ctx(), &key, &value,
                                              status()));

    EXPECT_THAT(entries, UnorderedElementsAre(Pair("single_int32", 2),
                                              Pair("single_int64", 1)));
  }
}

}  // namespace
