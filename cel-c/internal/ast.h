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

#ifndef THIRD_PARTY_CEL_C_INTERNAL_AST_H_
#define THIRD_PARTY_CEL_C_INTERNAL_AST_H_

#include <stdbool.h>  // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#include <type_traits>
#include <utility>

#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/internal/attribute.h"
#include "cel-c/internal/config.h"
#include "cel-c/internal/constant.h"
#include "cel-c/internal/malloc.h"
#include "cel-c/internal/ref.h"
#include "cel-c/operators.h"
#include "cel-c/source.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"

enum CEL_ATTRIBUTE_OPEN_ENUM cel_ExprKind {
  cel_ExprKind_kUnspecified = 0,
  cel_ExprKind_kIdent,
  cel_ExprKind_kConst,
  cel_ExprKind_kSelect,
  cel_ExprKind_kCall,
  cel_ExprKind_kCallArg,
  cel_ExprKind_kUnary,
  cel_ExprKind_kBinary,
  cel_ExprKind_kTernary,
  cel_ExprKind_kList,
  cel_ExprKind_kListElement,
  cel_ExprKind_kMapEntry,
  cel_ExprKind_kMap,
  cel_ExprKind_kStructField,
  cel_ExprKind_kStruct,
  cel_ExprKind_kComprehension,
  cel_ExprKind_kBind,
  cel_ExprKind_kBlock,
  cel_ExprKind_kBlockBinding,
  cel_ExprKind_kBlockIndex,
  cel_ExprKind_kAttrQual,
  cel_ExprKind_kAttr,
};

typedef int64_t cel_ExprId;

class cel_Ast;
class cel_ExprLink;
class cel_Expr;
class cel_UnspecifiedExpr;
class cel_IdentExpr;
class cel_ConstExpr;
class cel_SelectExpr;
class cel_CallArgExpr;
class cel_CallExpr;
class cel_UnaryExpr;
class cel_BinaryExpr;
class cel_TernaryExpr;
class cel_ListElementExpr;
class cel_ListExpr;
class cel_MapEntryExpr;
class cel_MapExpr;
class cel_StructFieldExpr;
class cel_StructExpr;
class cel_ComprehensionExpr;
class cel_BindExpr;
class cel_BlockIndexExpr;
class cel_BlockBindingExpr;
class cel_BlockExpr;
class cel_AttrQualExpr;
class cel_AttrExpr;

#define CEL_EXPR_DELETED_MEMBER_FUNCTIONS                                     \
  bool IsUnspecified() const = delete;                                        \
  bool IsIdent() const = delete;                                              \
  bool IsConst() const = delete;                                              \
  bool IsSelect() const = delete;                                             \
  bool IsCallArg() const = delete;                                            \
  bool IsCall() const = delete;                                               \
  bool IsUnary() const = delete;                                              \
  bool IsBinary() const = delete;                                             \
  bool IsTernary() const = delete;                                            \
  bool IsListElement() const = delete;                                        \
  bool IsList() const = delete;                                               \
  bool IsMapEntry() const = delete;                                           \
  bool IsMap() const = delete;                                                \
  bool IsStructField() const = delete;                                        \
  bool IsStruct() const = delete;                                             \
  bool IsComprehension() const = delete;                                      \
  bool IsBind() const = delete;                                               \
  bool IsBlockIndex() const = delete;                                         \
  bool IsBlockBinding() const = delete;                                       \
  bool IsBlock() const = delete;                                              \
  bool IsAttrQual() const = delete;                                           \
  bool IsAttr() const = delete;                                               \
  cel_UnspecifiedExpr* cel_nonnull GetUnspecified() = delete;                 \
  cel_IdentExpr* cel_nonnull GetIdent() = delete;                             \
  cel_ConstExpr* cel_nonnull GetConst() = delete;                             \
  cel_SelectExpr* cel_nonnull GetSelect() = delete;                           \
  cel_CallArgExpr* cel_nonnull GetCallArg() = delete;                         \
  cel_CallExpr* cel_nonnull GetCall() = delete;                               \
  cel_UnaryExpr* cel_nonnull GetUnary() = delete;                             \
  cel_BinaryExpr* cel_nonnull GetBinary() = delete;                           \
  cel_TernaryExpr* cel_nonnull GetTernary() = delete;                         \
  cel_ListElementExpr* cel_nonnull GetListElement() = delete;                 \
  cel_ListExpr* cel_nonnull GetList() = delete;                               \
  cel_MapEntryExpr* cel_nonnull GetMapEntry() = delete;                       \
  cel_MapExpr* cel_nonnull GetMap() = delete;                                 \
  cel_StructFieldExpr* cel_nonnull GetStructField() = delete;                 \
  cel_StructExpr* cel_nonnull GetStruct() = delete;                           \
  cel_ComprehensionExpr* cel_nonnull GetComprehension() = delete;             \
  cel_BindExpr* cel_nonnull GetBind() = delete;                               \
  cel_BlockIndexExpr* cel_nonnull GetBlockIndex() = delete;                   \
  cel_BlockBindingExpr* cel_nonnull GetBlockBinding() = delete;               \
  cel_BlockExpr* cel_nonnull GetBlock() = delete;                             \
  cel_AttrQualExpr* cel_nonnull GetAttrQual() = delete;                       \
  cel_AttrExpr* cel_nonnull GetAttr() = delete;                               \
  const cel_UnspecifiedExpr* cel_nonnull GetUnspecified() const = delete;     \
  const cel_IdentExpr* cel_nonnull GetIdent() const = delete;                 \
  const cel_ConstExpr* cel_nonnull GetConst() const = delete;                 \
  const cel_SelectExpr* cel_nonnull GetSelect() const = delete;               \
  const cel_CallArgExpr* cel_nonnull GetCallArg() const = delete;             \
  const cel_CallExpr* cel_nonnull GetCall() const = delete;                   \
  const cel_UnaryExpr* cel_nonnull GetUnary() const = delete;                 \
  const cel_BinaryExpr* cel_nonnull GetBinary() const = delete;               \
  const cel_TernaryExpr* cel_nonnull GetTernary() const = delete;             \
  const cel_ListElementExpr* cel_nonnull GetListElement() const = delete;     \
  const cel_ListExpr* cel_nonnull GetList() const = delete;                   \
  const cel_MapEntryExpr* cel_nonnull GetMapEntry() const = delete;           \
  const cel_MapExpr* cel_nonnull GetMap() const = delete;                     \
  const cel_StructFieldExpr* cel_nonnull GetStructField() const = delete;     \
  const cel_StructExpr* cel_nonnull GetStruct() const = delete;               \
  const cel_ComprehensionExpr* cel_nonnull GetComprehension() const = delete; \
  const cel_BindExpr* cel_nonnull GetBind() const = delete;                   \
  const cel_BlockIndexExpr* cel_nonnull GetBlockIndex() const = delete;       \
  const cel_BlockBindingExpr* cel_nonnull GetBlockBinding() const = delete;   \
  const cel_BlockExpr* cel_nonnull GetBlock() const = delete;                 \
  const cel_AttrQualExpr* cel_nonnull GetAttrQual() const = delete;           \
  const cel_AttrExpr* cel_nonnull GetAttr() const = delete;                   \
  cel_UnspecifiedExpr* cel_nullable AsUnspecified() = delete;                 \
  cel_IdentExpr* cel_nullable AsIdent() = delete;                             \
  cel_ConstExpr* cel_nullable AsConst() = delete;                             \
  cel_SelectExpr* cel_nullable AsSelect() = delete;                           \
  cel_CallArgExpr* cel_nullable AsCallArg() = delete;                         \
  cel_CallExpr* cel_nullable AsCall() = delete;                               \
  cel_UnaryExpr* cel_nullable AsUnary() = delete;                             \
  cel_BinaryExpr* cel_nullable AsBinary() = delete;                           \
  cel_TernaryExpr* cel_nullable AsTernary() = delete;                         \
  cel_ListElementExpr* cel_nullable AsListElement() = delete;                 \
  cel_ListExpr* cel_nullable AsList() = delete;                               \
  cel_MapEntryExpr* cel_nullable AsMapEntry() = delete;                       \
  cel_MapExpr* cel_nullable AsMap() = delete;                                 \
  cel_StructFieldExpr* cel_nullable AsStructField() = delete;                 \
  cel_StructExpr* cel_nullable AsStruct() = delete;                           \
  cel_ComprehensionExpr* cel_nullable AsComprehension() = delete;             \
  cel_BindExpr* cel_nullable AsBind() = delete;                               \
  cel_BlockIndexExpr* cel_nullable AsBlockIndex() = delete;                   \
  cel_BlockBindingExpr* cel_nullable AsBlockBinding() = delete;               \
  cel_BlockExpr* cel_nullable AsBlock() = delete;                             \
  cel_AttrQualExpr* cel_nullable AsAttrQual() = delete;                       \
  cel_AttrExpr* cel_nullable AsAttr() = delete;                               \
  const cel_UnspecifiedExpr* cel_nullable AsUnspecified() const = delete;     \
  const cel_IdentExpr* cel_nullable AsIdent() const = delete;                 \
  const cel_ConstExpr* cel_nullable AsConst() const = delete;                 \
  const cel_SelectExpr* cel_nullable AsSelect() const = delete;               \
  const cel_CallArgExpr* cel_nullable AsCallArg() const = delete;             \
  const cel_CallExpr* cel_nullable AsCall() const = delete;                   \
  const cel_UnaryExpr* cel_nullable AsUnary() const = delete;                 \
  const cel_BinaryExpr* cel_nullable AsBinary() const = delete;               \
  const cel_TernaryExpr* cel_nullable AsTernary() const = delete;             \
  const cel_ListElementExpr* cel_nullable AsListElement() const = delete;     \
  const cel_ListExpr* cel_nullable AsList() const = delete;                   \
  const cel_MapEntryExpr* cel_nullable AsMapEntry() const = delete;           \
  const cel_MapExpr* cel_nullable AsMap() const = delete;                     \
  const cel_StructFieldExpr* cel_nullable AsStructField() const = delete;     \
  const cel_StructExpr* cel_nullable AsStruct() const = delete;               \
  const cel_ComprehensionExpr* cel_nullable AsComprehension() const = delete; \
  const cel_BindExpr* cel_nullable AsBind() const = delete;                   \
  const cel_BlockIndexExpr* cel_nullable AsBlockIndex() const = delete;       \
  const cel_BlockBindingExpr* cel_nullable AsBlockBinding() const = delete;   \
  const cel_BlockExpr* cel_nullable AsBlock() const = delete;                 \
  const cel_AttrQualExpr* cel_nullable AsAttrQual() const = delete;           \
  const cel_AttrExpr* cel_nullable AsAttr() const = delete

class cel_Ast {
 public:
  [[nodiscard]]
  static cel_Ast* cel_nullable New(cel_Arena* cel_nonnull arena) {
    cel_Ast* ast =
        reinterpret_cast<cel_Ast*>(_cel_Malloc(sizeof(cel_Ast), nullptr));
    if (CEL_LIKELY(ast != nullptr)) {
      ::new (static_cast<void*>(ast)) cel_Ast(arena);
    }
    return ast;
  }

  static void Delete(cel_Ast* cel_nullable ast) {
    if (ast != nullptr) {
      ast->Delete();
    }
  }

  void Delete() {
    this->~cel_Ast();
    _cel_FreeSized(this, sizeof(*this));
  }

  cel_Ast(const cel_Ast&) = delete;
  cel_Ast(cel_Ast&&) = delete;
  cel_Ast& operator=(const cel_Ast&) = delete;
  cel_Ast& operator=(cel_Ast&&) = delete;

  [[nodiscard]]
  cel_UnspecifiedExpr* cel_nullable NewUnspecifiedExpr();

  [[nodiscard]]
  cel_IdentExpr* cel_nullable NewIdentExpr();

  [[nodiscard]]
  cel_ConstExpr* cel_nullable NewConstExpr();

  [[nodiscard]]
  cel_SelectExpr* cel_nullable NewSelectExpr();

  [[nodiscard]]
  cel_CallArgExpr* cel_nullable NewCallArgExpr();

  [[nodiscard]]
  cel_CallExpr* cel_nullable NewCallExpr();

  [[nodiscard]]
  cel_UnaryExpr* cel_nullable NewUnaryExpr();

  [[nodiscard]]
  cel_BinaryExpr* cel_nullable NewBinaryExpr();

  [[nodiscard]]
  cel_TernaryExpr* cel_nullable NewTernaryExpr();

  [[nodiscard]]
  cel_ListElementExpr* cel_nullable NewListElementExpr();

  [[nodiscard]]
  cel_ListExpr* cel_nullable NewListExpr();

  [[nodiscard]]
  cel_MapEntryExpr* cel_nullable NewMapEntryExpr();

  [[nodiscard]]
  cel_MapExpr* cel_nullable NewMapExpr();

  [[nodiscard]]
  cel_StructFieldExpr* cel_nullable NewStructFieldExpr();

  [[nodiscard]]
  cel_StructExpr* cel_nullable NewStructExpr();

  [[nodiscard]]
  cel_ComprehensionExpr* cel_nullable NewComprehensionExpr();

  [[nodiscard]]
  cel_BindExpr* cel_nullable NewBindExpr();

  [[nodiscard]]
  cel_BlockIndexExpr* cel_nullable NewBlockIndexExpr();

  [[nodiscard]]
  cel_BlockBindingExpr* cel_nullable NewBlockBindingExpr();

  [[nodiscard]]
  cel_BlockExpr* cel_nullable NewBlockExpr();

  [[nodiscard]]
  cel_AttrQualExpr* cel_nullable NewAttrQualExpr();

  [[nodiscard]]
  cel_AttrExpr* cel_nullable NewAttrExpr();

  [[nodiscard]]
  cel_ExprId NextId() {
    return ++next_id_;
  }

  [[nodiscard]]
  cel_ExprId MaxId() const {
    return next_id_;
  }

  [[nodiscard]]
  cel_Expr* cel_nullable expr() const {
    return expr_;
  }

  cel_Expr* cel_nullable SetExpr(cel_Expr* cel_nullable expr);

  cel_Expr* cel_nullable ReleaseExpr();

 private:
  friend class cel_Expr;

  explicit cel_Ast(cel_Arena* cel_nonnull arena) : arena_(arena) {}

  template <typename T>
  [[nodiscard]]
  T* cel_nullable NewExpr() {
    T* expr =
        reinterpret_cast<T*>(cel_Arena_Malloc(arena_, sizeof(T), nullptr));
    if (CEL_LIKELY(expr != nullptr)) {
      ::new (static_cast<void*>(expr)) T(this);
    }
    return expr;
  }

  cel_Arena* cel_nonnull const arena_;
  cel_ExprId next_id_ = 0;
  cel_Expr* cel_nullable expr_ = nullptr;
};

class cel_Expr {
 public:
  cel_Expr(const cel_Expr&) = delete;
  cel_Expr(cel_Expr&&) = delete;
  cel_Expr& operator=(const cel_Expr&) = delete;
  cel_Expr& operator=(cel_Expr&&) = delete;

  [[nodiscard]]
  cel_ExprKind kind() const {
    return kind_;
  }

  [[nodiscard]]
  cel_ExprId id() const {
    return id_;
  }

