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
#include "absl/strings/string_view.h"
#include "cel-c/activation.h"
#include "cel-c/alloc.h"
#include "cel-c/arena.h"
#include "cel-c/ast.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/program.h"
#include "cel-c/src/parsed_map_field_value.h"
#include "cel-c/src/runtime/program.h"
#include "cel-c/src/testing/compiler.h"
#include "cel-c/src/testing/def_pool.h"
#include "cel-c/src/testing/parser.h"
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

using ::testing::_;
using ::testing::Invoke;

struct _cel_AstDeleter {
  void operator()(const cel_Ast* ast) const noexcept {
    if (ast != nullptr) {
      cel_Ast_Delete(const_cast<cel_Ast*>(ast));
    }
  }
};

using _cel_AstPtr = std::unique_ptr<cel_Ast, _cel_AstDeleter>;

struct _cel_RuntimeDeleter {
  void operator()(const cel_Runtime* rt) const noexcept {
    if (rt != nullptr) {
      cel_Runtime_Delete(const_cast<cel_Runtime*>(rt));
    }
  }
};

using _cel_RuntimePtr = std::unique_ptr<cel_Runtime, _cel_RuntimeDeleter>;

struct _cel_ProgramDeleter {
  void operator()(const cel_Program* prog) const noexcept {
    if (prog != nullptr) {
      cel_Program_Delete(const_cast<cel_Program*>(prog));
    }
  }
};

using _cel_ProgramPtr = std::unique_ptr<cel_Program, _cel_ProgramDeleter>;

struct _cel_ActivationDeleter {
  void operator()(const cel_Activation* act) const noexcept {
    if (act != nullptr) {
      cel_Activation_Delete(const_cast<cel_Activation*>(act));
    }
  }
};

