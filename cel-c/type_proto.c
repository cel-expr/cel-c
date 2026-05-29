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

#include "cel-c/type_proto.h"

#include <stddef.h>

#include "cel/expr/checked.upb.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/type.h"

const cel_Type* cel_nullable cel_Type_FromProto(
    const cel_expr_Type* cel_nonnull in, cel_Arena* cel_nonnull arena,
    cel_Status* cel_nonnull status) {
  CEL_ASSERT_NOT_NULL(in);
  CEL_ASSERT_NOT_NULL(arena);
  CEL_ASSERT(cel_Status_Ok(status));

  const cel_expr_Type_type_kind_oneofcases type_kind =
      cel_expr_Type_type_kind_case(in);
  switch (type_kind) {
    case cel_expr_Type_type_kind_dyn:
      return cel_DynType;
    case cel_expr_Type_type_kind_null:
      return cel_NullType;
    case cel_expr_Type_type_kind_primitive: {
      const cel_expr_Type_PrimitiveType primitive =
          cel_expr_Type_primitive(in);
      switch (primitive) {
        case cel_expr_Type_BOOL:
          return cel_BoolType;
        case cel_expr_Type_INT64:
          return cel_IntType;
        case cel_expr_Type_UINT64:
          return cel_UintType;
        case cel_expr_Type_DOUBLE:
          return cel_DoubleType;
        case cel_expr_Type_STRING:
          return cel_StringType;
        case cel_expr_Type_BYTES:
          return cel_BytesType;
        default:
          cel_InvalidArgumentStatusF(
              status, "cel: unexpected google.api.expr.Type.PrimitiveType: %d",
              primitive);
          return cel_nullptr;
      }
    }
    case cel_expr_Type_type_kind_wrapper: {
      const cel_expr_Type_PrimitiveType primitive =
          cel_expr_Type_wrapper(in);
      switch (primitive) {
        case cel_expr_Type_BOOL:
          return cel_BoolWrapperType;
        case cel_expr_Type_INT64:
          return cel_IntWrapperType;
        case cel_expr_Type_UINT64:
          return cel_UintWrapperType;
        case cel_expr_Type_DOUBLE:
          return cel_DoubleWrapperType;
        case cel_expr_Type_STRING:
          return cel_StringWrapperType;
        case cel_expr_Type_BYTES:
          return cel_BytesWrapperType;
        default:
          cel_InvalidArgumentStatusF(
              status, "cel: unexpected google.api.expr.Type.PrimitiveType: %d",
              primitive);
          return cel_nullptr;
      }
    }
    case cel_expr_Type_type_kind_well_known: {
      const cel_expr_Type_WellKnownType well_known =
          cel_expr_Type_well_known(in);
      switch (well_known) {
        case cel_expr_Type_ANY:
          return cel_AnyType;
        case cel_expr_Type_TIMESTAMP:
          return cel_TimestampType;
        case cel_expr_Type_DURATION:
          return cel_DurationType;
        default:
          cel_InvalidArgumentStatusF(
              status, "cel: unexpected google.api.expr.Type.WellKnownType: %d",
              well_known);
          return cel_nullptr;
      }
    }
    case cel_expr_Type_type_kind_list_type: {
      const cel_expr_Type_ListType* in_list =
          cel_expr_Type_list_type(in);
      const cel_ListType* out_list;
      if (CEL_UNLIKELY(in_list == cel_nullptr)) {
        out_list = cel_ListType_New(cel_DynType, arena);
      } else {
        const cel_expr_Type* in_list_element =
            cel_expr_Type_ListType_elem_type(in_list);
        if (CEL_UNLIKELY(in_list_element == cel_nullptr)) {
          out_list = cel_ListType_New(cel_DynType, arena);
        } else {
          const cel_Type* out_list_element =
              cel_Type_FromProto(in_list_element, arena, status);
          if (CEL_UNLIKELY(out_list_element == cel_nullptr)) {
            return cel_nullptr;
          }
          out_list = cel_ListType_New(out_list_element, arena);
        }
      }
      if (CEL_UNLIKELY(out_list == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
      }
      return cel_Type_UpCast(out_list);
    }
    case cel_expr_Type_type_kind_map_type: {
      const cel_expr_Type_MapType* in_map =
          cel_expr_Type_map_type(in);
      const cel_MapType* out_map;
      if (CEL_UNLIKELY(in_map == cel_nullptr)) {
        out_map = cel_MapType_New(cel_DynType, cel_DynType, arena);
      } else {
        const cel_Type* out_map_key;
        const cel_Type* out_map_value;
        const cel_expr_Type* in_map_key =
            cel_expr_Type_MapType_key_type(in_map);
        if (CEL_UNLIKELY(in_map_key == cel_nullptr)) {
          out_map_key = cel_DynType;
        } else {
          out_map_key = cel_Type_FromProto(in_map_key, arena, status);
          if (CEL_UNLIKELY(out_map_key == cel_nullptr)) {
            return cel_nullptr;
          }
        }
        const cel_expr_Type* in_map_value =
            cel_expr_Type_MapType_value_type(in_map);
        if (CEL_UNLIKELY(in_map_value == cel_nullptr)) {
          out_map_value = cel_DynType;
        } else {
          out_map_value = cel_Type_FromProto(in_map_value, arena, status);
          if (CEL_UNLIKELY(out_map_value == cel_nullptr)) {
            return cel_nullptr;
          }
        }
        out_map = cel_MapType_New(out_map_key, out_map_value, arena);
      }
      if (CEL_UNLIKELY(out_map == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
      }
      return cel_Type_UpCast(out_map);
    }
    case cel_expr_Type_type_kind_function: {
      const cel_expr_Type_FunctionType* in_function =
          cel_expr_Type_function(in);
      const cel_expr_Type* in_function_result =
          cel_expr_Type_FunctionType_result_type(in_function);
      const cel_Type* out_function_result;
      if (CEL_UNLIKELY(in_function_result == cel_nullptr)) {
        out_function_result = cel_DynType;
      } else {
        out_function_result =
            cel_Type_FromProto(in_function_result, arena, status);
        if (CEL_UNLIKELY(out_function_result == cel_nullptr)) {
          return cel_nullptr;
        }
      }
      size_t function_args_len;
      const cel_expr_Type* const* in_function_args =
          cel_expr_Type_FunctionType_arg_types(in_function,
                                                      &function_args_len);
      const cel_Type** out_function_args;
      cel_FunctionType* out_function = cel_FunctionType_New(
          out_function_result, function_args_len, &out_function_args, arena);
      if (CEL_UNLIKELY(out_function == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      for (size_t i = 0; i < function_args_len; ++i) {
        const cel_expr_Type* in_function_arg = in_function_args[i];
        const cel_Type* out_function_arg;
        if (CEL_UNLIKELY(in_function_arg == cel_nullptr)) {
          out_function_arg = cel_DynType;
        } else {
          out_function_arg = cel_Type_FromProto(in_function_arg, arena, status);
          if (CEL_UNLIKELY(out_function_arg == cel_nullptr)) {
            return cel_nullptr;
          }
        }
        out_function_args[i] = out_function_arg;
      }
      return cel_Type_UpCast(out_function);
    }
    case cel_expr_Type_type_kind_message_type: {
      cel_StringView in_struct_name = cel_expr_Type_message_type(in);
      cel_StringView out_struct_name;
      if (CEL_UNLIKELY(
              !cel_Arena_StrDup(arena, &out_struct_name, in_struct_name))) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      const cel_StructType* out_struct =
          cel_StructType_New(out_struct_name, arena);
      if (CEL_UNLIKELY(out_struct == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
      }
      return cel_Type_UpCast(out_struct);
    }
    case cel_expr_Type_type_kind_type_param: {
      cel_StringView in_type_param_name = cel_expr_Type_type_param(in);
      cel_StringView out_type_param_name;
      if (CEL_UNLIKELY(!cel_Arena_StrDup(arena, &out_type_param_name,
                                         in_type_param_name))) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      const cel_TypeParamType* out_type_param =
          cel_TypeParamType_New(out_type_param_name, arena);
      if (CEL_UNLIKELY(out_type_param == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
      }
      return cel_Type_UpCast(out_type_param);
    }
    case cel_expr_Type_type_kind_type: {
      const cel_expr_Type* in_type = cel_expr_Type_type(in);
      const cel_Type* out_type;
      if (CEL_UNLIKELY(in_type == cel_nullptr)) {
        out_type = cel_DynType;
      } else {
        out_type = cel_Type_FromProto(in_type, arena, status);
        if (CEL_UNLIKELY(out_type == cel_nullptr)) {
          return cel_nullptr;
        }
      }
      const cel_TypeType* out_type_type = cel_TypeType_New(out_type, arena);
      if (CEL_UNLIKELY(out_type == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      return cel_Type_UpCast(out_type_type);
    }
    case cel_expr_Type_type_kind_error:
      return cel_ErrorType;
    case cel_expr_Type_type_kind_abstract_type: {
      const cel_expr_Type_AbstractType* in_opaque =
          cel_expr_Type_abstract_type(in);
      cel_StringView in_opaque_name =
          cel_expr_Type_AbstractType_name(in_opaque);
      cel_StringView out_opaque_name;
      if (CEL_UNLIKELY(
              !cel_Arena_StrDup(arena, &out_opaque_name, in_opaque_name))) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      size_t opaque_params_len;
      const cel_expr_Type* const* in_opaque_params =
          cel_expr_Type_AbstractType_parameter_types(in_opaque,
                                                            &opaque_params_len);
      const cel_Type** out_opaque_params;
      cel_OpaqueType* out_opaque = cel_OpaqueType_New(
          out_opaque_name, opaque_params_len, &out_opaque_params, arena);
      if (CEL_UNLIKELY(out_opaque == cel_nullptr)) {
        cel_OutOfMemoryStatus(status);
        return cel_nullptr;
      }
      for (size_t i = 0; i < opaque_params_len; ++i) {
        const cel_expr_Type* in_opaque_arg = in_opaque_params[i];
        const cel_Type* out_opaque_arg;
        if (CEL_UNLIKELY(in_opaque_arg == cel_nullptr)) {
          out_opaque_arg = cel_DynType;
        } else {
          out_opaque_arg = cel_Type_FromProto(in_opaque_arg, arena, status);
          if (CEL_UNLIKELY(out_opaque_arg == cel_nullptr)) {
            return cel_nullptr;
          }
        }
        out_opaque_params[i] = out_opaque_arg;
      }
      return cel_Type_UpCast(out_opaque);
    }
    default:
      cel_InvalidArgumentStatusF(
          status, "cel: unexpected google.api.expr.Type.type_kind: %d",
          type_kind);
      return cel_nullptr;
  }
}
