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

#include "cel-c/runtime.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/cleanup/cleanup.h"
#include "absl/container/flat_hash_map.h"
#include "absl/log/absl_check.h"
#include "absl/log/die_if_null.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/string_view.h"
#include "cel-c/activation.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/ast.h"
#include "cel-c/duration.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/parsed_map_field_value.h"
#include "cel-c/internal/runtime/program.h"
#include "cel-c/internal/testing/compiler.h"
#include "cel-c/internal/testing/def_pool.h"
#include "cel-c/internal/testing/parser.h"
#include "cel-c/program.h"
#include "cel-c/status.h"
#include "cel-c/status_absl.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"
#include "cel-c/timestamp.h"
#include "cel-c/trilean.h"
#include "cel-c/value.h"
#include "internal/testing_descriptor_pool.h"
#include "internal/testing_message_factory.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/text_format.h"
#include "upb/message/message.h"
#include "upb/reflection/def.h"
#include "upb/reflection/message.h"
#include "upb/wire/decode.h"

namespace {

using ::absl_testing::StatusIs;
using ::testing::_;
using ::testing::HasSubstr;

struct _cel_TestingVariableResolver : cel_VariableResolver {
  absl::flat_hash_map<absl::string_view, cel_Value> bindings;

  _cel_TestingVariableResolver();
  _cel_TestingVariableResolver(const _cel_TestingVariableResolver&) = delete;
  _cel_TestingVariableResolver(_cel_TestingVariableResolver&&) = default;
  _cel_TestingVariableResolver& operator=(const _cel_TestingVariableResolver&) =
      delete;
  _cel_TestingVariableResolver& operator=(_cel_TestingVariableResolver&&) =
      default;

  // NOLINTNEXTLINE(google-explicit-constructor)
  _cel_TestingVariableResolver(
      std::initializer_list<typename absl::flat_hash_map<absl::string_view,
                                                         cel_Value>::value_type>
          init);

  virtual ~_cel_TestingVariableResolver() = default;

  virtual cel_Trilean Find(cel_StringView name, cel_Value* value,
                           cel_Status* status) const {
    if (auto binding = bindings.find(cel_StringView_ToAbsl(name));
        binding != bindings.end()) {
      *value = binding->second;
      return cel_Trilean_kTrue;
    }
    return cel_Trilean_kFalse;
  }
};

struct _cel_MockVariableResolver : public _cel_TestingVariableResolver {
 public:
  _cel_MockVariableResolver() : _cel_TestingVariableResolver() {}

  MOCK_METHOD(cel_Trilean, Find, (cel_StringView, cel_Value*, cel_Status*),
              (const, override));
};

const cel_VariableResolverVTable _cel_TestingVariableResolverVTable = {
    .Find = [](const cel_VariableResolver* resolver, cel_StringView name,
               cel_Value* value, cel_Arena* arena,
               cel_Status* status) -> cel_Trilean {
      return static_cast<const _cel_TestingVariableResolver*>(resolver)->Find(
          name, value, status);
    },
};

_cel_TestingVariableResolver::_cel_TestingVariableResolver() {
  vtable = &_cel_TestingVariableResolverVTable;
}

_cel_TestingVariableResolver::_cel_TestingVariableResolver(
    std::initializer_list<
        typename absl::flat_hash_map<absl::string_view, cel_Value>::value_type>
        init)
    : bindings(std::move(init)) {
  vtable = &_cel_TestingVariableResolverVTable;
}

class RuntimeHarness {
 public:
  void SetUp() {
    cel_Status_Construct(&status_);
    arena_ = cel_ArenaPtr(ABSL_DIE_IF_NULL(cel_Arena_New(alloc())));
    rt_ = NewRuntime(nullptr);
    parser_ = _cel_TestingParserPtr(_cel_TestingParser_New());
    compiler_ = _cel_TestingCompilerPtr(_cel_TestingCompiler_New());
    std::memset(&context_, 0, sizeof(context_));
    context_.alloc = alloc();
    context_.arena = arena_.get();
    context_.def_pool = cel_Runtime_DefPool(rt_.get());
    context_.well_known_types = cel_Runtime_WellKnownTypes(rt_.get());
  }

  void TearDown() {
    compiler_.reset();
    parser_.reset();
    rt_.reset();
    arena_.reset();
    cel_Status_Destruct(&status_);
  }

  upb_Message* ParseTextProto(absl::string_view name, absl::string_view text) {
    const google::protobuf::Descriptor* proto_def = ABSL_DIE_IF_NULL(
        cel::internal::GetTestingDescriptorPool()->FindMessageTypeByName(name));
    std::unique_ptr<google::protobuf::Message> proto_message(
        ABSL_DIE_IF_NULL(
            cel::internal::GetTestingMessageFactory()->GetPrototype(proto_def))
            ->New());
    ABSL_CHECK(google::protobuf::TextFormat::ParseFromString(text, proto_message.get()));
    std::string proto_serialized;
    ABSL_CHECK(proto_message->SerializePartialToString(&proto_serialized));
    const upb_MessageDef* upb_def =
        ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByNameWithSize(
            def_pool(), name.data(), name.size()));
    upb_Message* upb_message = ABSL_DIE_IF_NULL(
        upb_Message_New(upb_MessageDef_MiniTable(upb_def), arena()));
    ABSL_CHECK_EQ(
        upb_Decode(proto_serialized.data(), proto_serialized.size(),
                   upb_message, upb_MessageDef_MiniTable(upb_def),
                   upb_DefPool_ExtensionRegistry(def_pool()), 0, arena()),
        kUpb_DecodeStatus_Ok);
    return upb_message;
  }

  const upb_FieldDef* FieldDef(absl::string_view name,
                               absl::string_view field) {
    return ABSL_DIE_IF_NULL(upb_MessageDef_FindFieldByNameWithSize(
        ABSL_DIE_IF_NULL(upb_DefPool_FindMessageByNameWithSize(
            def_pool(), name.data(), name.size())),
        field.data(), field.size()));
  }

  cel_Allocator* alloc() { return cel_DefaultAllocator; }

  cel_Arena* arena() { return ABSL_DIE_IF_NULL(arena_.get()); }

  cel_Status* status() { return &status_; }

  const upb_DefPool* def_pool() { return _cel_TestingDefPool(); }

  const cel_ValueContext* context() { return &context_; }

  cel_RuntimePtr NewRuntime(const cel_RuntimeOptions* opts) {
    cel_RuntimePtr rt(cel_Runtime_New(alloc(), def_pool(), opts, status()));
    ABSL_CHECK(rt != nullptr) << cel_Status_ToAbsl(status());
    return rt;
  }

  cel_ProgramPtr Parse(absl::string_view content,
                       const cel_Runtime* rt = nullptr) {
    cel_AstPtr ast(_cel_TestingParser_Parse(ABSL_DIE_IF_NULL(parser_.get()),
                                            cel_StringView_FromAbsl(content),
                                            arena()));
    cel_ProgramPtr prog(cel_Runtime_Compile(rt == nullptr ? rt_.get() : rt,
                                            ast.get(), nullptr, status()));
    ABSL_CHECK(prog != nullptr) << cel_Status_ToAbsl(status());
    return prog;
  }

  // Like `Parse`, but returns the planning status instead of crashing when
  // planning fails.
  absl::Status ParseStatus(absl::string_view content) {
    cel_AstPtr ast(_cel_TestingParser_Parse(ABSL_DIE_IF_NULL(parser_.get()),
                                            cel_StringView_FromAbsl(content),
                                            arena()));
    cel_ProgramPtr prog(
        cel_Runtime_Compile(rt_.get(), ast.get(), nullptr, status()));
    absl::Status result = cel_Status_ToAbsl(status());
    ABSL_CHECK_EQ(prog == nullptr, !result.ok());
    cel_Status_Clear(status());
    return result;
  }

  cel_ProgramPtr Compile(absl::string_view content) {
    cel_AstPtr ast(_cel_TestingCompiler_Compile(
        ABSL_DIE_IF_NULL(compiler_.get()), cel_StringView_FromAbsl(content),
        arena()));
    cel_ProgramPtr prog(
        cel_Runtime_Compile(rt_.get(), ast.get(), nullptr, status()));
    ABSL_CHECK(prog != nullptr) << cel_Status_ToAbsl(status());
    return prog;
  }

