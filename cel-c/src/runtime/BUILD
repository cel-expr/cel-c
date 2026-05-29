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
        "activation.c",
        "interpretable.c",
        "interpreter.c",
        "program.c",
        "runtime.c",
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
        "//cel-c:config",
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
        "//cel-c/src:arc",
        "//cel-c/src:array",
        "//cel-c/src:charconv",
        "//cel-c/src:ckdint",
        "//cel-c/src:compare",
        "//cel-c/src:config",
        "//cel-c/src:container",
        "//cel-c/src:deque",
        "//cel-c/src:durationconv",
        "//cel-c/src:flat_hash_map",
        "//cel-c/src:number",
        "//cel-c/src:regexp",
        "//cel-c/src:setjmp",
        "//cel-c/src:string",
        "//cel-c/src:timestampconv",
        "//cel-c/src:utf8",
        "//cel-c/src:value",
        "@protobuf//upb/reflection",
    ],
)