  void SetId(cel_ExprId id) {
    CEL_ASSERT_GE(id, 0);

    if (id < 0) {
      id = 0;
    }

    if (id > ast()->next_id_) {
      ast()->next_id_ = id;
    }

    id_ = id;
  }

  [[nodiscard]]
  cel_Expr* cel_nullable parent() const {
    return parent_;
  }

  [[nodiscard]]
  const cel_Ref* cel_nullable ref() const {
    return ref_;
  }

  void set_ref(const cel_Ref* cel_nullable ref) { ref_ = ref; }

  [[nodiscard]]
  const cel_Type* cel_nonnull type() const {
    return type_;
  }

  void set_type(const cel_Type* cel_nonnull type) {
    CEL_ASSERT_NOT_NULL(type);
    type_ = type;
  }

  [[nodiscard]]
  int32_t source_position() const {
    return source_range().begin;
  }

  [[nodiscard]]
  cel_SourceRange source_range() const {
    return range_;
  }

  void SetSourcePosition(int32_t position) {
    CEL_ASSERT_GE(position, -1);

    if (position < -1) {
      position = -1;
    }

    range_.begin = position;
    range_.end = -1;
  }

  void SetSourceRange(cel_SourceRange range) {
    CEL_ASSERT_GE(range.begin, -1);
    CEL_ASSERT_GE(range.end, -1);
    CEL_ASSERT(range.begin <= range.end || range.end == -1);

    if (range.begin < -1) {
      range.begin = -1;
    }
    if (range.end < -1) {
      range.end = -1;
    }

    range_ = range;
  }

  [[nodiscard]]
  size_t Depth() const {
    const cel_Expr* current = this;
    current = current->parent();
    size_t depth = 0;
    while (current != nullptr) {
      ++depth;
      current = current->parent();
    }
    return depth;
  }

  [[nodiscard]]
  bool IsUnspecified() const {
    return kind() == cel_ExprKind_kUnspecified;
  }

  [[nodiscard]]
  bool IsIdent() const {
    return kind() == cel_ExprKind_kIdent;
  }

  [[nodiscard]]
  bool IsConst() const {
    return kind() == cel_ExprKind_kConst;
  }

  [[nodiscard]]
  bool IsSelect() const {
    return kind() == cel_ExprKind_kSelect;
  }

  [[nodiscard]]
  bool IsCallArg() const {
    return kind() == cel_ExprKind_kCallArg;
  }

  [[nodiscard]]
  bool IsCall() const {
    return kind() == cel_ExprKind_kCall;
  }

  [[nodiscard]]
  bool IsUnary() const {
    return kind() == cel_ExprKind_kUnary;
  }

  [[nodiscard]]
  bool IsBinary() const {
    return kind() == cel_ExprKind_kBinary;
  }

  [[nodiscard]]
  bool IsTernary() const {
    return kind() == cel_ExprKind_kTernary;
  }

  [[nodiscard]]
  bool IsListElement() const {
    return kind() == cel_ExprKind_kListElement;
  }

  [[nodiscard]]
  bool IsList() const {
    return kind() == cel_ExprKind_kList;
  }

  [[nodiscard]]
  bool IsMapEntry() const {
    return kind() == cel_ExprKind_kMapEntry;
  }

  [[nodiscard]]
  bool IsMap() const {
    return kind() == cel_ExprKind_kMap;
  }

  [[nodiscard]]
  bool IsStructField() const {
    return kind() == cel_ExprKind_kStructField;
  }

  [[nodiscard]]
  bool IsStruct() const {
    return kind() == cel_ExprKind_kStruct;
  }

  [[nodiscard]]
  bool IsComprehension() const {
    return kind() == cel_ExprKind_kComprehension;
  }

  [[nodiscard]]
  bool IsBind() const {
    return kind() == cel_ExprKind_kBind;
  }

  [[nodiscard]]
  bool IsBlockIndex() const {
    return kind() == cel_ExprKind_kBlockIndex;
  }

  [[nodiscard]]
  bool IsBlockBinding() const {
    return kind() == cel_ExprKind_kBlockBinding;
  }

  [[nodiscard]]
  bool IsBlock() const {
    return kind() == cel_ExprKind_kBlock;
  }

  [[nodiscard]]
  bool IsAttrQual() const {
    return kind() == cel_ExprKind_kAttrQual;
  }

  [[nodiscard]]
  bool IsAttr() const {
    return kind() == cel_ExprKind_kAttr;
  }

  [[nodiscard]]
  cel_UnspecifiedExpr* cel_nonnull GetUnspecified();

  [[nodiscard]]
  cel_IdentExpr* cel_nonnull GetIdent();

  [[nodiscard]]
  cel_ConstExpr* cel_nonnull GetConst();

  [[nodiscard]]
  cel_SelectExpr* cel_nonnull GetSelect();

  [[nodiscard]]
  cel_CallArgExpr* cel_nonnull GetCallArg();

  [[nodiscard]]
  cel_CallExpr* cel_nonnull GetCall();

  [[nodiscard]]
  cel_UnaryExpr* cel_nonnull GetUnary();

  [[nodiscard]]
  cel_BinaryExpr* cel_nonnull GetBinary();

  [[nodiscard]]
  cel_TernaryExpr* cel_nonnull GetTernary();

  [[nodiscard]]
  cel_ListElementExpr* cel_nonnull GetListElement();

  [[nodiscard]]
  cel_ListExpr* cel_nonnull GetList();

  [[nodiscard]]
  cel_MapEntryExpr* cel_nonnull GetMapEntry();

  [[nodiscard]]
  cel_MapExpr* cel_nonnull GetMap();

  [[nodiscard]]
  cel_StructFieldExpr* cel_nonnull GetStructField();

  [[nodiscard]]
  cel_StructExpr* cel_nonnull GetStruct();

  [[nodiscard]]
  cel_ComprehensionExpr* cel_nonnull GetComprehension();

  [[nodiscard]]
  cel_BindExpr* cel_nonnull GetBind();

  [[nodiscard]]
  cel_BlockIndexExpr* cel_nonnull GetBlockIndex();

  [[nodiscard]]
  cel_BlockBindingExpr* cel_nonnull GetBlockBinding();

  [[nodiscard]]
  cel_BlockExpr* cel_nonnull GetBlock();

  [[nodiscard]]
  cel_AttrQualExpr* cel_nonnull GetAttrQual();

  [[nodiscard]]
  cel_AttrExpr* cel_nonnull GetAttr();

  [[nodiscard]]
  const cel_UnspecifiedExpr* cel_nonnull GetUnspecified() const;

  [[nodiscard]]
  const cel_IdentExpr* cel_nonnull GetIdent() const;

  [[nodiscard]]
  const cel_ConstExpr* cel_nonnull GetConst() const;

  [[nodiscard]]
  const cel_SelectExpr* cel_nonnull GetSelect() const;

  [[nodiscard]]
  const cel_CallArgExpr* cel_nonnull GetCallArg() const;

  [[nodiscard]]
  const cel_CallExpr* cel_nonnull GetCall() const;

  [[nodiscard]]
  const cel_UnaryExpr* cel_nonnull GetUnary() const;

  [[nodiscard]]
  const cel_BinaryExpr* cel_nonnull GetBinary() const;

  [[nodiscard]]
  const cel_TernaryExpr* cel_nonnull GetTernary() const;

  [[nodiscard]]
  const cel_ListElementExpr* cel_nonnull GetListElement() const;

  [[nodiscard]]
  const cel_ListExpr* cel_nonnull GetList() const;

  [[nodiscard]]
  const cel_MapEntryExpr* cel_nonnull GetMapEntry() const;

  [[nodiscard]]
  const cel_MapExpr* cel_nonnull GetMap() const;

  [[nodiscard]]
  const cel_StructFieldExpr* cel_nonnull GetStructField() const;

  [[nodiscard]]
  const cel_StructExpr* cel_nonnull GetStruct() const;

  [[nodiscard]]
  const cel_ComprehensionExpr* cel_nonnull GetComprehension() const;

  [[nodiscard]]
  const cel_BindExpr* cel_nonnull GetBind() const;

  [[nodiscard]]
  const cel_BlockIndexExpr* cel_nonnull GetBlockIndex() const;

  [[nodiscard]]
  const cel_BlockBindingExpr* cel_nonnull GetBlockBinding() const;

  [[nodiscard]]
  const cel_BlockExpr* cel_nonnull GetBlock() const;

  [[nodiscard]]
  const cel_AttrQualExpr* cel_nonnull GetAttrQual() const;

  [[nodiscard]]
  const cel_AttrExpr* cel_nonnull GetAttr() const;

  [[nodiscard]]
  cel_UnspecifiedExpr* cel_nullable AsUnspecified();

  [[nodiscard]]
  cel_IdentExpr* cel_nullable AsIdent();

  [[nodiscard]]
  cel_ConstExpr* cel_nullable AsConst();

  [[nodiscard]]
  cel_SelectExpr* cel_nullable AsSelect();

  [[nodiscard]]
  cel_CallArgExpr* cel_nullable AsCallArg();

  [[nodiscard]]
  cel_CallExpr* cel_nullable AsCall();

  [[nodiscard]]
  cel_UnaryExpr* cel_nullable AsUnary();

  [[nodiscard]]
  cel_BinaryExpr* cel_nullable AsBinary();

  [[nodiscard]]
  cel_TernaryExpr* cel_nullable AsTernary();

  [[nodiscard]]
  cel_ListElementExpr* cel_nullable AsListElement();

  [[nodiscard]]
  cel_ListExpr* cel_nullable AsList();

  [[nodiscard]]
  cel_MapEntryExpr* cel_nullable AsMapEntry();

  [[nodiscard]]
  cel_MapExpr* cel_nullable AsMap();

  [[nodiscard]]
  cel_StructFieldExpr* cel_nullable AsStructField();

  [[nodiscard]]
  cel_StructExpr* cel_nullable AsStruct();

  [[nodiscard]]
  cel_ComprehensionExpr* cel_nullable AsComprehension();

  [[nodiscard]]
  cel_BindExpr* cel_nullable AsBind();

  [[nodiscard]]
  cel_BlockIndexExpr* cel_nullable AsBlockIndex();

  [[nodiscard]]
  cel_BlockBindingExpr* cel_nullable AsBlockBinding();

  [[nodiscard]]
  cel_BlockExpr* cel_nullable AsBlock();

  [[nodiscard]]
  cel_AttrQualExpr* cel_nullable AsAttrQual();

  [[nodiscard]]
  cel_AttrExpr* cel_nullable AsAttr();

  [[nodiscard]]
  const cel_UnspecifiedExpr* cel_nullable AsUnspecified() const;

  [[nodiscard]]
  const cel_IdentExpr* cel_nullable AsIdent() const;

  [[nodiscard]]
  const cel_ConstExpr* cel_nullable AsConst() const;

  [[nodiscard]]
  const cel_SelectExpr* cel_nullable AsSelect() const;

  [[nodiscard]]
  const cel_CallArgExpr* cel_nullable AsCallArg() const;

  [[nodiscard]]
  const cel_CallExpr* cel_nullable AsCall() const;

  [[nodiscard]]
  const cel_UnaryExpr* cel_nullable AsUnary() const;

  [[nodiscard]]
  const cel_BinaryExpr* cel_nullable AsBinary() const;

  [[nodiscard]]
  const cel_TernaryExpr* cel_nullable AsTernary() const;

  [[nodiscard]]
  const cel_ListElementExpr* cel_nullable AsListElement() const;

  [[nodiscard]]
  const cel_ListExpr* cel_nullable AsList() const;

  [[nodiscard]]
  const cel_MapEntryExpr* cel_nullable AsMapEntry() const;

  [[nodiscard]]
  const cel_MapExpr* cel_nullable AsMap() const;

  [[nodiscard]]
  const cel_StructFieldExpr* cel_nullable AsStructField() const;

  [[nodiscard]]
  const cel_StructExpr* cel_nullable AsStruct() const;

  [[nodiscard]]
  const cel_ComprehensionExpr* cel_nullable AsComprehension() const;

  [[nodiscard]]
  const cel_BindExpr* cel_nullable AsBind() const;

  [[nodiscard]]
  const cel_BlockIndexExpr* cel_nullable AsBlockIndex() const;

  [[nodiscard]]
  const cel_BlockBindingExpr* cel_nullable AsBlockBinding() const;

  [[nodiscard]]
  const cel_BlockExpr* cel_nullable AsBlock() const;

  [[nodiscard]]
  const cel_AttrQualExpr* cel_nullable AsAttrQual() const;

  [[nodiscard]]
  const cel_AttrExpr* cel_nullable AsAttr() const;

 protected:
  cel_Expr(cel_Ast* cel_nonnull ast, cel_ExprKind kind)
      : ast_(ast), kind_(kind) {}

  template <typename T>
  [[nodiscard]]
  T* cel_nullable ReleaseChild(T * cel_nullable * cel_nonnull child_ptr) {
    cel_Expr* child = static_cast<cel_Expr*>(*child_ptr);
    if (child != nullptr) {
      CEL_ASSERT_EQ(child->parent(), this);
      *child_ptr = nullptr;
      child->parent_ = nullptr;
    }
    return static_cast<T*>(child);
  }

  template <typename T>
  [[nodiscard]]
  T* cel_nullable SetChild(T * cel_nullable * cel_nonnull child_ptr,
                           T* cel_nullable child) {
    CEL_ASSERT(child == nullptr ||
               (static_cast<cel_Expr*>(child)->ast() == ast() &&
                static_cast<cel_Expr*>(child)->parent() == nullptr));
    cel_Expr* old_child = static_cast<cel_Expr*>(ReleaseChild(child_ptr));
    *child_ptr = child;
    if (child != nullptr) {
      static_cast<cel_Expr*>(child)->parent_ = this;
    }
    return static_cast<T*>(old_child);
  }

  void LinkChild(cel_ExprLink * cel_nullable * cel_nonnull head_ptr,
                 cel_ExprLink * cel_nullable * cel_nonnull tail_ptr,
                 size_t* cel_nonnull size_ptr, cel_ExprLink* cel_nullable next,
                 cel_ExprLink* cel_nonnull child);

  void UnlinkChild(cel_ExprLink * cel_nullable * cel_nonnull head_ptr,
                   cel_ExprLink * cel_nullable * cel_nonnull tail_ptr,
                   size_t* cel_nonnull size_ptr,
                   cel_ExprLink* cel_nullable child);

  template <typename T>
  [[nodiscard]]
  static T* cel_nullable PrevChild(T* cel_nullable child);

  template <typename T>
  [[nodiscard]]
  static T* cel_nullable NextChild(T* cel_nullable child);

  [[nodiscard]]
  cel_Ast* cel_nonnull ast() const {
    return ast_;
  }

 private:
  friend class cel_Ast;
  friend class cel_ExprLink;

  cel_ExprId id_ = 0;
  cel_Ast* cel_nonnull const ast_;
  cel_Expr* cel_nullable parent_ = nullptr;
  const cel_ExprKind kind_;
  cel_SourceRange range_ = cel_SourceRange(-1, -1);
  const cel_Ref* cel_nullable ref_ = nullptr;
  const cel_Type* cel_nonnull type_ = cel_DynType;
};

inline cel_Expr* cel_nullable cel_Ast::SetExpr(cel_Expr* cel_nullable expr) {
  CEL_ASSERT(expr == nullptr || expr->ast() == this);
  return std::exchange(expr_, expr);
}

