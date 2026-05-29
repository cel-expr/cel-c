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

#include "cel-c/src/testing/parser.h"

#include <memory>
#include <string>
#include <utility>

#include "cel/expr/checked.pb.h"
#include "cel/expr/checked.upb.h"
#include "cel/expr/syntax.pb.h"
#include "cel/expr/syntax.upb.h"
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
#include "common/ast_proto.h"
#include "common/source.h"
#include "extensions/bindings_ext.h"
#include "parser/macro.h"
#include "parser/options.h"
#include "parser/parser.h"
#include "parser/parser_interface.h"

extern "C" CEL_ATTRIBUTE_NOTHROW _cel_TestingParser* cel_nonnull
_cel_TestingParser_New() {
  auto parser_builder =
      cel::NewParserBuilder(cel::ParserOptions{.enable_optional_syntax = true});
  for (const cel::Macro& macro : cel::extensions::bindings_macros()) {
    ABSL_CHECK_OK(parser_builder->AddMacro(macro));
  }
  auto status_or_parser = parser_builder->Build();
  ABSL_CHECK_OK(status_or_parser);
  auto parser = std::move(*status_or_parser);
  return reinterpret_cast<_cel_TestingParser*>(parser.release());
}

extern "C" CEL_ATTRIBUTE_NOTHROW void _cel_TestingParser_Delete(
    _cel_TestingParser* cel_nonnull parser) {
  CEL_ASSERT_NOT_NULL(parser);

  delete reinterpret_cast<cel::Parser*>(parser);
}

extern "C" CEL_ATTRIBUTE_NOTHROW cel_Ast* cel_nonnull
_cel_TestingParser_Parse(_cel_TestingParser* cel_nonnull parser,
                         cel_StringView content, cel_Arena* cel_nonnull arena) {
  auto status_or_source = cel::NewSource(cel_StringView_ToAbsl(content));
  ABSL_CHECK_OK(status_or_source);
  auto source = std::move(*status_or_source);
  auto status_or_ast = reinterpret_cast<cel::Parser*>(parser)->Parse(*source);
  ABSL_CHECK_OK(status_or_ast);
  auto ast = std::move(*status_or_ast);
  cel::expr::ParsedExpr parsed_expr_proto;
  ABSL_CHECK_OK(cel::AstToParsedExpr(*ast, &parsed_expr_proto));
  std::string serialized;
  ABSL_CHECK(parsed_expr_proto.SerializeToString(&serialized));
  cel_expr_CheckedExpr* checked_expr =
      ABSL_DIE_IF_NULL(cel_expr_CheckedExpr_new(arena));
  cel_expr_ParsedExpr* parsed_expr =
      ABSL_DIE_IF_NULL(cel_expr_ParsedExpr_parse(
          serialized.data(), serialized.size(), arena));
  cel_expr_CheckedExpr_set_expr(
      checked_expr, ABSL_DIE_IF_NULL(cel_expr_ParsedExpr_mutable_expr(
                        parsed_expr, arena)));
  cel_expr_CheckedExpr_set_source_info(
      checked_expr,
      ABSL_DIE_IF_NULL(
          cel_expr_ParsedExpr_mutable_source_info(parsed_expr, arena)));
  cel_Status status;
  cel_Status_Construct(&status);
  cel_Ast* c_ast = cel_Ast_FromProto(checked_expr, arena, &status);
  ABSL_CHECK(c_ast != nullptr);
  cel_Status_Destruct(&status);
  return c_ast;
}
