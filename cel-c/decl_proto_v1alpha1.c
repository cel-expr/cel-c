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

#include "cel-c/decl_proto_v1alpha1.h"

#include <stddef.h>

#include "google/api/expr/v1alpha1/checked.upb.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/constant_proto_v1alpha1.h"
#include "cel-c/decl.h"
#include "cel-c/function_scope.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"
#include "cel-c/type_proto_v1alpha1.h"

cel_Decl* cel_nullable cel_Decl_FromProtoV1Alpha1(
    const google_api_expr_v1alpha1_Decl* cel_nonnull in,
    cel_Arena* cel_nonnull arena, cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT(cel_Status_Ok(status));

  const google_api_expr_v1alpha1_Decl_decl_kind_oneofcases decl_kind =
      google_api_expr_v1alpha1_Decl_decl_kind_case(in);
  switch (decl_kind) {
    case google_api_expr_v1alpha1_Decl_decl_kind_ident: {
      cel_StringView out_name;
      if (CEL_UNLIKELY(!cel_Arena_StrDup(
              arena, &out_name, google_api_expr_v1alpha1_Decl_name(in)))) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      cel_IdentDecl* out = cel_IdentDecl_New(out_name, arena);
      if (CEL_UNLIKELY(out == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      const google_api_expr_v1alpha1_Decl_IdentDecl* in_ident =
          google_api_expr_v1alpha1_Decl_ident(in);
      if (google_api_expr_v1alpha1_Decl_IdentDecl_has_type(in_ident)) {
        const cel_Type* out_type = cel_Type_FromProtoV1Alpha1(
            google_api_expr_v1alpha1_Decl_IdentDecl_type(in_ident), arena,
            status);
        if (CEL_UNLIKELY(out_type == cel_nullptr)) {
          return cel_nullptr;
        }
        cel_IdentDecl_SetType(out, out_type);
      }
      if (google_api_expr_v1alpha1_Decl_IdentDecl_has_value(in_ident)) {
        if (CEL_UNLIKELY(!cel_Constant_FromProtoV1Alpha1(
                cel_IdentDecl_MutableValue(out),
                google_api_expr_v1alpha1_Decl_IdentDecl_value(in_ident), arena,
                status))) {
          return cel_nullptr;
        }
      }
      cel_StringView out_doc;
      if (CEL_UNLIKELY(!cel_Arena_StrDup(
              arena, &out_doc,
              google_api_expr_v1alpha1_Decl_IdentDecl_doc(in_ident)))) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      cel_Decl_SetDoc(cel_Decl_UpCast(out), out_doc);
      return cel_Decl_UpCast(out);
    }
    case google_api_expr_v1alpha1_Decl_decl_kind_function: {
      cel_StringView out_name;
      if (CEL_UNLIKELY(!cel_Arena_StrDup(
              arena, &out_name, google_api_expr_v1alpha1_Decl_name(in)))) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      cel_FunctionDecl* out = cel_FunctionDecl_New(out_name, arena);
      if (CEL_UNLIKELY(out == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      const google_api_expr_v1alpha1_Decl_FunctionDecl* in_function =
          google_api_expr_v1alpha1_Decl_function(in);
      size_t in_overloads_len;
      const google_api_expr_v1alpha1_Decl_FunctionDecl_Overload* const*
          in_overloads_ptr =
              google_api_expr_v1alpha1_Decl_FunctionDecl_overloads(
                  in_function, &in_overloads_len);
      for (size_t i = 0; i < in_overloads_len; ++i) {
        const google_api_expr_v1alpha1_Decl_FunctionDecl_Overload* in_overload =
            in_overloads_ptr[i];
        cel_StringView out_id;
        if (CEL_UNLIKELY(!cel_Arena_StrDup(
                arena, &out_id,
                google_api_expr_v1alpha1_Decl_FunctionDecl_Overload_overload_id(
                    in_overload)))) {
          cel_OutOfMemoryStatus(status);
          return cel_nullptr;
        }
        const cel_Type* out_result;
        if (google_api_expr_v1alpha1_Decl_FunctionDecl_Overload_has_result_type(
                in_overload)) {
          out_result = cel_Type_FromProtoV1Alpha1(
              google_api_expr_v1alpha1_Decl_FunctionDecl_Overload_result_type(
                  in_overload),
              arena, status);
          if (CEL_UNLIKELY(out_result == cel_nullptr)) {
            return cel_nullptr;
          }
        } else {
          out_result = cel_DynType;
        }
        size_t in_params_len;
        const google_api_expr_v1alpha1_Type* const* in_params_ptr =
            google_api_expr_v1alpha1_Decl_FunctionDecl_Overload_params(
                in_overload, &in_params_len);
        const cel_Type** out_params_ptr;
        cel_FunctionType* out_type = cel_FunctionType_New(
            out_result, in_params_len, &out_params_ptr, arena);
        if (CEL_UNLIKELY(out_type == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          return cel_nullptr;
        }
        for (size_t j = 0; j < in_params_len; ++j) {
          const google_api_expr_v1alpha1_Type* in_param = in_params_ptr[j];
          const cel_Type* out_param;
          if (in_param != cel_nullptr) {
            out_param = cel_Type_FromProtoV1Alpha1(in_param, arena, status);
            if (CEL_UNLIKELY(out_param == cel_nullptr)) {
              return cel_nullptr;
            }
          } else {
            out_param = cel_DynType;
          }
          out_params_ptr[j] = out_param;
        }
        cel_FunctionOverloadDecl* out_overload = cel_FunctionOverloadDecl_New(
            out_id,
            google_api_expr_v1alpha1_Decl_FunctionDecl_Overload_is_instance_function(
                in_overload)
                ? cel_FunctionScope_kMember
                : cel_FunctionScope_kGlobal,
            out_type, arena);
        if (CEL_UNLIKELY(out_overload == cel_nullptr)) {
          cel_OutOfMemoryStatus(status);
          return cel_nullptr;
        }
        cel_StringView out_doc;
        if (CEL_UNLIKELY(!cel_Arena_StrDup(
                arena, &out_doc,
                google_api_expr_v1alpha1_Decl_FunctionDecl_Overload_doc(
                    in_overload)))) {
          cel_OutOfMemoryStatus(status);
          return cel_nullptr;
        }
        cel_FunctionOverloadDecl_SetDoc(out_overload, out_doc);
        CEL_USED(cel_FunctionDecl_AddOverload(out, out_overload));
      }
      cel_StringView out_doc;
      if (CEL_UNLIKELY(!cel_Arena_StrDup(
              arena, &out_doc,
              google_api_expr_v1alpha1_Decl_FunctionDecl_doc(in_function)))) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      cel_Decl_SetDoc(cel_Decl_UpCast(out), out_doc);
      return cel_Decl_UpCast(out);
    }
    default:
      cel_InvalidArgumentStatusF(
          status, "cel: unexpected google.api.expr.v1alpha1.Decl.decl_kind: %d",
          decl_kind);
      return cel_nullptr;
  }
}