inline cel_Expr* cel_nullable cel_Ast::ReleaseExpr() {
  return std::exchange(expr_, nullptr);
}

template <typename T>
class cel_ExprRangeIterator;
template <typename T>
class cel_ExprRange;

template <typename T>
class cel_ExprLinkCrtp;

class cel_ExprLink : public cel_Expr {
 public:
  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

  [[nodiscard]] [[deprecated]]
  ptrdiff_t index() const {
    return index_;
  }

 protected:
  using cel_Expr::cel_Expr;

 private:
  friend class cel_Expr;
  template <typename T>
  friend class cel_ExprLinkCrtp;
  template <typename T>
  friend class cel_ExprRangeIterator;
  template <typename T>
  friend class cel_ExprRange;

  cel_ExprLink* cel_nullable left_ = nullptr;
  cel_ExprLink* cel_nullable right_ = nullptr;
  ptrdiff_t index_ = -1;
};

template <typename T>
class cel_ExprLinkCrtp : public cel_ExprLink {
 public:
  [[nodiscard]] [[deprecated]]
  T* cel_nullable prev() const {
    return static_cast<T*>(
        static_cast<const cel_ExprLink*>(static_cast<const T*>(this))->left_);
  }

  [[nodiscard]] [[deprecated]]
  T* cel_nullable next() const {
    return static_cast<T*>(
        static_cast<const cel_ExprLink*>(static_cast<const T*>(this))->right_);
  }

 protected:
  using cel_ExprLink::cel_ExprLink;
};

inline void cel_Expr::LinkChild(
    cel_ExprLink * cel_nullable * cel_nonnull head_ptr,
    cel_ExprLink * cel_nullable * cel_nonnull tail_ptr,
    size_t* cel_nonnull size_ptr, cel_ExprLink* cel_nullable next,
    cel_ExprLink* cel_nonnull child) {
  CEL_ASSERT(child != nullptr && child->ast() == ast() &&
             child->parent() == nullptr && child->left_ == nullptr &&
             child->right_ == nullptr &&
             (next == nullptr || next->parent() == this));

  if (next == nullptr) {
    // nullptr is end
    cel_ExprLink* head = *head_ptr;
    cel_ExprLink* tail = *tail_ptr;
    child->left_ = tail;
    if (tail != nullptr) {
      CEL_ASSERT_NOT_NULL(head);
      tail->right_ = child;
      child->index_ = tail->index_ + 1;
    } else {
      CEL_ASSERT_NULL(head);
      *head_ptr = child;
      child->index_ = 0;
    }
    *tail_ptr = child;
  } else {
    cel_ExprLink* prev = next->left_;
    child->right_ = next;
    child->index_ = next->index_;
    child->left_ = prev;
    next->left_ = child;
    if (prev != nullptr) {
      prev->right_ = child;
    } else {
      CEL_ASSERT_EQ(*head_ptr, next);
      *head_ptr = child;
    }
    // Fixup indices.
    do {
      ++next->index_;
      next = next->right_;
    } while (next != nullptr);
  }

  child->parent_ = this;
  ++(*size_ptr);
}

inline void cel_Expr::UnlinkChild(
    cel_ExprLink * cel_nullable * cel_nonnull head_ptr,
    cel_ExprLink * cel_nullable * cel_nonnull tail_ptr,
    size_t* cel_nonnull size_ptr, cel_ExprLink* cel_nullable child) {
  CEL_ASSERT(child != nullptr && child->ast() == ast() &&
             child->parent() == this);

  if (child->left_ != nullptr) {
    child->left_->right_ = child->right_;
  } else {
    CEL_ASSERT_EQ(*head_ptr, child);
    *head_ptr = child->right_;
  }

  cel_ExprLink* right = child->right_;
  if (right != nullptr) {
    right->left_ = child->left_;
    // Fixup indices.
    do {
      --right->index_;
      right = right->right_;
    } while (right != nullptr);
  } else {
    CEL_ASSERT_EQ(*tail_ptr, child);
    *tail_ptr = child->left_;
  }

  --(*size_ptr);

  child->parent_ = nullptr;
  child->left_ = child->right_ = nullptr;
  child->index_ = -1;
}

template <typename T>
[[nodiscard]]
inline T* cel_nullable cel_Expr::PrevChild(T* cel_nullable child) {
  if (child != nullptr) {
    child = static_cast<T*>(static_cast<cel_ExprLink*>(child)->left_);
  }
  return child;
}

template <typename T>
[[nodiscard]]
inline T* cel_nullable cel_Expr::NextChild(T* cel_nullable child) {
  if (child != nullptr) {
    child = static_cast<T*>(static_cast<cel_ExprLink*>(child)->right_);
  }
  return child;
}

class cel_UnspecifiedExpr : public cel_Expr {
 public:
  explicit cel_UnspecifiedExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kUnspecified) {}

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;
};

class cel_IdentExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_StringView name() const {
    return name_;
  }

  void set_name(cel_StringView name) { name_ = name; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_IdentExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kIdent) {}

  cel_StringView name_ = CEL_STRINGVIEW_INIT("", 0);
};

class cel_ConstExpr : public cel_Expr {
 public:
  [[nodiscard]]
  const cel_Constant& value() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return value_;
  }
  [[nodiscard]]
  cel_Constant& value() CEL_ATTRIBUTE_LIFETIME_BOUND {
    return value_;
  }

  void set_value(cel_Constant value) { value_ = value; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_ConstExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kConst) {}

  cel_Constant value_ = cel_UnspecifiedConstant();
};

class cel_SelectExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_Expr* cel_nullable operand() const {
    return operand_;
  }

  cel_Expr* cel_nullable SetOperand(cel_Expr* cel_nullable operand) {
    return SetChild(&operand_, operand);
  }

  cel_Expr* cel_nullable ReleaseOperand() { return ReleaseChild(&operand_); }

  [[nodiscard]]
  cel_StringView field() const {
    return field_;
  }

  void set_field(cel_StringView field) { field_ = field; }

  [[nodiscard]]
  bool test_only() const {
    return test_only_;
  }

  void set_test_only(bool test_only) { test_only_ = test_only; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_SelectExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kSelect) {}

  cel_Expr* cel_nullable operand_ = nullptr;
  cel_StringView field_ = CEL_STRINGVIEW_INIT("", 0);
  bool test_only_ = false;
};

template <typename T>
class cel_ExprRangeIterator {
 public:
  cel_ExprRangeIterator() = default;
  cel_ExprRangeIterator(const cel_ExprRangeIterator&) = default;
  cel_ExprRangeIterator& operator=(const cel_ExprRangeIterator&) = default;

  [[nodiscard]]
  T* cel_nonnull operator*() const {
    return static_cast<T*>(current_);
  }

  [[nodiscard]]
  T* cel_nonnull operator->() const {
    return static_cast<T*>(current_);
  }

  cel_ExprRangeIterator& operator++() {
    CEL_ASSERT_NOT_NULL(current_);
    current_ = current_->right_;
    return *this;
  }

  cel_ExprRangeIterator operator++(int) {
    cel_ExprRangeIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  cel_ExprRangeIterator& operator--() {
    CEL_ASSERT_NOT_NULL(tail_);
    CEL_ASSERT_NOT_NULL(*tail_);
    if (current_ != nullptr) {
      current_ = current_->left_;
    } else {
      current_ = *tail_;
    }
    return *this;
  }

  cel_ExprRangeIterator operator--(int) {
    cel_ExprRangeIterator tmp = *this;
    --(*this);
    return tmp;
  }

  friend bool operator==(const cel_ExprRangeIterator& lhs,
                         const cel_ExprRangeIterator& rhs) {
    CEL_ASSERT_EQ(lhs.tail_, rhs.tail_);
    return lhs.current_ == rhs.current_;
  }

  friend bool operator!=(const cel_ExprRangeIterator& lhs,
                         const cel_ExprRangeIterator& rhs) {
    CEL_ASSERT_EQ(lhs.tail_, rhs.tail_);
    return lhs.current_ != rhs.current_;
  }

 private:
  template <typename U>
  friend class cel_ExprRange;

  cel_ExprRangeIterator(cel_ExprLink* cel_nullable current,
                        cel_ExprLink* cel_nullable const* cel_nullable tail)
      : current_(current), tail_(tail) {}

  cel_ExprLink* cel_nullable current_ = nullptr;
  cel_ExprLink* cel_nullable const* cel_nullable tail_ = nullptr;
};

template <typename T>
class cel_ExprRange {
 public:
  using iterator = cel_ExprRangeIterator<T>;
  using const_iterator = iterator;

  cel_ExprRange() = default;
  cel_ExprRange(const cel_ExprRange&) = default;
  cel_ExprRange& operator=(const cel_ExprRange&) = delete;

  cel_ExprRange(cel_ExprLink* cel_nullable const* cel_nonnull head,
                cel_ExprLink* cel_nullable const* cel_nonnull tail)
      : head_(head), tail_(tail) {}

  [[nodiscard]]
  bool empty() const {
    return head_ == nullptr || *head_ == nullptr;
  }

  [[nodiscard]]
  size_t size() const {
    return tail_ == nullptr || *tail_ == nullptr
               ? 0
               : static_cast<size_t>((*tail_)->index_) + 1;
  }

  [[nodiscard]]
  const_iterator begin() const {
    return const_iterator(head_ != nullptr ? *head_ : nullptr, tail_);
  }

  [[nodiscard]]
  const_iterator end() const {
    return const_iterator(nullptr, tail_);
  }

  [[nodiscard]]
  T* cel_nullability_unknown Front() const {
    return head_ != nullptr ? static_cast<T*>(*head_) : nullptr;
  }

  [[nodiscard]]
  T* cel_nullability_unknown Back() const {
    return tail_ != nullptr ? static_cast<T*>(*tail_) : nullptr;
  }

  [[nodiscard]]
  T* cel_nonnull operator[](size_t index) const {
    CEL_ASSERT_LT(index, size());
    cel_ExprLink* expr = *head_;
    for (; index > 0; --index) {
      expr = expr->right_;
    }
    return static_cast<T*>(expr);
  }

 private:
  friend class cel_Ast;

  cel_ExprLink* cel_nullable const* cel_nullable head_ = nullptr;
  cel_ExprLink* cel_nullable const* cel_nullable tail_ = nullptr;
};

class cel_CallArgExpr : public cel_ExprLinkCrtp<cel_CallArgExpr> {
 public:
  [[nodiscard]]
  cel_CallExpr* cel_nullable parent() const {
    return cel_ExprLink::parent() != nullptr ? cel_ExprLink::parent()->GetCall()
                                             : nullptr;
  }

  [[nodiscard]]
  cel_Expr* cel_nullable value() const {
    return value_;
  }

  cel_Expr* cel_nullable SetValue(cel_Expr* cel_nullable value) {
    return SetChild(&value_, value);
  }

  cel_Expr* cel_nullable ReleaseValue() { return ReleaseChild(&value_); }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_CallArgExpr(cel_Ast* cel_nonnull ast)
      : cel_ExprLinkCrtp(ast, cel_ExprKind_kCallArg) {}

  cel_Expr* cel_nullable value_ = nullptr;
};

class cel_CallExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_StringView function() const {
    return function_;
  }

  void set_function(cel_StringView function) { function_ = function; }

  [[nodiscard]]
  cel_Expr* cel_nullable target() const {
    return target_;
  }

  cel_Expr* cel_nullable SetTarget(cel_Expr* cel_nullable target) {
    return SetChild(&target_, target);
  }

  cel_Expr* cel_nullable ReleaseTarget() { return ReleaseChild(&target_); }

  [[nodiscard]]
  cel_ExprRange<cel_CallArgExpr> args() const {
    return cel_ExprRange<cel_CallArgExpr>(&args_head_, &args_tail_);
  }

  void AppendArg(cel_CallArgExpr* cel_nonnull arg) {
    LinkChild(&args_head_, &args_tail_, &args_, nullptr, arg);
  }

  void PrependArg(cel_CallArgExpr* cel_nonnull arg) {
    LinkChild(&args_head_, &args_tail_, &args_, args_head_, arg);
  }

  void InsertArgBefore(cel_CallArgExpr* cel_nullable before,
                       cel_CallArgExpr* cel_nonnull arg) {
    LinkChild(&args_head_, &args_tail_, &args_, before, arg);
  }

  void InsertArgAfter(cel_CallArgExpr* cel_nullable after,
                      cel_CallArgExpr* cel_nonnull arg) {
    InsertArgBefore(NextChild(after), arg);
  }

  cel_CallArgExpr* cel_nonnull ReleaseArg(cel_CallArgExpr* cel_nonnull arg) {
    UnlinkChild(&args_head_, &args_tail_, &args_, arg);
    return arg;
  }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_CallExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kCall) {}

  cel_StringView function_ = CEL_STRINGVIEW_INIT("", 0);
  cel_Expr* cel_nullable target_ = nullptr;
  cel_ExprLink* cel_nullable args_head_ = nullptr;
  cel_ExprLink* cel_nullable args_tail_ = nullptr;
  size_t args_ = 0;
};

class cel_UnaryExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_UnaryOp op() const {
    return op_;
  }

  void set_op(cel_UnaryOp op) { op_ = op; }

  [[nodiscard]]
  cel_Expr* cel_nullable arg() const {
    return args_[0];
  }

  cel_Expr* cel_nullable SetArg(cel_Expr* cel_nullable arg) {
    return SetChild(&args_[0], arg);
  }

  cel_Expr* cel_nullable ReleaseArg() { return ReleaseChild(&args_[0]); }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_UnaryExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kUnary) {}

  cel_UnaryOp op_ = cel_UnaryOp_kUnspecified;
  cel_Expr* cel_nullable args_[1] = {nullptr};
};

class cel_BinaryExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_BinaryOp op() const {
    return op_;
  }

  void set_op(cel_BinaryOp op) { op_ = op; }

  [[nodiscard]]
  cel_Expr* cel_nullable left() const {
    return args_[0];
  }

  cel_Expr* cel_nullable SetLeft(cel_Expr* cel_nullable left) {
    return SetChild(&args_[0], left);
  }

  cel_Expr* cel_nullable ReleaseLeft() { return ReleaseChild(&args_[0]); }

  [[nodiscard]]
  cel_Expr* cel_nullable right() const {
    return args_[1];
  }

  cel_Expr* cel_nullable SetRight(cel_Expr* cel_nullable right) {
    return SetChild(&args_[1], right);
  }

  cel_Expr* cel_nullable ReleaseRight() { return ReleaseChild(&args_[1]); }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_BinaryExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kBinary) {}

  cel_BinaryOp op_ = cel_BinaryOp_kUnspecified;
  cel_Expr* cel_nullable args_[2] = {nullptr, nullptr};
};