  cel_ActivationPtr Activate(const cel_Program* prog,
                             const cel_VariableResolver* var_resolver) {
    cel_ActivationPtr act(cel_Program_Activate(prog, var_resolver, nullptr));
    ABSL_CHECK(act != nullptr) << cel_Status_ToAbsl(status());
    return act;
  }

  cel_Value Execute(cel_Activation* act) {
    cel_Value result;
    ABSL_CHECK(cel_Activation_Execute(act, &result, arena(), status()))
        << cel_Status_ToAbsl(status());
    ABSL_CHECK(cel_Status_Ok(status()));
    return result;
  }

 private:
  cel_Status status_;
  cel_ArenaPtr arena_;
  cel_RuntimePtr rt_;
  _cel_TestingParserPtr parser_;
  _cel_TestingCompilerPtr compiler_;
  cel_ValueContext context_;
};

class RuntimeTest : public ::testing::Test, public RuntimeHarness {
 public:
  void SetUp() override { RuntimeHarness::SetUp(); }

  void TearDown() override { RuntimeHarness::TearDown(); }
};

TEST_F(RuntimeTest, NullConst) {
  cel_ProgramPtr prog = Parse(R"cel(null)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsNull(&value));
}

TEST_F(RuntimeTest, FalseConst) {
  cel_ProgramPtr prog = Parse(R"cel(false)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsFalse(&value));
}

TEST_F(RuntimeTest, TrueConst) {
  cel_ProgramPtr prog = Parse(R"cel(true)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsTrue(&value));
}

TEST_F(RuntimeTest, IntConst) {
  cel_ProgramPtr prog = Parse(R"cel(1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, UintConst) {
  cel_ProgramPtr prog = Parse(R"cel(1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 1);
}

TEST_F(RuntimeTest, DoubleConst) {
  cel_ProgramPtr prog = Parse(R"cel(1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1);
}

TEST_F(RuntimeTest, BytesConst) {
  cel_ProgramPtr prog = Parse(R"cel(b'foo')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBytes(&value));
  EXPECT_EQ(cel_Value_GetBytes(&value), cel_StringView_From("foo"));
}

TEST_F(RuntimeTest, StringConst) {
  cel_ProgramPtr prog = Parse(R"cel('foo')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("foo"));
}

TEST_F(RuntimeTest, Ident) {
  cel_ProgramPtr prog = Parse(R"cel(foo)cel");
  _cel_TestingVariableResolver var_resolver({{"foo", cel_NullValue}});
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsNull(&value));
}

cel_Value IntValue(int64_t val) {
  cel_Value value;
  cel_Value_SetInt(&value, val);
  return value;
}

cel_Value DoubleValue(double val) {
  cel_Value value;
  cel_Value_SetDouble(&value, val);
  return value;
}

TEST_F(RuntimeTest, ContIdent) {
  cel_RuntimeOptions rt_opts;
  cel_RuntimeOptions_Default(&rt_opts);
  rt_opts.container = cel_StringView_FromString("bar.baz");
  cel_RuntimePtr rt = NewRuntime(&rt_opts);
  cel_ProgramPtr prog = Parse(R"cel(foo)cel", rt.get());
  _cel_TestingVariableResolver var_resolver({{"bar.baz.foo", IntValue(3)},
                                             {"bar.foo", IntValue(2)},
                                             {"foo", IntValue(1)}});
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);

  var_resolver.bindings.erase("bar.baz.foo");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  var_resolver.bindings.erase("bar.foo");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, NspaceVarIdent) {
  upb_Message* foo_msg =
      ParseTextProto("cel.expr.conformance.proto3.TestAllTypes",
                     R"pb(map_string_struct: {
                            key: "qux"
                            value: {
                              fields: {
                                key: "quux"
                                value: { number_value: 1.0 }
                              }
                            }
                          })pb");
  cel_Value foo_val;
  const upb_FieldDef* foo_field =
      FieldDef("cel.expr.conformance.proto3.TestAllTypes", "map_string_struct");
  _cel_ParsedMapFieldValue_Set(
      cel_Value_SetMap(&foo_val),
      upb_Message_GetFieldByDef(foo_msg, foo_field).map_val, foo_field);

  upb_Message* foo_qux_msg =
      ParseTextProto("cel.expr.conformance.proto3.TestAllTypes",
                     R"pb(map_string_double: { key: "quux" value: 2.0 })pb");
  cel_Value foo_qux_val;
  const upb_FieldDef* foo_qux_field =
      FieldDef("cel.expr.conformance.proto3.TestAllTypes", "map_string_double");
  _cel_ParsedMapFieldValue_Set(
      cel_Value_SetMap(&foo_qux_val),
      upb_Message_GetFieldByDef(foo_qux_msg, foo_qux_field).map_val,
      foo_qux_field);

  cel_ProgramPtr prog = Parse(R"cel(foo.qux.quux)cel");
  _cel_TestingVariableResolver var_resolver({{"foo.qux.quux", DoubleValue(3.0)},
                                             {"foo.qux", foo_qux_val},
                                             {"foo", foo_val}});
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 3);

  var_resolver.bindings.erase("foo.qux.quux");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 2);

  var_resolver.bindings.erase("foo.qux");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1);
}

TEST_F(RuntimeTest, NspaceVarContIdent) {
  cel_RuntimeOptions rt_opts;
  cel_RuntimeOptions_Default(&rt_opts);
  rt_opts.container = cel_StringView_FromString("bar.baz");
  cel_RuntimePtr rt = NewRuntime(&rt_opts);
  upb_Message* bar_baz_foo_msg =
      ParseTextProto("cel.expr.conformance.proto3.TestAllTypes",
                     R"pb(map_string_struct: {
                            key: "qux"
                            value: {
                              fields: {
                                key: "quux"
                                value: { number_value: 7.0 }
                              }
                            }
                          })pb");
  upb_Message* bar_foo_msg =
      ParseTextProto("cel.expr.conformance.proto3.TestAllTypes",
                     R"pb(map_string_struct: {
                            key: "qux"
                            value: {
                              fields: {
                                key: "quux"
                                value: { number_value: 4.0 }
                              }
                            }
                          })pb");
  upb_Message* foo_msg =
      ParseTextProto("cel.expr.conformance.proto3.TestAllTypes",
                     R"pb(map_string_struct: {
                            key: "qux"
                            value: {
                              fields: {
                                key: "quux"
                                value: { number_value: 1.0 }
                              }
                            }
                          })pb");
  cel_Value bar_baz_foo_val;
  cel_Value bar_foo_val;
  cel_Value foo_val;
  const upb_FieldDef* foo_field =
      FieldDef("cel.expr.conformance.proto3.TestAllTypes", "map_string_struct");
  _cel_ParsedMapFieldValue_Set(
      cel_Value_SetMap(&bar_baz_foo_val),
      upb_Message_GetFieldByDef(bar_baz_foo_msg, foo_field).map_val, foo_field);
  _cel_ParsedMapFieldValue_Set(
      cel_Value_SetMap(&bar_foo_val),
      upb_Message_GetFieldByDef(bar_foo_msg, foo_field).map_val, foo_field);
  _cel_ParsedMapFieldValue_Set(
      cel_Value_SetMap(&foo_val),
      upb_Message_GetFieldByDef(foo_msg, foo_field).map_val, foo_field);

  upb_Message* bar_baz_foo_qux_msg =
      ParseTextProto("cel.expr.conformance.proto3.TestAllTypes",
                     R"pb(map_string_double: { key: "quux" value: 8.0 })pb");
  upb_Message* bar_foo_qux_msg =
      ParseTextProto("cel.expr.conformance.proto3.TestAllTypes",
                     R"pb(map_string_double: { key: "quux" value: 5.0 })pb");
  upb_Message* foo_qux_msg =
      ParseTextProto("cel.expr.conformance.proto3.TestAllTypes",
                     R"pb(map_string_double: { key: "quux" value: 2.0 })pb");
  cel_Value bar_baz_foo_qux_val;
  cel_Value bar_foo_qux_val;
  cel_Value foo_qux_val;
  const upb_FieldDef* foo_qux_field =
      FieldDef("cel.expr.conformance.proto3.TestAllTypes", "map_string_double");
  _cel_ParsedMapFieldValue_Set(
      cel_Value_SetMap(&bar_baz_foo_qux_val),
      upb_Message_GetFieldByDef(bar_baz_foo_qux_msg, foo_qux_field).map_val,
      foo_qux_field);
  _cel_ParsedMapFieldValue_Set(
      cel_Value_SetMap(&bar_foo_qux_val),
      upb_Message_GetFieldByDef(bar_foo_qux_msg, foo_qux_field).map_val,
      foo_qux_field);
  _cel_ParsedMapFieldValue_Set(
      cel_Value_SetMap(&foo_qux_val),
      upb_Message_GetFieldByDef(foo_qux_msg, foo_qux_field).map_val,
      foo_qux_field);

  cel_ProgramPtr prog = Parse(R"cel(foo.qux.quux)cel", rt.get());
  _cel_TestingVariableResolver var_resolver(
      {{"bar.baz.foo.qux.quux", DoubleValue(9.0)},
       {"bar.baz.foo.qux", bar_baz_foo_qux_val},
       {"bar.baz.foo", bar_baz_foo_val},
       {"bar.foo.qux.quux", DoubleValue(6.0)},
       {"bar.foo.qux", bar_foo_qux_val},
       {"bar.foo", bar_foo_val},
       {"foo.qux.quux", DoubleValue(3.0)},
       {"foo.qux", foo_qux_val},
       {"foo", foo_val}});
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 9);

  var_resolver.bindings.erase("bar.baz.foo.qux.quux");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 6);

  var_resolver.bindings.erase("bar.foo.qux.quux");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 3);

  var_resolver.bindings.erase("foo.qux.quux");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 8);

  var_resolver.bindings.erase("bar.baz.foo.qux");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 5);

  var_resolver.bindings.erase("bar.foo.qux");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 2);

  var_resolver.bindings.erase("foo.qux");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 7);

  var_resolver.bindings.erase("bar.baz.foo");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 4);

  var_resolver.bindings.erase("bar.foo");
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1);
}

