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

#include "cel-c/src/runtime/interpretable.h"

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cel-c/activation.h"
#include "cel-c/arena.h"
#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/duration.h"
#include "cel-c/error.h"
#include "cel-c/error_code.h"
#include "cel-c/src/array.h"
#include "cel-c/src/charconv.h"
#include "cel-c/src/ckdint.h"
#include "cel-c/src/compare.h"
#include "cel-c/src/config.h"
#include "cel-c/src/durationconv.h"
#include "cel-c/src/empty_list_value.h"
#include "cel-c/src/empty_map_value.h"
#include "cel-c/src/mutable_list_value.h"
#include "cel-c/src/mutable_map_value.h"
#include "cel-c/src/number.h"
#include "cel-c/src/regexp.h"
#include "cel-c/src/runtime/activation.h"
#include "cel-c/src/runtime/instr.h"
#include "cel-c/src/runtime/program.h"
#include "cel-c/src/runtime/runtime.h"
#include "cel-c/src/setjmp.h"
#include "cel-c/src/string.h"
#include "cel-c/src/timestampconv.h"
#include "cel-c/src/utf8.h"
#include "cel-c/status.h"
#include "cel-c/string_view.h"
#include "cel-c/timestamp.h"
#include "cel-c/trilean.h"
#include "cel-c/value.h"
#include "cel-c/value_kind.h"
#include "upb/reflection/def.h"

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Activation* _cel_Interpretable_ToActivation(
    _cel_Interpretable* cel_nonnull interp) {
  CEL_ASSERT_NOT_NULL(interp);

  return cel_containerof(interp, cel_Activation, interp);
}

CEL_ATTRIBUTE_NORETURN
static CEL_INLINE void _cel_Interpretable_Throw(
    _cel_Interpretable* cel_nonnull interp) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT(cel_Status_Ok(interp->status));

  _cel_longjmp(interp->jmp);
}

CEL_ATTRIBUTE_NORETURN
static CEL_INLINE void _cel_Interpretable_ThrowOutOfMemory(
    _cel_Interpretable* cel_nonnull interp) {
  CEL_ASSERT_NOT_NULL(interp);

  cel_OutOfMemoryStatus(interp->status);
  _cel_Interpretable_Throw(interp);
}

CEL_ATTRIBUTE_NORETURN
static CEL_INLINE void _cel_Interpretable_ThrowStackOverflow(
    _cel_Interpretable* cel_nonnull interp, const _cel_InstrData* instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_InternalStatusF(
      interp->status, "cel: stack overflow: pc=%" PRIuPTR,
      cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
  _cel_Interpretable_Throw(interp);
}

CEL_ATTRIBUTE_NORETURN
static CEL_INLINE void _cel_Interpretable_ThrowStackUnderflow(
    _cel_Interpretable* cel_nonnull interp, const _cel_InstrData* instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_InternalStatusF(
      interp->status, "cel: stack underflow: pc=%" PRIuPTR,
      cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
  _cel_Interpretable_Throw(interp);
}

static CEL_INLINE void _cel_Interpretable_ThrowIfError(
    _cel_Interpretable* cel_nonnull interp) {
  if (CEL_UNLIKELY(!cel_Status_Ok(interp->status))) {
    _cel_Interpretable_Throw(interp);
  }
}

CEL_ATTRIBUTE_NORETURN
static void _cel_Interpretable_Unreachable(
    _cel_Interpretable* cel_nonnull interp, const _cel_InstrData* instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_InternalStatusF(
      interp->status, "cel: unreachable code reached: pc=%" PRIuPTR,
      cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
  _cel_Interpretable_Throw(interp);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_StringView _cel_Interpretable_InternedString(
    const _cel_Interpretable* cel_nonnull interp, uint32_t index) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_LT(index, interp->strings_table_len);

  return _cel_String_ToStringView(&interp->strings_table_ptr[index]);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const _cel_CandidateNames* cel_nonnull
_cel_Interpretable_InternedCandidateNames(
    const _cel_Interpretable* cel_nonnull interp, uint32_t index) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_LT(index, interp->candidate_names_len);

  return &interp->candidate_names_ptr[index];
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Value* cel_nonnull _cel_Interpretable_PushN(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr, uint32_t count) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_GT(count, 0);

  cel_Value* top = interp->value_stack_top;
  if (CEL_UNLIKELY(interp->value_stack_end - top < count)) {
    _cel_Interpretable_ThrowStackOverflow(interp, instr);
  }
  interp->value_stack_top += count;
  return top;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Value* cel_nonnull
_cel_Interpretable_Push(_cel_Interpretable* cel_nonnull interp,
                        const _cel_InstrData* cel_nonnull instr) {
  return _cel_Interpretable_PushN(interp, instr, 1);
}

static CEL_INLINE void _cel_Interpretable_PopN(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr, uint32_t count) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_GT(count, 0);

  cel_Value* top = interp->value_stack_top;
  if (CEL_UNLIKELY(top - interp->value_stack_base < count)) {
    _cel_Interpretable_ThrowStackUnderflow(interp, instr);
  }
  interp->value_stack_top -= count;
}

static CEL_INLINE void _cel_Interpretable_Pop(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  _cel_Interpretable_PopN(interp, instr, 1);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Value* cel_nonnull _cel_Interpretable_TopN(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr, uint32_t count) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_GT(count, 0);
  CEL_ASSERT(interp->value_stack_top >= interp->value_stack_base);
  CEL_ASSERT(interp->value_stack_top <= interp->value_stack_end);

  cel_Value* top = interp->value_stack_top;
  if (CEL_UNLIKELY(top - interp->value_stack_base < count)) {
    _cel_Interpretable_ThrowStackUnderflow(interp, instr);
  }
  return top - count;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Value* cel_nonnull
_cel_Interpretable_Top(_cel_Interpretable* cel_nonnull interp,
                       const _cel_InstrData* cel_nonnull instr) {
  return _cel_Interpretable_TopN(interp, instr, 1);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Value* _cel_Interpretable_PushAndPopN(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr, uint32_t count) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  if (count == 0) {
    return _cel_Interpretable_Push(interp, instr);
  }
  if (count > 1) {
    _cel_Interpretable_PopN(interp, instr, count - 1);
  }
  return _cel_Interpretable_Top(interp, instr);
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE const _cel_Instr* cel_nonnull _cel_Interpretable_ShortJump(
    const _cel_InstrData* cel_nonnull instr, int32_t offset) {
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_NE(offset, 0);

  return cel_containerof(instr, _cel_Instr, data) + offset;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewNoSuchOverloadError(
    _cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: no such overload"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewModuloByZeroError(
    _cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: modulo by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewDivideByZeroError(
    _cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: divide by zero"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewIntOverflowError(_cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: int overflow"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewUintOverflowError(
    _cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: uint overflow"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewDurationOverflowError(
    _cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: duration overflow"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewTimestampOverflowError(
    _cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: int overflow"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewDuplicateKeyError(
    _cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: duplicate key in map"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kAlreadyExists);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewBadKeyError(_cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: bad map key"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewIndexOutOfRangeError(
    _cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: index out of range"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
  return error;
}

CEL_ATTRIBUTE_NODISCARD
static CEL_INLINE cel_Error* cel_nonnull
_cel_Interpretable_NewNoSuchKeyError(_cel_Interpretable* cel_nonnull interp) {
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetMessage(error, cel_StringView_From("cel: no such key in map"));
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
  return error;
}

static inline void _cel_Interpretable_NullConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetNull(_cel_Interpretable_Push(interp, instr));
}

static inline void _cel_Interpretable_FalseConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetFalse(_cel_Interpretable_Push(interp, instr));
}

static inline void _cel_Interpretable_TrueConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetTrue(_cel_Interpretable_Push(interp, instr));
}

static inline void _cel_Interpretable_IntConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetInt(_cel_Interpretable_Push(interp, instr),
                   instr->int_const.value);
}

static inline void _cel_Interpretable_UintConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetUint(_cel_Interpretable_Push(interp, instr),
                    instr->uint_const.value);
}

static inline void _cel_Interpretable_DoubleConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetDouble(_cel_Interpretable_Push(interp, instr),
                      instr->double_const.value);
}

static inline void _cel_Interpretable_BytesConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetBytes(
      _cel_Interpretable_Push(interp, instr),
      _cel_PackedStringView_ToStringView(instr->bytes_const.value.direct));
}

static inline void _cel_Interpretable_StringConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetString(
      _cel_Interpretable_Push(interp, instr),
      _cel_PackedStringView_ToStringView(instr->string_const.value.direct));
}

static inline void _cel_Interpretable_DurationConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetDuration(_cel_Interpretable_Push(interp, instr),
                        instr->duration_const.value);
}

static inline void _cel_Interpretable_TimestampConst(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value_SetTimestamp(_cel_Interpretable_Push(interp, instr),
                         instr->timestamp_const.value);
}

