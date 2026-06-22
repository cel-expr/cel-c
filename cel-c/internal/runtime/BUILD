# Copyright 2025 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("@rules_cc//cc:cc_library.bzl", "cc_library")

package(
    default_applicable_licenses = ["//cel-c:license"],
    default_visibility = ["//cel-c:internal"],
)

cc_library(
    name = "runtime",
    srcs = [
        "activation.cc",
        "interpretable.cc",
        "interpreter.cc",
        "program.cc",
        "runtime.cc",
    ],
    hdrs = [
        "activation.h",
        "instr.h",
        "interpretable.h",
        "interpreter.h",
        "program.h",
        "runtime.h",
    ],
    deps = [
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:ast",
        "//cel-c:ast_traverse",
        "//cel-c:ast_visitor",
        "//cel-c:constant",
        "//cel-c:duration",
        "//cel-c:error",
        "//cel-c:error_code",
        "//cel-c:hash",
        "//cel-c:operators",
        "//cel-c:ref",
        "//cel-c:runtime_headers",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:timestamp",
        "//cel-c:trilean",
        "//cel-c:type",
        "//cel-c:value",
        "//cel-c:value_kind",
        "//cel-c:well_known_types",
        "//cel-c/internal:arc",
        "//cel-c/internal:array",
        "//cel-c/internal:charconv",
        "//cel-c/internal:ckdint",
        "//cel-c/internal:compare",
        "//cel-c/internal:config",
        "//cel-c/internal:container",
        "//cel-c/internal:deque",
        "//cel-c/internal:durationconv",
        "//cel-c/internal:flat_hash_map",
        "//cel-c/internal:number",
        "//cel-c/internal:regexp",
        "//cel-c/internal:setjmp",
        "//cel-c/internal:string",
        "//cel-c/internal:timestampconv",
        "//cel-c/internal:utf8",
        "//cel-c/internal:value",
        "@protobuf//upb/reflection",
    ],
)