class cel_TernaryExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_TernaryOp op() const {
    return op_;
  }

  void set_op(cel_TernaryOp op) { op_ = op; }

  [[nodiscard]]
  cel_Expr* cel_nullable condition() const {
    return args_[0];
  }

  cel_Expr* cel_nullable SetCondition(cel_Expr* cel_nullable condition) {
    return SetChild(&args_[0], condition);
  }

  cel_Expr* cel_nullable ReleaseCondition() { return ReleaseChild(&args_[0]); }

  [[nodiscard]]
  cel_Expr* cel_nullable then() const {
    return args_[1];
  }

  cel_Expr* cel_nullable SetThen(cel_Expr* cel_nullable then) {
    return SetChild(&args_[1], then);
  }

  cel_Expr* cel_nullable ReleaseThen() { return ReleaseChild(&args_[1]); }

  [[nodiscard]]
  cel_Expr* cel_nullable otherwise() const {
    return args_[2];
  }

  cel_Expr* cel_nullable SetOtherwise(cel_Expr* cel_nullable otherwise) {
    return SetChild(&args_[2], otherwise);
  }

  cel_Expr* cel_nullable ReleaseOtherwise() { return ReleaseChild(&args_[2]); }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_TernaryExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kTernary) {}

  cel_TernaryOp op_ = cel_TernaryOp_kUnspecified;
  cel_Expr* cel_nullable args_[3] = {nullptr, nullptr, nullptr};
};

class cel_ListElementExpr : public cel_ExprLinkCrtp<cel_ListElementExpr> {
 public:
  [[nodiscard]]
  cel_ListExpr* cel_nullable parent() const {
    return cel_ExprLink::parent() != nullptr ? cel_ExprLink::parent()->GetList()
                                             : nullptr;
  }

  [[nodiscard]]
  cel_Expr* cel_nullable value() const {
    return value_;
  }

  cel_Expr* cel_nullable SetValue(cel_Expr* cel_nullable value) {
    return SetChild(&value_, value);
  }

  cel_Expr* cel_nullable ReleaseValue() { return ReleaseChild(&value_); }

  [[nodiscard]]
  bool optional() const {
    return optional_;
  }

  void set_optional(bool optional) { optional_ = optional; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_ListElementExpr(cel_Ast* cel_nonnull ast)
      : cel_ExprLinkCrtp(ast, cel_ExprKind_kListElement) {}

  cel_Expr* cel_nullable value_ = nullptr;
  bool optional_ = false;
};

class cel_ListExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_ExprRange<cel_ListElementExpr> elements() const {
    return cel_ExprRange<cel_ListElementExpr>(&elems_head_, &elems_tail_);
  }

  void AppendElement(cel_ListElementExpr* cel_nonnull element) {
    LinkChild(&elems_head_, &elems_tail_, &elems_, nullptr, element);
  }

  void PrependElement(cel_ListElementExpr* cel_nonnull element) {
    LinkChild(&elems_head_, &elems_tail_, &elems_, elems_head_, element);
  }

  void InsertElementBefore(cel_ListElementExpr* cel_nullable before,
                           cel_ListElementExpr* cel_nonnull element) {
    LinkChild(&elems_head_, &elems_tail_, &elems_, before, element);
  }

  void InsertElementAfter(cel_ListElementExpr* cel_nullable after,
                          cel_ListElementExpr* cel_nonnull element) {
    InsertElementBefore(NextChild(after), element);
  }

  cel_ListElementExpr* cel_nonnull
  ReleaseElement(cel_ListElementExpr* cel_nonnull element) {
    UnlinkChild(&elems_head_, &elems_tail_, &elems_, element);
    return element;
  }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_ListExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kList) {}

  cel_ExprLink* cel_nullable elems_head_ = nullptr;
  cel_ExprLink* cel_nullable elems_tail_ = nullptr;
  size_t elems_ = 0;
};

class cel_MapEntryExpr : public cel_ExprLinkCrtp<cel_MapEntryExpr> {
 public:
  [[nodiscard]]
  cel_MapExpr* cel_nullable parent() const {
    return cel_ExprLink::parent() != nullptr ? cel_ExprLink::parent()->GetMap()
                                             : nullptr;
  }

  [[nodiscard]]
  cel_Expr* cel_nullable key() const {
    return key_;
  }

  cel_Expr* cel_nullable SetKey(cel_Expr* cel_nullable key) {
    return SetChild(&key_, key);
  }

  cel_Expr* cel_nullable ReleaseKey() { return ReleaseChild(&key_); }

  [[nodiscard]]
  cel_Expr* cel_nullable value() const {
    return value_;
  }

  cel_Expr* cel_nullable SetValue(cel_Expr* cel_nullable value) {
    return SetChild(&value_, value);
  }

  cel_Expr* cel_nullable ReleaseValue() { return ReleaseChild(&value_); }

  [[nodiscard]]
  bool optional() const {
    return optional_;
  }

  void set_optional(bool optional) { optional_ = optional; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_MapEntryExpr(cel_Ast* cel_nonnull ast)
      : cel_ExprLinkCrtp(ast, cel_ExprKind_kMapEntry) {}

  cel_Expr* cel_nullable key_ = nullptr;
  cel_Expr* cel_nullable value_ = nullptr;
  bool optional_ = false;
};

class cel_MapExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_ExprRange<cel_MapEntryExpr> entries() const {
    return cel_ExprRange<cel_MapEntryExpr>(&ents_head_, &ents_tail_);
  }

  void AppendEntry(cel_MapEntryExpr* cel_nonnull entry) {
    LinkChild(&ents_head_, &ents_tail_, &ents_, nullptr, entry);
  }

  void PrependEntry(cel_MapEntryExpr* cel_nonnull entry) {
    LinkChild(&ents_head_, &ents_tail_, &ents_, ents_head_, entry);
  }

  void InsertEntryBefore(cel_MapEntryExpr* cel_nullable before,
                         cel_MapEntryExpr* cel_nonnull entry) {
    LinkChild(&ents_head_, &ents_tail_, &ents_, before, entry);
  }

  void InsertEntryAfter(cel_MapEntryExpr* cel_nullable after,
                        cel_MapEntryExpr* cel_nonnull entry) {
    InsertEntryBefore(NextChild(after), entry);
  }

  cel_MapEntryExpr* cel_nonnull
  ReleaseEntry(cel_MapEntryExpr* cel_nonnull entry) {
    UnlinkChild(&ents_head_, &ents_tail_, &ents_, entry);
    return entry;
  }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_MapExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kMap) {}

  cel_ExprLink* cel_nullable ents_head_ = nullptr;
  cel_ExprLink* cel_nullable ents_tail_ = nullptr;
  size_t ents_ = 0;
};

class cel_StructFieldExpr : public cel_ExprLinkCrtp<cel_StructFieldExpr> {
 public:
  [[nodiscard]]
  cel_StructExpr* cel_nullable parent() const {
    return cel_ExprLink::parent() != nullptr
               ? cel_ExprLink::parent()->GetStruct()
               : nullptr;
  }

  [[nodiscard]]
  cel_StringView name() const {
    return name_;
  }

  void set_name(cel_StringView name) { name_ = name; }

  [[nodiscard]]
  cel_Expr* cel_nullable value() const {
    return value_;
  }

  cel_Expr* cel_nullable SetValue(cel_Expr* cel_nullable value) {
    return SetChild(&value_, value);
  }

  cel_Expr* cel_nullable ReleaseValue() { return ReleaseChild(&value_); }

  [[nodiscard]]
  bool optional() const {
    return optional_;
  }

  void set_optional(bool optional) { optional_ = optional; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_StructFieldExpr(cel_Ast* cel_nonnull ast)
      : cel_ExprLinkCrtp(ast, cel_ExprKind_kStructField) {}

  cel_StringView name_ = CEL_STRINGVIEW_INIT("", 0);
  cel_Expr* cel_nullable value_ = nullptr;
  bool optional_ = false;
};

class cel_StructExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_StringView name() const {
    return name_;
  }

  void set_name(cel_StringView name) { name_ = name; }

  [[nodiscard]]
  cel_ExprRange<cel_StructFieldExpr> fields() const {
    return cel_ExprRange<cel_StructFieldExpr>(&flds_head_, &flds_tail_);
  }

  void AppendField(cel_StructFieldExpr* cel_nonnull field) {
    LinkChild(&flds_head_, &flds_tail_, &flds_, nullptr, field);
  }

  void PrependField(cel_StructFieldExpr* cel_nonnull field) {
    LinkChild(&flds_head_, &flds_tail_, &flds_, flds_head_, field);
  }

  void InsertFieldBefore(cel_StructFieldExpr* cel_nullable before,
                         cel_StructFieldExpr* cel_nonnull field) {
    LinkChild(&flds_head_, &flds_tail_, &flds_, before, field);
  }

  void InsertFieldAfter(cel_StructFieldExpr* cel_nullable after,
                        cel_StructFieldExpr* cel_nonnull field) {
    InsertFieldBefore(NextChild(after), field);
  }

  cel_StructFieldExpr* cel_nonnull
  ReleaseField(cel_StructFieldExpr* cel_nonnull field) {
    UnlinkChild(&flds_head_, &flds_tail_, &flds_, field);
    return field;
  }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_StructExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kStruct) {}

  cel_StringView name_ = CEL_STRINGVIEW_INIT("", 0);
  cel_ExprLink* cel_nullable flds_head_ = nullptr;
  cel_ExprLink* cel_nullable flds_tail_ = nullptr;
  size_t flds_ = 0;
};

class cel_ComprehensionExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_StringView iter_var() const {
    return iter_var_;
  }

  void set_iter_var(cel_StringView iter_var) { iter_var_ = iter_var; }

  [[nodiscard]]
  cel_StringView iter_var2() const {
    return iter_var2_;
  }

  void set_iter_var2(cel_StringView iter_var2) { iter_var2_ = iter_var2; }

  [[nodiscard]]
  cel_StringView accu_var() const {
    return accu_var_;
  }

  void set_accu_var(cel_StringView accu_var) { accu_var_ = accu_var; }

  [[nodiscard]]
  cel_Expr* cel_nullable iter_range() const {
    return iter_range_;
  }

  cel_Expr* cel_nullable SetIterRange(cel_Expr* cel_nullable iter_range) {
    return SetChild(&iter_range_, iter_range);
  }

  cel_Expr* cel_nullable ReleaseIterRange() {
    return ReleaseChild(&iter_range_);
  }

  [[nodiscard]]
  cel_Expr* cel_nullable accu_init() const {
    return accu_init_;
  }

  cel_Expr* cel_nullable SetAccuInit(cel_Expr* cel_nullable accu_init) {
    return SetChild(&accu_init_, accu_init);
  }

  cel_Expr* cel_nullable ReleaseAccuInit() { return ReleaseChild(&accu_init_); }

  [[nodiscard]]
  cel_Expr* cel_nullable loop_condition() const {
    return loop_condition_;
  }

  cel_Expr* cel_nullable
  SetLoopCondition(cel_Expr* cel_nullable loop_condition) {
    return SetChild(&loop_condition_, loop_condition);
  }

  cel_Expr* cel_nullable ReleaseLoopCondition() {
    return ReleaseChild(&loop_condition_);
  }

  [[nodiscard]]
  cel_Expr* cel_nullable loop_step() const {
    return loop_step_;
  }

  cel_Expr* cel_nullable SetLoopStep(cel_Expr* cel_nullable loop_step) {
    return SetChild(&loop_step_, loop_step);
  }

  cel_Expr* cel_nullable ReleaseLoopStep() { return ReleaseChild(&loop_step_); }

  [[nodiscard]]
  cel_Expr* cel_nullable result() const {
    return result_;
  }

  cel_Expr* cel_nullable SetResult(cel_Expr* cel_nullable result) {
    return SetChild(&result_, result);
  }

  cel_Expr* cel_nullable ReleaseResult() { return ReleaseChild(&result_); }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_ComprehensionExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kComprehension) {}

  cel_StringView iter_var_ = CEL_STRINGVIEW_INIT("", 0);
  cel_StringView iter_var2_ = CEL_STRINGVIEW_INIT("", 0);
  cel_Expr* cel_nullable iter_range_ = nullptr;
  cel_StringView accu_var_ = CEL_STRINGVIEW_INIT("", 0);
  cel_Expr* cel_nullable accu_init_ = nullptr;
  cel_Expr* cel_nullable loop_condition_ = nullptr;
  cel_Expr* cel_nullable loop_step_ = nullptr;
  cel_Expr* cel_nullable result_ = nullptr;
};

class cel_BindExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_StringView var() const {
    return var_;
  }

  void set_var(cel_StringView var) { var_ = var; }

  [[nodiscard]]
  cel_Expr* cel_nullable binding() const {
    return binding_;
  }

  cel_Expr* cel_nullable SetBinding(cel_Expr* cel_nullable binding) {
    return SetChild(&binding_, binding);
  }

  cel_Expr* cel_nullable ReleaseBinding() { return ReleaseChild(&binding_); }

  [[nodiscard]]
  cel_Expr* cel_nullable bound() const {
    return bound_;
  }

  cel_Expr* cel_nullable SetBound(cel_Expr* cel_nullable bound) {
    return SetChild(&bound_, bound);
  }

  cel_Expr* cel_nullable ReleaseBound() { return ReleaseChild(&bound_); }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_BindExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kBind) {}

  cel_StringView var_ = CEL_STRINGVIEW_INIT("", 0);
  cel_Expr* cel_nullable binding_ = nullptr;
  cel_Expr* cel_nullable bound_ = nullptr;
};

class cel_BlockBindingExpr : public cel_ExprLinkCrtp<cel_BlockBindingExpr> {
 public:
  [[nodiscard]]
  cel_BlockExpr* cel_nullable parent() const {
    return cel_ExprLink::parent() != nullptr
               ? cel_ExprLink::parent()->GetBlock()
               : nullptr;
  }

  [[nodiscard]]
  cel_Expr* cel_nullable bound() const {
    return bound_;
  }

  cel_Expr* cel_nullable SetBound(cel_Expr* cel_nullable bound) {
    return SetChild(&bound_, bound);
  }

  cel_Expr* cel_nullable ReleaseBound() { return ReleaseChild(&bound_); }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_BlockBindingExpr(cel_Ast* cel_nonnull ast)
      : cel_ExprLinkCrtp(ast, cel_ExprKind_kBlockBinding) {}

  cel_Expr* cel_nullable bound_ = nullptr;
};

class cel_BlockIndexExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_BlockExpr* cel_nullable Block() const {
    cel_Expr* p = parent();
    while (p != nullptr) {
      if (p->IsBlock()) {
        return p->GetBlock();
      }
      p = p->parent();
    }
    return nullptr;
  }

  [[nodiscard]]
  int64_t value() const {
    return value_;
  }

  void set_value(int64_t value) { value_ = value; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_BlockIndexExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kBlockIndex) {}

  int64_t value_ = -1;
};

