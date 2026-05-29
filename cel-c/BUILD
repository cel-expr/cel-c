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
load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("@rules_license//rules:license.bzl", "license")

package(default_applicable_licenses = [":license"])

license(
    name = "license",
    package_name = "c",
)

licenses(["notice"])

exports_files(["LICENSE"])

# Adding anybody besides gRPC Core to this visibility list requires us to establish a formal
# relationship with upb and do design reviews.
package_group(
    name = "friends",
    includes = [
        ":internal",
    ],
)

package_group(
    name = "internal",
    packages = [
        "//cel-c/...",
    ],
)

cc_library(
    name = "alloc",
    srcs = [
        "alloc.c",
    ],
    hdrs = [
        "alloc.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        "//cel-c/src:ckdint",
        "//cel-c/src:malloc",
        "@protobuf//upb/mem",
    ],
)

cc_test(
    name = "alloc_test",
    srcs = ["alloc_test.cc"],
    deps = [
        ":alloc",
        ":config",
        "//cel-c/src:align",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "arena",
    srcs = [
        "arena.c",
    ],
    hdrs = [
        "arena.h",
    ],
    visibility = [":friends"],
    deps = [
        ":alloc",
        ":assert",
        ":config",
        "//cel-c/src:align",
        "//cel-c/src:ckdint",
        "@protobuf//upb/base",
        "@protobuf//upb/mem",
    ],
)

cc_test(
    name = "arena_test",
    srcs = ["arena_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":config",
        "@googletest//:gtest_main",
        "@protobuf//upb/base",
    ],
)

cc_library(
    name = "assert",
    srcs = ["assert.c"],
    hdrs = ["assert.h"],
    visibility = [":friends"],
    deps = [
        ":config",
    ],
)

cc_test(
    name = "assert_test",
    srcs = ["assert_test.cc"],
    deps = [
        ":assert",
        ":config",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "ast",
    srcs = [
        "ast.c",
    ],
    hdrs = [
        "ast.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant",
        ":operators",
        ":ref",
        ":source",
        ":string_view",
        ":type",
    ],
)

cc_test(
    name = "ast_test",
    srcs = ["ast_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":ast",
        ":config",
        ":operators",
        ":source",
        ":string_view",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "ast_proto",
    srcs = [
        "ast_proto.c",
    ],
    hdrs = [
        "ast_proto.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":ast",
        ":config",
        ":constant_proto",
        ":operators",
        ":ref",
        ":ref_proto",
        ":status",
        ":string_view",
        ":type",
        ":type_proto",
        "//cel-c/src:malloc",
        "//cel-c/src:setjmp",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
        "@cel-spec//proto/cel/expr:syntax_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_test(
    name = "ast_proto_test",
    srcs = ["ast_proto_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":ast",
        ":ast_proto",
        ":config",
        ":constant",
        ":duration",
        ":operators",
        ":status",
        ":string_view",
        ":timestamp",
        "@abseil-cpp//absl/cleanup",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@cel-spec//proto/cel/expr:checked_cc_proto",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_library(
    name = "ast_proto_v1alpha1",
    srcs = [
        "ast_proto_v1alpha1.c",
    ],
    hdrs = [
        "ast_proto_v1alpha1.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":ast",
        ":config",
        ":constant_proto_v1alpha1",
        ":operators",
        ":ref",
        ":ref_proto_v1alpha1",
        ":status",
        ":string_view",
        ":type",
        ":type_proto_v1alpha1",
        "//cel-c/src:malloc",
        "//cel-c/src:setjmp",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_test(
    name = "ast_proto_v1alpha1_test",
    srcs = ["ast_proto_v1alpha1_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":ast",
        ":ast_proto_v1alpha1",
        ":config",
        ":constant",
        ":duration",
        ":operators",
        ":status",
        ":string_view",
        ":timestamp",
        "@abseil-cpp//absl/cleanup",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@googleapis//google/api/expr/v1alpha1:checked_cc_proto",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_library(
    name = "ast_traverse",
    srcs = [
        "ast_traverse.c",
    ],
    hdrs = [
        "ast_traverse.h",
    ],
    visibility = [":friends"],
    deps = [
        ":alloc",
        ":assert",
        ":ast",
        ":ast_visitor",
        ":config",
        ":status",
        ":string_view",
        "//cel-c/src:deque",
        "//cel-c/src:malloc",
    ],
)

cc_test(
    name = "ast_traverse_test",
    srcs = ["ast_traverse_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":ast",
        ":ast_traverse",
        ":ast_visitor",
        ":config",
        ":status",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "ast_visitor",
    hdrs = [
        "ast_visitor.h",
    ],
    visibility = [":friends"],
    deps = [
        ":ast",
        ":config",
    ],
)

cc_library(
    name = "config",
    hdrs = ["config.h"],
    visibility = [":friends"],
)

cc_library(
    name = "constant",
    srcs = ["constant.c"],
    hdrs = ["constant.h"],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":duration",
        ":string_view",
        ":timestamp",
    ],
)

cc_test(
    name = "constant_test",
    srcs = ["constant_test.cc"],
    deps = [
        ":config",
        ":constant",
        ":duration",
        ":string_view",
        ":timestamp",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "constant_proto",
    srcs = ["constant_proto.c"],
    hdrs = ["constant_proto.h"],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant",
        ":duration",
        ":duration_proto",
        ":status",
        ":string_view",
        ":timestamp",
        ":timestamp_proto",
        "@cel-spec//proto/cel/expr:syntax_upb_proto",
        "@protobuf//:duration_upb_proto",
        "@protobuf//:struct_upb_proto",
        "@protobuf//:timestamp_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_test(
    name = "constant_proto_test",
    srcs = ["constant_proto_test.cc"],
    deps = [
        ":config",
        ":constant",
        ":constant_proto",
        ":duration",
        ":status",
        ":string_view",
        ":timestamp",
        "@abseil-cpp//absl/log:die_if_null",
        "@cel-spec//proto/cel/expr:syntax_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf//:duration_upb_proto",
        "@protobuf//:struct_upb_proto",
        "@protobuf//:timestamp_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_library(
    name = "constant_proto_v1alpha1",
    srcs = ["constant_proto_v1alpha1.c"],
    hdrs = ["constant_proto_v1alpha1.h"],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant",
        ":duration",
        ":duration_proto",
        ":status",
        ":string_view",
        ":timestamp",
        ":timestamp_proto",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@protobuf//:duration_upb_proto",
        "@protobuf//:struct_upb_proto",
        "@protobuf//:timestamp_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_test(
    name = "constant_proto_v1alpha1_test",
    srcs = ["constant_proto_v1alpha1_test.cc"],
    deps = [
        ":config",
        ":constant",
        ":constant_proto_v1alpha1",
        ":duration",
        ":status",
        ":string_view",
        ":timestamp",
        "@abseil-cpp//absl/log:die_if_null",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf//:duration_upb_proto",
        "@protobuf//:struct_upb_proto",
        "@protobuf//:timestamp_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_library(
    name = "cstring_view",
    srcs = [
        "cstring_view.c",
    ],
    hdrs = [
        "cstring_view.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":hash",
    ],
)

cc_test(
    name = "cstring_view_test",
    srcs = ["cstring_view_test.cc"],
    deps = [
        ":cstring_view",
        ":hash",
        ":string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "decl",
    srcs = [
        "decl.c",
    ],
    hdrs = [
        "decl.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant",
        ":function_scope",
        ":string_view",
        ":type",
        ":type_kind",
    ],
)

cc_test(
    name = "decl_test",
    srcs = ["decl_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":decl",
        ":function_scope",
        ":string_view",
        ":string_view_absl",
        ":type",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "decl_proto",
    srcs = [
        "decl_proto.c",
    ],
    hdrs = [
        "decl_proto.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant_proto",
        ":decl",
        ":function_scope",
        ":status",
        ":string_view",
        ":type",
        ":type_proto",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
    ],
)

cc_test(
    name = "decl_proto_test",
    srcs = ["decl_proto_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":constant",
        ":decl",
        ":decl_proto",
        ":function_scope",
        ":status",
        ":string_view",
        ":type",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@cel-spec//proto/cel/expr:checked_cc_proto",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_library(
    name = "decl_proto_v1alpha1",
    srcs = [
        "decl_proto_v1alpha1.c",
    ],
    hdrs = [
        "decl_proto_v1alpha1.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant_proto_v1alpha1",
        ":decl",
        ":function_scope",
        ":status",
        ":string_view",
        ":type",
        ":type_proto_v1alpha1",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
    ],
)

cc_test(
    name = "decl_proto_v1alpha1_test",
    srcs = ["decl_proto_v1alpha1_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":constant",
        ":decl",
        ":decl_proto_v1alpha1",
        ":function_scope",
        ":status",
        ":string_view",
        ":type",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@googleapis//google/api/expr/v1alpha1:checked_cc_proto",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_library(
    name = "duration",
    srcs = [
        "duration.c",
    ],
    hdrs = [
        "duration.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        "//cel-c/src:ckdint",
    ],
)

cc_test(
    name = "duration_test",
    srcs = ["duration_test.cc"],
    deps = [
        ":config",
        ":duration",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "duration_absl",
    hdrs = ["duration_absl.h"],
    visibility = [":friends"],
    deps = [
        ":config",
        ":duration",
        "@abseil-cpp//absl/time",
    ],
)

cc_test(
    name = "duration_absl_test",
    srcs = ["duration_absl_test.cc"],
    deps = [
        ":duration",
        ":duration_absl",
        "@abseil-cpp//absl/time",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "duration_proto",
    srcs = [
        "duration_proto.c",
    ],
    hdrs = [
        "duration_proto.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":duration",
        "@protobuf//:duration_upb_proto",
    ],
)

cc_test(
    name = "duration_proto_test",
    srcs = ["duration_proto_test.cc"],
    deps = [
        ":duration",
        ":duration_proto",
        "@abseil-cpp//absl/cleanup",
        "@googletest//:gtest_main",
        "@protobuf//:duration_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_library(
    name = "error",
    srcs = [
        "error.c",
    ],
    hdrs = [
        "error.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":error_code",
        ":error_space",
        ":status_code",
        ":string_view",
    ],
)

cc_test(
    name = "error_test",
    srcs = ["error_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":config",
        ":error",
        ":error_code",
        ":error_space",
        ":string_view",
        ":string_view_absl",
        "@abseil-cpp//absl/container:btree",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "error_absl",
    srcs = ["error_absl.cc"],
    hdrs = ["error_absl.h"],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":error",
        ":error_code_absl",
        ":string_view",
        ":string_view_absl",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/strings:cord",
    ],
)

cc_test(
    name = "error_absl_test",
    srcs = ["error_absl_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":config",
        ":error",
        ":error_absl",
        ":error_code",
        ":string_view",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:status_matchers",
        "@abseil-cpp//absl/strings:cord",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "error_proto",
    srcs = ["error_proto.c"],
    hdrs = ["error_proto.h"],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":error",
        ":error_code",
        ":error_code_proto",
        ":string_view",
        "@googleapis//google/rpc:code_upb_proto",
        "@googleapis//google/rpc:status_upb_proto",
        "@protobuf//:any_upb_proto",
        "@protobuf//upb/base",
    ],
)

cc_test(
    name = "error_proto_test",
    srcs = ["error_proto_test.cc"],
    deps = [
        ":config",
        ":error",
        ":error_code",
        ":error_proto",
        ":error_space",
        ":string_view",
        "@abseil-cpp//absl/log:die_if_null",
        "@googleapis//google/rpc:code_upb_proto",
        "@googleapis//google/rpc:status_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf//:any_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_library(
    name = "error_code",
    srcs = [
        "error_code.c",
    ],
    hdrs = [
        "error_code.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
    ],
)

cc_test(
    name = "error_code_test",
    srcs = ["error_code_test.cc"],
    deps = [
        ":error_code",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "error_code_absl",
    hdrs = ["error_code_absl.h"],
    visibility = [":friends"],
    deps = [
        ":config",
        ":error_code",
        "@abseil-cpp//absl/status",
    ],
)

cc_test(
    name = "error_code_absl_test",
    srcs = ["error_code_absl_test.cc"],
    deps = [
        ":error_code",
        ":error_code_absl",
        "@abseil-cpp//absl/status",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "error_code_proto",
    hdrs = ["error_code_proto.h"],
    visibility = [":friends"],
    deps = [
        ":config",
        ":error_code",
        "@googleapis//google/rpc:code_upb_proto",
    ],
)

cc_test(
    name = "error_code_proto_test",
    srcs = ["error_code_proto_test.cc"],
    deps = [
        ":error_code",
        ":error_code_proto",
        "@googleapis//google/rpc:code_upb_proto",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "error_space",
    srcs = [
        "error_space.c",
    ],
    hdrs = [
        "error_space.h",
    ],
    local_defines = select({
        "@platforms//os:linux": ["_GNU_SOURCE=1"],
        "//conditions:default": [],
    }),
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":cstring_view",
        ":status_code",
    ],
)

cc_test(
    name = "error_space_test",
    srcs = ["error_space_test.cc"],
    deps = [
        ":cstring_view",
        ":error_space",
        ":status_code",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "error_status",
    srcs = [
        "error_status.c",
    ],
    hdrs = [
        "error_status.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":error",
        ":status",
        ":string_view",
    ],
)

cc_test(
    name = "error_status_test",
    srcs = ["error_status_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":config",
        ":error",
        ":error_code",
        ":error_space",
        ":error_status",
        ":status",
        ":string_view",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "function_scope",
    hdrs = ["function_scope.h"],
    visibility = [":friends"],
    deps = [
        ":config",
    ],
)

cc_library(
    name = "hash",
    srcs = [
        "hash.c",
    ],
    hdrs = [
        "hash.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        "//cel-c/src:bit",
        "//cel-c/src:config",
        "//cel-c/src:uint128",
        "//cel-c/src:unaligned",
        "@protobuf//upb/base",
    ],
)

cc_test(
    name = "hash_test",
    srcs = ["hash_test.cc"],
    deps = [
        ":hash",
        "@googletest//:gtest_main",
        "@protobuf//upb/base",
    ],
)

cc_library(
    name = "operators",
    srcs = [
        "operators.c",
    ],
    hdrs = [
        "operators.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":string_view",
    ],
)

cc_test(
    name = "operators_test",
    srcs = ["operators_test.cc"],
    deps = [
        ":operators",
        ":string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "runtime_headers",
    tags = ["avoid_dep"],
    textual_hdrs = [
        "activation.h",
        "program.h",
        "runtime.h",
    ],
    visibility = [":internal"],
    deps = [
        ":alloc",
        ":arena",
        ":assert",
        ":ast",
        ":config",
        ":decl",
        ":function_scope",
        ":status",
        ":string_view",
        ":trilean",
        ":type",
        ":value",
        ":well_known_types",
        "@protobuf//upb/reflection",
    ],
)

cc_library(
    name = "runtime",
    hdrs = [
        "activation.h",
        "program.h",
        "runtime.h",
    ],
    visibility = [":friends"],
    deps = [
        ":alloc",
        ":arena",
        ":assert",
        ":ast",
        ":config",
        ":decl",
        ":function_scope",
        ":status",
        ":string_view",
        ":trilean",
        ":type",
        ":value",
        ":well_known_types",
        "//cel-c/src/runtime",  # buildcleaner: keep
        "@protobuf//upb/reflection",
    ],
)

cc_library(
    name = "source",
    hdrs = ["source.h"],
    visibility = [":friends"],
    deps = [
        ":config",
    ],
)

cc_library(
    name = "ref",
    srcs = [
        "ref.c",
    ],
    hdrs = [
        "ref.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant",
        ":string_view",
    ],
)

cc_library(
    name = "ref_proto",
    srcs = [
        "ref_proto.c",
    ],
    hdrs = [
        "ref_proto.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant_proto",
        ":ref",
        ":status",
        ":string_view",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
        "@protobuf//upb/base",
    ],
)

cc_test(
    name = "ref_proto_test",
    srcs = ["ref_proto_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":constant",
        ":ref",
        ":ref_proto",
        ":status",
        ":string_view",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@cel-spec//proto/cel/expr:checked_cc_proto",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_library(
    name = "ref_proto_v1alpha1",
    srcs = [
        "ref_proto_v1alpha1.c",
    ],
    hdrs = [
        "ref_proto_v1alpha1.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":constant_proto",
        ":constant_proto_v1alpha1",
        ":ref",
        ":status",
        ":string_view",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@protobuf//upb/base",
    ],
)

cc_test(
    name = "ref_proto_v1alpha1_test",
    srcs = ["ref_proto_v1alpha1_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":constant",
        ":ref",
        ":ref_proto_v1alpha1",
        ":status",
        ":string_view",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@googleapis//google/api/expr/v1alpha1:checked_cc_proto",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_library(
    name = "status",
    srcs = [
        "status.c",
    ],
    hdrs = [
        "status.h",
    ],
    visibility = [":friends"],
    deps = [
        ":alloc",
        ":assert",
        ":config",
        ":error_space",
        ":status_code",
        ":string_view",
        "//cel-c/src:array",
        "//cel-c/src:string",
    ],
)

cc_test(
    name = "status_test",
    srcs = ["status_test.cc"],
    deps = [
        ":config",
        ":error_space",
        ":status",
        ":status_code",
        ":string_view",
        ":string_view_absl",
        "@abseil-cpp//absl/container:btree",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "status_absl",
    srcs = ["status_absl.cc"],
    hdrs = ["status_absl.h"],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":status",
        ":status_code_absl",
        ":string_view",
        ":string_view_absl",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/strings:cord",
        "@abseil-cpp//absl/strings:string_view",
    ],
)

cc_test(
    name = "status_absl_test",
    srcs = ["status_absl_test.cc"],
    deps = [
        ":config",
        ":error_space",
        ":status",
        ":status_absl",
        ":status_code",
        ":string_view",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:status_matchers",
        "@abseil-cpp//absl/strings:cord",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "status_proto",
    srcs = ["status_proto.c"],
    hdrs = ["status_proto.h"],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":status",
        ":status_code",
        ":status_code_proto",
        ":string_view",
        "@googleapis//google/rpc:code_upb_proto",
        "@googleapis//google/rpc:status_upb_proto",
        "@protobuf//:any_upb_proto",
        "@protobuf//upb/base",
        "@protobuf//upb/mem",
    ],
)

cc_test(
    name = "status_proto_test",
    srcs = ["status_proto_test.cc"],
    deps = [
        ":config",
        ":error_space",
        ":status",
        ":status_code",
        ":status_proto",
        ":string_view",
        "@abseil-cpp//absl/log:die_if_null",
        "@googleapis//google/rpc:code_upb_proto",
        "@googleapis//google/rpc:status_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf//:any_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_library(
    name = "status_code",
    srcs = [
        "status_code.c",
    ],
    hdrs = [
        "status_code.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":error_code",
    ],
)

cc_test(
    name = "status_code_test",
    srcs = ["status_code_test.cc"],
    deps = [
        ":status_code",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "status_code_absl",
    hdrs = ["status_code_absl.h"],
    visibility = [":friends"],
    deps = [
        ":config",
        ":status_code",
        "@abseil-cpp//absl/status",
    ],
)

cc_test(
    name = "status_code_absl_test",
    srcs = ["status_code_absl_test.cc"],
    deps = [
        ":status_code",
        ":status_code_absl",
        "@abseil-cpp//absl/status",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "status_code_proto",
    hdrs = ["status_code_proto.h"],
    visibility = [":friends"],
    deps = [
        ":config",
        ":status_code",
        "@googleapis//google/rpc:code_upb_proto",
    ],
)

cc_test(
    name = "status_code_proto_test",
    srcs = ["status_code_proto_test.cc"],
    deps = [
        ":status_code",
        ":status_code_proto",
        "@googleapis//google/rpc:code_upb_proto",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "string_view",
    srcs = [
        "string_view.c",
    ],
    hdrs = [
        "string_view.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":cstring_view",
        ":hash",
        "//cel-c/src:memory",
        "@protobuf//upb/base",
    ],
)

cc_test(
    name = "string_view_test",
    srcs = ["string_view_test.cc"],
    deps = [
        ":cstring_view",
        ":hash",
        ":string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "string_view_absl",
    hdrs = ["string_view_absl.h"],
    visibility = [":friends"],
    deps = [
        ":config",
        ":string_view",
        "@abseil-cpp//absl/strings:string_view",
    ],
)

cc_test(
    name = "string_view_absl_test",
    srcs = ["string_view_absl_test.cc"],
    deps = [
        ":string_view",
        ":string_view_absl",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "timestamp",
    srcs = [
        "timestamp.c",
    ],
    hdrs = [
        "timestamp.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":duration",
        "//cel-c/src:ckdint",
    ],
)

cc_test(
    name = "timestamp_test",
    srcs = ["timestamp_test.cc"],
    deps = [
        ":config",
        ":duration",
        ":timestamp",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "timestamp_absl",
    hdrs = ["timestamp_absl.h"],
    visibility = [":friends"],
    deps = [
        ":config",
        ":timestamp",
        "@abseil-cpp//absl/time",
    ],
)

cc_test(
    name = "timestamp_absl_test",
    srcs = ["timestamp_absl_test.cc"],
    deps = [
        ":timestamp",
        ":timestamp_absl",
        "@abseil-cpp//absl/time",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "timestamp_proto",
    srcs = [
        "timestamp_proto.c",
    ],
    hdrs = [
        "timestamp_proto.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":timestamp",
        "@protobuf//:timestamp_upb_proto",
    ],
)

cc_test(
    name = "timestamp_proto_test",
    srcs = ["timestamp_proto_test.cc"],
    deps = [
        ":timestamp",
        ":timestamp_proto",
        "@abseil-cpp//absl/cleanup",
        "@googletest//:gtest_main",
        "@protobuf//:timestamp_upb_proto",
        "@protobuf//upb/mem",
    ],
)

cc_library(
    name = "trilean",
    hdrs = ["trilean.h"],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
    ],
)

cc_library(
    name = "type",
    srcs = [
        "type.c",
    ],
    hdrs = [
        "type.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":string_view",
        ":type_kind",
        "@protobuf//upb/reflection",
    ],
)

cc_test(
    name = "type_test",
    srcs = ["type_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":config",
        ":string_view",
        ":type",
        ":type_kind",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
        "@protobuf//:any_upb_reflection_proto",
        "@protobuf//:api_upb_reflection_proto",
        "@protobuf//:duration_upb_reflection_proto",
        "@protobuf//:source_context_upb_reflection_proto",
        "@protobuf//:struct_upb_reflection_proto",
        "@protobuf//:timestamp_upb_reflection_proto",
        "@protobuf//:type_upb_reflection_proto",
        "@protobuf//:wrappers_upb_reflection_proto",
        "@protobuf//upb/reflection",
    ],
)

cc_library(
    name = "type_kind",
    hdrs = ["type_kind.h"],
    visibility = [":friends"],
    deps = [
        ":config",
    ],
)

cc_library(
    name = "type_proto",
    srcs = [
        "type_proto.c",
    ],
    hdrs = [
        "type_proto.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":status",
        ":string_view",
        ":type",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
    ],
)

cc_test(
    name = "type_proto_test",
    srcs = ["type_proto_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":status",
        ":string_view",
        ":type",
        ":type_proto",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@cel-spec//proto/cel/expr:checked_cc_proto",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_library(
    name = "type_proto_v1alpha1",
    srcs = [
        "type_proto_v1alpha1.c",
    ],
    hdrs = [
        "type_proto_v1alpha1.h",
    ],
    visibility = [":friends"],
    deps = [
        ":arena",
        ":assert",
        ":config",
        ":status",
        ":string_view",
        ":type",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
    ],
)

cc_test(
    name = "type_proto_v1alpha1_test",
    srcs = ["type_proto_v1alpha1_test.cc"],
    deps = [
        ":alloc",
        ":arena",
        ":status",
        ":string_view",
        ":type",
        ":type_proto_v1alpha1",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@googleapis//google/api/expr/v1alpha1:checked_cc_proto",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_library(
    name = "value_headers",
    tags = ["avoid_dep"],
    textual_hdrs = ["value.h"],
    visibility = [":internal"],
    deps = [
        ":alloc",
        ":arena",
        ":assert",
        ":config",
        ":duration",
        ":error",
        ":status",
        ":string_view",
        ":timestamp",
        ":value_kind",
        ":well_known_types",
        "@protobuf//upb/message",
        "@protobuf//upb/reflection",
    ],
)

cc_library(
    name = "value",
    hdrs = [
        "value.h",
    ],
    visibility = [":friends"],
    deps = [
        ":alloc",
        ":arena",
        ":assert",
        ":config",
        ":duration",
        ":error",
        ":status",
        ":string_view",
        ":timestamp",
        ":value_kind",
        ":well_known_types",
        "//cel-c/src:value",  # buildcleaner: keep
        "@protobuf//upb/message",
        "@protobuf//upb/reflection",
    ],
)

cc_library(
    name = "value_kind",
    hdrs = ["value_kind.h"],
    visibility = [":friends"],
    deps = [
        ":config",
    ],
)

cc_library(
    name = "well_known_types",
    srcs = [
        "well_known_types.c",
    ],
    hdrs = [
        "well_known_types.h",
    ],
    visibility = [":friends"],
    deps = [
        ":assert",
        ":config",
        ":status",
        ":string_view",
        "@protobuf//upb/base",
        "@protobuf//upb/reflection",
    ],
)

cc_test(
    name = "well_known_types_test",
    srcs = ["well_known_types_test.cc"],
    deps = [
        ":config",
        ":status",
        ":well_known_types",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
        "@protobuf//:any_upb_reflection_proto",
        "@protobuf//:duration_upb_reflection_proto",
        "@protobuf//:field_mask_upb_reflection_proto",
        "@protobuf//:struct_upb_reflection_proto",
        "@protobuf//:timestamp_upb_reflection_proto",
        "@protobuf//:wrappers_upb_reflection_proto",
        "@protobuf//upb/reflection",
    ],
)
