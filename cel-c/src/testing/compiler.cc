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

#include "cel-c/src/testing/compiler.h"

#include <memory>
#include <string>
#include <utility>

#include "cel/expr/checked.pb.h"
#include "cel/expr/checked.upb.h"
#include "absl/log/absl_check.h"
#include "absl/log/die_if_null.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/ast.h"
#include "cel-c/ast_proto.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/string_view_absl.h"
#include "checker/checker_options.h"
#include "checker/optional.h"
#include "checker/standard_library.h"
#include "common/ast_proto.h"
#include "compiler/compiler.h"
#include "compiler/compiler_factory.h"
#include "extensions/bindings_ext.h"
#include "internal/testing_descriptor_pool.h"
#include "parser/options.h"

extern "C" CEL_ATTRIBUTE_NOTHROW _cel_TestingCompiler* cel_nonnull
_cel_TestingCompiler_New() {
  auto status_or_compiler_builder = cel::NewCompilerBuilder(
      cel::internal::GetTestingDescriptorPool(),
      cel::CompilerOptions{
          .parser_options = cel::ParserOptions{.enable_optional_syntax = true},
          .checker_options =
              cel::CheckerOptions{.enable_cross_numeric_comparisons = true,
                                  .update_struct_type_names = true}});
  ABSL_CHECK_OK(status_or_compiler_builder);
  auto compiler_builder = std::move(*status_or_compiler_builder);
  ABSL_CHECK_OK(compiler_builder->AddLibrary(cel::StandardCheckerLibrary()));
  ABSL_CHECK_OK(compiler_builder->AddLibrary(cel::OptionalCheckerLibrary()));
  ABSL_CHECK_OK(
      compiler_builder->AddLibrary(cel::extensions::BindingsCompilerLibrary()));
  auto status_or_compiler = compiler_builder->Build();
  ABSL_CHECK_OK(status_or_compiler);
  auto compiler = std::move(*status_or_compiler);
  return reinterpret_cast<_cel_TestingCompiler*>(compiler.release());
}

extern "C" CEL_ATTRIBUTE_NOTHROW void _cel_TestingCompiler_Delete(
    _cel_TestingCompiler* cel_nonnull compiler) {
  CEL_ASSERT_NOT_NULL(compiler);

  delete reinterpret_cast<cel::Compiler*>(compiler);
}

extern "C" CEL_ATTRIBUTE_NOTHROW cel_Ast* cel_nonnull
_cel_TestingCompiler_Compile(_cel_TestingCompiler* cel_nonnull compiler,
                             cel_StringView content,
                             cel_Arena* cel_nonnull arena) {
  auto status_or_validation_result =
      reinterpret_cast<cel::Compiler*>(compiler)->Compile(
          cel_StringView_ToAbsl(content));
  ABSL_CHECK_OK(status_or_validation_result);
  auto validation_result = std::move(*status_or_validation_result);
  ABSL_CHECK(validation_result.IsValid()) << validation_result.FormatError();
  cel::expr::CheckedExpr checked_expr_proto;
  ABSL_CHECK_OK(
      cel::AstToCheckedExpr(*validation_result.GetAst(), &checked_expr_proto));
  std::string serialized;
  ABSL_CHECK(checked_expr_proto.SerializeToString(&serialized));
  cel_expr_CheckedExpr* checked_expr =
      ABSL_DIE_IF_NULL(cel_expr_CheckedExpr_parse(
          serialized.data(), serialized.size(), arena));
  cel_Status status;
  cel_Status_Construct(&status);
  cel_Ast* c_ast = cel_Ast_FromProto(checked_expr, arena, &status);
  ABSL_CHECK(c_ast != nullptr);
  cel_Status_Destruct(&status);
  return c_ast;
}