class cel_BlockExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_ExprRange<cel_BlockBindingExpr> bindings() const {
    return cel_ExprRange<cel_BlockBindingExpr>(&bindings_head_,
                                               &bindings_tail_);
  }

  void AppendBinding(cel_BlockBindingExpr* cel_nonnull binding) {
    LinkChild(&bindings_head_, &bindings_tail_, &bindings_, nullptr, binding);
  }

  void PrependBinding(cel_BlockBindingExpr* cel_nonnull binding) {
    LinkChild(&bindings_head_, &bindings_tail_, &bindings_, bindings_head_,
              binding);
  }

  void InsertBindingBefore(cel_BlockBindingExpr* cel_nullable before,
                           cel_BlockBindingExpr* cel_nonnull binding) {
    LinkChild(&bindings_head_, &bindings_tail_, &bindings_, before, binding);
  }

  void InsertBindingAfter(cel_BlockBindingExpr* cel_nullable after,
                          cel_BlockBindingExpr* cel_nonnull binding) {
    InsertBindingBefore(NextChild(after), binding);
  }

  cel_BlockBindingExpr* cel_nonnull
  ReleaseBinding(cel_BlockBindingExpr* cel_nonnull binding) {
    UnlinkChild(&bindings_head_, &bindings_tail_, &bindings_, binding);
    return binding;
  }

  [[nodiscard]]
  cel_Expr* cel_nullable bound() const {
    return bound_;
  }

  cel_Expr* cel_nullable SetBound(cel_Expr* cel_nullable bound) {
    return SetChild(&bound_, bound);
  }

  cel_Expr* cel_nullable ReleaseBound() { return ReleaseChild(&bound_); }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_BlockExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kBlock) {}

  cel_ExprLink* cel_nullable bindings_head_ = nullptr;
  cel_ExprLink* cel_nullable bindings_tail_ = nullptr;
  size_t bindings_ = 0;
  cel_Expr* cel_nullable bound_ = nullptr;
};

class cel_AttrQualExpr : public cel_ExprLinkCrtp<cel_AttrQualExpr> {
 public:
  [[nodiscard]]
  cel_AttrExpr* cel_nullable parent() const {
    return cel_ExprLink::parent() != nullptr ? cel_ExprLink::parent()->GetAttr()
                                             : nullptr;
  }

  [[nodiscard]]
  const cel_AttributeQualifier& value() const CEL_ATTRIBUTE_LIFETIME_BOUND {
    return value_;
  }

  [[nodiscard]]
  cel_AttributeQualifier& value() CEL_ATTRIBUTE_LIFETIME_BOUND {
    return value_;
  }

  void set_value(cel_AttributeQualifier value) { value_ = value; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_AttrQualExpr(cel_Ast* cel_nonnull ast)
      : cel_ExprLinkCrtp(ast, cel_ExprKind_kAttrQual) {}

  cel_AttributeQualifier value_ = cel_UnspecifiedAttributeQualifier();
};

class cel_AttrExpr : public cel_Expr {
 public:
  [[nodiscard]]
  cel_Expr* cel_nullable operand() const {
    return operand_;
  }

  cel_Expr* cel_nullable SetOperand(cel_Expr* cel_nullable operand) {
    return SetChild(&operand_, operand);
  }

  cel_Expr* cel_nullable ReleaseOperand() { return ReleaseChild(&operand_); }

  [[nodiscard]]
  cel_ExprRange<cel_AttrQualExpr> qualifiers() const {
    return cel_ExprRange<cel_AttrQualExpr>(&quals_head_, &quals_tail_);
  }

  void AppendQualifier(cel_AttrQualExpr* cel_nonnull qual) {
    LinkChild(&quals_head_, &quals_tail_, &quals_, nullptr, qual);
  }

  void PrependQualifier(cel_AttrQualExpr* cel_nonnull qual) {
    LinkChild(&quals_head_, &quals_tail_, &quals_, quals_head_, qual);
  }

  void InsertQualifierBefore(cel_AttrQualExpr* cel_nullable before,
                             cel_AttrQualExpr* cel_nonnull qual) {
    LinkChild(&quals_head_, &quals_tail_, &quals_, before, qual);
  }

  void InsertQualifierAfter(cel_AttrQualExpr* cel_nullable after,
                            cel_AttrQualExpr* cel_nonnull qual) {
    InsertQualifierBefore(NextChild(after), qual);
  }

  cel_AttrQualExpr* cel_nonnull
  ReleaseQualifier(cel_AttrQualExpr* cel_nonnull qual) {
    UnlinkChild(&quals_head_, &quals_tail_, &quals_, qual);
    return qual;
  }

  [[nodiscard]]
  bool test_only() const {
    return test_only_;
  }

  void set_test_only(bool test_only) { test_only_ = test_only; }

  CEL_EXPR_DELETED_MEMBER_FUNCTIONS;

 private:
  friend class cel_Ast;

  explicit cel_AttrExpr(cel_Ast* cel_nonnull ast)
      : cel_Expr(ast, cel_ExprKind_kAttr) {}

