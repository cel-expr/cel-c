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

#include "cel-c/operators.h"

#include <stdbool.h>
#include <stddef.h>

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/string_view.h"

typedef struct {
  cel_StringView string;
  cel_StringView alias;
  int precedence;
} cel_OpData;

static const cel_OpData cel_kUnaryOpData[] = {
    {
        .string = CEL_STRINGVIEW_C(""),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 0,
    },
    {
        .string = CEL_STRINGVIEW_C("!_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 2,
    },
    {
        .string = CEL_STRINGVIEW_C("-_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 2,
    },
    {
        .string = CEL_STRINGVIEW_C("@not_strictly_false"),
        .alias = CEL_STRINGVIEW_C("__not_strictly_false__"),
        .precedence = 0,
    },
};

bool cel_UnaryOp_FromString(cel_StringView str, CEL_NONNULL(cel_UnaryOp*) op) {
  CEL_ASSERT_NOT_NULL(op);

  for (size_t i = 1; i < cel_arraysize(cel_kUnaryOpData); ++i) {
    const cel_OpData* const data = &cel_kUnaryOpData[i];
    if (cel_StringView_Equals(str, data->string) ||
        (!cel_StringView_Empty(data->alias) &&
         cel_StringView_Equals(str, data->alias))) {
      *op = (cel_UnaryOp)(int)i;
      return true;
    }
  }
  return false;
}

cel_StringView cel_UnaryOp_ToString(cel_UnaryOp op) {
  CEL_ASSERT_GE(op, 0);
  CEL_ASSERT_LT(op, cel_arraysize(cel_kUnaryOpData));

  return cel_kUnaryOpData[op].string;
}

int cel_UnaryOp_Precedence(cel_UnaryOp op) {
  CEL_ASSERT_GE(op, 0);
  CEL_ASSERT_LT(op, cel_arraysize(cel_kUnaryOpData));

  return cel_kUnaryOpData[op].precedence;
}

static const cel_OpData cel_kBinaryOpData[] = {
    {
        .string = CEL_STRINGVIEW_C(""),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 0,
    },
    {
        .string = CEL_STRINGVIEW_C("_&&_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 6,
    },
    {
        .string = CEL_STRINGVIEW_C("_||_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 7,
    },
    {
        .string = CEL_STRINGVIEW_C("_==_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 5,
    },
    {
        .string = CEL_STRINGVIEW_C("_!=_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 5,
    },
    {
        .string = CEL_STRINGVIEW_C("_<_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 5,
    },
    {
        .string = CEL_STRINGVIEW_C("_<=_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 5,
    },
    {
        .string = CEL_STRINGVIEW_C("_>_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 5,
    },
    {
        .string = CEL_STRINGVIEW_C("_>=_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 5,
    },
    {
        .string = CEL_STRINGVIEW_C("_+_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 4,
    },
    {
        .string = CEL_STRINGVIEW_C("_-_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 4,
    },
    {
        .string = CEL_STRINGVIEW_C("_*_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 3,
    },
    {
        .string = CEL_STRINGVIEW_C("_/_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 3,
    },
    {
        .string = CEL_STRINGVIEW_C("_%_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 3,
    },
    {
        .string = CEL_STRINGVIEW_C("_[_]"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 1,
    },
    {
        .string = CEL_STRINGVIEW_C("@in"),
        .alias = CEL_STRINGVIEW_C("_in_"),
        .precedence = 5,
    },
    {
        .string = CEL_STRINGVIEW_C("_[?_]"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 0,
    },
    {
        .string = CEL_STRINGVIEW_C("_?._"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 0,
    },
};

bool cel_BinaryOp_FromString(cel_StringView str,
                             CEL_NONNULL(cel_BinaryOp*) op) {
  CEL_ASSERT_NOT_NULL(op);

  for (size_t i = 1; i < cel_arraysize(cel_kBinaryOpData); ++i) {
    const cel_OpData* const data = &cel_kBinaryOpData[i];
    if (cel_StringView_Equals(str, data->string) ||
        (!cel_StringView_Empty(data->alias) &&
         cel_StringView_Equals(str, data->alias))) {
      *op = (cel_BinaryOp)(int)i;
      return true;
    }
  }
  return false;
}

cel_StringView cel_BinaryOp_ToString(cel_BinaryOp op) {
  CEL_ASSERT_GE(op, 0);
  CEL_ASSERT_LT(op, cel_arraysize(cel_kBinaryOpData));

  return cel_kBinaryOpData[op].string;
}

int cel_BinaryOp_Precedence(cel_BinaryOp op) {
  CEL_ASSERT_GE(op, 0);
  CEL_ASSERT_LT(op, cel_arraysize(cel_kBinaryOpData));

  return cel_kBinaryOpData[op].precedence;
}

static const cel_OpData cel_kTernaryOpData[] = {
    {
        .string = CEL_STRINGVIEW_C(""),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 0,
    },
    {
        .string = CEL_STRINGVIEW_C("_?_:_"),
        .alias = CEL_STRINGVIEW_C(""),
        .precedence = 8,
    },
};

bool cel_TernaryOp_FromString(cel_StringView str,
                              CEL_NONNULL(cel_TernaryOp*) op) {
  CEL_ASSERT_NOT_NULL(op);

  for (size_t i = 1; i < cel_arraysize(cel_kTernaryOpData); ++i) {
    const cel_OpData* const data = &cel_kTernaryOpData[i];
    if (cel_StringView_Equals(str, data->string) ||
        (!cel_StringView_Empty(data->alias) &&
         cel_StringView_Equals(str, data->alias))) {
      *op = (cel_TernaryOp)(int)i;
      return true;
    }
  }
  return false;
}

cel_StringView cel_TernaryOp_ToString(cel_TernaryOp op) {
  CEL_ASSERT_GE(op, 0);
  CEL_ASSERT_LT(op, cel_arraysize(cel_kTernaryOpData));

  return cel_kTernaryOpData[op].string;
}

int cel_TernaryOp_Precedence(cel_TernaryOp op) {
  CEL_ASSERT_GE(op, 0);
  CEL_ASSERT_LT(op, cel_arraysize(cel_kTernaryOpData));

  return cel_kTernaryOpData[op].precedence;
}