static inline void _cel_Interpretable_Ident(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_StringView name =
      _cel_PackedStringView_ToStringView(instr->ident.name.direct);
  cel_Value* top = _cel_Interpretable_Push(interp, instr);
  switch (_cel_Activation_FindVariable(_cel_Interpretable_ToActivation(interp),
                                       name, top, interp->arena,
                                       interp->status)) {
    case cel_Trilean_kFalse: {
      CEL_ASSERT(cel_Status_Ok(interp->status));
      // Did not find the variable.
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
      if (CEL_UNLIKELY(
              !cel_Error_FormatMessage(error, interp->arena,
                                       "cel: variable binding not found in "
                                       "activation: " CEL_STRINGVIEW_FMT,
                                       CEL_STRINGVIEW_ARGS(name)))) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Value_SetError(top, error);
    } break;
    case cel_Trilean_kTrue:
      CEL_ASSERT(cel_Status_Ok(interp->status));
      break;
    case cel_Trilean_kError:
      _cel_Interpretable_Throw(interp);
    default:
      CEL_UNREACHABLE();
  }
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_IdentJump(_cel_Interpretable* cel_nonnull interp,
                             const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_NE(instr->ident_jump.found_jump, 0);
  CEL_ASSERT_NE(instr->ident_jump.missing_jump, 0);

  ptrdiff_t jump;

  cel_StringView name =
      _cel_Interpretable_InternedString(interp, instr->ident_jump.name);
  cel_Value* top = _cel_Interpretable_Push(interp, instr);
  switch (_cel_Activation_FindVariable(_cel_Interpretable_ToActivation(interp),
                                       name, top, interp->arena,
                                       interp->status)) {
    case cel_Trilean_kFalse:
      CEL_ASSERT(cel_Status_Ok(interp->status));
      if (instr->ident_jump.missing_error) {
        // Did not find the variable.
        cel_Error* error = cel_Error_New(interp->arena);
        if (CEL_UNLIKELY(error == cel_nullptr)) {
          _cel_Interpretable_ThrowOutOfMemory(interp);
        }
        cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
        if (CEL_UNLIKELY(
                !cel_Error_FormatMessage(error, interp->arena,
                                         "cel: variable binding not found in "
                                         "activation: " CEL_STRINGVIEW_FMT,
                                         CEL_STRINGVIEW_ARGS(name)))) {
          _cel_Interpretable_ThrowOutOfMemory(interp);
        }
        cel_Value_SetError(top, error);
      } else {
        _cel_Interpretable_Pop(interp, instr);
      }
      jump = instr->ident_jump.missing_jump;
      break;
    case cel_Trilean_kTrue:
      CEL_ASSERT(cel_Status_Ok(interp->status));
      jump = instr->ident_jump.found_jump;
      break;
    case cel_Trilean_kError:
      _cel_Interpretable_Pop(interp, instr);
      _cel_Interpretable_Throw(interp);
    default:
      CEL_UNREACHABLE();
  }
  return _cel_Interpretable_ShortJump(instr, jump);
}

static inline void _cel_Interpretable_ContIdent(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* top = _cel_Interpretable_Push(interp, instr);
  const _cel_CandidateNames* candidate_names =
      _cel_Interpretable_InternedCandidateNames(
          interp, instr->cont_ident.candidate_names);
  for (size_t i = 0; i < candidate_names->size; ++i) {
    switch (_cel_Activation_FindVariable(
        _cel_Interpretable_ToActivation(interp),
        _cel_Interpretable_InternedString(interp, candidate_names->data[i]),
        top, interp->arena, interp->status)) {
      case cel_Trilean_kFalse:
        CEL_ASSERT(cel_Status_Ok(interp->status));
        break;
      case cel_Trilean_kTrue:
        CEL_ASSERT(cel_Status_Ok(interp->status));
        return;
      case cel_Trilean_kError:
        _cel_Interpretable_Throw(interp);
      default:
        CEL_UNREACHABLE();
    }
  }
  // Did not find the variable.
  cel_Error* error = cel_Error_New(interp->arena);
  if (CEL_UNLIKELY(error == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
  if (CEL_UNLIKELY(!cel_Error_FormatMessage(
          error, interp->arena,
          "cel: variable binding not found in "
          "activation: " CEL_STRINGVIEW_FMT,
          CEL_STRINGVIEW_ARGS(_cel_Interpretable_InternedString(
              interp, candidate_names->data[candidate_names->size - 1]))))) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Value_SetError(top, error);
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_ContIdentJump(_cel_Interpretable* cel_nonnull interp,
                                 const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_NE(instr->cont_ident_jump.found_jump, 0);
  CEL_ASSERT_NE(instr->cont_ident_jump.missing_jump, 0);

  cel_Value* top = _cel_Interpretable_Push(interp, instr);
  const _cel_CandidateNames* candidate_names =
      _cel_Interpretable_InternedCandidateNames(
          interp, instr->cont_ident_jump.candidate_names);
  for (size_t i = 0; i < candidate_names->size; ++i) {
    switch (_cel_Activation_FindVariable(
        _cel_Interpretable_ToActivation(interp),
        _cel_Interpretable_InternedString(interp, candidate_names->data[i]),
        top, interp->arena, interp->status)) {
      case cel_Trilean_kFalse:
        CEL_ASSERT(cel_Status_Ok(interp->status));
        break;
      case cel_Trilean_kTrue:
        CEL_ASSERT(cel_Status_Ok(interp->status));
        return _cel_Interpretable_ShortJump(instr,
                                            instr->cont_ident_jump.found_jump);
      case cel_Trilean_kError:
        _cel_Interpretable_Pop(interp, instr);
        _cel_Interpretable_Throw(interp);
      default:
        CEL_UNREACHABLE();
    }
  }
  if (instr->cont_ident_jump.missing_error) {
    // Did not find the variable.
    cel_Error* error = cel_Error_New(interp->arena);
    if (CEL_UNLIKELY(error == cel_nullptr)) {
      _cel_Interpretable_ThrowOutOfMemory(interp);
    }
    cel_Error_SetCanonicalCode(error, cel_ErrorCode_kNotFound);
    if (CEL_UNLIKELY(!cel_Error_FormatMessage(
            error, interp->arena,
            "cel: variable binding not found in "
            "activation: " CEL_STRINGVIEW_FMT,
            CEL_STRINGVIEW_ARGS(_cel_Interpretable_InternedString(
                interp, candidate_names->data[candidate_names->size - 1]))))) {
      _cel_Interpretable_ThrowOutOfMemory(interp);
    }
    cel_Value_SetError(top, error);
  } else {
    _cel_Interpretable_Pop(interp, instr);
  }
  return _cel_Interpretable_ShortJump(instr,
                                      instr->cont_ident_jump.missing_jump);
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_Jump(_cel_Interpretable* cel_nonnull interp,
                        const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_NE(instr->jump.jump, 0);

  return _cel_Interpretable_ShortJump(instr, instr->jump.jump);
}

static inline void _cel_Interpretable_Has(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  cel_Value* top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kMap: {
      cel_MapValue map_value = *cel_Value_GetMap(top);
      cel_MapValueKey map_value_key;
      cel_MapValueKey_SetString(
          &map_value_key,
          _cel_PackedStringView_ToStringView(instr->has.field.direct));
      if (CEL_UNLIKELY(!cel_MapValue_Has(&map_value, &interp->context,
                                         &map_value_key, top,
                                         interp->status))) {
        _cel_Interpretable_Throw(interp);
      }
    } break;
    case cel_ValueKind_kStruct: {
      cel_StructValue struct_value = *cel_Value_GetStruct(top);
      cel_StructValueKey struct_value_key;
      cel_StructValueKey_SetName(
          &struct_value_key,
          _cel_PackedStringView_ToStringView(instr->has.field.direct));
      if (CEL_UNLIKELY(!cel_StructValue_Has(&struct_value, &interp->context,
                                            &struct_value_key, top,
                                            interp->status))) {
        _cel_Interpretable_Throw(interp);
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }
}

// Similar to _cel_Interpretable_Has, except this is used when we know the
// underlying field is in a message. This is an optimization and allows us to
// avoid looking up the field by reflection everytime.
static inline void _cel_Interpretable_MessageHas(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kMap: {
      cel_MapValue map_value = *cel_Value_GetMap(top);
      cel_MapValueKey map_value_key;
      cel_MapValueKey_SetString(
          &map_value_key, cel_StringView_FromString(
                              upb_FieldDef_Name(instr->message_has.field)));
      if (CEL_UNLIKELY(!cel_MapValue_Has(&map_value, &interp->context,
                                         &map_value_key, top,
                                         interp->status))) {
        _cel_Interpretable_Throw(interp);
      }
    } break;
    case cel_ValueKind_kStruct: {
      cel_StructValue struct_value = *cel_Value_GetStruct(top);
      cel_StructValueKey struct_value_key;
      cel_StructValueKey_SetDef(&struct_value_key, instr->message_has.field);
      if (CEL_UNLIKELY(!cel_StructValue_Has(&struct_value, &interp->context,
                                            &struct_value_key, top,
                                            interp->status))) {
        _cel_Interpretable_Throw(interp);
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }
}

static inline void _cel_Interpretable_Select(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kMap: {
      cel_MapValue map_value = *cel_Value_GetMap(top);
      cel_MapValueKey map_value_key;
      cel_MapValueKey_SetString(
          &map_value_key,
          _cel_PackedStringView_ToStringView(instr->select.field.direct));
      if (CEL_UNLIKELY(!cel_MapValue_Get(&map_value, &interp->context,
                                         &map_value_key, top,
                                         interp->status))) {
        _cel_Interpretable_Throw(interp);
      }
    } break;
    case cel_ValueKind_kStruct: {
      cel_StructValue struct_value = *cel_Value_GetStruct(top);
      cel_StructValueKey struct_value_key;
      cel_StructValueKey_SetName(
          &struct_value_key,
          _cel_PackedStringView_ToStringView(instr->select.field.direct));
      if (CEL_UNLIKELY(!cel_StructValue_Get(&struct_value, &interp->context,
                                            &struct_value_key, top,
                                            interp->status))) {
        _cel_Interpretable_Throw(interp);
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }
}

// Similar to _cel_Interpretable_Select, except this is used when we know the
// underlying field is in a message. This is an optimization and allows us to
// avoid looking up the field by reflection everytime.
static inline void _cel_Interpretable_MessageSelect(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kMap: {
      cel_MapValue map_value = *cel_Value_GetMap(top);
      cel_MapValueKey map_value_key;
      cel_MapValueKey_SetString(
          &map_value_key, cel_StringView_FromString(
                              upb_FieldDef_Name(instr->message_has.field)));
      if (CEL_UNLIKELY(!cel_MapValue_Get(&map_value, &interp->context,
                                         &map_value_key, top,
                                         interp->status))) {
        _cel_Interpretable_Throw(interp);
      }
    } break;
    case cel_ValueKind_kStruct: {
      cel_StructValue struct_value = *cel_Value_GetStruct(top);
      cel_StructValueKey struct_value_key;
      cel_StructValueKey_SetDef(&struct_value_key, instr->message_has.field);
      if (CEL_UNLIKELY(!cel_StructValue_Get(&struct_value, &interp->context,
                                            &struct_value_key, top,
                                            interp->status))) {
        _cel_Interpretable_Throw(interp);
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }
}

CEL_ATTRIBUTE_NODISCARD
static inline cel_StringView _cel_Interpretable_Concat(
    _cel_Interpretable* cel_nonnull interp, cel_StringView lhs,
    cel_StringView rhs) {
  CEL_ASSERT_NOT_NULL(interp);

  size_t lhs_size = cel_StringView_Size(lhs);
  size_t rhs_size = cel_StringView_Size(rhs);
  if (lhs_size == 0) {
    return rhs;
  }
  if (rhs_size == 0) {
    return lhs;
  }
  size_t size;
  if (_cel_ckd_add(&size, lhs_size, rhs_size) ||
      size > (size_t)(uint32_t)INT32_MAX) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  char* data = reinterpret_cast<char*>(
      cel_Arena_Malloc(interp->arena, size, cel_nullptr));
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  memcpy(data, cel_StringView_Data(lhs), lhs_size);
  memcpy(data + lhs_size, cel_StringView_Data(rhs), rhs_size);
  return cel_StringView_FromArray(data, size);
}

static inline void _cel_Interpretable_Add(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;
  switch (cel_Value_Kind(lhs)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kInt: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          *top = *rhs;
          break;
        case cel_ValueKind_kInt: {
          int64_t result;
          if (_cel_ckd_add(&result, cel_Value_GetInt(lhs),
                           cel_Value_GetInt(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewIntOverflowError(interp));
            break;
          }
          cel_Value_SetInt(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kUint: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          *top = *rhs;
          break;
        case cel_ValueKind_kUint: {
          uint64_t result;
          if (_cel_ckd_add(&result, cel_Value_GetUint(lhs),
                           cel_Value_GetUint(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewUintOverflowError(interp));
            break;
          }
          cel_Value_SetUint(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kDouble: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          *top = *rhs;
          break;
        case cel_ValueKind_kDouble:
          cel_Value_SetDouble(
              top, cel_Value_GetDouble(lhs) + cel_Value_GetDouble(rhs));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kBytes: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          *top = *rhs;
          break;
        case cel_ValueKind_kBytes:
          cel_Value_SetBytes(
              top, _cel_Interpretable_Concat(interp, cel_Value_GetBytes(lhs),
                                             cel_Value_GetBytes(rhs)));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kString: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          *top = *rhs;
          break;
        case cel_ValueKind_kString:
          cel_Value_SetString(
              top, _cel_Interpretable_Concat(interp, cel_Value_GetString(lhs),
                                             cel_Value_GetString(rhs)));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kDuration: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          *top = *rhs;
          break;
        case cel_ValueKind_kDuration: {
          cel_Duration result;
          if (!cel_Duration_Add(&result, cel_Value_GetDuration(lhs),
                                cel_Value_GetDuration(rhs))) {
            cel_Value_SetError(
                top, _cel_Interpretable_NewDurationOverflowError(interp));
            break;
          }
          cel_Value_SetDuration(top, result);
        } break;
        case cel_ValueKind_kTimestamp: {
          cel_Timestamp result;
          if (!cel_Timestamp_Add(&result, cel_Value_GetTimestamp(rhs),
                                 cel_Value_GetDuration(lhs))) {
            cel_Value_SetError(
                top, _cel_Interpretable_NewTimestampOverflowError(interp));
            break;
          }
          cel_Value_SetTimestamp(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kTimestamp: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          *top = *rhs;
          break;
        case cel_ValueKind_kDuration: {
          cel_Timestamp result;
          if (!cel_Timestamp_Add(&result, cel_Value_GetTimestamp(lhs),
                                 cel_Value_GetDuration(rhs))) {
            cel_Value_SetError(
                top, _cel_Interpretable_NewTimestampOverflowError(interp));
            break;
          }
          cel_Value_SetTimestamp(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    default:
      if (cel_Value_IsError(rhs)) {
        *top = *rhs;
        break;
      }
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
      break;
  }
  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_Subtract(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;
  switch (cel_Value_Kind(lhs)) {
    case cel_ValueKind_kError:
      // error - T -> error
      break;
    case cel_ValueKind_kInt: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // int - error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kInt: {
          // int - int -> int
          int64_t result;
          if (_cel_ckd_sub(&result, cel_Value_GetInt(lhs),
                           cel_Value_GetInt(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewIntOverflowError(interp));
            break;
          }
          cel_Value_SetInt(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kUint: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // uint - error -> error
          *top = *rhs;
          *top = *rhs;
          break;
        case cel_ValueKind_kUint: {
          // uint - uint -> uint
          uint64_t result;
          if (_cel_ckd_sub(&result, cel_Value_GetUint(lhs),
                           cel_Value_GetUint(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewUintOverflowError(interp));
            break;
          }
          cel_Value_SetUint(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kDouble: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // double - error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kDouble:
          // double - double -> double
          cel_Value_SetDouble(
              top, cel_Value_GetDouble(lhs) - cel_Value_GetDouble(rhs));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kDuration: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // google.protobuf.Duration - error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kDuration: {
          // google.protobuf.Duration - google.protobuf.Duration ->
          // google.protobuf.Duration
          cel_Duration result;
          if (!cel_Duration_Sub(&result, cel_Value_GetDuration(lhs),
                                cel_Value_GetDuration(rhs))) {
            cel_Value_SetError(
                top, _cel_Interpretable_NewDurationOverflowError(interp));
            break;
          }
          cel_Value_SetDuration(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kTimestamp: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // google.protobuf.Timestamp - error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kDuration: {
          // google.protobuf.Timestamp - google.protobuf.Duration ->
          // google.protobuf.Timestamp
          cel_Timestamp result;
          if (!cel_Timestamp_Sub(&result, cel_Value_GetTimestamp(lhs),
                                 cel_Value_GetDuration(rhs))) {
            cel_Value_SetError(
                top, _cel_Interpretable_NewTimestampOverflowError(interp));
            break;
          }
          cel_Value_SetTimestamp(top, result);
        } break;
        case cel_ValueKind_kTimestamp: {
          // google.protobuf.Timestamp - google.protobuf.Timestamp ->
          // google.protobuf.Duration
          cel_Duration result;
          if (!cel_Timestamp_Diff(&result, cel_Value_GetTimestamp(lhs),
                                  cel_Value_GetTimestamp(rhs))) {
            cel_Value_SetError(
                top, _cel_Interpretable_NewDurationOverflowError(interp));
            break;
          }
          cel_Value_SetDuration(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }
  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_Multiply(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  switch (cel_Value_Kind(lhs)) {
    case cel_ValueKind_kError:
      // error * T -> error
      break;
    case cel_ValueKind_kInt: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // int * error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kInt: {
          // int * int -> int
          int64_t result;
          if (_cel_ckd_mul(&result, cel_Value_GetInt(lhs),
                           cel_Value_GetInt(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewIntOverflowError(interp));
            break;
          }
          cel_Value_SetInt(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kUint: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // uint * error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kUint: {
          // uint * uint -> uint
          uint64_t result;
          if (_cel_ckd_mul(&result, cel_Value_GetUint(lhs),
                           cel_Value_GetUint(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewUintOverflowError(interp));
            break;
          }
          cel_Value_SetUint(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kDouble: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // double * error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kDouble:
          // double * double -> double
          cel_Value_SetDouble(
              top, cel_Value_GetDouble(lhs) * cel_Value_GetDouble(rhs));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_Divide(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  switch (cel_Value_Kind(lhs)) {
    case cel_ValueKind_kError:
      // error / T -> error
      break;
    case cel_ValueKind_kInt: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // int / error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kInt: {
          // int / 0 -> error
          if (cel_Value_GetInt(rhs) == 0) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewDivideByZeroError(interp));
            break;
          }
          // int / int -> int
          int64_t result;
          if (_cel_ckd_div(&result, cel_Value_GetInt(lhs),
                           cel_Value_GetInt(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewIntOverflowError(interp));
            break;
          }
          cel_Value_SetInt(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kUint: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // uint / error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kUint: {
          // uint / 0 -> error
          if (cel_Value_GetUint(rhs) == 0) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewDivideByZeroError(interp));
            break;
          }
          // uint / uint -> uint
          uint64_t result;
          if (_cel_ckd_div(&result, cel_Value_GetUint(lhs),
                           cel_Value_GetUint(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewUintOverflowError(interp));
            break;
          }
          cel_Value_SetUint(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kDouble: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // double / error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kDouble:
          // double / 0 -> error
          if (cel_Value_GetDouble(rhs) == 0) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewDivideByZeroError(interp));
            break;
          }
          // double / double -> double
          cel_Value_SetDouble(
              top, cel_Value_GetDouble(lhs) / cel_Value_GetDouble(rhs));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }
  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_Modulo(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  switch (cel_Value_Kind(lhs)) {
    case cel_ValueKind_kError:
      // error % T -> error
      break;
    case cel_ValueKind_kInt: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // int % error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kInt: {
          // int % 0 -> error
          if (cel_Value_GetInt(rhs) == 0) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewModuloByZeroError(interp));
            break;
          }
          // int % int -> int
          int64_t result;
          if (_cel_ckd_mod(&result, cel_Value_GetInt(lhs),
                           cel_Value_GetInt(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewIntOverflowError(interp));
            break;
          }
          cel_Value_SetInt(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    case cel_ValueKind_kUint: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // uint % error -> error
          *top = *rhs;
          break;
        case cel_ValueKind_kUint: {
          // uint % 0 -> error
          if (cel_Value_GetUint(rhs) == 0) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewModuloByZeroError(interp));
            break;
          }
          // uint % uint -> uint
          uint64_t result;
          if (_cel_ckd_mod(&result, cel_Value_GetUint(lhs),
                           cel_Value_GetUint(rhs))) {
            cel_Value_SetError(top,
                               _cel_Interpretable_NewUintOverflowError(interp));
            break;
          }
          cel_Value_SetUint(top, result);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_LogicalNot(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 1);
  cel_Value* const val = top;

  switch (cel_Value_Kind(val)) {
    case cel_ValueKind_kError:
      //  !error -> error
      break;
    case cel_ValueKind_kBool: {
      // !true -> false
      cel_Value_SetBool(top, !cel_Value_GetBool(val));
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }
}

static inline void _cel_Interpretable_Negate(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 1);
  cel_Value* const val = top;

  switch (cel_Value_Kind(val)) {
    case cel_ValueKind_kError:
      //  -error -> error
      break;
    case cel_ValueKind_kInt: {
      // -(int) -> -int
      int64_t result;
      if (_cel_ckd_sub(&result, 0, cel_Value_GetInt(val))) {
        cel_Value_SetError(top, _cel_Interpretable_NewIntOverflowError(interp));
        break;
      }
      cel_Value_SetInt(top, result);
    } break;
    case cel_ValueKind_kDouble: {
      // -(double) -> -double
      cel_Value_SetDouble(top, -cel_Value_GetDouble(val));
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }
}

static inline void _cel_Interpretable_LogicalAnd(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  switch (cel_Value_Kind(lhs)) {
    case cel_ValueKind_kError: {
      // error && T
      if (cel_Value_Kind(rhs) == cel_ValueKind_kBool &&
          cel_Value_GetBool(rhs) == false) {
        // error && False -> False
        cel_Value_SetFalse(top);
      }
      // Otherwise, error && T -> error, which is already the value of *top.
    } break;
    case cel_ValueKind_kBool: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // bool && error
          if (cel_Value_GetBool(lhs) == false) {
            // False && error -> False
            cel_Value_SetFalse(top);
          } else {
            // True && error -> error
            *top = *rhs;
          }
          break;
        case cel_ValueKind_kBool:
          // bool && bool -> bool
          cel_Value_SetBool(top,
                            cel_Value_GetBool(lhs) && cel_Value_GetBool(rhs));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_LogicalOr(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  switch (cel_Value_Kind(lhs)) {
    case cel_ValueKind_kError: {
      // error || T
      if (cel_Value_Kind(rhs) == cel_ValueKind_kBool &&
          cel_Value_GetBool(rhs) == true) {
        // error || True -> True
        cel_Value_SetTrue(top);
      }
      // Otherwise, error || T -> error, which is already the value of *top.
    } break;
    case cel_ValueKind_kBool: {
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          // bool || error
          if (cel_Value_GetBool(lhs) == true) {
            // True || error -> True
            cel_Value_SetTrue(top);
          } else {
            // False || error -> error
            *top = *rhs;
          }
          break;
        case cel_ValueKind_kBool:
          // bool || bool -> bool
          cel_Value_SetBool(top,
                            cel_Value_GetBool(lhs) || cel_Value_GetBool(rhs));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
    } break;
    default: {
      cel_Error* error = cel_Error_New(interp->arena);
      if (CEL_UNLIKELY(error == cel_nullptr)) {
        _cel_Interpretable_ThrowOutOfMemory(interp);
      }
      cel_Error_SetCanonicalCode(error, cel_ErrorCode_kInvalidArgument);
      cel_Value_SetError(top, error);
    } break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_Equals(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  if (cel_Value_IsError(lhs)) {
    // Already on top.
  } else if (cel_Value_IsError(rhs)) {
    *top = *rhs;
  } else {
    if (!cel_Value_Equals(lhs, &interp->context, rhs, top, interp->status)) {
      _cel_Interpretable_ThrowIfError(interp);
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
    }
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_NotEquals(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  if (cel_Value_IsError(lhs)) {
    // Already on top.
  } else if (cel_Value_IsError(rhs)) {
    *top = *rhs;
  } else {
    if (cel_Value_Equals(lhs, &interp->context, rhs, top, interp->status)) {
      cel_Value_SetBool(top, !cel_Value_GetBool(top));
    } else {
      _cel_Interpretable_ThrowIfError(interp);
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
    }
  }

  _cel_Interpretable_Pop(interp, instr);
}

CEL_ATTRIBUTE_NODISCARD
static inline bool _cel_Interpretable_Compare(
    const cel_Value* cel_nonnull lhs, const cel_Value* cel_nonnull rhs,
    cel_ValueKind lhs_kind, cel_ValueKind rhs_kind,
    _cel_PartialOrdering* cel_nonnull order) {
  if (lhs_kind != rhs_kind) {
    // Heterogeneous
    switch (lhs_kind) {
      case cel_ValueKind_kInt: {
        _cel_Number lhs_number = _cel_IntNumber(cel_Value_GetInt(lhs));
        switch (rhs_kind) {
          case cel_ValueKind_kUint: {
            _cel_Number rhs_number = _cel_UintNumber(cel_Value_GetUint(rhs));
            *order = _cel_Number_Compare(lhs_number, rhs_number);
            return true;
          }
          case cel_ValueKind_kDouble: {
            _cel_Number rhs_number =
                _cel_DoubleNumber(cel_Value_GetDouble(rhs));
            *order = _cel_Number_Compare(lhs_number, rhs_number);
            return true;
          }
          default:
            return false;
        }
      }
      case cel_ValueKind_kUint: {
        _cel_Number lhs_number = _cel_UintNumber(cel_Value_GetUint(lhs));
        switch (rhs_kind) {
          case cel_ValueKind_kInt: {
            _cel_Number rhs_number = _cel_IntNumber(cel_Value_GetInt(rhs));
            *order = _cel_Number_Compare(lhs_number, rhs_number);
            return true;
          }
          case cel_ValueKind_kDouble: {
            _cel_Number rhs_number =
                _cel_DoubleNumber(cel_Value_GetDouble(rhs));
            *order = _cel_Number_Compare(lhs_number, rhs_number);
            return true;
          }
          default:
            return false;
        }
      }
      case cel_ValueKind_kDouble: {
        _cel_Number lhs_number = _cel_DoubleNumber(cel_Value_GetDouble(lhs));
        switch (rhs_kind) {
          case cel_ValueKind_kInt: {
            _cel_Number rhs_number = _cel_IntNumber(cel_Value_GetInt(rhs));
            *order = _cel_Number_Compare(lhs_number, rhs_number);
            return true;
          }
          case cel_ValueKind_kUint: {
            _cel_Number rhs_number = _cel_UintNumber(cel_Value_GetUint(rhs));
            *order = _cel_Number_Compare(lhs_number, rhs_number);
            return true;
          }
          default:
            return false;
        }
      }
      default:
        return false;
    }
  }
  switch (lhs_kind) {
    case cel_ValueKind_kBool: {
      bool lhs_val = cel_Value_GetBool(lhs);
      bool rhs_val = cel_Value_GetBool(rhs);
      *order = lhs_val < rhs_val   ? _cel_PartialOrdering_kLess
               : lhs_val > rhs_val ? _cel_PartialOrdering_kGreater
                                   : _cel_PartialOrdering_kEquivalent;
      return true;
    }
    case cel_ValueKind_kInt: {
      int64_t lhs_val = cel_Value_GetInt(lhs);
      int64_t rhs_val = cel_Value_GetInt(rhs);
      *order = lhs_val < rhs_val   ? _cel_PartialOrdering_kLess
               : lhs_val > rhs_val ? _cel_PartialOrdering_kGreater
                                   : _cel_PartialOrdering_kEquivalent;
      return true;
    }
    case cel_ValueKind_kUint: {
      uint64_t lhs_val = cel_Value_GetUint(lhs);
      uint64_t rhs_val = cel_Value_GetUint(rhs);
      *order = lhs_val < rhs_val   ? _cel_PartialOrdering_kLess
               : lhs_val > rhs_val ? _cel_PartialOrdering_kGreater
                                   : _cel_PartialOrdering_kEquivalent;
      return true;
    }
    case cel_ValueKind_kDouble: {
      double lhs_val = cel_Value_GetDouble(lhs);
      double rhs_val = cel_Value_GetDouble(rhs);
      *order = _cel_Number_DoubleCompare(lhs_val, rhs_val);
      return true;
    }
    case cel_ValueKind_kString: {
      *order = _cel_PartialOrdering_FromInt(cel_StringView_Compare(
          cel_Value_GetString(lhs), cel_Value_GetString(rhs)));
      return true;
    }
    case cel_ValueKind_kBytes: {
      *order = _cel_PartialOrdering_FromInt(cel_StringView_Compare(
          cel_Value_GetBytes(lhs), cel_Value_GetBytes(rhs)));
      return true;
    }
    case cel_ValueKind_kDuration: {
      *order = _cel_PartialOrdering_FromInt(cel_Duration_Compare(
          cel_Value_GetDuration(lhs), cel_Value_GetDuration(rhs)));
      return true;
    }
    case cel_ValueKind_kTimestamp: {
      *order = _cel_PartialOrdering_FromInt(cel_Timestamp_Compare(
          cel_Value_GetTimestamp(lhs), cel_Value_GetTimestamp(rhs)));
      return true;
    }
    default:
      return false;
  }
}

static inline void _cel_Interpretable_Less(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  const cel_ValueKind lhs_kind = cel_Value_Kind(lhs);
  const cel_ValueKind rhs_kind = cel_Value_Kind(rhs);

  if (lhs_kind == cel_ValueKind_kError) {
    // Already on top.
  } else if (rhs_kind == cel_ValueKind_kError) {
    *top = *rhs;
  } else {
    _cel_PartialOrdering order;
    if (_cel_Interpretable_Compare(lhs, rhs, lhs_kind, rhs_kind, &order)) {
      cel_Value_SetBool(top, order == _cel_PartialOrdering_kLess);
    } else {
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
    }
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_LessEquals(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  const cel_ValueKind lhs_kind = cel_Value_Kind(lhs);
  const cel_ValueKind rhs_kind = cel_Value_Kind(rhs);

  if (lhs_kind == cel_ValueKind_kError) {
    // Already on top.
  } else if (rhs_kind == cel_ValueKind_kError) {
    *top = *rhs;
  } else {
    _cel_PartialOrdering order;
    if (_cel_Interpretable_Compare(lhs, rhs, lhs_kind, rhs_kind, &order)) {
      cel_Value_SetBool(top, order == _cel_PartialOrdering_kLess ||
                                 order == _cel_PartialOrdering_kEquivalent);
    } else {
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
    }
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_Greater(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  const cel_ValueKind lhs_kind = cel_Value_Kind(lhs);
  const cel_ValueKind rhs_kind = cel_Value_Kind(rhs);

  if (lhs_kind == cel_ValueKind_kError) {
    // Already on top.
  } else if (rhs_kind == cel_ValueKind_kError) {
    *top = *rhs;
  } else {
    _cel_PartialOrdering order;
    if (_cel_Interpretable_Compare(lhs, rhs, lhs_kind, rhs_kind, &order)) {
      cel_Value_SetBool(top, order == _cel_PartialOrdering_kGreater);
    } else {
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
    }
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_GreaterEquals(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  const cel_ValueKind lhs_kind = cel_Value_Kind(lhs);
  const cel_ValueKind rhs_kind = cel_Value_Kind(rhs);

  if (lhs_kind == cel_ValueKind_kError) {
    // Already on top.
  } else if (rhs_kind == cel_ValueKind_kError) {
    *top = *rhs;
  } else {
    _cel_PartialOrdering order;
    if (_cel_Interpretable_Compare(lhs, rhs, lhs_kind, rhs_kind, &order)) {
      cel_Value_SetBool(top, order == _cel_PartialOrdering_kGreater ||
                                 order == _cel_PartialOrdering_kEquivalent);
    } else {
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
    }
  }

  _cel_Interpretable_Pop(interp, instr);
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_CondJump(_cel_Interpretable* cel_nonnull interp,
                            const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_NE(instr->cond_jump.jump, 0);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 1);
  if (cel_Value_IsBool(top) &&
      cel_Value_GetBool(top) == instr->cond_jump.cond) {
    return _cel_Interpretable_ShortJump(instr, instr->cond_jump.jump);
  }
  return _cel_Interpretable_ShortJump(instr, 1);
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_TrileanJump(_cel_Interpretable* cel_nonnull interp,
                               const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_NE(instr->trilean_jump.false_jump, 0);
  CEL_ASSERT_NE(instr->trilean_jump.error_jump, 0);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 1);
  if (cel_Value_IsError(top)) {
    return _cel_Interpretable_ShortJump(instr, instr->trilean_jump.error_jump);
  }
  if (cel_Value_IsBool(top)) {
    int32_t jump = cel_Value_GetBool(top) ? 1 : instr->trilean_jump.false_jump;
    _cel_Interpretable_Pop(interp, instr);
    return _cel_Interpretable_ShortJump(instr, jump);
  }
  cel_Value_SetError(top, _cel_Interpretable_NewNoSuchOverloadError(interp));
  return _cel_Interpretable_ShortJump(instr, instr->trilean_jump.error_jump);
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_ErrorJump(_cel_Interpretable* cel_nonnull interp,
                             const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_NE(instr->error_jump.jump, 0);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 1);
  if (cel_Value_IsError(top)) {
    uint32_t pop = instr->error_jump.pop;
    if (pop > 0) {
      cel_Value error = *top;
      _cel_Interpretable_PopN(interp, instr, pop);
      *_cel_Interpretable_Top(interp, instr) = error;
    }
    return _cel_Interpretable_ShortJump(instr, instr->error_jump.jump);
  }
  return _cel_Interpretable_ShortJump(instr, 1);
}

static inline void _cel_Interpretable_List(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  uint32_t element_count = instr->list.count;
  if (element_count == 0) {
    _cel_EmptyListValue_Set(
        cel_Value_SetList(_cel_Interpretable_Push(interp, instr)));
    return;
  }
  cel_Value* src_elements =
      _cel_Interpretable_TopN(interp, instr, element_count);
  cel_Value value;
  cel_ListValue* list_value = cel_Value_SetList(&value);
  _cel_MutableListValue_Set(list_value);
  if (!_cel_MutableListValue_Reserve(list_value, element_count,
                                     interp->arena)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  cel_Value* dst_elements =
      _cel_MutableListValue_AddN(list_value, element_count, interp->arena);
  if (dst_elements == cel_nullptr) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  memcpy(dst_elements, src_elements, element_count * sizeof(cel_Value));
  *_cel_Interpretable_PushAndPopN(interp, instr, element_count) = value;
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_KeyJump(_cel_Interpretable* cel_nonnull interp,
                           const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT_NE(instr->key_jump.jump, 0);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 1);
  if (cel_Value_IsError(top)) {
    uint32_t pop = instr->key_jump.pop;
    if (pop > 0) {
      cel_Value error = *top;
      _cel_Interpretable_PopN(interp, instr, pop);
      *_cel_Interpretable_Top(interp, instr) = error;
    }
    return _cel_Interpretable_ShortJump(instr, instr->key_jump.jump);
  }
  if (!cel_Value_IsMapKey(top)) {
    cel_Value_SetError(
        _cel_Interpretable_PushAndPopN(interp, instr, instr->key_jump.pop + 1),
        _cel_Interpretable_NewBadKeyError(interp));
    return _cel_Interpretable_ShortJump(instr, instr->key_jump.jump);
  }
  return _cel_Interpretable_ShortJump(instr, 1);
}

static inline void _cel_Interpretable_Map(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  uint32_t entry_count = instr->map.count;
  if (entry_count == 0) {
    _cel_EmptyMapValue_Set(
        cel_Value_SetMap(_cel_Interpretable_Push(interp, instr)));
    return;
  }
  cel_Value* src_entries =
      _cel_Interpretable_TopN(interp, instr, entry_count * 2);
  cel_Value value;
  cel_MapValue* map_value = cel_Value_SetMap(&value);
  _cel_MutableMapValue_Set(map_value);
  if (!_cel_MutableMapValue_Reserve(map_value, entry_count, interp->arena)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  for (size_t i = 0; i < entry_count; ++i) {
    cel_Value* src_entry_key = &src_entries[i * 2];
    cel_MapValueKey entry_key;
    switch (cel_Value_Kind(src_entry_key)) {
      case cel_ValueKind_kBool:
        cel_MapValueKey_SetBool(&entry_key, cel_Value_GetBool(src_entry_key));
        break;
      case cel_ValueKind_kInt:
        cel_MapValueKey_SetInt(&entry_key, cel_Value_GetInt(src_entry_key));
        break;
      case cel_ValueKind_kUint:
        cel_MapValueKey_SetUint(&entry_key, cel_Value_GetUint(src_entry_key));
        break;
      case cel_ValueKind_kString:
        cel_MapValueKey_SetString(&entry_key,
                                  cel_Value_GetString(src_entry_key));
        break;
      default:
        CEL_UNREACHABLE();
    }
    cel_Value* entry_value;
    switch (_cel_MutableMapValue_Insert(map_value, &entry_key, cel_nullptr,
                                        &entry_value, interp->arena)) {
      case _cel_MutableMapValueInsertResult_kInserted: {
        *entry_value = src_entries[i * 2 + 1];
      } break;
      case _cel_MutableMapValueInsertResult_kReplaced: {
        cel_Value_SetError(
            _cel_Interpretable_PushAndPopN(interp, instr, entry_count * 2),
            _cel_Interpretable_NewDuplicateKeyError(interp));
        return;
      }
      case _cel_MutableMapValueInsertResult_kOutOfMemory:
        _cel_Interpretable_ThrowOutOfMemory(interp);
    }
  }
  *_cel_Interpretable_PushAndPopN(interp, instr, entry_count * 2) = value;
}

CEL_ATTRIBUTE_NODISCARD
int _cel_Interpretable_FromCharsToInt(cel_StringView string, int64_t* val) {
  const char* data = cel_StringView_Data(string);
  size_t size = cel_StringView_Size(string);
  _cel_FromCharsResult result = _cel_FromChars(data, data + size, val, 10);
  if (result.ec == 0 && result.ptr != data + size) {
    return EINVAL;
  }
  return result.ec;
}

CEL_ATTRIBUTE_NODISCARD
int _cel_Interpretable_FromCharsToUint(cel_StringView string, uint64_t* val) {
  const char* data = cel_StringView_Data(string);
  size_t size = cel_StringView_Size(string);
  _cel_FromCharsResult result = _cel_FromChars(data, data + size, val, 10);
  if (result.ec == 0 && result.ptr != data + size) {
    return EINVAL;
  }
  return result.ec;
}

CEL_ATTRIBUTE_NODISCARD
int _cel_Interpretable_FromCharsToDouble(cel_StringView string, double* val) {
  const char* data = cel_StringView_Data(string);
  size_t size = cel_StringView_Size(string);
  _cel_FromCharsResult result = _cel_FromChars(data, data + size, val);
  if (result.ec == 0 && result.ptr != data + size) {
    return EINVAL;
  }
  return result.ec;
}

CEL_ATTRIBUTE_NODISCARD
cel_StringView _cel_Interpretable_FromIntToChars(_cel_Interpretable* interp,
                                                 int64_t val) {
  CEL_ASSERT_NOT_NULL(interp);
  char buffer[_CEL_MAX_INT_CHARS];
  size_t buffer_size = _cel_ToChars(buffer, val, 10);
  char* data = (char*)cel_Arena_Malloc(interp->arena, buffer_size, cel_nullptr);
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  memcpy(data, buffer, buffer_size);
  return cel_StringView_FromArray(data, buffer_size);
}

CEL_ATTRIBUTE_NODISCARD
cel_StringView _cel_Interpretable_FromUintToChars(_cel_Interpretable* interp,
                                                  uint64_t val) {
  CEL_ASSERT_NOT_NULL(interp);
  char buffer[_CEL_MAX_UINT_CHARS];
  size_t buffer_size = _cel_ToChars(buffer, val, 10);
  char* data = (char*)cel_Arena_Malloc(interp->arena, buffer_size, cel_nullptr);
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  memcpy(data, buffer, buffer_size);
  return cel_StringView_FromArray(data, buffer_size);
}

CEL_ATTRIBUTE_NODISCARD
cel_StringView _cel_Interpretable_FromDoubleToChars(_cel_Interpretable* interp,
                                                    double val) {
  CEL_ASSERT_NOT_NULL(interp);
  char buffer[_CEL_MAX_DOUBLE_CHARS];
  size_t buffer_size = _cel_ToChars(buffer, val);
  char* data = (char*)cel_Arena_Malloc(interp->arena, buffer_size, cel_nullptr);
  if (CEL_UNLIKELY(data == cel_nullptr)) {
    _cel_Interpretable_ThrowOutOfMemory(interp);
  }
  memcpy(data, buffer, buffer_size);
  return cel_StringView_FromArray(data, buffer_size);
}

static inline void _cel_Interpretable_CallUint(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kInt: {
      _cel_Number number = _cel_IntNumber(cel_Value_GetInt(top));
      uint64_t uint_value;
      if (!_cel_Number_ToUint(number, &uint_value)) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetUint(top, uint_value);
      }
    } break;
    case cel_ValueKind_kUint:
      cel_Value_SetUint(top, cel_Value_GetUint(top));
      break;
    case cel_ValueKind_kDouble: {
      _cel_Number number = _cel_DoubleNumber(cel_Value_GetDouble(top));
      uint64_t uint_value;
      if (!_cel_Number_ToUint(number, &uint_value)) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetUint(top, uint_value);
      }
    } break;
    case cel_ValueKind_kString: {
      uint64_t value;
      cel_StringView str = cel_Value_GetString(top);
      if (_cel_Interpretable_FromCharsToUint(str, &value) != 0) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetUint(top, value);
      }
    } break;
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_CallDouble(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kInt: {
      _cel_Number number = _cel_IntNumber(cel_Value_GetInt(top));
      double double_value = _cel_Number_ToDouble(number);
      if (double_value == NAN) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetDouble(top, double_value);
      }
    } break;
    case cel_ValueKind_kUint: {
      _cel_Number number = _cel_UintNumber(cel_Value_GetUint(top));
      double double_value = _cel_Number_ToDouble(number);
      if (double_value == NAN) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetDouble(top, double_value);
      }
    } break;
    case cel_ValueKind_kDouble:
      cel_Value_SetDouble(top, cel_Value_GetDouble(top));
      break;
    case cel_ValueKind_kString: {
      double value;
      cel_StringView str = cel_Value_GetString(top);
      if (_cel_Interpretable_FromCharsToDouble(str, &value) != 0) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetDouble(top, value);
      }
    } break;
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_CallBytes(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kBytes:
      cel_Value_SetBytes(top, cel_Value_GetBytes(top));
      break;
    case cel_ValueKind_kString:
      cel_Value_SetBytes(top, cel_Value_GetString(top));
      break;
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_CallString(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kBool: {
      cel_Value_SetString(top, cel_Value_GetBool(top)
                                   ? cel_StringView_From("true")
                                   : cel_StringView_From("false"));
    } break;
    case cel_ValueKind_kInt: {
      int64_t value = cel_Value_GetInt(top);
      cel_Value_SetString(top,
                          _cel_Interpretable_FromIntToChars(interp, value));
    } break;
    case cel_ValueKind_kUint: {
      uint64_t value = cel_Value_GetUint(top);
      cel_Value_SetString(top,
                          _cel_Interpretable_FromUintToChars(interp, value));
    } break;
    case cel_ValueKind_kDouble: {
      double value = cel_Value_GetDouble(top);
      cel_Value_SetString(top,
                          _cel_Interpretable_FromDoubleToChars(interp, value));
    } break;
    case cel_ValueKind_kBytes: {
      cel_StringView bytes = cel_Value_GetBytes(top);
      if (!_cel_Utf8_IsValid(bytes)) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetString(top, bytes);
      }
    } break;
    case cel_ValueKind_kString:
      cel_Value_SetString(top, cel_Value_GetString(top));
      break;
    case cel_ValueKind_kTimestamp: {
      cel_StringView str;
      if (!_cel_Timestamp_ToRFC3339(cel_Value_GetTimestamp(top), &str,
                                    interp->arena)) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetString(top, str);
      }
    } break;
    case cel_ValueKind_kDuration: {
      cel_StringView str;
      if (!_cel_Duration_ToStringView(cel_Value_GetDuration(top), interp->arena,
                                      &str)) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetString(top, str);
      }
    } break;
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_CallBool(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kBool:
      cel_Value_SetBool(top, cel_Value_GetBool(top));
      break;
    case cel_ValueKind_kString: {
      cel_StringView value = cel_Value_GetString(top);
      if (cel_StringView_EqualsIgnoreCase(value, cel_StringView_From("true"))) {
        cel_Value_SetBool(top, true);
      } else if (cel_StringView_EqualsIgnoreCase(
                     value, cel_StringView_From("false"))) {
        cel_Value_SetBool(top, false);
      } else {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      }
    } break;
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_CallTimestamp(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kTimestamp:
      cel_Value_SetTimestamp(top, cel_Value_GetTimestamp(top));
      break;
    case cel_ValueKind_kString: {
      cel_Timestamp value;
      cel_StringView str = cel_Value_GetString(top);
      if (!_cel_Timestamp_FromRFC3339(&value, str)) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetTimestamp(top, value);
      }
    } break;
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_CallDuration(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kDuration:
      cel_Value_SetDuration(top, cel_Value_GetDuration(top));
      break;
    case cel_ValueKind_kString: {
      cel_Duration value;
      cel_StringView str = cel_Value_GetString(top);
      if (!_cel_Duration_FromStringView(str, &value)) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetDuration(top, value);
      }
    } break;
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_CallContainsString(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 2);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const str = top;
  cel_Value* const substr = str + 1;

  switch (cel_Value_Kind(str)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kString:
      switch (cel_Value_Kind(substr)) {
        case cel_ValueKind_kError:
          *top = *substr;
          break;
        case cel_ValueKind_kString:
          cel_Value_SetBool(
              top, cel_StringView_FindFirst(cel_Value_GetString(str),
                                            cel_Value_GetString(substr)) !=
                       cel_nullptr);
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
      break;
    default:
      if (cel_Value_IsError(substr)) {
        *top = *substr;
      } else {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      }
      break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_CallStartsWithString(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 2);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const str = top;
  cel_Value* const substr = str + 1;

  switch (cel_Value_Kind(str)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kString:
      switch (cel_Value_Kind(substr)) {
        case cel_ValueKind_kError:
          *top = *substr;
          break;
        case cel_ValueKind_kString:
          cel_Value_SetBool(
              top, cel_StringView_StartsWith(cel_Value_GetString(str),
                                             cel_Value_GetString(substr)));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
      break;
    default:
      if (cel_Value_IsError(substr)) {
        *top = *substr;
      } else {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      }
      break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_CallEndsWithString(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 2);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const str = top;
  cel_Value* const substr = str + 1;

  switch (cel_Value_Kind(str)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kString:
      switch (cel_Value_Kind(substr)) {
        case cel_ValueKind_kError:
          *top = *substr;
          break;
        case cel_ValueKind_kString:
          cel_Value_SetBool(
              top, cel_StringView_EndsWith(cel_Value_GetString(str),
                                           cel_Value_GetString(substr)));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
      break;
    default:
      if (cel_Value_IsError(substr)) {
        *top = *substr;
      } else {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      }
      break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_CallRegexExpMatch(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 2);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const str = top;
  cel_Value* const regex = str + 1;

  switch (cel_Value_Kind(str)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kString:
      switch (cel_Value_Kind(regex)) {
        case cel_ValueKind_kError:
          *top = *regex;
          break;
        case cel_ValueKind_kString: {
          cel_Status status;
          cel_Status_Construct(&status);
          cel_StringView pattern = cel_Value_GetString(regex);
          cel_StringView subject = cel_Value_GetString(str);
          // TODO: Enable regexp limits to be configurable.
          bool matched =
              _cel_RegExp_Matches(pattern, cel_nullptr, subject, &status);
          if (CEL_UNLIKELY(!cel_Status_Ok(&status))) {
            cel_Error* error = cel_Error_New(interp->arena);
            if (CEL_UNLIKELY(error == cel_nullptr)) {
              cel_Status_Destruct(&status);
              _cel_Interpretable_ThrowOutOfMemory(interp);
            }
            cel_Error_SetMessage(error, cel_Status_Message(&status));
            cel_Value_SetError(top, error);
          } else {
            cel_Value_SetBool(top, matched);
          }
          cel_Status_Destruct(&status);
        } break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          break;
      }
      break;
    default:
      if (cel_Value_IsError(regex)) {
        *top = *regex;
      } else {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      }
      break;
  }
  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_CallInt(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kInt:
      cel_Value_SetInt(top, cel_Value_GetInt(top));
      break;
    case cel_ValueKind_kUint: {
      _cel_Number number = _cel_UintNumber(cel_Value_GetUint(top));
      int64_t int_value;
      if (!_cel_Number_ToInt(number, &int_value)) {
        cel_Value_SetError(top, _cel_Interpretable_NewIntOverflowError(interp));
      } else {
        cel_Value_SetInt(top, int_value);
      }
    } break;
    case cel_ValueKind_kDouble: {
      _cel_Number number = _cel_DoubleNumber(cel_Value_GetDouble(top));
      int64_t int_value;
      if (!_cel_Number_ToInt(number, &int_value)) {
        cel_Value_SetError(top, _cel_Interpretable_NewIntOverflowError(interp));
      } else {
        cel_Value_SetInt(top, int_value);
      }
    } break;
    case cel_ValueKind_kString: {
      int64_t value;
      cel_StringView str = cel_Value_GetString(top);
      if (_cel_Interpretable_FromCharsToInt(str, &value) != 0) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
      } else {
        cel_Value_SetInt(top, value);
      }
    } break;
    case cel_ValueKind_kTimestamp: {
      cel_Value_SetInt(
          top, cel_Timestamp_ToUnixSeconds(cel_Value_GetTimestamp(top)));
    } break;
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_CallSize(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);
  CEL_ASSERT(instr->call.args == 1);

  cel_Value* const top = _cel_Interpretable_Top(interp, instr);
  switch (cel_Value_Kind(top)) {
    case cel_ValueKind_kError:
      break;
    case cel_ValueKind_kString:
      cel_Value_SetInt(top, _cel_Utf8_DecodedSize(cel_Value_GetString(top)));
      break;
    case cel_ValueKind_kBytes:
      cel_Value_SetInt(top, cel_StringView_Size(cel_Value_GetBytes(top)));
      break;
    // TODO: Add support for other types.
    default:
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
  }
}

static inline void _cel_Interpretable_Index(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  cel_ValueContext context = interp->context;

  switch (cel_Value_Kind(lhs)) {
    case cel_ValueKind_kError:
      // error[T] -> error
      break;
    case cel_ValueKind_kList: {
      int64_t index;
      bool index_is_int = false;
      if (cel_Value_Kind(rhs) == cel_ValueKind_kError) {
        *top = *rhs;
        break;
      }
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kInt:
          index = cel_Value_GetInt(rhs);
          index_is_int = true;
          break;
        case cel_ValueKind_kUint:
          index_is_int = _cel_Number_ToIntLossless(
              _cel_UintNumber(cel_Value_GetUint(rhs)), &index);
          break;
        case cel_ValueKind_kDouble:
          index_is_int = _cel_Number_ToIntLossless(
              _cel_DoubleNumber(cel_Value_GetDouble(rhs)), &index);
          break;
        default:
          index_is_int = false;
          break;
      }

      if (!index_is_int) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewNoSuchOverloadError(interp));
        break;
      }

      // List[int] -> cel_value
      cel_Value list_size;
      const cel_ListValue* list_value = cel_Value_GetList(lhs);
      if (!cel_ListValue_Size(list_value, &context, &list_size,
                              interp->status)) {
        _cel_Interpretable_Throw(interp);
      }
      if (index < 0 || index >= cel_Value_GetInt(&list_size)) {
        cel_Value_SetError(top,
                           _cel_Interpretable_NewIndexOutOfRangeError(interp));
      } else {
        if (!cel_ListValue_Get(list_value, &context, index, top,
                               interp->status)) {
          _cel_Interpretable_Throw(interp);
        }
      }
    } break;
    case cel_ValueKind_kMap: {
      cel_MapValueKey key;
      bool in_error = false;
      switch (cel_Value_Kind(rhs)) {
        case cel_ValueKind_kError:
          *top = *rhs;
          in_error = true;
          break;
        case cel_ValueKind_kInt:
          cel_MapValueKey_SetInt(&key, cel_Value_GetInt(rhs));
          break;
        case cel_ValueKind_kUint:
          cel_MapValueKey_SetUint(&key, cel_Value_GetUint(rhs));
          break;
        case cel_ValueKind_kBool:
          cel_MapValueKey_SetBool(&key, cel_Value_GetBool(rhs));
          break;
        case cel_ValueKind_kString:
          cel_MapValueKey_SetString(&key, cel_Value_GetString(rhs));
          break;
        default:
          cel_Value_SetError(top,
                             _cel_Interpretable_NewNoSuchOverloadError(interp));
          in_error = true;
          break;
      }
      if (!in_error) {
        cel_MapValue map_value = *cel_Value_GetMap(lhs);
        cel_Value has_result;
        if (!cel_MapValue_Has(&map_value, &interp->context, &key, &has_result,
                              interp->status)) {
          _cel_Interpretable_Throw(interp);
        }
        if (!cel_Value_GetBool(&has_result)) {
          cel_Value_SetError(top, _cel_Interpretable_NewNoSuchKeyError(interp));
        } else {
          if (!cel_MapValue_Get(&map_value, &interp->context, &key, top,
                                interp->status)) {
            _cel_Interpretable_Throw(interp);
          }
        }
      }
    } break;
    default: {
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
    } break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

static inline void _cel_Interpretable_In(_cel_Interpretable* cel_nonnull interp,
                                         const _cel_InstrData* cel_nonnull
                                             instr) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(instr);

  cel_Value* const top = _cel_Interpretable_TopN(interp, instr, 2);
  cel_Value* const lhs = top;
  cel_Value* const rhs = lhs + 1;

  switch (cel_Value_Kind(rhs)) {
    case cel_ValueKind_kError:
      if (!cel_Value_IsError(lhs)) {
        *top = *rhs;
      }
      break;
    case cel_ValueKind_kList: {
      if (cel_Value_IsError(lhs)) {
        break;
      }
      const cel_ListValue* list_value = cel_Value_GetList(rhs);
      cel_Value list_size_value;
      if (!cel_ListValue_Size(list_value, &interp->context, &list_size_value,
                              interp->status)) {
        _cel_Interpretable_Throw(interp);
      }
      int64_t list_size = cel_Value_GetInt(&list_size_value);
      bool present = false;
      for (int64_t i = 0; i < list_size; ++i) {
        cel_Value element;
        if (!cel_ListValue_Get(list_value, &interp->context, i, &element,
                               interp->status)) {
          _cel_Interpretable_Throw(interp);
        }
        cel_Value eq_result;
        if (!cel_Value_Equals(lhs, &interp->context, &element, &eq_result,
                              interp->status)) {
          _cel_Interpretable_ThrowIfError(interp);
          continue;
        }
        if (cel_Value_IsError(&eq_result)) {
          continue;
        }
        if (cel_Value_IsBool(&eq_result) && cel_Value_GetBool(&eq_result)) {
          present = true;
          break;
        }
      }
      cel_Value_SetBool(top, present);
    } break;
    case cel_ValueKind_kMap: {
      if (cel_Value_IsError(lhs)) {
        break;
      }
      if (!cel_Value_IsMapKey(lhs)) {
        cel_Value_SetError(top, _cel_Interpretable_NewBadKeyError(interp));
        break;
      }
      cel_MapValueKey key;
      switch (cel_Value_Kind(lhs)) {
        case cel_ValueKind_kBool:
          cel_MapValueKey_SetBool(&key, cel_Value_GetBool(lhs));
          break;
        case cel_ValueKind_kInt:
          cel_MapValueKey_SetInt(&key, cel_Value_GetInt(lhs));
          break;
        case cel_ValueKind_kUint:
          cel_MapValueKey_SetUint(&key, cel_Value_GetUint(lhs));
          break;
        case cel_ValueKind_kString:
          cel_MapValueKey_SetString(&key, cel_Value_GetString(lhs));
          break;
        default:
          // Unreachable
          CEL_UNREACHABLE();
      }
      const cel_MapValue* map_value = cel_Value_GetMap(rhs);
      if (!cel_MapValue_Has(map_value, &interp->context, &key, top,
                            interp->status)) {
        _cel_Interpretable_Throw(interp);
      }
      if (cel_Value_IsError(top)) {
        cel_Value_SetBool(top, false);
      }
    } break;
    default: {
      if (cel_Value_IsError(lhs)) {
        break;
      }
      cel_Value_SetError(top,
                         _cel_Interpretable_NewNoSuchOverloadError(interp));
    } break;
  }

  _cel_Interpretable_Pop(interp, instr);
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_LazyCall(_cel_Interpretable* cel_nonnull interp,
                            const _cel_InstrData* cel_nonnull instr) {
  if (CEL_UNLIKELY(instr->lazy_call.slot) >= interp->slots_len) {
    cel_InternalStatusF(
        interp->status, "cel: bad slot: pc=%" PRIuPTR,
        cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
    _cel_Interpretable_Throw(interp);
  }
  _cel_InterpretableSlot* slot = &interp->slots[instr->lazy_call.slot];
  const _cel_Instr* next = _cel_Interpretable_ShortJump(instr, 1);
  if (slot->active) {
    *_cel_Interpretable_Push(interp, instr) = slot->value;
    return next;
  }
  if (CEL_UNLIKELY(interp->lazy_stack_top == interp->lazy_stack_end)) {
    cel_InternalStatusF(
        interp->status, "cel: lazy stack overflow: pc=%" PRIuPTR,
        cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
    _cel_Interpretable_Throw(interp);
  }
  *interp->lazy_stack_top = next;
  ++interp->lazy_stack_top;
  return _cel_Interpretable_ShortJump(instr, instr->lazy_call.jump);
}

CEL_ATTRIBUTE_NODISCARD
static inline const _cel_Instr* cel_nonnull
_cel_Interpretable_LazyReturn(_cel_Interpretable* cel_nonnull interp,
                              const _cel_InstrData* cel_nonnull instr) {
  if (CEL_UNLIKELY(instr->lazy_return.slot) >= interp->slots_len) {
    cel_InternalStatusF(
        interp->status, "cel: bad slot: pc=%" PRIuPTR,
        cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
    _cel_Interpretable_Throw(interp);
  }
  if (CEL_UNLIKELY(interp->lazy_stack_top == interp->lazy_stack_base)) {
    cel_InternalStatusF(
        interp->status, "cel: lazy stack underflow: pc=%" PRIuPTR,
        cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
    _cel_Interpretable_Throw(interp);
  }
  _cel_InterpretableSlot* slot = &interp->slots[instr->lazy_call.slot];
  if (CEL_UNLIKELY(slot->active)) {
    cel_InternalStatusF(
        interp->status, "cel: slot reused: pc=%" PRIuPTR,
        cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
    _cel_Interpretable_Throw(interp);
  }
  slot->value = *_cel_Interpretable_Top(interp, instr);
  slot->active = true;
  --interp->lazy_stack_top;
  return *interp->lazy_stack_top;
}

static inline void _cel_Interpretable_LazyEnter(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_LT(instr->lazy_enter.slot, interp->slots_len);
  if (CEL_UNLIKELY(interp->slots[instr->lazy_enter.slot].active)) {
    cel_InternalStatusF(
        interp->status, "cel: slot reused: pc=%" PRIuPTR,
        cel_containerof(instr, _cel_Instr, data) - interp->instr_ptr);
    _cel_Interpretable_Throw(interp);
  }
}

static inline void _cel_Interpretable_LazyLeave(
    _cel_Interpretable* cel_nonnull interp,
    const _cel_InstrData* cel_nonnull instr) {
  CEL_ASSERT_GE(instr->lazy_leave.slot, 0);
  CEL_ASSERT_LT(instr->lazy_leave.slot, interp->slots_len);
  CEL_ASSERT_LE(instr->lazy_leave.slot + instr->lazy_leave.num_slots,
                interp->slots_len);
  for (uint32_t i = 0; i < instr->lazy_leave.num_slots; ++i) {
    interp->slots[instr->lazy_leave.slot + i].active = false;
  }
}

#if defined(__GNUC__) || defined(__clang__)
static inline void _cel_Interpretable_ComputedGoto(
    _cel_Interpretable* const cel_nonnull interp,
    cel_Value* const cel_nonnull result) {
  static const void* const dispatch_table[] = {
      &&INSTR_UNREACHABLE,
      &&INSTR_NULL_CONST,
      &&INSTR_FALSE_CONST,
      &&INSTR_TRUE_CONST,
      &&INSTR_INT_CONST,
      &&INSTR_UINT_CONST,
      &&INSTR_DOUBLE_CONST,
      &&INSTR_BYTES_CONST,
      &&INSTR_STRING_CONST,
      &&INSTR_DURATION_CONST,
      &&INSTR_TIMESTAMP_CONST,
      &&INSTR_IDENT,
      &&INSTR_IDENT_JUMP,
      &&INSTR_CONT_IDENT,
      &&INSTR_CONT_IDENT_JUMP,
      &&INSTR_JUMP,
      &&INSTR_HAS,
      &&INSTR_MESSAGE_HAS,
      &&INSTR_SELECT,
      &&INSTR_MESSAGE_SELECT,
      &&INSTR_ADD,
      &&INSTR_SUBTRACT,
      &&INSTR_MULTIPLY,
      &&INSTR_DIVIDE,
      &&INSTR_MODULO,
      &&INSTR_LOGICAL_NOT,
      &&INSTR_NEGATE,
      &&INSTR_LOGICAL_AND,
      &&INSTR_LOGICAL_OR,
      &&INSTR_EQUALS,
      &&INSTR_NOT_EQUALS,
      &&INSTR_LESS,
      &&INSTR_LESS_EQUALS,
      &&INSTR_GREATER,
      &&INSTR_GREATER_EQUALS,
      &&INSTR_COND_JUMP,
      &&INSTR_TRILEAN_JUMP,
      &&INSTR_ERROR_JUMP,
      &&INSTR_LIST,
      &&INSTR_KEY_JUMP,
      &&INSTR_MAP,
      &&INSTR_CALL_BOOL,
      &&INSTR_CALL_INT,
      &&INSTR_CALL_UINT,
      &&INSTR_CALL_DOUBLE,
      &&INSTR_CALL_BYTES,
      &&INSTR_CALL_STRING,
      &&INSTR_CALL_TIMESTAMP,
      &&INSTR_CALL_DURATION,
      &&INSTR_CALL_SIZE,
      &&INSTR_CALL_CONTAINS_STRING,
      &&INSTR_CALL_STARTS_WITH_STRING,
      &&INSTR_CALL_ENDS_WITH_STRING,
      &&INSTR_CALL_REGEX_EXP_MATCH,
      &&INSTR_IN,
      &&INSTR_INDEX,
      &&INSTR_LAZY_CALL,
      &&INSTR_LAZY_RETURN,
      &&INSTR_LAZY_ENTER,
      &&INSTR_LAZY_LEAVE,
      &&INSTR_EXIT,
  };
#define DISPATCH_INSTR() goto* dispatch_table[instr->kind]
#define NEXT_INSTR() ++instr
#define DISPATCH_NEXT_INSTR() \
  NEXT_INSTR();               \
  DISPATCH_INSTR()
  const _cel_Instr* instr = interp->instr_ptr;
  DISPATCH_INSTR();
INSTR_UNREACHABLE:
  _cel_Interpretable_Unreachable(interp, &instr->data);
INSTR_NULL_CONST:
  _cel_Interpretable_NullConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_FALSE_CONST:
  _cel_Interpretable_FalseConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_TRUE_CONST:
  _cel_Interpretable_TrueConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_INT_CONST:
  _cel_Interpretable_IntConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_UINT_CONST:
  _cel_Interpretable_UintConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_DOUBLE_CONST:
  _cel_Interpretable_DoubleConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_BYTES_CONST:
  _cel_Interpretable_BytesConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_STRING_CONST:
  _cel_Interpretable_StringConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_DURATION_CONST:
  _cel_Interpretable_DurationConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_TIMESTAMP_CONST:
  _cel_Interpretable_TimestampConst(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_IDENT:
  _cel_Interpretable_Ident(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_IDENT_JUMP:
  instr = _cel_Interpretable_IdentJump(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_CONT_IDENT:
  _cel_Interpretable_ContIdent(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CONT_IDENT_JUMP:
  instr = _cel_Interpretable_ContIdentJump(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_JUMP:
  instr = _cel_Interpretable_Jump(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_HAS:
  _cel_Interpretable_Has(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_MESSAGE_HAS:
  _cel_Interpretable_MessageHas(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_SELECT:
  _cel_Interpretable_Select(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_MESSAGE_SELECT:
  _cel_Interpretable_MessageSelect(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_ADD:
  _cel_Interpretable_Add(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_SUBTRACT:
  _cel_Interpretable_Subtract(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_MULTIPLY:
  _cel_Interpretable_Multiply(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_DIVIDE:
  _cel_Interpretable_Divide(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_MODULO:
  _cel_Interpretable_Modulo(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_LOGICAL_NOT:
  _cel_Interpretable_LogicalNot(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_NEGATE:
  _cel_Interpretable_Negate(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_LOGICAL_AND:
  _cel_Interpretable_LogicalAnd(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_LOGICAL_OR:
  _cel_Interpretable_LogicalOr(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_EQUALS:
  _cel_Interpretable_Equals(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_NOT_EQUALS:
  _cel_Interpretable_NotEquals(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_LESS:
  _cel_Interpretable_Less(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_LESS_EQUALS:
  _cel_Interpretable_LessEquals(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_GREATER:
  _cel_Interpretable_Greater(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_GREATER_EQUALS:
  _cel_Interpretable_GreaterEquals(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_COND_JUMP:
  instr = _cel_Interpretable_CondJump(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_TRILEAN_JUMP:
  instr = _cel_Interpretable_TrileanJump(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_ERROR_JUMP:
  instr = _cel_Interpretable_ErrorJump(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_LIST:
  _cel_Interpretable_List(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_KEY_JUMP:
  instr = _cel_Interpretable_KeyJump(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_MAP:
  _cel_Interpretable_Map(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_BOOL:
  _cel_Interpretable_CallBool(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_INT:
  _cel_Interpretable_CallInt(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_UINT:
  _cel_Interpretable_CallUint(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_DOUBLE:
  _cel_Interpretable_CallDouble(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_BYTES:
  _cel_Interpretable_CallBytes(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_STRING:
  _cel_Interpretable_CallString(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_TIMESTAMP:
  _cel_Interpretable_CallTimestamp(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_DURATION:
  _cel_Interpretable_CallDuration(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_SIZE:
  _cel_Interpretable_CallSize(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_CONTAINS_STRING:
  _cel_Interpretable_CallContainsString(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_STARTS_WITH_STRING:
  _cel_Interpretable_CallStartsWithString(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_ENDS_WITH_STRING:
  _cel_Interpretable_CallEndsWithString(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_CALL_REGEX_EXP_MATCH:
  _cel_Interpretable_CallRegexExpMatch(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_INDEX:
  _cel_Interpretable_Index(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_IN:
  _cel_Interpretable_In(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_LAZY_CALL:
  instr = _cel_Interpretable_LazyCall(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_LAZY_RETURN:
  instr = _cel_Interpretable_LazyReturn(interp, &instr->data);
  DISPATCH_INSTR();
INSTR_LAZY_ENTER:
  _cel_Interpretable_LazyEnter(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_LAZY_LEAVE:
  _cel_Interpretable_LazyLeave(interp, &instr->data);
  DISPATCH_NEXT_INSTR();
INSTR_EXIT:
  return;
#undef DISPATCH_NEXT_INSTR
#undef NEXT_INSTR
#undef DISPATCH_INSTR
}
#endif

_CEL_ATTRIBUTE_UNUSED
static inline void _cel_Interpretable_Switch(
    _cel_Interpretable* const cel_nonnull interp,
    cel_Value* const cel_nonnull result) {
  const _cel_Instr* instr = interp->instr_ptr;
  while (true) {
    CEL_ASSERT_GE(instr, interp->instr_ptr);
    CEL_ASSERT_LT(instr, interp->instr_ptr + interp->instr_len);
    switch (instr->kind) {
      case _cel_InstrKind_kUnreachable:
        _cel_Interpretable_Unreachable(interp, &instr->data);
      case _cel_InstrKind_kNullConst:
        _cel_Interpretable_NullConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kFalseConst:
        _cel_Interpretable_FalseConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kTrueConst:
        _cel_Interpretable_TrueConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kIntConst:
        _cel_Interpretable_IntConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kUintConst:
        _cel_Interpretable_UintConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kDoubleConst:
        _cel_Interpretable_DoubleConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kBytesConst:
        _cel_Interpretable_BytesConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kStringConst:
        _cel_Interpretable_StringConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kDurationConst:
        _cel_Interpretable_DurationConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kTimestampConst:
        _cel_Interpretable_TimestampConst(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kIdent:
        _cel_Interpretable_Ident(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kIdentJump:
        instr = _cel_Interpretable_IdentJump(interp, &instr->data);
        break;
      case _cel_InstrKind_kContIdent:
        _cel_Interpretable_ContIdent(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kContIdentJump:
        instr = _cel_Interpretable_ContIdentJump(interp, &instr->data);
        break;
      case _cel_InstrKind_kJump:
        instr = _cel_Interpretable_Jump(interp, &instr->data);
        break;
      case _cel_InstrKind_kHas:
        _cel_Interpretable_Has(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kMessageHas:
        _cel_Interpretable_MessageHas(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kSelect:
        _cel_Interpretable_Select(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kMessageSelect:
        _cel_Interpretable_MessageSelect(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kAdd:
        _cel_Interpretable_Add(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kSubtract:
        _cel_Interpretable_Subtract(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kMultiply:
        _cel_Interpretable_Multiply(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kDivide:
        _cel_Interpretable_Divide(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kModulo:
        _cel_Interpretable_Modulo(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kLogicalNot:
        _cel_Interpretable_LogicalNot(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kNegate:
        _cel_Interpretable_Negate(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kLogicalAnd:
        _cel_Interpretable_LogicalAnd(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kLogicalOr:
        _cel_Interpretable_LogicalOr(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kEquals:
        _cel_Interpretable_Equals(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kNotEquals:
        _cel_Interpretable_NotEquals(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kLess:
        _cel_Interpretable_Less(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kLessEquals:
        _cel_Interpretable_LessEquals(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kGreater:
        _cel_Interpretable_Greater(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kGreaterEquals:
        _cel_Interpretable_GreaterEquals(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCondJump:
        instr = _cel_Interpretable_CondJump(interp, &instr->data);
        break;
      case _cel_InstrKind_kTrileanJump:
        instr = _cel_Interpretable_TrileanJump(interp, &instr->data);
        break;
      case _cel_InstrKind_kErrorJump:
        instr = _cel_Interpretable_ErrorJump(interp, &instr->data);
        break;
      case _cel_InstrKind_kList:
        _cel_Interpretable_List(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kKeyJump:
        instr = _cel_Interpretable_KeyJump(interp, &instr->data);
        break;
      case _cel_InstrKind_kMap:
        _cel_Interpretable_Map(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallBool:
        _cel_Interpretable_CallBool(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallInt:
        _cel_Interpretable_CallInt(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallUint:
        _cel_Interpretable_CallUint(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallDouble:
        _cel_Interpretable_CallDouble(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallBytes:
        _cel_Interpretable_CallBytes(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallString:
        _cel_Interpretable_CallString(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallTimestamp:
        _cel_Interpretable_CallTimestamp(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallDuration:
        _cel_Interpretable_CallDuration(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallSize:
        _cel_Interpretable_CallSize(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallContainsString:
        _cel_Interpretable_CallContainsString(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallStartsWithString:
        _cel_Interpretable_CallStartsWithString(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallEndsWithString:
        _cel_Interpretable_CallEndsWithString(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kCallRegexExpMatch:
        _cel_Interpretable_CallRegexExpMatch(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kIndex:
        _cel_Interpretable_Index(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kIn:
        _cel_Interpretable_In(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kLazyCall:
        instr = _cel_Interpretable_LazyCall(interp, &instr->data);
        break;
      case _cel_InstrKind_kLazyReturn:
        instr = _cel_Interpretable_LazyReturn(interp, &instr->data);
        break;
      case _cel_InstrKind_kLazyEnter:
        _cel_Interpretable_LazyEnter(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kLazyLeave:
        _cel_Interpretable_LazyLeave(interp, &instr->data);
        ++instr;
        break;
      case _cel_InstrKind_kExit:
        return;
      default:
        CEL_UNREACHABLE();
    }
  }
}

extern "C" void _cel_Interpretable_Initialize(
    _cel_Interpretable* cel_nonnull interp, const cel_Program* cel_nonnull prog,
    cel_Value* cel_nonnull value_stack_base, size_t value_stack_size,
    _cel_InterpretableSlot* cel_nonnull slots, size_t num_slots,
    const _cel_Instr * cel_nonnull * cel_nonnull lazy_stack_base,
    size_t lazy_stack_size) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(prog);
  CEL_ASSERT_NOT_NULL(value_stack_base);
  CEL_ASSERT_GE(value_stack_size, 1);

  memset(interp, 0, sizeof(*interp));
  interp->alloc = prog->rt->alloc;
  interp->def_pool = prog->rt->def_pool;
  interp->wkts = &prog->rt->wkts;
  interp->instr_ptr = _cel_Array_Data(&prog->instrs);
  interp->instr_len = _cel_Array_Size(&prog->instrs);
  interp->strings_table_ptr = _cel_Array_Data(&prog->strings_table);
  interp->strings_table_len = _cel_Array_Size(&prog->strings_table);
  interp->candidate_names_ptr = _cel_Array_Data(&prog->candidate_names_table);
  interp->candidate_names_len = _cel_Array_Size(&prog->candidate_names_table);
  interp->value_stack_base = interp->value_stack_top = value_stack_base;
  interp->value_stack_end = value_stack_base + value_stack_size;
  interp->context.alloc = interp->alloc;
  interp->context.def_pool = interp->def_pool;
  interp->context.well_known_types = interp->wkts;
  interp->slots = slots;
  interp->slots_len = num_slots;
  interp->lazy_stack_base = interp->lazy_stack_top = lazy_stack_base;
  interp->lazy_stack_end = lazy_stack_base + lazy_stack_size;
}

extern "C" CEL_ATTRIBUTE_NOINLINE bool _cel_Interpretable_Execute(
    _cel_Interpretable* const volatile cel_nonnull interp,
    cel_Value* const volatile cel_nonnull result) {
  CEL_ASSERT_NOT_NULL(interp);
  CEL_ASSERT_NOT_NULL(result);

  switch (_cel_setjmp(interp->jmp)) {
    case 0: {
      // Under GCC and clang, we can use computed gotos. This is faster than the
      // switch alternative. On other compilers we fallback to the switch
      // implementation.
#if defined(__GNUC__) || defined(__clang__)
      _cel_Interpretable_ComputedGoto(interp, result);
#else
      _cel_Interpretable_Switch(interp, result);
#endif
      CEL_ASSERT(cel_Status_Ok(interp->status));
      CEL_ASSERT(interp->value_stack_top == interp->value_stack_base + 1);
      *result = *(--interp->value_stack_top);
      return true;
    }
    case 1: {
      CEL_ASSERT_NOT(cel_Status_Ok(interp->status));
      interp->value_stack_top = interp->value_stack_base;
      return false;
    }
    default:
      CEL_UNREACHABLE();
  }
}