TEST_F(RuntimeTest, IntAdd) {
  cel_ProgramPtr prog = Parse(R"cel(1 + 1 + 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);
}

TEST_F(RuntimeTest, UintAdd) {
  cel_ProgramPtr prog = Parse(R"cel(1u + 1u + 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 3);
}

TEST_F(RuntimeTest, DoubleAdd) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 + 1.0 + 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 3.0);
}

TEST_F(RuntimeTest, BytesAdd) {
  cel_ProgramPtr prog = Parse(R"cel(b'Hello' + b' ' + b'World!')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBytes(&value));
  EXPECT_EQ(cel_Value_GetBytes(&value),
            cel_StringView_FromString("Hello World!"));
}

TEST_F(RuntimeTest, StringAdd) {
  cel_ProgramPtr prog = Parse(R"cel('Hello' + ' ' + 'World!')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value),
            cel_StringView_FromString("Hello World!"));
}

TEST_F(RuntimeTest, CallBool) {
  cel_ProgramPtr prog = Parse(R"cel(bool("true"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(bool("false"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(bool("1"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, CallInt) {
  cel_ProgramPtr prog = Parse(R"cel(int("123"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 123);

  prog = Parse(R"cel(int("-123"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -123);

  prog = Parse(R"cel(int("abc"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, CallUint) {
  cel_ProgramPtr prog = Parse(R"cel(uint("123"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 123u);

  prog = Parse(R"cel(uint("-123"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel(uint("abc"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, CallDouble) {
  cel_ProgramPtr prog = Parse(R"cel(double("1.23"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1.23);

  prog = Parse(R"cel(double("-1.23"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), -1.23);

  prog = Parse(R"cel(double("abc"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, CallBytes) {
  cel_ProgramPtr prog = Parse(R"cel(bytes("hello"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBytes(&value));
  EXPECT_EQ(cel_Value_GetBytes(&value), cel_StringView_From("hello"));

  prog = Parse(R"cel(bytes(b"hello"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBytes(&value));
  EXPECT_EQ(cel_Value_GetBytes(&value), cel_StringView_From("hello"));
}

TEST_F(RuntimeTest, CallString) {
  cel_ProgramPtr prog = Parse(R"cel(string(true))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("true"));

  prog = Parse(R"cel(string(123))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  cel_StringView got = cel_Value_GetString(&value);
  cel_StringView want = cel_StringView_From("123");
  EXPECT_EQ(got, want);

  prog = Parse(R"cel(string(123u))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("123"));

  prog = Parse(R"cel(string(1.23))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("1.23"));

  prog = Parse(R"cel(string(b"hello"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("hello"));

  prog = Parse(R"cel(string(timestamp("2025-12-01T10:00:00Z")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value),
            cel_StringView_From("2025-12-01T10:00:00Z"));

  prog = Parse(R"cel(string(duration("1h")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("3600s"));
}

TEST_F(RuntimeTest, CallTimestamp) {
  cel_ProgramPtr prog = Parse(R"cel(timestamp("2025-12-01T10:00:00Z"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsTimestamp(&value));
  EXPECT_EQ(cel_Timestamp_ToUnixSeconds(cel_Value_GetTimestamp(&value)),
            1764583200);

  prog = Parse(R"cel(timestamp("invalid"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, CallDuration) {
  cel_ProgramPtr prog = Parse(R"cel(duration("1h"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDuration(&value));
  EXPECT_EQ(cel_Duration_ToUnixSeconds(cel_Value_GetDuration(&value)), 3600);

  prog = Parse(R"cel(duration("invalid"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, CallGetHoursDuration) {
  cel_ProgramPtr prog = Parse(R"cel(duration("10h20m30.456s").getHours())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 10);

  prog = Parse(R"cel(getHours(duration("10h20m30.456s")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 10);

  prog = Parse(R"cel(duration("59m59.999s").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  prog = Parse(R"cel(duration("0s").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Negative durations truncate towards zero.
  prog = Parse(R"cel(duration("-10h20m30.456s").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -10);

  prog = Parse(R"cel(duration("-59m59.999s").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Duration bounds.
  prog = Parse(R"cel(duration("315576000000s").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 87660000);

  prog = Parse(R"cel(duration("-315576000000s").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -87660000);
}

TEST_F(RuntimeTest, CallGetMinutesDuration) {
  cel_ProgramPtr prog =
      Parse(R"cel(duration("10h20m30.456s").getMinutes())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 620);

  prog = Parse(R"cel(duration("59.999s").getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Negative durations truncate towards zero.
  prog = Parse(R"cel(duration("-10h20m30.456s").getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -620);

  // Duration bounds.
  prog = Parse(R"cel(duration("315576000000s").getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 5259600000);

  prog = Parse(R"cel(duration("-315576000000s").getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -5259600000);
}

TEST_F(RuntimeTest, CallGetSecondsDuration) {
  cel_ProgramPtr prog =
      Parse(R"cel(duration("10h20m30.456s").getSeconds())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 37230);

  prog = Parse(R"cel(duration("0.999s").getSeconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Negative durations truncate towards zero.
  prog = Parse(R"cel(duration("-10h20m30.456s").getSeconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -37230);

  // Duration bounds.
  prog = Parse(R"cel(duration("315576000000s").getSeconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 315576000000);

  prog = Parse(R"cel(duration("-315576000000s").getSeconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -315576000000);
}

TEST_F(RuntimeTest, CallGetMillisecondsDuration) {
  cel_ProgramPtr prog =
      Parse(R"cel(duration("10h20m30.456s").getMilliseconds())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 456);

  prog = Parse(R"cel(duration("1.5s").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 500);

  prog = Parse(R"cel(duration("0.0005s").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  prog = Parse(R"cel(duration("0s").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Negative durations truncate towards zero.
  prog = Parse(R"cel(duration("-10h20m30.456s").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -456);

  prog = Parse(R"cel(duration("-1.5s").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -500);

  // Duration bounds.
  prog = Parse(R"cel(duration("-315576000000s").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);
}

TEST_F(RuntimeTest, CallGetHoursTimestamp) {
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getHours())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 10);

  prog = Parse(R"cel(getHours(timestamp("2025-12-01T10:20:30.456Z")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 10);

  prog =
      Parse(R"cel(timestamp("2025-12-01T23:59:59.999999999Z").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 23);

  prog = Parse(R"cel(timestamp("1970-01-01T00:00:00Z").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Non-UTC offsets are normalized to UTC.
  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30+05:30").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 4);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T22:58:57Z").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 22);

  prog = Parse(R"cel(timestamp("0001-01-01T01:02:03Z").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog =
      Parse(R"cel(timestamp("9999-12-31T23:59:59.999999999Z").getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 23);
}

TEST_F(RuntimeTest, CallGetMinutesTimestamp) {
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getMinutes())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 20);

  prog = Parse(
      R"cel(timestamp("2025-12-01T23:59:59.999999999Z").getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 59);

  // Non-UTC offsets are normalized to UTC.
  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30+05:30").getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 50);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T22:58:57Z").getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 58);

  prog = Parse(R"cel(timestamp("0001-01-01T01:02:03Z").getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);
}

TEST_F(RuntimeTest, CallGetSecondsTimestamp) {
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getSeconds())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 30);

  prog = Parse(
      R"cel(timestamp("2025-12-01T23:59:59.999999999Z").getSeconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 59);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T22:58:57Z").getSeconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 57);

  prog = Parse(R"cel(timestamp("0001-01-01T01:02:03Z").getSeconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);
}

TEST_F(RuntimeTest, CallGetMillisecondsTimestamp) {
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getMilliseconds())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 456);

  prog = Parse(
      R"cel(timestamp("2025-12-01T23:59:59.999999999Z").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 999);

  prog = Parse(R"cel(timestamp("1970-01-01T00:00:00Z").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T22:58:57Z").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  prog = Parse(R"cel(timestamp("0001-01-01T01:02:03Z").getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);
}

TEST_F(RuntimeTest, CallGetHoursTimestampWithTimeZone) {
  // Empty and UTC time zones match the single argument overloads.
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getHours(""))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 10);

  prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getHours("UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 10);

  prog =
      Parse(R"cel(getHours(timestamp("2025-12-01T10:20:30.456Z"), "UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 10);

  // IANA time zones.
  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getHours("America/Los_Angeles"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(
      R"cel(timestamp("2025-07-01T10:20:30.456Z").getHours("America/Los_Angeles"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);

  prog = Parse(
      R"cel(timestamp("2025-12-01T02:00:00Z").getHours("America/New_York"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 21);

  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getHours("Asia/Kolkata"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 15);

  // Fixed UTC offsets.
  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getHours("+05:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 15);

  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getHours("-08:00"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getHours("-01:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 8);

  // Time zones before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T23:59:58Z").getHours("+05:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 5);
}

TEST_F(RuntimeTest, CallGetMinutesTimestampWithTimeZone) {
  // IANA time zones.
  cel_ProgramPtr prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getMinutes("Asia/Kolkata"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 50);

  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getMinutes("Asia/Kathmandu"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 5);

  // Fixed UTC offsets.
  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getMinutes("+05:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 50);

  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getMinutes("-01:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 50);
}

TEST_F(RuntimeTest, CallGetSecondsTimestampWithTimeZone) {
  // IANA time zones.
  cel_ProgramPtr prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getSeconds("Asia/Kolkata"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 30);

  // Fixed UTC offsets.
  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getSeconds("-01:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 30);
}

TEST_F(RuntimeTest, CallGetMillisecondsTimestampWithTimeZone) {
  // IANA time zones.
  cel_ProgramPtr prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getMilliseconds("Asia/Kolkata"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 456);

  // Fixed UTC offsets.
  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30.456Z").getMilliseconds("-01:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 456);

  // Time zones before the Unix epoch.
  prog = Parse(
      R"cel(timestamp("1969-12-31T23:59:58Z").getMilliseconds("+05:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);
}

TEST_F(RuntimeTest, CallGetFullYearTimestamp) {
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getFullYear())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2025);

  prog = Parse(R"cel(getFullYear(timestamp("2025-12-01T10:20:30.456Z")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2025);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T23:59:59Z").getFullYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1969);

  // Timestamp bounds.
  prog = Parse(R"cel(timestamp("0001-01-01T00:00:00Z").getFullYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(
      R"cel(timestamp("9999-12-31T23:59:59.999999999Z").getFullYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 9999);
}

TEST_F(RuntimeTest, CallGetMonthTimestamp) {
  // Months are zero-based.
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getMonth())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 11);

  prog = Parse(R"cel(getMonth(timestamp("2025-01-31T23:59:59Z")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T23:59:59Z").getMonth())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 11);

  // Timestamp bounds.
  prog = Parse(R"cel(timestamp("0001-01-01T00:00:00Z").getMonth())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);
}

TEST_F(RuntimeTest, CallGetDateTimestamp) {
  // Dates are one-based.
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getDate())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel(getDate(timestamp("2025-01-31T23:59:59Z")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 31);

  // Leap day.
  prog = Parse(R"cel(timestamp("2024-02-29T12:00:00Z").getDate())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 29);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T23:59:59Z").getDate())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 31);
}

TEST_F(RuntimeTest, CallGetDayOfMonthTimestamp) {
  // Days of the month are zero-based.
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getDayOfMonth())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  prog = Parse(R"cel(getDayOfMonth(timestamp("2025-01-31T23:59:59Z")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 30);

  // Leap day.
  prog = Parse(R"cel(timestamp("2024-02-29T12:00:00Z").getDayOfMonth())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 28);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T23:59:59Z").getDayOfMonth())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 30);
}

TEST_F(RuntimeTest, CallGetDayOfWeekTimestamp) {
  // Monday.
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getDayOfWeek())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel(getDayOfWeek(timestamp("2025-12-01T10:20:30.456Z")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  // Days of the week start on Sunday.
  prog = Parse(R"cel(timestamp("2025-11-30T23:59:59Z").getDayOfWeek())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Saturday.
  prog = Parse(R"cel(timestamp("2025-12-06T00:00:00Z").getDayOfWeek())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 6);

  // Thursday.
  prog = Parse(R"cel(timestamp("1970-01-01T00:00:00Z").getDayOfWeek())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 4);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T23:59:59Z").getDayOfWeek())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);

  // Timestamp bounds.
  prog = Parse(R"cel(timestamp("0001-01-01T00:00:00Z").getDayOfWeek())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(
      R"cel(timestamp("9999-12-31T23:59:59.999999999Z").getDayOfWeek())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 5);
}

TEST_F(RuntimeTest, CallGetDayOfYearTimestamp) {
  // Days of the year are zero-based.
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30.456Z").getDayOfYear())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 334);

  prog = Parse(R"cel(getDayOfYear(timestamp("2025-01-01T00:00:00Z")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  prog = Parse(R"cel(timestamp("2025-12-31T23:59:59Z").getDayOfYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 364);

  // Leap years.
  prog = Parse(R"cel(timestamp("2024-03-01T00:00:00Z").getDayOfYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 60);

  prog = Parse(R"cel(timestamp("2024-12-31T23:59:59Z").getDayOfYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 365);

  // Timestamps before the Unix epoch.
  prog = Parse(R"cel(timestamp("1969-12-31T23:59:59Z").getDayOfYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 364);
}

TEST_F(RuntimeTest, CallDateAccessorsTimestampWithTimeZone) {
  // Empty and UTC time zones match the single argument overloads.
  cel_ProgramPtr prog =
      Parse(R"cel(timestamp("2025-12-01T02:00:00Z").getDate(""))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel(timestamp("2025-12-01T02:00:00Z").getDate("UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel(getDate(timestamp("2025-12-01T02:00:00Z"), "UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  // IANA time zones crossing the day and month boundary.
  prog = Parse(
      R"cel(timestamp("2025-12-01T02:00:00Z").getFullYear("America/Los_Angeles"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2025);

  prog = Parse(
      R"cel(timestamp("2025-12-01T02:00:00Z").getMonth("America/Los_Angeles"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 10);

  prog = Parse(
      R"cel(timestamp("2025-12-01T02:00:00Z").getDate("America/Los_Angeles"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 30);

  prog = Parse(
      R"cel(timestamp("2025-12-01T02:00:00Z").getDayOfMonth("America/Los_Angeles"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 29);

  prog = Parse(
      R"cel(timestamp("2025-12-01T02:00:00Z").getDayOfWeek("America/Los_Angeles"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  prog = Parse(
      R"cel(timestamp("2025-12-01T02:00:00Z").getDayOfYear("America/Los_Angeles"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 333);

  // IANA time zones crossing the year boundary backwards, into a leap year.
  prog = Parse(
      R"cel(timestamp("2025-01-01T03:00:00Z").getFullYear("America/New_York"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2024);

  prog = Parse(
      R"cel(timestamp("2025-01-01T03:00:00Z").getMonth("America/New_York"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 11);

  prog = Parse(
      R"cel(timestamp("2025-01-01T03:00:00Z").getDate("America/New_York"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 31);

  prog = Parse(
      R"cel(timestamp("2025-01-01T03:00:00Z").getDayOfWeek("America/New_York"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(
      R"cel(timestamp("2025-01-01T03:00:00Z").getDayOfYear("America/New_York"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 365);

  // IANA time zones crossing the year boundary forwards.
  prog = Parse(
      R"cel(timestamp("2025-12-31T20:00:00Z").getFullYear("Asia/Tokyo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2026);

  prog = Parse(
      R"cel(timestamp("2025-12-31T20:00:00Z").getMonth("Asia/Tokyo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  prog =
      Parse(R"cel(timestamp("2025-12-31T20:00:00Z").getDate("Asia/Tokyo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(
      R"cel(timestamp("2025-12-31T20:00:00Z").getDayOfMonth("Asia/Tokyo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  prog = Parse(
      R"cel(timestamp("2025-12-31T20:00:00Z").getDayOfWeek("Asia/Tokyo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 4);

  prog = Parse(
      R"cel(timestamp("2025-12-31T20:00:00Z").getDayOfYear("Asia/Tokyo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);

  // Fixed UTC offsets.
  prog = Parse(R"cel(timestamp("2025-12-01T20:00:00Z").getDate("+05:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(
      R"cel(timestamp("2025-12-01T20:00:00Z").getDayOfWeek("+05:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(R"cel(timestamp("2025-12-01T02:00:00Z").getDate("-08:00"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 30);

  prog = Parse(R"cel(timestamp("2024-02-28T20:00:00Z").getDate("+05:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 29);

  prog = Parse(
      R"cel(timestamp("2024-02-28T20:00:00Z").getDayOfYear("+05:30"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 59);

  // Time zones before the Unix epoch.
  prog =
      Parse(R"cel(timestamp("1970-01-01T02:00:00Z").getFullYear("-05:00"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1969);

  prog = Parse(
      R"cel(timestamp("1970-01-01T02:00:00Z").getDayOfWeek("-05:00"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);
}

TEST_F(RuntimeTest, CallTimeAccessorsNoSuchOverload) {
  cel_ProgramPtr prog = Parse(R"cel(getHours(1))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(getMinutes("foo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(getSeconds(1.0))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(getMilliseconds(true))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(1.getHours())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel("foo".getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  // Durations do not have a time zone overload.
  prog = Parse(R"cel(duration("1h").getHours("UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(getMinutes(duration("1h"), "UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  // The time zone must be a string.
  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getSeconds(1))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(1.getMilliseconds("UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));
}

TEST_F(RuntimeTest, CallTimeAccessorsInvalidTimeZone) {
  cel_ProgramPtr prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30Z").getHours("Invalid/Zone"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));

  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getMinutes("foo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));

  prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getSeconds("+aa:bb"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));

  prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getMilliseconds(":"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));
}

TEST_F(RuntimeTest, CallTimeAccessorsPropagatesError) {
  cel_ProgramPtr prog = Parse(R"cel(getHours(1 / 0))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel((1 / 0).getMinutes())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel((1 / 0).getSeconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel((1 / 0).getMilliseconds())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  // Errors in either argument of the time zone overload propagate.
  prog = Parse(R"cel((1 / 0).getHours("UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getHours(1 / 0))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel(duration("1h").getMinutes(1 / 0))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel(true.getSeconds(1 / 0))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));
}

TEST_F(RuntimeTest, CallDateAccessorsNoSuchOverload) {
  // Durations do not support the date accessors.
  cel_ProgramPtr prog = Parse(R"cel(duration("1h").getFullYear())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(getMonth(duration("1h")))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(duration("1h").getDate())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(duration("1h").getDayOfMonth())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(duration("1h").getDayOfWeek())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(duration("1h").getDayOfYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(duration("1h").getDayOfYear("UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(getFullYear(1))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel("foo".getMonth())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  prog = Parse(R"cel(getDate(1.0, "UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));

  // The time zone must be a string.
  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getDayOfWeek(1))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: no such overload"));
}

TEST_F(RuntimeTest, CallDateAccessorsInvalidTimeZone) {
  cel_ProgramPtr prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30Z").getFullYear("Invalid/Zone"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));

  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getMonth("foo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));

  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getDate("+aa:bb"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));

  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getDayOfMonth(":"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));

  prog =
      Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getDayOfWeek("foo"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));

  prog = Parse(
      R"cel(timestamp("2025-12-01T10:20:30Z").getDayOfYear("Invalid/Zone"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: invalid time zone"));
}

TEST_F(RuntimeTest, CallDateAccessorsPropagatesError) {
  cel_ProgramPtr prog = Parse(R"cel(getFullYear(1 / 0))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel((1 / 0).getMonth())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel((1 / 0).getDate())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel((1 / 0).getDayOfMonth())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel((1 / 0).getDayOfWeek())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel((1 / 0).getDayOfYear())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  // Errors in either argument of the time zone overload propagate.
  prog = Parse(R"cel((1 / 0).getDayOfYear("UTC"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel(timestamp("2025-12-01T10:20:30Z").getMonth(1 / 0))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel(duration("1h").getDate(1 / 0))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));

  prog = Parse(R"cel(true.getDayOfWeek(1 / 0))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));
}

TEST_F(RuntimeTest, CallDyn) {
  cel_ProgramPtr prog = Parse(R"cel(dyn(1))cel");
  _cel_TestingVariableResolver var_resolver({{"foo", IntValue(42)}});
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel(dyn(2u))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 2u);

  prog = Parse(R"cel(dyn(null))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsNull(&value));

  prog = Parse(R"cel(dyn("foo") + "bar")cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("foobar"));

  prog = Parse(R"cel(dyn(dyn(true)))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(dyn(foo) + 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 43);

  prog = Parse(R"cel(dyn([1, 2]) == [1, 2])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(dyn(false) ? 1 : dyn(2))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);
}

TEST_F(RuntimeTest, CallDynInControlFlow) {
  cel_ProgramPtr prog = Parse(R"cel(dyn(false) && (1 / 0 == 1))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(dyn(true) || (1 / 0 == 1))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(true ? dyn(1) : 2)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel({dyn("k"): dyn(1)}["k"])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel([dyn(1), 2][0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel(cel.bind(x, dyn(2), x * x))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 4);

  prog = Parse(R"cel(cel.bind(x, 2, dyn(x) * dyn(x)))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 4);
}

TEST_F(RuntimeTest, CallDynRequiresSingleGlobalArg) {
  EXPECT_THAT(ParseStatus(R"cel(dyn())cel"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("function with name dyn")));
  EXPECT_THAT(ParseStatus(R"cel(dyn(1, 2))cel"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("function with name dyn")));
  EXPECT_THAT(ParseStatus(R"cel(1.dyn())cel"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("function with name dyn")));
  EXPECT_THAT(ParseStatus(R"cel(1.dyn(2))cel"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("function with name dyn")));
}

TEST_F(RuntimeTest, CallDynPropagatesError) {
  cel_ProgramPtr prog = Parse(R"cel(dyn(1 / 0))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_StringView_From("cel: divide by zero"));
}

TEST_F(RuntimeTest, CallDynChecked) {
  cel_ProgramPtr prog = Compile(R"cel(dyn(1) + 2)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);
}

// TODO: Add addition tests for duration/timestamp

TEST_F(RuntimeTest, IntSub) {
  cel_ProgramPtr prog = Parse(R"cel(4 - 2 - 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, UintSub) {
  cel_ProgramPtr prog = Parse(R"cel(4u - 2u - 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 1);
}

TEST_F(RuntimeTest, DoubleSub) {
  cel_ProgramPtr prog = Parse(R"cel(4.0 - 2.0 - 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1.0);
}

// TODO: Add subtraction tests for duration/timestamp

TEST_F(RuntimeTest, IntMul) {
  cel_ProgramPtr prog = Parse(R"cel(4 * 2 * 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 8);
}

TEST_F(RuntimeTest, UintMul) {
  cel_ProgramPtr prog = Parse(R"cel(4u * 2u * 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 8);
}

TEST_F(RuntimeTest, DoubleMul) {
  cel_ProgramPtr prog = Parse(R"cel(4.0 * 2.0 * 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 8.0);
}

TEST_F(RuntimeTest, DivByZero) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel(4 / 0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, DivByZeroUint) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel(4u / 0u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, DivByZeroDouble) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel(4.0 / 0.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, MismatchedTypes) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: no such overload"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel(1u / 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  cel_ProgramPtr prog2 = Parse(R"cel(1.0 / 1)cel");
  cel_ActivationPtr act2(Activate(prog2.get(), &var_resolver));
  value = Execute(act2.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  cel_ProgramPtr prog3 = Parse(R"cel(1.0 / 1u)cel");
  cel_ActivationPtr act3(Activate(prog3.get(), &var_resolver));
  value = Execute(act3.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, IntDiv) {
  cel_ProgramPtr prog = Parse(R"cel(4 / 2)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);
}

TEST_F(RuntimeTest, UintDiv) {
  cel_ProgramPtr prog = Parse(R"cel(4u / 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 2);
}

TEST_F(RuntimeTest, DoubleDiv) {
  cel_ProgramPtr prog = Parse(R"cel(4.0 / 2.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 2.0);
}

TEST_F(RuntimeTest, NegativeIntModTest) {
  cel_ProgramPtr prog = Parse(R"cel(-10 % 3)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -1);
}

TEST_F(RuntimeTest, IntModTest) {
  cel_ProgramPtr prog = Parse(R"cel(10 % 3)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, UintMod) {
  cel_ProgramPtr prog = Parse(R"cel(4u % 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 0);
}

TEST_F(RuntimeTest, IntModByZero) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: modulo by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel(4 % 0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  cel_ProgramPtr prog2 = Parse(R"cel(1.0 / 1)cel");
  cel_ActivationPtr act2(Activate(prog2.get(), &var_resolver));
  value = Execute(act2.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  cel_ProgramPtr prog3 = Parse(R"cel(1.0 / 1u)cel");
  cel_ActivationPtr act3(Activate(prog3.get(), &var_resolver));
  value = Execute(act3.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, LogicalNot) {
  cel_ProgramPtr prog = Parse(R"cel(!true)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntNeg) {
  cel_ProgramPtr prog = Parse(R"cel(-(1))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -1);
}

TEST_F(RuntimeTest, UintNeg) {
  cel_ProgramPtr prog = Parse(R"cel(-1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, DoubleNeg) {
  cel_ProgramPtr prog = Parse(R"cel(-1.0 - 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), -2.0);
}

TEST_F(RuntimeTest, LogicalAnd) {
  cel_ProgramPtr prog = Parse(R"cel(true && true)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(true && true || false)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(true && (true || false))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, LogicalOr) {
  cel_ProgramPtr prog = Parse(R"cel(true || true)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(true || true && false)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel((true || true) && false)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, LogicalAndWithError) {
  cel_ProgramPtr prog = Parse(R"cel(false && (1 / 0))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, LogicalOrWithError) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel(false || (1 / 0))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, BoolEquals) {
  cel_ProgramPtr prog = Parse(R"cel(true == true)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 == 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, UintEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1u == 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 == 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BytesEquals) {
  cel_ProgramPtr prog = Parse(R"cel(b'Hello' == b'Hello')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, StringEquals) {
  cel_ProgramPtr prog = Parse(R"cel('Hello' == 'Hello')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, ListEquals) {
  cel_ProgramPtr prog = Parse(R"cel([] == [])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(['a'] == ['a'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(['a', 'b'] == ['a', 'b'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel([1, 2u, 3.0] == [1u, 2.0, 3])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel([['a']] == [['a']])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(['a'] == ['b'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(['a', 'b'] == ['x', 'b'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(['a', 'b'] == ['a', 'x'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel([1, 2] == [3, 2])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel([['a']] == [['b']])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel([{'a': 1}] == [{'a': 2}])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(['a'] == ['a', 'b'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, MapEquals) {
  cel_ProgramPtr prog = Parse(R"cel({} == {})cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': 1} == {'a': 1})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': 1, 'b': 2} == {'b': 2, 'a': 1})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': 1} == {'a': 1u})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({1: 'a'} == {1u: 'a'})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({1u: 'a'} == {1: 'a'})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({1: 'a', 2u: 'b'} == {1u: 'a', 2: 'b'})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': 1} == {'a': 2})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': 1, 'b': 2} == {'a': 3, 'b': 2})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': 1, 'b': 2} == {'a': 1, 'b': 3})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': 1} == {'b': 1})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': ['x']} == {'a': ['y']})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': {'b': 1}} == {'a': {'b': 2}})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BoolNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(true != false)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(true != true)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 != 2)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 != 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, UintNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1u != 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1u != 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 != 2.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 != 1.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BytesNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(b'a' != b'b')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'a' != b'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, StringNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel('a' != 'b')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('a' != 'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, ListNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(['a', 'b'] != ['x', 'b'])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(['a'] != ['a'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, MapNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel({'a': 1} != {'a': 2})cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel({'a': 1} != {'a': 1})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntLess) {
  cel_ProgramPtr prog = Parse(R"cel(1 < 2)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2 < 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 < 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, UintLess) {
  cel_ProgramPtr prog = Parse(R"cel(1u < 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2u < 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1u < 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleLess) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 < 2.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2.0 < 1.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 < 1.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, StringLess) {
  cel_ProgramPtr prog = Parse(R"cel('a' < 'b')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('b' < 'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('a' < 'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntLessEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 <= 2)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 <= 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2 <= 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, UintLessEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1u <= 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1u <= 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2u <= 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleLessEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 <= 2.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 <= 1.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2.0 <= 1.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, StringLessEquals) {
  cel_ProgramPtr prog = Parse(R"cel('a' <= 'b')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('a' <= 'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('b' <= 'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntGreater) {
  cel_ProgramPtr prog = Parse(R"cel(2 > 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 > 2)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 > 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, UintGreater) {
  cel_ProgramPtr prog = Parse(R"cel(2u > 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1u > 2u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1u > 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleGreater) {
  cel_ProgramPtr prog = Parse(R"cel(2.0 > 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 > 2.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 > 1.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, StringGreater) {
  cel_ProgramPtr prog = Parse(R"cel('b' > 'a')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('a' > 'b')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('a' > 'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntGreaterEquals) {
  cel_ProgramPtr prog = Parse(R"cel(2 >= 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 >= 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 >= 2)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, UintGreaterEquals) {
  cel_ProgramPtr prog = Parse(R"cel(2u >= 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1u >= 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1u >= 2u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleGreaterEquals) {
  cel_ProgramPtr prog = Parse(R"cel(2.0 >= 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 >= 1.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 >= 2.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, StringGreaterEquals) {
  cel_ProgramPtr prog = Parse(R"cel('b' >= 'a')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('a' >= 'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('a' >= 'b')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BytesLess) {
  cel_ProgramPtr prog = Parse(R"cel(b'a' < b'b')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'b' < b'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'a' < b'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BytesLessEquals) {
  cel_ProgramPtr prog = Parse(R"cel(b'a' <= b'b')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'a' <= b'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'b' <= b'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BytesGreater) {
  cel_ProgramPtr prog = Parse(R"cel(b'b' > b'a')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'a' > b'b')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'a' > b'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BytesGreaterEquals) {
  cel_ProgramPtr prog = Parse(R"cel(b'b' >= b'a')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'a' >= b'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'a' >= b'b')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntDoubleEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 == 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntDoubleNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 != 1.1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntDoubleLess) {
  cel_ProgramPtr prog = Parse(R"cel(1 < 1.1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 < 1.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntDoubleLessEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 <= 1.1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.1 <= 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntDoubleGreater) {
  cel_ProgramPtr prog = Parse(R"cel(1.1 > 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 > 1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntDoubleGreaterEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1.1 >= 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 >= 1.1)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntUintEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 == 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 == 2u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntUintNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 != 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 != 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntUintLess) {
  cel_ProgramPtr prog = Parse(R"cel(1 < 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2 < 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 < 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntUintLessEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1 <= 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 <= 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntUintGreater) {
  cel_ProgramPtr prog = Parse(R"cel(2 > 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 > 2u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 > 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntUintGreaterEquals) {
  cel_ProgramPtr prog = Parse(R"cel(2 >= 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 >= 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1 >= 2u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleUintEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 == 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 == 2u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleUintNotEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 != 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 != 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleUintLess) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 < 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2.0 < 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 < 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleUintLessEquals) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 <= 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 <= 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2.0 <= 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleUintGreater) {
  cel_ProgramPtr prog = Parse(R"cel(2.0 > 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 > 2u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 > 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleUintGreaterEquals) {
  cel_ProgramPtr prog = Parse(R"cel(2.0 >= 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 >= 1u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(1.0 >= 2u)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, TernaryConditional) {
  cel_ProgramPtr prog = Parse(R"cel(1 < 2 ? 1 : 2)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, ComplexTernaryConditional) {
  cel_ProgramPtr prog = Parse(R"cel(1 > 2 ? 1 : 2 > 3 ? 3 : 4 > 5 ? 5 : 6)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 6);
}

TEST_F(RuntimeTest, TernaryConditionalDifferentTypes) {
  cel_ProgramPtr prog = Parse(R"cel(1 > 2 ? 1 : "2")cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("2"));

  cel_ProgramPtr prog2 = Parse(R"cel(true ? true : 1)cel");
  _cel_TestingVariableResolver var_resolver2;
  cel_ActivationPtr act2(Activate(prog2.get(), &var_resolver2));
  value = Execute(act2.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_EQ(cel_Value_GetBool(&value), true);

  cel_ProgramPtr prog3 = Parse(R"cel(false ? 1 : 1.0)cel");
  _cel_TestingVariableResolver var_resolver3;
  cel_ActivationPtr act3(Activate(prog3.get(), &var_resolver3));
  value = Execute(act3.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1.0);

  cel_ProgramPtr prog4 = Parse(R"cel(true ? 1.0 : "1.0")cel");
  _cel_TestingVariableResolver var_resolver4;
  cel_ActivationPtr act4(Activate(prog4.get(), &var_resolver4));
  value = Execute(act4.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1.0);
}

TEST_F(RuntimeTest, TernaryConditionalString) {
  cel_ProgramPtr prog = Parse(R"cel(true ? 'hello' : 'world')cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("hello"));

  prog = Parse(R"cel(false ? 'hello' : 'world')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("world"));
}

TEST_F(RuntimeTest, TernaryConditionalDouble) {
  cel_ProgramPtr prog = Parse(R"cel(true ? 1.0 : 2.5)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1.0);

  prog = Parse(R"cel(false ? 1.0 : 2.5)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 2.5);
}

TEST_F(RuntimeTest, TernaryConditionalBool) {
  cel_ProgramPtr prog = Parse(R"cel(true ? false : true)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(false ? false : true)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, TernaryConditionalError) {
  cel_ProgramPtr prog = Parse(R"cel(1.0 / 0.0 ? true : false)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel(true ? 1.0 / 0.0 : false)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel(false ? true : 1.0 / 0.0)cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, ListEmpty) {
  cel_ProgramPtr prog = Parse(R"cel([])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsList(&value));
  cel_Value size;
  ASSERT_TRUE(cel_ListValue_Size(cel_Value_GetList(&value), context(), &size,
                                 status()));
  ASSERT_TRUE(cel_Value_IsInt(&size));
  EXPECT_EQ(cel_Value_GetInt(&size), 0);
}

TEST_F(RuntimeTest, List) {
  cel_ProgramPtr prog = Parse(R"cel([true, 2, 3u, 4.0])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsList(&value));
  cel_Value size;
  ASSERT_TRUE(cel_ListValue_Size(cel_Value_GetList(&value), context(), &size,
                                 status()));
  ASSERT_TRUE(cel_Value_IsInt(&size));
  EXPECT_EQ(cel_Value_GetInt(&size), 4);

  cel_Value element;

  ASSERT_TRUE(cel_ListValue_Get(cel_Value_GetList(&value), context(), 0,
                                &element, status()));
  ASSERT_TRUE(cel_Value_IsBool(&element));
  EXPECT_TRUE(cel_Value_GetBool(&element));

  ASSERT_TRUE(cel_ListValue_Get(cel_Value_GetList(&value), context(), 1,
                                &element, status()));
  ASSERT_TRUE(cel_Value_IsInt(&element));
  EXPECT_EQ(cel_Value_GetInt(&element), 2);

  ASSERT_TRUE(cel_ListValue_Get(cel_Value_GetList(&value), context(), 2,
                                &element, status()));
  ASSERT_TRUE(cel_Value_IsUint(&element));
  EXPECT_EQ(cel_Value_GetUint(&element), 3);

  ASSERT_TRUE(cel_ListValue_Get(cel_Value_GetList(&value), context(), 3,
                                &element, status()));
  ASSERT_TRUE(cel_Value_IsDouble(&element));
  EXPECT_EQ(cel_Value_GetDouble(&element), 4);
}

TEST_F(RuntimeTest, ListError) {
  cel_ProgramPtr prog = Parse(R"cel([1.0 / 0.0, 2, 3u, 4.0])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel([false, 1.0 / 0.0, 3u, 4.0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel([false, 2, 1.0 / 0.0, 4.0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel([false, 2, 3u, 1.0 / 0.0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, MapEmpty) {
  cel_ProgramPtr prog = Parse(R"cel({})cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsMap(&value));
  cel_Value size;
  ASSERT_TRUE(
      cel_MapValue_Size(cel_Value_GetMap(&value), context(), &size, status()));
  ASSERT_TRUE(cel_Value_IsInt(&size));
  EXPECT_EQ(cel_Value_GetInt(&size), 0);
}

TEST_F(RuntimeTest, Map) {
  cel_ProgramPtr prog =
      Parse(R"cel({true: 2, 3: 4u, 5u: "foo", "bar": null})cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsMap(&value));
  cel_Value size;
  ASSERT_TRUE(
      cel_MapValue_Size(cel_Value_GetMap(&value), context(), &size, status()));
  ASSERT_TRUE(cel_Value_IsInt(&size));
  EXPECT_EQ(cel_Value_GetInt(&size), 4);

  cel_MapValueKey entry_key;
  cel_Value entry_value;

  cel_MapValueKey_SetBool(&entry_key, true);
  ASSERT_TRUE(cel_MapValue_Get(cel_Value_GetMap(&value), context(), &entry_key,
                               &entry_value, status()));
  ASSERT_TRUE(cel_Value_IsInt(&entry_value));
  EXPECT_EQ(cel_Value_GetInt(&entry_value), 2);

  cel_MapValueKey_SetInt(&entry_key, 3);
  ASSERT_TRUE(cel_MapValue_Get(cel_Value_GetMap(&value), context(), &entry_key,
                               &entry_value, status()));
  ASSERT_TRUE(cel_Value_IsUint(&entry_value));
  EXPECT_EQ(cel_Value_GetUint(&entry_value), 4);

  cel_MapValueKey_SetUint(&entry_key, 5);
  ASSERT_TRUE(cel_MapValue_Get(cel_Value_GetMap(&value), context(), &entry_key,
                               &entry_value, status()));
  ASSERT_TRUE(cel_Value_IsString(&entry_value));
  EXPECT_TRUE(cel_StringView_Equals(cel_Value_GetString(&entry_value),
                                    cel_StringView_From("foo")));

  cel_MapValueKey_SetString(&entry_key, cel_StringView_From("bar"));
  ASSERT_TRUE(cel_MapValue_Get(cel_Value_GetMap(&value), context(), &entry_key,
                               &entry_value, status()));
  ASSERT_TRUE(cel_Value_IsNull(&entry_value));
}

TEST_F(RuntimeTest, MapError) {
  cel_ProgramPtr prog = Parse(R"cel({1 / 0: 2, 3: 4u})cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel({true: 1 / 0, 3: 4u})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel({true: 2, 1 / 0: 4u})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel({true: 2, 3: 1 / 0})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel({1.0: 2})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  prog = Parse(R"cel({true: 2, 3.0: 4u})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, CallSize) {
  cel_ProgramPtr prog = Parse(R"cel("hello".size())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 5);

  prog = Parse(R"cel(size("hello"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 5);

  prog = Parse(R"cel(b"hello".size())cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 5);

  prog = Parse(R"cel(size(b"hello"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 5);

  prog = Parse(R"cel(size(1))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, CallSizeList) {
  cel_ProgramPtr prog = Parse(R"cel([1, 2, 3].size())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);

  prog = Parse(R"cel(size([1, 2, 3]))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);

  prog = Parse(R"cel(size([]))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);
}

TEST_F(RuntimeTest, CallSizeMap) {
  cel_ProgramPtr prog = Parse(R"cel({1: "a", 2: "b"}.size())cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(R"cel(size({1: "a", 2: "b"}))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(R"cel(size({}))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 0);
}

TEST_F(RuntimeTest, CallContainsString) {
  cel_ProgramPtr prog = Parse(R"cel("hello".contains("el"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallContainsNotString) {
  cel_ProgramPtr prog = Parse(R"cel("hello".contains("le"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallStartsWithString) {
  cel_ProgramPtr prog = Parse(R"cel("hello".startsWith("el"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallStartsWithNotString) {
  cel_ProgramPtr prog = Parse(R"cel("hello".startsWith("ello"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallEndsWithString) {
  cel_ProgramPtr prog = Parse(R"cel("hello".endsWith("lo"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallEndsWithNotString) {
  cel_ProgramPtr prog = Parse(R"cel("hello".endsWith("ell"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallRegexExpMatch) {
  cel_ProgramPtr prog = Parse(
      R"cel(" Hello  World!   ".matches("(\\s*)Hello(\\s+)World!(\\s*)"))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(
      R"cel(matches(" Hello  World!   ", "(\\s*)Hello(\\s+)World!(\\s*)"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, ListIndex) {
  cel_ProgramPtr prog =
      Parse(R"cel([true, 2, 3u, 4.0, 'foo', b'bar', null][0])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel([true, 2, 3u, 4.0, 'foo', b'bar', null][1])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(R"cel([true, 2, 3u, 4.0, 'foo', b'bar', null][2])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 3);

  prog = Parse(R"cel([true, 2, 3u, 4.0, 'foo', b'bar', null][3])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 4.0);

  prog = Parse(R"cel([true, 2, 3u, 4.0, 'foo', b'bar', null][4])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("foo"));

  prog = Parse(R"cel([true, 2, 3u, 4.0, 'foo', b'bar', null][5])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBytes(&value));
  EXPECT_EQ(cel_Value_GetBytes(&value), cel_StringView_From("bar"));

  prog = Parse(R"cel([true, 2, 3u, 4.0, 'foo', b'bar', null][6])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsNull(&value));

  prog = Parse(R"cel([1, 2, 3][1u])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);

  prog = Parse(R"cel([1, 2, 3][2.0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);
}

TEST_F(RuntimeTest, ListIndexOutOfRange) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: index out of range"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel([1, 2, 3][3])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  prog = Parse(R"cel([1, 2, 3][-1])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  prog = Parse(R"cel([1, 2, 3][3u])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  prog = Parse(R"cel([1, 2, 3][-1.0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  prog = Parse(R"cel([1, 2, 3][1.1])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  cel_Error_SetMessage(error, cel_StringView_From("cel: no such overload"));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, MapIndex) {
  cel_ProgramPtr prog = Parse(R"cel({'foo': 1}['foo'])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);

  prog = Parse(R"cel({1: 'bar'}[1])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("bar"));

  prog = Parse(R"cel({1u: 2.0}[1u])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 2.0);

  prog = Parse(R"cel({true: false}[true])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, MapIndexNoSuchKey) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: no such key in map"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
  cel_ProgramPtr prog = Parse(R"cel({'foo': 1}['bar'])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  prog = Parse(R"cel({1: 'bar'}[2])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  prog = Parse(R"cel({1u: 2.0}[2u])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  prog = Parse(R"cel({true: false}[false])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, InList) {
  // int
  cel_ProgramPtr prog = Parse(R"cel(1 in [1, 2, 3])cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(4 in [1, 2, 3])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // uint
  prog = Parse(R"cel(1u in [1u, 2u, 3u])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(4u in [1u, 2u, 3u])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // double
  prog = Parse(R"cel(1.0 in [1.0, 2.0, 3.0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(4.0 in [1.0, 2.0, 3.0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // bool
  prog = Parse(R"cel(true in [true, false])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(true in [false, false])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // string
  prog = Parse(R"cel('foo' in ['foo', 'bar'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('baz' in ['foo', 'bar'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // bytes
  prog = Parse(R"cel(b'foo' in [b'foo', b'bar'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(b'baz' in [b'foo', b'bar'])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // null
  prog = Parse(R"cel(null in [null])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(null in [1])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // heterogenous
  prog = Parse(R"cel(1 in [1.0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, InListError) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: no such overload"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel(1 in 1)cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  cel_Error* error2 = cel_Error_New(arena());
  cel_Error_SetMessage(error2, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error2, cel_ErrorCode_kInvalidArgument);
  prog = Parse(R"cel(1 / 0 in [1])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error2));

  prog = Parse(R"cel(1 in [1 / 0])cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error2));
}

TEST_F(RuntimeTest, InMap) {
  // int
  cel_ProgramPtr prog = Parse(R"cel(1 in {1: 2})cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2 in {1: 2})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // uint
  prog = Parse(R"cel(1u in {1u: 2u})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(2u in {1u: 2u})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // bool
  prog = Parse(R"cel(true in {true: false})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel(false in {true: false})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));

  // string
  prog = Parse(R"cel('foo' in {'foo': 'bar'})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('baz' in {'foo': 'bar'})cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, InMapError) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: bad map key"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  cel_ProgramPtr prog = Parse(R"cel(1.0 in {1: 2})cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, BindMemoizes) {
  cel_ProgramPtr prog = Parse(R"cel(cel.bind(a, x, a + a))cel");
  _cel_MockVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  EXPECT_CALL(var_resolver, Find(cel_StringView_From("x"), _, _))
      .WillOnce([](cel_StringView name, cel_Value* out,
                   cel_Status* status) -> cel_Trilean {
        cel_Value_SetInt(out, 1);
        return cel_Trilean_kTrue;
      });
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);
}

TEST_F(RuntimeTest, BindError) {
  cel_ProgramPtr prog = Parse(R"cel(cel.bind(a, 1 / 0, a))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, NestedBind) {
  cel_ProgramPtr prog =
      Parse(R"cel(cel.bind(a, cel.bind(b, 3, b), cel.bind(c, a, c)))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);
}

TEST_F(RuntimeTest, BindMaxValueStack) {
  cel_ProgramPtr prog = Parse(
      R"cel(cel.bind(a, cel.bind(b, [0, 1, 2, 3, 4], [0, 1, 2, 3, 4, 5, b]), cel.bind(c, [0, 1, 2, 3, 4, a], [0, 1, 2, 3, 4, 5, c])))cel");
  _cel_TestingVariableResolver var_resolver;
  cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsList(&value));
  EXPECT_EQ(prog->max_stack_size, 22);
}

}  // namespace