using _cel_ActivationPtr =
    std::unique_ptr<cel_Activation, _cel_ActivationDeleter>;

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
    arena_ = ABSL_DIE_IF_NULL(cel_Arena_New(alloc()));
    rt_ = NewRuntime(nullptr);
    parser_ = _cel_TestingParserPtr(_cel_TestingParser_New());
    compiler_ = _cel_TestingCompilerPtr(_cel_TestingCompiler_New());
    std::memset(&context_, 0, sizeof(context_));
    context_.alloc = alloc();
    context_.arena = arena_;
    context_.def_pool = cel_Runtime_DefPool(rt_.get());
    context_.well_known_types = cel_Runtime_WellKnownTypes(rt_.get());
  }

  void TearDown() {
    compiler_.reset();
    parser_.reset();
    rt_.reset();
    cel_Arena_Delete(arena_);
    arena_ = nullptr;
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

  cel_Arena* arena() { return ABSL_DIE_IF_NULL(arena_); }

  cel_Status* status() { return &status_; }

  const upb_DefPool* def_pool() { return _cel_TestingDefPool(); }

  const cel_ValueContext* context() { return &context_; }

  _cel_RuntimePtr NewRuntime(const cel_RuntimeOptions* opts) {
    _cel_RuntimePtr rt(cel_Runtime_New(alloc(), def_pool(), opts, status()));
    ABSL_CHECK(rt != nullptr) << cel_Status_ToAbsl(status());
    return rt;
  }

  _cel_ProgramPtr Parse(absl::string_view content,
                        const cel_Runtime* rt = nullptr) {
    _cel_AstPtr ast(_cel_TestingParser_Parse(ABSL_DIE_IF_NULL(parser_.get()),
                                             cel_StringView_FromAbsl(content),
                                             arena()));
    _cel_ProgramPtr prog(cel_Runtime_Compile(rt == nullptr ? rt_.get() : rt,
                                             ast.get(), nullptr, status()));
    ABSL_CHECK(prog != nullptr) << cel_Status_ToAbsl(status());
    return prog;
  }

  _cel_ProgramPtr Compile(absl::string_view content) {
    _cel_AstPtr ast(_cel_TestingCompiler_Compile(
        ABSL_DIE_IF_NULL(compiler_.get()), cel_StringView_FromAbsl(content),
        arena()));
    _cel_ProgramPtr prog(
        cel_Runtime_Compile(rt_.get(), ast.get(), nullptr, status()));
    ABSL_CHECK(prog != nullptr) << cel_Status_ToAbsl(status());
    return prog;
  }

  _cel_ActivationPtr Activate(const cel_Program* prog,
                              const cel_VariableResolver* var_resolver) {
    _cel_ActivationPtr act(cel_Program_Activate(prog, var_resolver, nullptr));
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
  cel_Arena* arena_ = nullptr;
  _cel_RuntimePtr rt_;
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
  _cel_ProgramPtr prog = Parse(R"cel(null)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsNull(&value));
}

TEST_F(RuntimeTest, FalseConst) {
  _cel_ProgramPtr prog = Parse(R"cel(false)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsFalse(&value));
}

TEST_F(RuntimeTest, TrueConst) {
  _cel_ProgramPtr prog = Parse(R"cel(true)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsTrue(&value));
}

TEST_F(RuntimeTest, IntConst) {
  _cel_ProgramPtr prog = Parse(R"cel(1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, UintConst) {
  _cel_ProgramPtr prog = Parse(R"cel(1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 1);
}

TEST_F(RuntimeTest, DoubleConst) {
  _cel_ProgramPtr prog = Parse(R"cel(1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1);
}

TEST_F(RuntimeTest, BytesConst) {
  _cel_ProgramPtr prog = Parse(R"cel(b'foo')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBytes(&value));
  EXPECT_EQ(cel_Value_GetBytes(&value), cel_StringView_From("foo"));
}

TEST_F(RuntimeTest, StringConst) {
  _cel_ProgramPtr prog = Parse(R"cel('foo')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("foo"));
}

TEST_F(RuntimeTest, Ident) {
  _cel_ProgramPtr prog = Parse(R"cel(foo)cel");
  _cel_TestingVariableResolver var_resolver({{"foo", cel_NullValue}});
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_RuntimePtr rt = NewRuntime(&rt_opts);
  _cel_ProgramPtr prog = Parse(R"cel(foo)cel", rt.get());
  _cel_TestingVariableResolver var_resolver({{"bar.baz.foo", IntValue(3)},
                                             {"bar.foo", IntValue(2)},
                                             {"foo", IntValue(1)}});
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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

  _cel_ProgramPtr prog = Parse(R"cel(foo.qux.quux)cel");
  _cel_TestingVariableResolver var_resolver({{"foo.qux.quux", DoubleValue(3.0)},
                                             {"foo.qux", foo_qux_val},
                                             {"foo", foo_val}});
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_RuntimePtr rt = NewRuntime(&rt_opts);
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

  _cel_ProgramPtr prog = Parse(R"cel(foo.qux.quux)cel", rt.get());
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
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 + 1 + 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);
}

TEST_F(RuntimeTest, UintAdd) {
  _cel_ProgramPtr prog = Parse(R"cel(1u + 1u + 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 3);
}

TEST_F(RuntimeTest, DoubleAdd) {
  _cel_ProgramPtr prog = Parse(R"cel(1.0 + 1.0 + 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 3.0);
}

TEST_F(RuntimeTest, BytesAdd) {
  _cel_ProgramPtr prog = Parse(R"cel(b'Hello' + b' ' + b'World!')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBytes(&value));
  EXPECT_EQ(cel_Value_GetBytes(&value),
            cel_StringView_FromString("Hello World!"));
}

TEST_F(RuntimeTest, StringAdd) {
  _cel_ProgramPtr prog = Parse(R"cel('Hello' + ' ' + 'World!')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value),
            cel_StringView_FromString("Hello World!"));
}

TEST_F(RuntimeTest, CallBool) {
  _cel_ProgramPtr prog = Parse(R"cel(bool("true"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(int("123"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(uint("123"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(double("1.23"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(bytes("hello"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(string(true))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(timestamp("2025-12-01T10:00:00Z"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(duration("1h"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDuration(&value));
  EXPECT_EQ(cel_Duration_ToUnixSeconds(cel_Value_GetDuration(&value)), 3600);

  prog = Parse(R"cel(duration("invalid"))cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

// TODO: Add addition tests for duration/timestamp

TEST_F(RuntimeTest, IntSub) {
  _cel_ProgramPtr prog = Parse(R"cel(4 - 2 - 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, UintSub) {
  _cel_ProgramPtr prog = Parse(R"cel(4u - 2u - 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 1);
}

TEST_F(RuntimeTest, DoubleSub) {
  _cel_ProgramPtr prog = Parse(R"cel(4.0 - 2.0 - 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1.0);
}

// TODO: Add subtraction tests for duration/timestamp

TEST_F(RuntimeTest, IntMul) {
  _cel_ProgramPtr prog = Parse(R"cel(4 * 2 * 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 8);
}

TEST_F(RuntimeTest, UintMul) {
  _cel_ProgramPtr prog = Parse(R"cel(4u * 2u * 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 8);
}

TEST_F(RuntimeTest, DoubleMul) {
  _cel_ProgramPtr prog = Parse(R"cel(4.0 * 2.0 * 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 8.0);
}

TEST_F(RuntimeTest, DivByZero) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  _cel_ProgramPtr prog = Parse(R"cel(4 / 0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, DivByZeroUint) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  _cel_ProgramPtr prog = Parse(R"cel(4u / 0u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, DivByZeroDouble) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  _cel_ProgramPtr prog = Parse(R"cel(4.0 / 0.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, MismatchedTypes) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: no such overload"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  _cel_ProgramPtr prog = Parse(R"cel(1u / 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  _cel_ProgramPtr prog2 = Parse(R"cel(1.0 / 1)cel");
  _cel_ActivationPtr act2(Activate(prog2.get(), &var_resolver));
  value = Execute(act2.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  _cel_ProgramPtr prog3 = Parse(R"cel(1.0 / 1u)cel");
  _cel_ActivationPtr act3(Activate(prog3.get(), &var_resolver));
  value = Execute(act3.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, IntDiv) {
  _cel_ProgramPtr prog = Parse(R"cel(4 / 2)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);
}

TEST_F(RuntimeTest, UintDiv) {
  _cel_ProgramPtr prog = Parse(R"cel(4u / 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 2);
}

TEST_F(RuntimeTest, DoubleDiv) {
  _cel_ProgramPtr prog = Parse(R"cel(4.0 / 2.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 2.0);
}

TEST_F(RuntimeTest, NegativeIntModTest) {
  _cel_ProgramPtr prog = Parse(R"cel(-10 % 3)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -1);
}

TEST_F(RuntimeTest, IntModTest) {
  _cel_ProgramPtr prog = Parse(R"cel(10 % 3)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, UintMod) {
  _cel_ProgramPtr prog = Parse(R"cel(4u % 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsUint(&value));
  EXPECT_EQ(cel_Value_GetUint(&value), 0);
}

TEST_F(RuntimeTest, IntModByZero) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: modulo by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  _cel_ProgramPtr prog = Parse(R"cel(4 % 0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));

  _cel_ProgramPtr prog2 = Parse(R"cel(1.0 / 1)cel");
  _cel_ActivationPtr act2(Activate(prog2.get(), &var_resolver));
  value = Execute(act2.get());
  ASSERT_TRUE(cel_Value_IsError(&value));

  _cel_ProgramPtr prog3 = Parse(R"cel(1.0 / 1u)cel");
  _cel_ActivationPtr act3(Activate(prog3.get(), &var_resolver));
  value = Execute(act3.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, LogicalNot) {
  _cel_ProgramPtr prog = Parse(R"cel(!true)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntNeg) {
  _cel_ProgramPtr prog = Parse(R"cel(-(1))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), -1);
}

TEST_F(RuntimeTest, UintNeg) {
  _cel_ProgramPtr prog = Parse(R"cel(-1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, DoubleNeg) {
  _cel_ProgramPtr prog = Parse(R"cel(-1.0 - 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), -2.0);
}

TEST_F(RuntimeTest, LogicalAnd) {
  _cel_ProgramPtr prog = Parse(R"cel(true && true)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(true || true)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(false && (1 / 0))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, LogicalOrWithError) {
  cel_Error* error = cel_Error_New(arena());
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  _cel_ProgramPtr prog = Parse(R"cel(false || (1 / 0))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, BoolEquals) {
  _cel_ProgramPtr prog = Parse(R"cel(true == true)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntEquals) {
  _cel_ProgramPtr prog = Parse(R"cel(1 == 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, UintEquals) {
  _cel_ProgramPtr prog = Parse(R"cel(1u == 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, DoubleEquals) {
  _cel_ProgramPtr prog = Parse(R"cel(1.0 == 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BytesEquals) {
  _cel_ProgramPtr prog = Parse(R"cel(b'Hello' == b'Hello')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, StringEquals) {
  _cel_ProgramPtr prog = Parse(R"cel('Hello' == 'Hello')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, BoolNotEquals) {
  _cel_ProgramPtr prog = Parse(R"cel(true != false)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 != 2)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1u != 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 != 2.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(b'a' != b'b')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel('a' != 'b')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));

  prog = Parse(R"cel('a' != 'a')cel");
  act = Activate(prog.get(), &var_resolver);
  value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntLess) {
  _cel_ProgramPtr prog = Parse(R"cel(1 < 2)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1u < 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 < 2.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel('a' < 'b')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 <= 2)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1u <= 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 <= 2.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel('a' <= 'b')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2 > 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2u > 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2.0 > 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel('b' > 'a')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2 >= 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2u >= 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2.0 >= 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel('b' >= 'a')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(b'a' < b'b')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(b'a' <= b'b')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(b'b' > b'a')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(b'b' >= b'a')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 == 1.0)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntDoubleNotEquals) {
  _cel_ProgramPtr prog = Parse(R"cel(1 != 1.1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, IntDoubleLess) {
  _cel_ProgramPtr prog = Parse(R"cel(1 < 1.1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 <= 1.1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.1 > 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.1 >= 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 == 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 != 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 < 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 <= 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2 > 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2 >= 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 == 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 != 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 < 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 <= 2u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2.0 > 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(2.0 >= 1u)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 < 2 ? 1 : 2)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 1);
}

TEST_F(RuntimeTest, ComplexTernaryConditional) {
  _cel_ProgramPtr prog =
      Parse(R"cel(1 > 2 ? 1 : 2 > 3 ? 3 : 4 > 5 ? 5 : 6)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 6);
}

TEST_F(RuntimeTest, TernaryConditionalDifferentTypes) {
  _cel_ProgramPtr prog = Parse(R"cel(1 > 2 ? 1 : "2")cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsString(&value));
  EXPECT_EQ(cel_Value_GetString(&value), cel_StringView_From("2"));

  _cel_ProgramPtr prog2 = Parse(R"cel(true ? true : 1)cel");
  _cel_TestingVariableResolver var_resolver2;
  _cel_ActivationPtr act2(Activate(prog2.get(), &var_resolver2));
  value = Execute(act2.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_EQ(cel_Value_GetBool(&value), true);

  _cel_ProgramPtr prog3 = Parse(R"cel(false ? 1 : 1.0)cel");
  _cel_TestingVariableResolver var_resolver3;
  _cel_ActivationPtr act3(Activate(prog3.get(), &var_resolver3));
  value = Execute(act3.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1.0);

  _cel_ProgramPtr prog4 = Parse(R"cel(true ? 1.0 : "1.0")cel");
  _cel_TestingVariableResolver var_resolver4;
  _cel_ActivationPtr act4(Activate(prog4.get(), &var_resolver4));
  value = Execute(act4.get());
  ASSERT_TRUE(cel_Value_IsDouble(&value));
  EXPECT_EQ(cel_Value_GetDouble(&value), 1.0);
}

TEST_F(RuntimeTest, TernaryConditionalString) {
  _cel_ProgramPtr prog = Parse(R"cel(true ? 'hello' : 'world')cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(true ? 1.0 : 2.5)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(true ? false : true)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 / 0.0 ? true : false)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel([])cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsList(&value));
  cel_Value size;
  ASSERT_TRUE(cel_ListValue_Size(cel_Value_GetList(&value), context(), &size,
                                 status()));
  ASSERT_TRUE(cel_Value_IsInt(&size));
  EXPECT_EQ(cel_Value_GetInt(&size), 0);
}

TEST_F(RuntimeTest, List) {
  _cel_ProgramPtr prog = Parse(R"cel([true, 2, 3u, 4.0])cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel([1.0 / 0.0, 2, 3u, 4.0])cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel({})cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsMap(&value));
  cel_Value size;
  ASSERT_TRUE(
      cel_MapValue_Size(cel_Value_GetMap(&value), context(), &size, status()));
  ASSERT_TRUE(cel_Value_IsInt(&size));
  EXPECT_EQ(cel_Value_GetInt(&size), 0);
}

TEST_F(RuntimeTest, Map) {
  _cel_ProgramPtr prog =
      Parse(R"cel({true: 2, 3: 4u, 5u: "foo", "bar": null})cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel({1 / 0: 2, 3: 4u})cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel("hello".size())cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
}

TEST_F(RuntimeTest, CallContainsString) {
  _cel_ProgramPtr prog = Parse(R"cel("hello".contains("el"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallContainsNotString) {
  _cel_ProgramPtr prog = Parse(R"cel("hello".contains("le"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallStartsWithString) {
  _cel_ProgramPtr prog = Parse(R"cel("hello".startsWith("el"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallStartsWithNotString) {
  _cel_ProgramPtr prog = Parse(R"cel("hello".startsWith("ello"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallEndsWithString) {
  _cel_ProgramPtr prog = Parse(R"cel("hello".endsWith("lo"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_TRUE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallEndsWithNotString) {
  _cel_ProgramPtr prog = Parse(R"cel("hello".endsWith("ell"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsBool(&value));
  EXPECT_FALSE(cel_Value_GetBool(&value));
}

TEST_F(RuntimeTest, CallRegexExpMatch) {
  _cel_ProgramPtr prog = Parse(
      R"cel(" Hello  World!   ".matches("(\\s*)Hello(\\s+)World!(\\s*)"))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog =
      Parse(R"cel([true, 2, 3u, 4.0, 'foo', b'bar', null][0])cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel([1, 2, 3][3])cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel({'foo': 1}['foo'])cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel({'foo': 1}['bar'])cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 in [1, 2, 3])cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 in 1)cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1 in {1: 2})cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
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
  _cel_ProgramPtr prog = Parse(R"cel(1.0 in {1: 2})cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsError(&value));
  EXPECT_EQ(cel_Error_Message(cel_Value_GetError(&value)),
            cel_Error_Message(error));
}

TEST_F(RuntimeTest, BindMemoizes) {
  _cel_ProgramPtr prog = Parse(R"cel(cel.bind(a, x, a + a))cel");
  _cel_MockVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  EXPECT_CALL(var_resolver, Find(cel_StringView_From("x"), _, _))
      .WillOnce(Invoke([](cel_StringView name, cel_Value* out,
                          cel_Status* status) -> cel_Trilean {
        cel_Value_SetInt(out, 1);
        return cel_Trilean_kTrue;
      }));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 2);
}

TEST_F(RuntimeTest, BindError) {
  _cel_ProgramPtr prog = Parse(R"cel(cel.bind(a, 1 / 0, a))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  EXPECT_TRUE(cel_Value_IsError(&value));
}

TEST_F(RuntimeTest, NestedBind) {
  _cel_ProgramPtr prog =
      Parse(R"cel(cel.bind(a, cel.bind(b, 3, b), cel.bind(c, a, c)))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsInt(&value));
  EXPECT_EQ(cel_Value_GetInt(&value), 3);
}

TEST_F(RuntimeTest, BindMaxValueStack) {
  _cel_ProgramPtr prog = Parse(
      R"cel(cel.bind(a, cel.bind(b, [0, 1, 2, 3, 4], [0, 1, 2, 3, 4, 5, b]), cel.bind(c, [0, 1, 2, 3, 4, a], [0, 1, 2, 3, 4, 5, c])))cel");
  _cel_TestingVariableResolver var_resolver;
  _cel_ActivationPtr act(Activate(prog.get(), &var_resolver));
  cel_Value value = Execute(act.get());
  ASSERT_TRUE(cel_Value_IsList(&value));
  EXPECT_EQ(prog->max_stack_size, 22);
}

}  // namespace