  cel_ExprLink* cel_nullable quals_head_ = nullptr;
  cel_ExprLink* cel_nullable quals_tail_ = nullptr;
  size_t quals_ = 0;
  cel_Expr* cel_nullable operand_ = nullptr;
  bool test_only_ = false;
};

[[nodiscard]]
inline cel_UnspecifiedExpr* cel_nonnull cel_Expr::GetUnspecified() {
  CEL_ASSERT(IsUnspecified());
  return static_cast<cel_UnspecifiedExpr*>(this);
}

[[nodiscard]]
inline cel_IdentExpr* cel_nonnull cel_Expr::GetIdent() {
  CEL_ASSERT(IsIdent());
  return static_cast<cel_IdentExpr*>(this);
}

[[nodiscard]]
inline cel_ConstExpr* cel_nonnull cel_Expr::GetConst() {
  CEL_ASSERT(IsConst());
  return static_cast<cel_ConstExpr*>(this);
}

[[nodiscard]]
inline cel_SelectExpr* cel_nonnull cel_Expr::GetSelect() {
  CEL_ASSERT(IsSelect());
  return static_cast<cel_SelectExpr*>(this);
}

[[nodiscard]]
inline cel_CallArgExpr* cel_nonnull cel_Expr::GetCallArg() {
  CEL_ASSERT(IsCallArg());
  return static_cast<cel_CallArgExpr*>(this);
}

[[nodiscard]]
inline cel_CallExpr* cel_nonnull cel_Expr::GetCall() {
  CEL_ASSERT(IsCall());
  return static_cast<cel_CallExpr*>(this);
}

[[nodiscard]]
inline cel_UnaryExpr* cel_nonnull cel_Expr::GetUnary() {
  CEL_ASSERT(IsUnary());
  return static_cast<cel_UnaryExpr*>(this);
}

[[nodiscard]]
inline cel_BinaryExpr* cel_nonnull cel_Expr::GetBinary() {
  CEL_ASSERT(IsBinary());
  return static_cast<cel_BinaryExpr*>(this);
}

[[nodiscard]]
inline cel_TernaryExpr* cel_nonnull cel_Expr::GetTernary() {
  CEL_ASSERT(IsTernary());
  return static_cast<cel_TernaryExpr*>(this);
}

[[nodiscard]]
inline cel_ListElementExpr* cel_nonnull cel_Expr::GetListElement() {
  CEL_ASSERT(IsListElement());
  return static_cast<cel_ListElementExpr*>(this);
}

[[nodiscard]]
inline cel_ListExpr* cel_nonnull cel_Expr::GetList() {
  CEL_ASSERT(IsList());
  return static_cast<cel_ListExpr*>(this);
}

[[nodiscard]]
inline cel_MapEntryExpr* cel_nonnull cel_Expr::GetMapEntry() {
  CEL_ASSERT(IsMapEntry());
  return static_cast<cel_MapEntryExpr*>(this);
}

[[nodiscard]]
inline cel_MapExpr* cel_nonnull cel_Expr::GetMap() {
  CEL_ASSERT(IsMap());
  return static_cast<cel_MapExpr*>(this);
}

[[nodiscard]]
inline cel_StructFieldExpr* cel_nonnull cel_Expr::GetStructField() {
  CEL_ASSERT(IsStructField());
  return static_cast<cel_StructFieldExpr*>(this);
}

[[nodiscard]]
inline cel_StructExpr* cel_nonnull cel_Expr::GetStruct() {
  CEL_ASSERT(IsStruct());
  return static_cast<cel_StructExpr*>(this);
}

[[nodiscard]]
inline cel_ComprehensionExpr* cel_nonnull cel_Expr::GetComprehension() {
  CEL_ASSERT(IsComprehension());
  return static_cast<cel_ComprehensionExpr*>(this);
}

[[nodiscard]]
inline cel_BindExpr* cel_nonnull cel_Expr::GetBind() {
  CEL_ASSERT(IsBind());
  return static_cast<cel_BindExpr*>(this);
}

[[nodiscard]]
inline cel_BlockIndexExpr* cel_nonnull cel_Expr::GetBlockIndex() {
  CEL_ASSERT(IsBlockIndex());
  return static_cast<cel_BlockIndexExpr*>(this);
}

[[nodiscard]]
inline cel_BlockBindingExpr* cel_nonnull cel_Expr::GetBlockBinding() {
  CEL_ASSERT(IsBlockBinding());
  return static_cast<cel_BlockBindingExpr*>(this);
}

[[nodiscard]]
inline cel_BlockExpr* cel_nonnull cel_Expr::GetBlock() {
  CEL_ASSERT(IsBlock());
  return static_cast<cel_BlockExpr*>(this);
}

[[nodiscard]]
inline cel_AttrQualExpr* cel_nonnull cel_Expr::GetAttrQual() {
  CEL_ASSERT(IsAttrQual());
  return static_cast<cel_AttrQualExpr*>(this);
}

[[nodiscard]]
inline cel_AttrExpr* cel_nonnull cel_Expr::GetAttr() {
  CEL_ASSERT(IsAttr());
  return static_cast<cel_AttrExpr*>(this);
}

[[nodiscard]]
inline const cel_UnspecifiedExpr* cel_nonnull cel_Expr::GetUnspecified() const {
  CEL_ASSERT(IsUnspecified());
  return static_cast<const cel_UnspecifiedExpr*>(this);
}

[[nodiscard]]
inline const cel_IdentExpr* cel_nonnull cel_Expr::GetIdent() const {
  CEL_ASSERT(IsIdent());
  return static_cast<const cel_IdentExpr*>(this);
}

[[nodiscard]]
inline const cel_ConstExpr* cel_nonnull cel_Expr::GetConst() const {
  CEL_ASSERT(IsConst());
  return static_cast<const cel_ConstExpr*>(this);
}

[[nodiscard]]
inline const cel_SelectExpr* cel_nonnull cel_Expr::GetSelect() const {
  CEL_ASSERT(IsSelect());
  return static_cast<const cel_SelectExpr*>(this);
}

[[nodiscard]]
inline const cel_CallArgExpr* cel_nonnull cel_Expr::GetCallArg() const {
  CEL_ASSERT(IsCallArg());
  return static_cast<const cel_CallArgExpr*>(this);
}

[[nodiscard]]
inline const cel_CallExpr* cel_nonnull cel_Expr::GetCall() const {
  CEL_ASSERT(IsCall());
  return static_cast<const cel_CallExpr*>(this);
}

[[nodiscard]]
inline const cel_UnaryExpr* cel_nonnull cel_Expr::GetUnary() const {
  CEL_ASSERT(IsUnary());
  return static_cast<const cel_UnaryExpr*>(this);
}

[[nodiscard]]
inline const cel_BinaryExpr* cel_nonnull cel_Expr::GetBinary() const {
  CEL_ASSERT(IsBinary());
  return static_cast<const cel_BinaryExpr*>(this);
}

[[nodiscard]]
inline const cel_TernaryExpr* cel_nonnull cel_Expr::GetTernary() const {
  CEL_ASSERT(IsTernary());
  return static_cast<const cel_TernaryExpr*>(this);
}

[[nodiscard]]
inline const cel_ListElementExpr* cel_nonnull cel_Expr::GetListElement() const {
  CEL_ASSERT(IsListElement());
  return static_cast<const cel_ListElementExpr*>(this);
}

[[nodiscard]]
inline const cel_ListExpr* cel_nonnull cel_Expr::GetList() const {
  CEL_ASSERT(IsList());
  return static_cast<const cel_ListExpr*>(this);
}

[[nodiscard]]
inline const cel_MapEntryExpr* cel_nonnull cel_Expr::GetMapEntry() const {
  CEL_ASSERT(IsMapEntry());
  return static_cast<const cel_MapEntryExpr*>(this);
}

[[nodiscard]]
inline const cel_MapExpr* cel_nonnull cel_Expr::GetMap() const {
  CEL_ASSERT(IsMap());
  return static_cast<const cel_MapExpr*>(this);
}

[[nodiscard]]
inline const cel_StructFieldExpr* cel_nonnull cel_Expr::GetStructField() const {
  CEL_ASSERT(IsStructField());
  return static_cast<const cel_StructFieldExpr*>(this);
}

[[nodiscard]]
inline const cel_StructExpr* cel_nonnull cel_Expr::GetStruct() const {
  CEL_ASSERT(IsStruct());
  return static_cast<const cel_StructExpr*>(this);
}

[[nodiscard]]
inline const cel_ComprehensionExpr* cel_nonnull
cel_Expr::GetComprehension() const {
  CEL_ASSERT(IsComprehension());
  return static_cast<const cel_ComprehensionExpr*>(this);
}

[[nodiscard]]
inline const cel_BindExpr* cel_nonnull cel_Expr::GetBind() const {
  CEL_ASSERT(IsBind());
  return static_cast<const cel_BindExpr*>(this);
}

[[nodiscard]]
inline const cel_BlockIndexExpr* cel_nonnull cel_Expr::GetBlockIndex() const {
  CEL_ASSERT(IsBlockIndex());
  return static_cast<const cel_BlockIndexExpr*>(this);
}

[[nodiscard]]
inline const cel_BlockBindingExpr* cel_nonnull
cel_Expr::GetBlockBinding() const {
  CEL_ASSERT(IsBlockBinding());
  return static_cast<const cel_BlockBindingExpr*>(this);
}

[[nodiscard]]
inline const cel_BlockExpr* cel_nonnull cel_Expr::GetBlock() const {
  CEL_ASSERT(IsBlock());
  return static_cast<const cel_BlockExpr*>(this);
}

[[nodiscard]]
inline const cel_AttrQualExpr* cel_nonnull cel_Expr::GetAttrQual() const {
  CEL_ASSERT(IsAttrQual());
  return static_cast<const cel_AttrQualExpr*>(this);
}

[[nodiscard]]
inline const cel_AttrExpr* cel_nonnull cel_Expr::GetAttr() const {
  CEL_ASSERT(IsAttr());
  return static_cast<const cel_AttrExpr*>(this);
}

[[nodiscard]]
inline cel_UnspecifiedExpr* cel_nullable cel_Expr::AsUnspecified() {
  return IsUnspecified() ? static_cast<cel_UnspecifiedExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_IdentExpr* cel_nullable cel_Expr::AsIdent() {
  return IsIdent() ? static_cast<cel_IdentExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_ConstExpr* cel_nullable cel_Expr::AsConst() {
  return IsConst() ? static_cast<cel_ConstExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_SelectExpr* cel_nullable cel_Expr::AsSelect() {
  return IsSelect() ? static_cast<cel_SelectExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_CallArgExpr* cel_nullable cel_Expr::AsCallArg() {
  return IsCallArg() ? static_cast<cel_CallArgExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_CallExpr* cel_nullable cel_Expr::AsCall() {
  return IsCall() ? static_cast<cel_CallExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_UnaryExpr* cel_nullable cel_Expr::AsUnary() {
  return IsUnary() ? static_cast<cel_UnaryExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_BinaryExpr* cel_nullable cel_Expr::AsBinary() {
  return IsBinary() ? static_cast<cel_BinaryExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_TernaryExpr* cel_nullable cel_Expr::AsTernary() {
  return IsTernary() ? static_cast<cel_TernaryExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_ListElementExpr* cel_nullable cel_Expr::AsListElement() {
  return IsListElement() ? static_cast<cel_ListElementExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_ListExpr* cel_nullable cel_Expr::AsList() {
  return IsList() ? static_cast<cel_ListExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_MapEntryExpr* cel_nullable cel_Expr::AsMapEntry() {
  return IsMapEntry() ? static_cast<cel_MapEntryExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_MapExpr* cel_nullable cel_Expr::AsMap() {
  return IsMap() ? static_cast<cel_MapExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_StructFieldExpr* cel_nullable cel_Expr::AsStructField() {
  return IsStructField() ? static_cast<cel_StructFieldExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_StructExpr* cel_nullable cel_Expr::AsStruct() {
  return IsStruct() ? static_cast<cel_StructExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_ComprehensionExpr* cel_nullable cel_Expr::AsComprehension() {
  return IsComprehension() ? static_cast<cel_ComprehensionExpr*>(this)
                           : nullptr;
}

[[nodiscard]]
inline cel_BindExpr* cel_nullable cel_Expr::AsBind() {
  return IsBind() ? static_cast<cel_BindExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_BlockIndexExpr* cel_nullable cel_Expr::AsBlockIndex() {
  return IsBlockIndex() ? static_cast<cel_BlockIndexExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_BlockBindingExpr* cel_nullable cel_Expr::AsBlockBinding() {
  return IsBlockBinding() ? static_cast<cel_BlockBindingExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_BlockExpr* cel_nullable cel_Expr::AsBlock() {
  return IsBlock() ? static_cast<cel_BlockExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_AttrQualExpr* cel_nullable cel_Expr::AsAttrQual() {
  return IsAttrQual() ? static_cast<cel_AttrQualExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_AttrExpr* cel_nullable cel_Expr::AsAttr() {
  return IsAttr() ? static_cast<cel_AttrExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_UnspecifiedExpr* cel_nullable cel_Expr::AsUnspecified() const {
  return IsUnspecified() ? static_cast<const cel_UnspecifiedExpr*>(this)
                         : nullptr;
}

[[nodiscard]]
inline const cel_IdentExpr* cel_nullable cel_Expr::AsIdent() const {
  return IsIdent() ? static_cast<const cel_IdentExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_ConstExpr* cel_nullable cel_Expr::AsConst() const {
  return IsConst() ? static_cast<const cel_ConstExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_SelectExpr* cel_nullable cel_Expr::AsSelect() const {
  return IsSelect() ? static_cast<const cel_SelectExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_CallArgExpr* cel_nullable cel_Expr::AsCallArg() const {
  return IsCallArg() ? static_cast<const cel_CallArgExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_CallExpr* cel_nullable cel_Expr::AsCall() const {
  return IsCall() ? static_cast<const cel_CallExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_UnaryExpr* cel_nullable cel_Expr::AsUnary() const {
  return IsUnary() ? static_cast<const cel_UnaryExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_BinaryExpr* cel_nullable cel_Expr::AsBinary() const {
  return IsBinary() ? static_cast<const cel_BinaryExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_TernaryExpr* cel_nullable cel_Expr::AsTernary() const {
  return IsTernary() ? static_cast<const cel_TernaryExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_ListElementExpr* cel_nullable cel_Expr::AsListElement() const {
  return IsListElement() ? static_cast<const cel_ListElementExpr*>(this)
                         : nullptr;
}

[[nodiscard]]
inline const cel_ListExpr* cel_nullable cel_Expr::AsList() const {
  return IsList() ? static_cast<const cel_ListExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_MapEntryExpr* cel_nullable cel_Expr::AsMapEntry() const {
  return IsMapEntry() ? static_cast<const cel_MapEntryExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_MapExpr* cel_nullable cel_Expr::AsMap() const {
  return IsMap() ? static_cast<const cel_MapExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_StructFieldExpr* cel_nullable cel_Expr::AsStructField() const {
  return IsStructField() ? static_cast<const cel_StructFieldExpr*>(this)
                         : nullptr;
}

[[nodiscard]]
inline const cel_StructExpr* cel_nullable cel_Expr::AsStruct() const {
  return IsStruct() ? static_cast<const cel_StructExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_ComprehensionExpr* cel_nullable
cel_Expr::AsComprehension() const {
  return IsComprehension() ? static_cast<const cel_ComprehensionExpr*>(this)
                           : nullptr;
}

[[nodiscard]]
inline const cel_BindExpr* cel_nullable cel_Expr::AsBind() const {
  return IsBind() ? static_cast<const cel_BindExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_BlockIndexExpr* cel_nullable cel_Expr::AsBlockIndex() const {
  return IsBlockIndex() ? static_cast<const cel_BlockIndexExpr*>(this)
                        : nullptr;
}

[[nodiscard]]
inline const cel_BlockBindingExpr* cel_nullable
cel_Expr::AsBlockBinding() const {
  return IsBlockBinding() ? static_cast<const cel_BlockBindingExpr*>(this)
                          : nullptr;
}

[[nodiscard]]
inline const cel_BlockExpr* cel_nullable cel_Expr::AsBlock() const {
  return IsBlock() ? static_cast<const cel_BlockExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_AttrQualExpr* cel_nullable cel_Expr::AsAttrQual() const {
  return IsAttrQual() ? static_cast<const cel_AttrQualExpr*>(this) : nullptr;
}

[[nodiscard]]
inline const cel_AttrExpr* cel_nullable cel_Expr::AsAttr() const {
  return IsAttr() ? static_cast<const cel_AttrExpr*>(this) : nullptr;
}

[[nodiscard]]
inline cel_UnspecifiedExpr* cel_nullable cel_Ast::NewUnspecifiedExpr() {
  return NewExpr<cel_UnspecifiedExpr>();
}

[[nodiscard]]
inline cel_IdentExpr* cel_nullable cel_Ast::NewIdentExpr() {
  return NewExpr<cel_IdentExpr>();
}

[[nodiscard]]
inline cel_ConstExpr* cel_nullable cel_Ast::NewConstExpr() {
  return NewExpr<cel_ConstExpr>();
}

[[nodiscard]]
inline cel_SelectExpr* cel_nullable cel_Ast::NewSelectExpr() {
  return NewExpr<cel_SelectExpr>();
}

[[nodiscard]]
inline cel_CallArgExpr* cel_nullable cel_Ast::NewCallArgExpr() {
  return NewExpr<cel_CallArgExpr>();
}

[[nodiscard]]
inline cel_CallExpr* cel_nullable cel_Ast::NewCallExpr() {
  return NewExpr<cel_CallExpr>();
}

[[nodiscard]]
inline cel_UnaryExpr* cel_nullable cel_Ast::NewUnaryExpr() {
  return NewExpr<cel_UnaryExpr>();
}

[[nodiscard]]
inline cel_BinaryExpr* cel_nullable cel_Ast::NewBinaryExpr() {
  return NewExpr<cel_BinaryExpr>();
}

[[nodiscard]]
inline cel_TernaryExpr* cel_nullable cel_Ast::NewTernaryExpr() {
  return NewExpr<cel_TernaryExpr>();
}

[[nodiscard]]
inline cel_ListElementExpr* cel_nullable cel_Ast::NewListElementExpr() {
  return NewExpr<cel_ListElementExpr>();
}

[[nodiscard]]
inline cel_ListExpr* cel_nullable cel_Ast::NewListExpr() {
  return NewExpr<cel_ListExpr>();
}

[[nodiscard]]
inline cel_MapEntryExpr* cel_nullable cel_Ast::NewMapEntryExpr() {
  return NewExpr<cel_MapEntryExpr>();
}

[[nodiscard]]
inline cel_MapExpr* cel_nullable cel_Ast::NewMapExpr() {
  return NewExpr<cel_MapExpr>();
}

[[nodiscard]]
inline cel_StructFieldExpr* cel_nullable cel_Ast::NewStructFieldExpr() {
  return NewExpr<cel_StructFieldExpr>();
}

[[nodiscard]]
inline cel_StructExpr* cel_nullable cel_Ast::NewStructExpr() {
  return NewExpr<cel_StructExpr>();
}

[[nodiscard]]
inline cel_ComprehensionExpr* cel_nullable cel_Ast::NewComprehensionExpr() {
  return NewExpr<cel_ComprehensionExpr>();
}

[[nodiscard]]
inline cel_BindExpr* cel_nullable cel_Ast::NewBindExpr() {
  return NewExpr<cel_BindExpr>();
}

[[nodiscard]]
inline cel_BlockIndexExpr* cel_nullable cel_Ast::NewBlockIndexExpr() {
  return NewExpr<cel_BlockIndexExpr>();
}

[[nodiscard]]
inline cel_BlockBindingExpr* cel_nullable cel_Ast::NewBlockBindingExpr() {
  return NewExpr<cel_BlockBindingExpr>();
}

[[nodiscard]]
inline cel_BlockExpr* cel_nullable cel_Ast::NewBlockExpr() {
  return NewExpr<cel_BlockExpr>();
}

[[nodiscard]]
inline cel_AttrQualExpr* cel_nullable cel_Ast::NewAttrQualExpr() {
  return NewExpr<cel_AttrQualExpr>();
}

[[nodiscard]]
inline cel_AttrExpr* cel_nullable cel_Ast::NewAttrExpr() {
  return NewExpr<cel_AttrExpr>();
}

#undef CEL_EXPR_DELETED_MEMBER_FUNCTIONS

// cel_Ast

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Ast* cel_nullable cel_Ast_New(cel_Arena* cel_nonnull arena) {
  return cel_Ast::New(arena);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void _cel_Ast_Delete(cel_Ast* cel_nullable ast) {
  cel_Ast::Delete(ast);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ExprId cel_Ast_NextId(cel_Ast* cel_nonnull ast) {
  return ast->NextId();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ExprId cel_Ast_MaxId(const cel_Ast* cel_nonnull ast) {
  return ast->MaxId();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_Ast_Expr(const cel_Ast* cel_nonnull ast) {
  return ast->expr();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_Ast_SetExpr(cel_Ast* cel_nonnull ast,
                                                  cel_Expr* cel_nullable expr) {
  return ast->SetExpr(expr);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_Ast_ReleaseExpr(cel_Ast* cel_nonnull ast) {
  return ast->ReleaseExpr();
}

// cel_Expr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ExprKind cel_Expr_Kind(const cel_Expr* cel_nonnull expr) {
  return expr->kind();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ExprId cel_Expr_Id(const cel_Expr* cel_nonnull expr) {
  return expr->id();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_Expr_SetId(cel_Expr* cel_nonnull expr, cel_ExprId id) {
  expr->SetId(id);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE int32_t cel_Expr_SourcePosition(const cel_Expr* cel_nonnull expr) {
  return expr->source_position();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_SourceRange
cel_Expr_SourceRange(const cel_Expr* cel_nonnull expr) {
  return expr->source_range();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_Expr_SetSourcePosition(cel_Expr* cel_nonnull expr,
                                           int32_t source_position) {
  expr->SetSourcePosition(source_position);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_Expr_SetSourceRange(cel_Expr* cel_nonnull expr,
                                        cel_SourceRange source_range) {
  expr->SetSourceRange(source_range);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_Expr_Parent(const cel_Expr* cel_nonnull expr) {
  return expr->parent();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE size_t cel_Expr_Depth(const cel_Expr* cel_nonnull expr) {
  return expr->Depth();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE const cel_Ref* cel_nullable
cel_Expr_Ref(const cel_Expr* cel_nonnull expr) {
  return expr->ref();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_Expr_SetRef(cel_Expr* cel_nonnull expr,
                                const cel_Ref* cel_nullable ref) {
  expr->set_ref(ref);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE const cel_Type* cel_nonnull
cel_Expr_Type(const cel_Expr* cel_nonnull expr) {
  return expr->type();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_Expr_SetType(cel_Expr* cel_nonnull expr,
                                 const cel_Type* cel_nonnull type) {
  expr->set_type(type);
}

// cel_UnspecifiedExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_UnspecifiedExpr* cel_nullable
cel_UnspecifiedExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewUnspecifiedExpr();
}

// cel_IdentExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_IdentExpr* cel_nullable
cel_IdentExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewIdentExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_IdentExpr_Name(const cel_IdentExpr* cel_nonnull expr) {
  return expr->name();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_IdentExpr_SetName(cel_IdentExpr* cel_nonnull expr,
                                      cel_StringView name) {
  expr->set_name(name);
}

// cel_ConstExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ConstExpr* cel_nullable
cel_ConstExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewConstExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE const cel_Constant* cel_nonnull
cel_ConstExpr_Value(const cel_ConstExpr* cel_nonnull expr) {
  return &expr->value();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Constant* cel_nonnull
cel_ConstExpr_MutableValue(cel_ConstExpr* cel_nonnull expr) {
  return &expr->value();
}

// cel_SelectExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_SelectExpr* cel_nullable
cel_SelectExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewSelectExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_SelectExpr_Operand(const cel_SelectExpr* cel_nonnull expr) {
  return expr->operand();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_SelectExpr_SetOperand(
    cel_SelectExpr* cel_nonnull expr, cel_Expr* cel_nullable operand) {
  return expr->SetOperand(operand);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_SelectExpr_ReleaseOperand(cel_SelectExpr* cel_nonnull expr) {
  return expr->ReleaseOperand();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_SelectExpr_Field(const cel_SelectExpr* cel_nonnull expr) {
  return expr->field();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_SelectExpr_SetField(cel_SelectExpr* cel_nonnull expr,
                                        cel_StringView field) {
  expr->set_field(field);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE bool cel_SelectExpr_TestOnly(
    const cel_SelectExpr* cel_nonnull expr) {
  return expr->test_only();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_SelectExpr_SetTestOnly(cel_SelectExpr* cel_nonnull expr,
                                           bool test_only) {
  expr->set_test_only(test_only);
}

// cel_CallArgExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_CallArgExpr* cel_nullable
cel_CallArgExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewCallArgExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_CallExpr* cel_nullable
cel_CallArgExpr_Parent(const cel_CallArgExpr* cel_nonnull expr) {
  return expr->parent();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE ptrdiff_t
cel_CallArgExpr_Index(const cel_CallArgExpr* cel_nonnull expr) {
  return expr->index();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_CallArgExpr_Value(const cel_CallArgExpr* cel_nonnull expr) {
  return expr->value();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_CallArgExpr_SetValue(
    cel_CallArgExpr* cel_nonnull expr, cel_Expr* cel_nullable value) {
  return expr->SetValue(value);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_CallArgExpr_ReleaseValue(cel_CallArgExpr* cel_nonnull expr) {
  return expr->ReleaseValue();
}

// cel_CallExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_CallExpr* cel_nullable
cel_CallExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewCallExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_CallExpr_Target(const cel_CallExpr* cel_nonnull expr) {
  return expr->target();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_CallExpr_SetTarget(
    cel_CallExpr* cel_nonnull expr, cel_Expr* cel_nullable target) {
  return expr->SetTarget(target);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_CallExpr_ReleaseTarget(cel_CallExpr* cel_nonnull expr) {
  return expr->ReleaseTarget();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_CallExpr_Function(const cel_CallExpr* cel_nonnull expr) {
  return expr->function();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_CallExpr_SetFunction(cel_CallExpr* cel_nonnull expr,
                                         cel_StringView function) {
  expr->set_function(function);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_CallExpr_AppendArg(cel_CallExpr* cel_nonnull expr,
                                       cel_CallArgExpr* cel_nonnull arg) {
  expr->AppendArg(arg);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_CallExpr_PrependArg(cel_CallExpr* cel_nonnull expr,
                                        cel_CallArgExpr* cel_nonnull arg) {
  expr->PrependArg(arg);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_CallExpr_InsertArg(cel_CallExpr* cel_nonnull expr,
                                       cel_CallArgExpr* cel_nullable before,
                                       cel_CallArgExpr* cel_nonnull arg) {
  expr->InsertArgBefore(before, arg);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_CallArgExpr* cel_nonnull cel_CallExpr_ReleaseArg(
    cel_CallExpr* cel_nonnull expr, cel_CallArgExpr* cel_nonnull arg) {
  return expr->ReleaseArg(arg);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_CallArgExpr* cel_nonnull
cel_CallExpr_Arg(const cel_CallExpr* cel_nonnull expr, size_t index) {
  return expr->args()[index];
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE size_t cel_CallExpr_Args(const cel_CallExpr* cel_nonnull expr,
                                    cel_CallArgExpr * cel_nullable *
                                        cel_nullable head,
                                    cel_CallArgExpr * cel_nullable *
                                        cel_nullable tail) {
  auto args = expr->args();
  if (head != nullptr) {
    *head = args.Front();
  }
  if (tail != nullptr) {
    *tail = args.Back();
  }
  return args.size();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_CallArgExpr* cel_nullable
cel_CallExpr_PrevArg(const cel_CallArgExpr* cel_nullable arg) {
  return arg != nullptr ? arg->prev() : nullptr;
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_CallArgExpr* cel_nullable
cel_CallExpr_NextArg(const cel_CallArgExpr* cel_nullable arg) {
  return arg != nullptr ? arg->next() : nullptr;
}

// cel_UnaryExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_UnaryExpr* cel_nullable
cel_UnaryExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewUnaryExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_UnaryOp cel_UnaryExpr_Op(const cel_UnaryExpr* cel_nonnull expr) {
  return expr->op();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_UnaryExpr_SetOp(cel_UnaryExpr* cel_nonnull expr,
                                    cel_UnaryOp op) {
  expr->set_op(op);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_UnaryExpr_Arg(const cel_UnaryExpr* cel_nonnull expr) {
  return expr->arg();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_UnaryExpr_SetArg(
    cel_UnaryExpr* cel_nonnull expr, cel_Expr* cel_nullable arg) {
  return expr->SetArg(arg);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_UnaryExpr_ReleaseArg(cel_UnaryExpr* cel_nonnull expr) {
  return expr->ReleaseArg();
}

// cel_BinaryExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BinaryExpr* cel_nullable
cel_BinaryExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewBinaryExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BinaryOp
cel_BinaryExpr_Op(const cel_BinaryExpr* cel_nonnull expr) {
  return expr->op();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_BinaryExpr_SetOp(cel_BinaryExpr* cel_nonnull expr,
                                     cel_BinaryOp op) {
  expr->set_op(op);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BinaryExpr_Left(const cel_BinaryExpr* cel_nonnull expr) {
  return expr->left();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_BinaryExpr_SetLeft(
    cel_BinaryExpr* cel_nonnull expr, cel_Expr* cel_nullable left) {
  return expr->SetLeft(left);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BinaryExpr_ReleaseLeft(cel_BinaryExpr* cel_nonnull expr) {
  return expr->ReleaseLeft();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BinaryExpr_Right(const cel_BinaryExpr* cel_nonnull expr) {
  return expr->right();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_BinaryExpr_SetRight(
    cel_BinaryExpr* cel_nonnull expr, cel_Expr* cel_nullable right) {
  return expr->SetRight(right);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BinaryExpr_ReleaseRight(cel_BinaryExpr* cel_nonnull expr) {
  return expr->ReleaseRight();
}

// cel_TernaryExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_TernaryExpr* cel_nullable
cel_TernaryExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewTernaryExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_TernaryOp
cel_TernaryExpr_Op(const cel_TernaryExpr* cel_nonnull expr) {
  return expr->op();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_TernaryExpr_SetOp(cel_TernaryExpr* cel_nonnull expr,
                                      cel_TernaryOp op) {
  expr->set_op(op);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_TernaryExpr_Condition(const cel_TernaryExpr* cel_nonnull expr) {
  return expr->condition();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_TernaryExpr_SetCondition(
    cel_TernaryExpr* cel_nonnull expr, cel_Expr* cel_nullable condition) {
  return expr->SetCondition(condition);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_TernaryExpr_ReleaseCondition(cel_TernaryExpr* cel_nonnull expr) {
  return expr->ReleaseCondition();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_TernaryExpr_IfTrue(const cel_TernaryExpr* cel_nonnull expr) {
  return expr->then();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_TernaryExpr_SetIfTrue(
    cel_TernaryExpr* cel_nonnull expr, cel_Expr* cel_nullable if_true) {
  return expr->SetThen(if_true);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_TernaryExpr_ReleaseIfTrue(cel_TernaryExpr* cel_nonnull expr) {
  return expr->ReleaseThen();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_TernaryExpr_IfFalse(const cel_TernaryExpr* cel_nonnull expr) {
  return expr->otherwise();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_TernaryExpr_SetIfFalse(
    cel_TernaryExpr* cel_nonnull expr, cel_Expr* cel_nullable if_false) {
  return expr->SetOtherwise(if_false);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_TernaryExpr_ReleaseIfFalse(cel_TernaryExpr* cel_nonnull expr) {
  return expr->ReleaseOtherwise();
}

// cel_ListElementExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ListElementExpr* cel_nullable
cel_ListElementExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewListElementExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ListExpr* cel_nullable
cel_ListElementExpr_Parent(const cel_ListElementExpr* cel_nonnull expr) {
  return expr->parent();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE ptrdiff_t
cel_ListElementExpr_Index(const cel_ListElementExpr* cel_nonnull expr) {
  return expr->index();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ListElementExpr_Value(const cel_ListElementExpr* cel_nonnull expr) {
  return expr->value();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_ListElementExpr_SetValue(
    cel_ListElementExpr* cel_nonnull expr, cel_Expr* cel_nullable value) {
  return expr->SetValue(value);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ListElementExpr_ReleaseValue(cel_ListElementExpr* cel_nonnull expr) {
  return expr->ReleaseValue();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE bool cel_ListElementExpr_Optional(
    const cel_ListElementExpr* cel_nonnull expr) {
  return expr->optional();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_ListElementExpr_SetOptional(
    cel_ListElementExpr* cel_nonnull expr, bool optional) {
  expr->set_optional(optional);
}

// cel_ListExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ListExpr* cel_nullable
cel_ListExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewListExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_ListExpr_AppendElement(
    cel_ListExpr* cel_nonnull expr, cel_ListElementExpr* cel_nonnull element) {
  expr->AppendElement(element);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_ListExpr_PrependElement(
    cel_ListExpr* cel_nonnull expr, cel_ListElementExpr* cel_nonnull element) {
  expr->PrependElement(element);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_ListExpr_InsertElement(
    cel_ListExpr* cel_nonnull expr, cel_ListElementExpr* cel_nullable before,
    cel_ListElementExpr* cel_nonnull element) {
  expr->InsertElementBefore(before, element);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ListElementExpr* cel_nonnull cel_ListExpr_ReleaseElement(
    cel_ListExpr* cel_nonnull expr, cel_ListElementExpr* cel_nonnull element) {
  return expr->ReleaseElement(element);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ListElementExpr* cel_nonnull
cel_ListExpr_Element(const cel_ListExpr* cel_nonnull expr, size_t index) {
  return expr->elements()[index];
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE size_t cel_ListExpr_Elements(const cel_ListExpr* cel_nonnull expr,
                                        cel_ListElementExpr * cel_nullable *
                                            cel_nullable head,
                                        cel_ListElementExpr * cel_nullable *
                                            cel_nullable tail) {
  auto elements = expr->elements();
  if (head != nullptr) {
    *head = elements.Front();
  }
  if (tail != nullptr) {
    *tail = elements.Back();
  }
  return elements.size();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ListElementExpr* cel_nullable
cel_ListExpr_PrevElement(const cel_ListElementExpr* cel_nullable element) {
  return element != nullptr ? element->prev() : nullptr;
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ListElementExpr* cel_nullable
cel_ListExpr_NextElement(const cel_ListElementExpr* cel_nullable element) {
  return element != nullptr ? element->next() : nullptr;
}

// cel_MapEntryExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_MapEntryExpr* cel_nullable
cel_MapEntryExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewMapEntryExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_MapExpr* cel_nullable
cel_MapEntryExpr_Parent(const cel_MapEntryExpr* cel_nonnull expr) {
  return expr->parent();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE ptrdiff_t
cel_MapEntryExpr_Index(const cel_MapEntryExpr* cel_nonnull expr) {
  return expr->index();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_MapEntryExpr_Key(const cel_MapEntryExpr* cel_nonnull expr) {
  return expr->key();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_MapEntryExpr_SetKey(
    cel_MapEntryExpr* cel_nonnull expr, cel_Expr* cel_nullable key) {
  return expr->SetKey(key);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_MapEntryExpr_ReleaseKey(cel_MapEntryExpr* cel_nonnull expr) {
  return expr->ReleaseKey();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_MapEntryExpr_Value(const cel_MapEntryExpr* cel_nonnull expr) {
  return expr->value();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_MapEntryExpr_SetValue(
    cel_MapEntryExpr* cel_nonnull expr, cel_Expr* cel_nullable value) {
  return expr->SetValue(value);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_MapEntryExpr_ReleaseValue(cel_MapEntryExpr* cel_nonnull expr) {
  return expr->ReleaseValue();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE bool cel_MapEntryExpr_Optional(
    const cel_MapEntryExpr* cel_nonnull expr) {
  return expr->optional();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_MapEntryExpr_SetOptional(cel_MapEntryExpr* cel_nonnull expr,
                                             bool optional) {
  expr->set_optional(optional);
}

// cel_MapExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_MapExpr* cel_nullable cel_MapExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewMapExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_MapExpr_AppendEntry(cel_MapExpr* cel_nonnull expr,
                                        cel_MapEntryExpr* cel_nonnull entry) {
  expr->AppendEntry(entry);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_MapExpr_PrependEntry(cel_MapExpr* cel_nonnull expr,
                                         cel_MapEntryExpr* cel_nonnull entry) {
  expr->PrependEntry(entry);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_MapExpr_InsertEntry(cel_MapExpr* cel_nonnull expr,
                                        cel_MapEntryExpr* cel_nullable before,
                                        cel_MapEntryExpr* cel_nonnull entry) {
  expr->InsertEntryBefore(before, entry);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_MapEntryExpr* cel_nonnull cel_MapExpr_ReleaseEntry(
    cel_MapExpr* cel_nonnull expr, cel_MapEntryExpr* cel_nonnull entry) {
  return expr->ReleaseEntry(entry);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_MapEntryExpr* cel_nonnull
cel_MapExpr_Entry(const cel_MapExpr* cel_nonnull expr, size_t index) {
  return expr->entries()[index];
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE size_t cel_MapExpr_Entries(const cel_MapExpr* cel_nonnull expr,
                                      cel_MapEntryExpr * cel_nullable *
                                          cel_nullable head,
                                      cel_MapEntryExpr * cel_nullable *
                                          cel_nullable tail) {
  auto entries = expr->entries();
  if (head != nullptr) {
    *head = entries.Front();
  }
  if (tail != nullptr) {
    *tail = entries.Back();
  }
  return entries.size();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_MapEntryExpr* cel_nullable
cel_MapExpr_PrevEntry(const cel_MapEntryExpr* cel_nullable entry) {
  return entry != nullptr ? entry->prev() : nullptr;
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_MapEntryExpr* cel_nullable
cel_MapExpr_NextEntry(const cel_MapEntryExpr* cel_nullable entry) {
  return entry != nullptr ? entry->next() : nullptr;
}

// cel_StructFieldExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StructFieldExpr* cel_nullable
cel_StructFieldExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewStructFieldExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StructExpr* cel_nullable
cel_StructFieldExpr_Parent(const cel_StructFieldExpr* cel_nonnull expr) {
  return expr->parent();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE ptrdiff_t
cel_StructFieldExpr_Index(const cel_StructFieldExpr* cel_nonnull expr) {
  return expr->index();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_StructFieldExpr_Name(const cel_StructFieldExpr* cel_nonnull expr) {
  return expr->name();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_StructFieldExpr_SetName(
    cel_StructFieldExpr* cel_nonnull expr, cel_StringView name) {
  return expr->set_name(name);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_StructFieldExpr_Value(const cel_StructFieldExpr* cel_nonnull expr) {
  return expr->value();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_StructFieldExpr_SetValue(
    cel_StructFieldExpr* cel_nonnull expr, cel_Expr* cel_nullable value) {
  return expr->SetValue(value);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_StructFieldExpr_ReleaseValue(cel_StructFieldExpr* cel_nonnull expr) {
  return expr->ReleaseValue();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE bool cel_StructFieldExpr_Optional(
    const cel_StructFieldExpr* cel_nonnull expr) {
  return expr->optional();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_StructFieldExpr_SetOptional(
    cel_StructFieldExpr* cel_nonnull expr, bool optional) {
  expr->set_optional(optional);
}

// cel_StructExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StructExpr* cel_nullable
cel_StructExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewStructExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_StructExpr_Name(const cel_StructExpr* cel_nonnull expr) {
  return expr->name();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_StructExpr_SetName(cel_StructExpr* cel_nonnull expr,
                                       cel_StringView name) {
  expr->set_name(name);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_StructExpr_AppendField(
    cel_StructExpr* cel_nonnull expr, cel_StructFieldExpr* cel_nonnull field) {
  expr->AppendField(field);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_StructExpr_PrependField(
    cel_StructExpr* cel_nonnull expr, cel_StructFieldExpr* cel_nonnull field) {
  expr->PrependField(field);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_StructExpr_InsertField(
    cel_StructExpr* cel_nonnull expr, cel_StructFieldExpr* cel_nullable before,
    cel_StructFieldExpr* cel_nonnull field) {
  expr->InsertFieldBefore(before, field);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StructFieldExpr* cel_nonnull cel_StructExpr_ReleaseField(
    cel_StructExpr* cel_nonnull expr, cel_StructFieldExpr* cel_nonnull field) {
  return expr->ReleaseField(field);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StructFieldExpr* cel_nonnull
cel_StructExpr_Field(const cel_StructExpr* cel_nonnull expr, size_t index) {
  return expr->fields()[index];
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE size_t cel_StructExpr_Fields(const cel_StructExpr* cel_nonnull expr,
                                        cel_StructFieldExpr * cel_nullable *
                                            cel_nullable head,
                                        cel_StructFieldExpr * cel_nullable *
                                            cel_nullable tail) {
  auto fields = expr->fields();
  if (head != nullptr) {
    *head = fields.Front();
  }
  if (tail != nullptr) {
    *tail = fields.Back();
  }
  return fields.size();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StructFieldExpr* cel_nullable
cel_StructExpr_PrevField(const cel_StructFieldExpr* cel_nullable field) {
  return field != nullptr ? field->prev() : nullptr;
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StructFieldExpr* cel_nullable
cel_StructExpr_NextField(const cel_StructFieldExpr* cel_nullable field) {
  return field != nullptr ? field->next() : nullptr;
}

// cel_ComprehensionExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_ComprehensionExpr* cel_nullable
cel_ComprehensionExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewComprehensionExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_ComprehensionExpr_IterVar(const cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->iter_var();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_ComprehensionExpr_SetIterVar(
    cel_ComprehensionExpr* cel_nonnull expr, cel_StringView iter_var) {
  expr->set_iter_var(iter_var);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_ComprehensionExpr_IterVar2(const cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->iter_var2();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_ComprehensionExpr_SetIterVar2(
    cel_ComprehensionExpr* cel_nonnull expr, cel_StringView iter_var2) {
  expr->set_iter_var2(iter_var2);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_IterRange(const cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->iter_range();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_SetIterRange(cel_ComprehensionExpr* cel_nonnull expr,
                                   cel_Expr* cel_nullable iter_range) {
  return expr->SetIterRange(iter_range);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_ComprehensionExpr_ReleaseIterRange(
    cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->ReleaseIterRange();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_ComprehensionExpr_AccuVar(const cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->accu_var();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_ComprehensionExpr_SetAccuVar(
    cel_ComprehensionExpr* cel_nonnull expr, cel_StringView accu_var) {
  expr->set_accu_var(accu_var);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_AccuInit(const cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->accu_init();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_ComprehensionExpr_SetAccuInit(
    cel_ComprehensionExpr* cel_nonnull expr, cel_Expr* cel_nullable accu_init) {
  return expr->SetAccuInit(accu_init);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_ReleaseAccuInit(cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->ReleaseAccuInit();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_ComprehensionExpr_LoopCondition(
    const cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->loop_condition();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_SetLoopCondition(cel_ComprehensionExpr* cel_nonnull expr,
                                       cel_Expr* cel_nullable loop_condition) {
  return expr->SetLoopCondition(loop_condition);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_ComprehensionExpr_ReleaseLoopCondition(
    cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->ReleaseLoopCondition();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_LoopStep(const cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->loop_step();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_ComprehensionExpr_SetLoopStep(
    cel_ComprehensionExpr* cel_nonnull expr, cel_Expr* cel_nullable loop_step) {
  return expr->SetLoopStep(loop_step);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_ReleaseLoopStep(cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->ReleaseLoopStep();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_Result(const cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->result();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_ComprehensionExpr_SetResult(
    cel_ComprehensionExpr* cel_nonnull expr, cel_Expr* cel_nullable result) {
  return expr->SetResult(result);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_ComprehensionExpr_ReleaseResult(cel_ComprehensionExpr* cel_nonnull expr) {
  return expr->ReleaseResult();
}

// cel_BindExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BindExpr* cel_nullable
cel_BindExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewBindExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_StringView
cel_BindExpr_Var(const cel_BindExpr* cel_nonnull expr) {
  return expr->var();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_BindExpr_SetVar(cel_BindExpr* cel_nonnull expr,
                                    cel_StringView var) {
  expr->set_var(var);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BindExpr_Binding(const cel_BindExpr* cel_nonnull expr) {
  return expr->binding();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_BindExpr_SetBinding(
    cel_BindExpr* cel_nonnull expr, cel_Expr* cel_nullable binding) {
  return expr->SetBinding(binding);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BindExpr_ReleaseBinding(cel_BindExpr* cel_nonnull expr) {
  return expr->ReleaseBinding();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BindExpr_Bound(const cel_BindExpr* cel_nonnull expr) {
  return expr->bound();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_BindExpr_SetBound(
    cel_BindExpr* cel_nonnull expr, cel_Expr* cel_nullable bound) {
  return expr->SetBound(bound);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BindExpr_ReleaseBound(cel_BindExpr* cel_nonnull expr) {
  return expr->ReleaseBound();
}

// cel_BlockBindingExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockBindingExpr* cel_nullable
cel_BlockBindingExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewBlockBindingExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE ptrdiff_t
cel_BlockBindingExpr_Index(const cel_BlockBindingExpr* cel_nonnull expr) {
  return expr->index();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockExpr* cel_nullable
cel_BlockBindingExpr_Parent(const cel_BlockBindingExpr* cel_nonnull expr) {
  return expr->parent();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BlockBindingExpr_Bound(const cel_BlockBindingExpr* cel_nonnull expr) {
  return expr->bound();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_BlockBindingExpr_SetBound(
    cel_BlockBindingExpr* cel_nonnull expr, cel_Expr* cel_nullable bound) {
  return expr->SetBound(bound);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BlockBindingExpr_ReleaseBound(cel_BlockBindingExpr* cel_nonnull expr) {
  return expr->ReleaseBound();
}

// cel_BlockIndexExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockIndexExpr* cel_nullable
cel_BlockIndexExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewBlockIndexExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockExpr* cel_nullable
cel_BlockIndexExpr_Block(const cel_BlockIndexExpr* cel_nonnull expr) {
  return expr->Block();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE int64_t
cel_BlockIndexExpr_Value(const cel_BlockIndexExpr* cel_nonnull expr) {
  return expr->value();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_BlockIndexExpr_SetValue(
    cel_BlockIndexExpr* cel_nonnull expr, int64_t value) {
  expr->set_value(value);
}

// cel_BlockExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockExpr* cel_nullable
cel_BlockExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewBlockExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BlockExpr_Bound(const cel_BlockExpr* cel_nonnull expr) {
  return expr->bound();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_BlockExpr_SetBound(
    cel_BlockExpr* cel_nonnull expr, cel_Expr* cel_nullable bound) {
  return expr->SetBound(bound);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_BlockExpr_ReleaseBound(cel_BlockExpr* cel_nonnull expr) {
  return expr->ReleaseBound();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_BlockExpr_AppendBinding(cel_BlockExpr* cel_nonnull expr,
                                            cel_BlockBindingExpr* cel_nonnull
                                                binding) {
  return expr->AppendBinding(binding);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_BlockExpr_PrependBinding(cel_BlockExpr* cel_nonnull expr,
                                             cel_BlockBindingExpr* cel_nonnull
                                                 binding) {
  return expr->PrependBinding(binding);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_BlockExpr_InsertBinding(
    cel_BlockExpr* cel_nonnull expr, cel_BlockBindingExpr* cel_nullable before,
    cel_BlockBindingExpr* cel_nonnull binding) {
  return expr->InsertBindingBefore(before, binding);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockBindingExpr* cel_nonnull
cel_BlockExpr_ReleaseBinding(cel_BlockExpr* cel_nonnull expr,
                             cel_BlockBindingExpr* cel_nonnull binding) {
  return expr->ReleaseBinding(binding);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockBindingExpr* cel_nonnull
cel_BlockExpr_Binding(const cel_BlockExpr* cel_nonnull expr, size_t index) {
  return expr->bindings()[index];
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE size_t cel_BlockExpr_Bindings(const cel_BlockExpr* cel_nonnull expr,
                                         cel_BlockBindingExpr * cel_nullable *
                                             cel_nullable head,
                                         cel_BlockBindingExpr * cel_nullable *
                                             cel_nullable tail) {
  auto bindings = expr->bindings();
  if (head != nullptr) {
    *head = bindings.Front();
  }
  if (tail != nullptr) {
    *tail = bindings.Back();
  }
  return bindings.size();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockBindingExpr* cel_nullable
cel_BlockExpr_PrevBinding(const cel_BlockBindingExpr* cel_nullable binding) {
  return binding != nullptr ? binding->prev() : nullptr;
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_BlockBindingExpr* cel_nullable
cel_BlockExpr_NextBinding(const cel_BlockBindingExpr* cel_nullable binding) {
  return binding != nullptr ? binding->next() : nullptr;
}

// cel_AttrQualExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_AttrQualExpr* cel_nullable
cel_AttrQualExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewAttrQualExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_AttrExpr* cel_nullable
cel_AttrQualExpr_Parent(const cel_AttrQualExpr* cel_nonnull expr) {
  return expr->parent();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE const cel_AttributeQualifier* cel_nonnull
cel_AttrQualExpr_Value(const cel_AttrQualExpr* cel_nonnull expr) {
  return &expr->value();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_AttributeQualifier* cel_nonnull
cel_AttrQualExpr_MutableValue(cel_AttrQualExpr* cel_nonnull expr) {
  return &expr->value();
}

// cel_AttrExpr

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_AttrExpr* cel_nullable
cel_AttrExpr_New(cel_Ast* cel_nonnull ast) {
  return ast->NewAttrExpr();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_AttrExpr_Operand(const cel_AttrExpr* cel_nonnull expr) {
  return expr->operand();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable cel_AttrExpr_SetOperand(
    cel_AttrExpr* cel_nonnull expr, cel_Expr* cel_nullable operand) {
  return expr->SetOperand(operand);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_Expr* cel_nullable
cel_AttrExpr_ReleaseOperand(cel_AttrExpr* cel_nonnull expr) {
  return expr->ReleaseOperand();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_AttrExpr_AppendQualifier(
    cel_AttrExpr* cel_nonnull expr, cel_AttrQualExpr* cel_nonnull qualifier) {
  expr->AppendQualifier(qualifier);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_AttrExpr_PrependQualifier(
    cel_AttrExpr* cel_nonnull expr, cel_AttrQualExpr* cel_nonnull qualifier) {
  expr->PrependQualifier(qualifier);
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_AttrExpr_InsertQualifier(
    cel_AttrExpr* cel_nonnull expr, cel_AttrQualExpr* cel_nullable before,
    cel_AttrQualExpr* cel_nonnull qualifier) {
  expr->InsertQualifierBefore(before, qualifier);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_AttrQualExpr* cel_nonnull cel_AttrExpr_ReleaseQualifier(
    cel_AttrExpr* cel_nonnull expr, cel_AttrQualExpr* cel_nonnull qualifier) {
  return expr->ReleaseQualifier(qualifier);
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_AttrQualExpr* cel_nonnull
cel_AttrExpr_Qualifier(const cel_AttrExpr* cel_nonnull expr, size_t index) {
  return expr->qualifiers()[index];
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE size_t cel_AttrExpr_Qualifiers(const cel_AttrExpr* cel_nonnull expr,
                                          cel_AttrQualExpr * cel_nullable *
                                              cel_nullable head,
                                          cel_AttrQualExpr * cel_nullable *
                                              cel_nullable tail) {
  auto qualifiers = expr->qualifiers();
  if (head != nullptr) {
    *head = qualifiers.Front();
  }
  if (tail != nullptr) {
    *tail = qualifiers.Back();
  }
  return qualifiers.size();
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_AttrQualExpr* cel_nullable
cel_AttrExpr_PrevQualifier(const cel_AttrQualExpr* cel_nullable qualifier) {
  return qualifier != nullptr ? qualifier->prev() : nullptr;
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE cel_AttrQualExpr* cel_nullable
cel_AttrExpr_NextQualifier(const cel_AttrQualExpr* cel_nullable qualifier) {
  return qualifier != nullptr ? qualifier->next() : nullptr;
}

CEL_ATTRIBUTE_NODISCARD
CEL_DEPRECATE_AND_INLINE
CEL_INLINE bool cel_AttrExpr_TestOnly(const cel_AttrExpr* cel_nonnull expr) {
  return expr->test_only();
}

CEL_DEPRECATE_AND_INLINE
CEL_INLINE void cel_AttrExpr_SetTestOnly(cel_AttrExpr* cel_nonnull expr,
                                         bool test_only) {
  expr->set_test_only(test_only);
}

template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE
    CEL_INLINE cel_Expr* cel_nullability_unknown
    cel_Expr_UpCast(T* cel_nullability_unknown expr) {
  return expr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE
    CEL_INLINE const cel_Expr* cel_nullability_unknown
    cel_Expr_UpCast(const T* cel_nullability_unknown expr) {
  return expr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_UnspecifiedExpr,
                       cel_UnspecifiedExpr>* cel_nullability_unknown
    cel_UnspecifiedExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetUnspecified() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_IdentExpr, cel_IdentExpr>*
        cel_nullability_unknown
        cel_IdentExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetIdent() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_ConstExpr, cel_ConstExpr>*
        cel_nullability_unknown
        cel_ConstExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetConst() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_SelectExpr,
                       cel_SelectExpr>* cel_nullability_unknown
    cel_SelectExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetSelect() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_CallExpr, cel_CallExpr>*
        cel_nullability_unknown
        cel_CallExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetCall() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_CallArgExpr,
                       cel_CallArgExpr>* cel_nullability_unknown
    cel_CallArgExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetCallArg() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_UnaryExpr, cel_UnaryExpr>*
        cel_nullability_unknown
        cel_UnaryExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetUnary() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_BinaryExpr,
                       cel_BinaryExpr>* cel_nullability_unknown
    cel_BinaryExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetBinary() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_TernaryExpr,
                       cel_TernaryExpr>* cel_nullability_unknown
    cel_TernaryExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetTernary() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_ListElementExpr,
                       cel_ListElementExpr>* cel_nullability_unknown
    cel_ListElementExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetListElement() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_ListExpr, cel_ListExpr>*
        cel_nullability_unknown
        cel_ListExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetList() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_MapEntryExpr,
                       cel_MapEntryExpr>* cel_nullability_unknown
    cel_MapEntryExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetMapEntry() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_MapExpr, cel_MapExpr>*
        cel_nullability_unknown
        cel_MapExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetMap() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_StructFieldExpr,
                       cel_StructFieldExpr>* cel_nullability_unknown
    cel_StructFieldExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetStructField() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_StructExpr,
                       cel_StructExpr>* cel_nullability_unknown
    cel_StructExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetStruct() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_ComprehensionExpr,
                       cel_ComprehensionExpr>* cel_nullability_unknown
    cel_ComprehensionExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetComprehension() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_BindExpr, cel_BindExpr>*
        cel_nullability_unknown
        cel_BindExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetBind() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_BlockBindingExpr,
                       cel_BlockBindingExpr>* cel_nullability_unknown
    cel_BlockBindingExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetBlockBinding() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_BlockIndexExpr,
                       cel_BlockIndexExpr>* cel_nullability_unknown
    cel_BlockIndexExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetBlockIndex() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_BlockExpr, cel_BlockExpr>*
        cel_nullability_unknown
        cel_BlockExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetBlock() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_AttrQualExpr,
                       cel_AttrQualExpr>* cel_nullability_unknown
    cel_AttrQualExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetAttrQual() : nullptr;
}
template <typename T>
CEL_ATTRIBUTE_NODISCARD CEL_DEPRECATE_AND_INLINE CEL_INLINE
    std::conditional_t<std::is_const_v<T>, const cel_AttrExpr, cel_AttrExpr>*
        cel_nullability_unknown
        cel_AttrExpr_DownCast(T* cel_nullability_unknown expr) {
  return expr != nullptr ? expr->GetAttr() : nullptr;
}

#endif  // THIRD_PARTY_CEL_C_INTERNAL_AST_H_
