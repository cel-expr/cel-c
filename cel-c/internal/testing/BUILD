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
    default_testonly = True,
    default_visibility = ["//cel-c:internal"],
)

cc_library(
    name = "compiler",
    srcs = ["compiler.cc"],
    hdrs = ["compiler.h"],
    deps = [
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:ast",
        "//cel-c:ast_proto",
        "//cel-c:config",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@cel-cpp//checker:checker_options",
        "@cel-cpp//checker:optional",
        "@cel-cpp//checker:standard_library",
        "@cel-cpp//common:ast_proto",
        "@cel-cpp//compiler",
        "@cel-cpp//compiler:compiler_factory",
        "@cel-cpp//extensions:bindings_ext",
        "@cel-cpp//internal:testing_descriptor_pool",
        "@cel-cpp//parser:options",
        "@cel-spec//proto/cel/expr:checked_cc_proto",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
    ],
)

cc_library(
    name = "compile",
    srcs = ["compile.cc"],
    hdrs = ["compile.h"],
    deps = [
        ":compiler",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:ast",
        "//cel-c:config",
        "//cel-c:string_view",
    ],
)

cc_library(
    name = "def_pool",
    srcs = ["def_pool.cc"],
    hdrs = ["def_pool.h"],
    deps = [
        "//cel-c:assert",
        "//cel-c:config",
        "@abseil-cpp//absl/log:die_if_null",
        "@cel-spec//proto/cel/expr/conformance/proto3:test_all_types_upb_proto_reflection",
        "@protobuf//:any_upb_reflection_proto",
        "@protobuf//:duration_upb_reflection_proto",
        "@protobuf//:field_mask_upb_reflection_proto",
        "@protobuf//:source_context_upb_reflection_proto",
        "@protobuf//:struct_upb_reflection_proto",
        "@protobuf//:timestamp_upb_reflection_proto",
        "@protobuf//:wrappers_upb_reflection_proto",
        "@protobuf//upb/reflection",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:ast",
        "//cel-c:ast_proto",
        "//cel-c:config",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@cel-cpp//common:ast_proto",
        "@cel-cpp//common:source",
        "@cel-cpp//extensions:bindings_ext",
        "@cel-cpp//parser",
        "@cel-cpp//parser:macro",
        "@cel-cpp//parser:options",
        "@cel-cpp//parser:parser_interface",
        "@cel-spec//proto/cel/expr:checked_cc_proto",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
        "@cel-spec//proto/cel/expr:syntax_cc_proto",
        "@cel-spec//proto/cel/expr:syntax_upb_proto",
    ],
)

cc_library(
    name = "parse",
    srcs = ["parse.cc"],
    hdrs = ["parse.h"],
    deps = [
        ":parser",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:ast",
        "//cel-c:config",
        "//cel-c:string_view",
    ],
)
