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

package(
    default_applicable_licenses = ["//cel-c:license"],
    default_visibility = ["//cel-c:internal"],
)

cc_library(
    name = "align",
    hdrs = ["align.h"],
    deps = [
        ":bit",
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "align_test",
    srcs = ["align_test.cc"],
    deps = [
        ":align",
        ":config",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "alloca",
    hdrs = ["alloca.h"],
    deps = [":config"],
)

cc_library(
    name = "any",
    srcs = ["any.cc"],
    hdrs = ["any.h"],
    deps = [
        ":config",
        "//cel-c:assert",
        "//cel-c:status",
        "//cel-c:status_code",
        "//cel-c:string_view",
        "//cel-c:well_known_types",
        "@protobuf//upb/mem",
        "@protobuf//upb/message",
        "@protobuf//upb/mini_table",
        "@protobuf//upb/reflection",
        "@protobuf//upb/wire",
    ],
)

cc_test(
    name = "any_test",
    srcs = ["any_test.cc"],
    deps = [
        ":any",
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:status",
        "//cel-c:status_code",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "//cel-c:type_kind",
        "//cel-c:well_known_types",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
        "@protobuf//:any_cc_proto",
        "@protobuf//:any_upb_proto",
        "@protobuf//:any_upb_reflection_proto",
        "@protobuf//:wrappers_cc_proto",
        "@protobuf//:wrappers_upb_proto",
        "@protobuf//:wrappers_upb_reflection_proto",
        "@protobuf//upb/base",
        "@protobuf//upb/message",
        "@protobuf//upb/reflection",
    ],
)

cc_library(
    name = "arc",
    hdrs = ["arc.h"],
    deps = [
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "arc_test",
    srcs = ["arc_test.cc"],
    deps = [
        ":arc",
        ":config",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "array",
    hdrs = ["array.h"],
    deps = [
        ":config",
        ":generic_array",
        ":trivial_relocation",
        "//cel-c:alloc",
        "//cel-c:assert",
        "//cel-c:config",
    ],
)

cc_test(
    name = "array_test",
    srcs = ["array_test.cc"],
    deps = [
        ":array",
        ":config",
        "//cel-c:alloc",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "asan",
    hdrs = ["asan.h"],
    deps = [
        ":config",
    ],
)

cc_library(
    name = "ast",
    srcs = [
        "ast.cc",
    ],
    hdrs = [
        "ast.h",
    ],
    deps = [
        ":attribute",
        ":constant",
        ":ref",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:operators",
        "//cel-c:source",
        "//cel-c:string_view",
        "//cel-c:type",
    ],
)

cc_test(
    name = "ast_test",
    srcs = ["ast_test.cc"],
    deps = [
        ":ast",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:ast",
        "//cel-c:config",
        "//cel-c:operators",
        "//cel-c:source",
        "//cel-c:string_view",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "ast_traverse",
    srcs = [
        "ast_traverse.cc",
    ],
    hdrs = [
        "ast_traverse.h",
    ],
    deps = [
        ":ast",
        ":ast_visitor",
        "//cel-c:alloc",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c/internal:deque",
        "//cel-c/internal:malloc",
    ],
)

cc_test(
    name = "ast_traverse_test",
    srcs = ["ast_traverse_test.cc"],
    deps = [
        ":ast",
        ":ast_traverse",
        ":ast_visitor",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:ast",
        "//cel-c:config",
        "//cel-c:status",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "ast_visitor",
    hdrs = [
        "ast_visitor.h",
    ],
    deps = [
        ":ast",
        "//cel-c:config",
    ],
)

cc_library(
    name = "attribute",
    srcs = ["attribute.cc"],
    hdrs = [
        "attribute.h",
    ],
    deps = [
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:string_view",
    ],
)

cc_test(
    name = "attribute_test",
    srcs = ["attribute_test.cc"],
    deps = [
        ":attribute",
        "//cel-c:config",
        "//cel-c:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "binary_search",
    hdrs = ["binary_search.h"],
    deps = [
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "binary_search_test",
    srcs = ["binary_search_test.cc"],
    deps = [
        ":binary_search",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "bit",
    hdrs = ["bit.h"],
    deps = [
        ":config",
        "@abseil-cpp//absl/numeric:bits",
    ],
)

cc_test(
    name = "bit_test",
    srcs = ["bit_test.cc"],
    deps = [
        ":bit",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "bitset",
    srcs = ["bitset.cc"],
    hdrs = ["bitset.h"],
    deps = [
        ":bit",
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "bitset_test",
    srcs = ["bitset_test.cc"],
    deps = [
        ":bitset",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "charconv",
    srcs = ["charconv.cc"],
    hdrs = ["charconv.h"],
    deps = [
        ":ckdint",
        ":config",
        ":ctype",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "charconv_test",
    srcs = ["charconv_test.cc"],
    deps = [
        ":charconv",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "ckdint",
    hdrs = ["ckdint.h"],
    deps = [
        ":config",
    ],
)

cc_test(
    name = "ckdint_test",
    srcs = ["ckdint_test.cc"],
    deps = [
        ":ckdint",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "compare",
    hdrs = ["compare.h"],
    deps = [
        ":config",
    ],
)

cc_library(
    name = "config",
    hdrs = ["config.h"],
    deps = [
        "//cel-c:config",
    ],
)

cc_test(
    name = "config_test",
    srcs = ["config_test.cc"],
    deps = [
        ":config",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "constant",
    srcs = ["constant.cc"],
    hdrs = ["constant.h"],
    deps = [
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:duration",
        "//cel-c:string_view",
        "//cel-c:timestamp",
    ],
)

cc_test(
    name = "constant_test",
    srcs = ["constant_test.cc"],
    deps = [
        ":constant",
        "//cel-c:config",
        "//cel-c:duration",
        "//cel-c:string_view",
        "//cel-c:timestamp",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "constant_proto",
    srcs = ["constant_proto.cc"],
    hdrs = ["constant_proto.h"],
    deps = [
        ":constant",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:duration",
        "//cel-c:duration_proto",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:timestamp",
        "//cel-c:timestamp_proto",
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
        ":constant",
        ":constant_proto",
        "//cel-c:config",
        "//cel-c:duration",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:timestamp",
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
    srcs = ["constant_proto_v1alpha1.cc"],
    hdrs = ["constant_proto_v1alpha1.h"],
    deps = [
        ":constant",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:duration",
        "//cel-c:duration_proto",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:timestamp",
        "//cel-c:timestamp_proto",
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
        ":constant",
        ":constant_proto_v1alpha1",
        "//cel-c:config",
        "//cel-c:duration",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:timestamp",
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
    name = "container",
    hdrs = ["container.h"],
    deps = [
        ":config",
        ":string",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:string_view",
    ],
)

cc_test(
    name = "container_test",
    srcs = ["container_test.cc"],
    deps = [
        ":container",
        "//cel-c:alloc",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "ctype",
    hdrs = ["ctype.h"],
    deps = [
        ":config",
        "@abseil-cpp//absl/strings",
    ],
)

cc_test(
    name = "ctype_test",
    srcs = ["ctype_test.cc"],
    deps = [
        ":ctype",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "decl",
    srcs = [
        "decl.cc",
    ],
    hdrs = [
        "decl.h",
    ],
    deps = [
        ":constant",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:function_scope",
        "//cel-c:string_view",
        "//cel-c:type",
        "//cel-c:type_kind",
    ],
)

cc_test(
    name = "decl_test",
    srcs = ["decl_test.cc"],
    deps = [
        ":decl",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:function_scope",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "//cel-c:type",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "decl_proto",
    srcs = [
        "decl_proto.cc",
    ],
    hdrs = [
        "decl_proto.h",
    ],
    deps = [
        ":constant_proto",
        ":decl",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:function_scope",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:type",
        "//cel-c:type_proto",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
    ],
)

cc_test(
    name = "decl_proto_test",
    srcs = ["decl_proto_test.cc"],
    deps = [
        ":constant",
        ":decl",
        ":decl_proto",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:function_scope",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:type",
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
        "decl_proto_v1alpha1.cc",
    ],
    hdrs = [
        "decl_proto_v1alpha1.h",
    ],
    deps = [
        ":constant_proto_v1alpha1",
        ":decl",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:function_scope",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:type",
        "//cel-c:type_proto_v1alpha1",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
    ],
)

cc_test(
    name = "decl_proto_v1alpha1_test",
    srcs = ["decl_proto_v1alpha1_test.cc"],
    deps = [
        ":constant",
        ":decl",
        ":decl_proto_v1alpha1",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:function_scope",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:type",
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
    name = "deque",
    hdrs = ["deque.h"],
    deps = [
        ":config",
        ":generic_deque",
        "//cel-c:alloc",
    ],
)

cc_test(
    name = "deque_test",
    srcs = ["deque_test.cc"],
    deps = [
        ":config",
        ":deque",
        "//cel-c:alloc",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "flat_hash_map",
    hdrs = ["flat_hash_map.h"],
    deps = [
        ":config",
        ":generic_flat_hash",
        "//cel-c:alloc",
    ],
)

cc_test(
    name = "flat_hash_map_test",
    srcs = ["flat_hash_map_test.cc"],
    deps = [
        ":config",
        ":flat_hash_map",
        "//cel-c:alloc",
        "@abseil-cpp//absl/hash",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "flat_hash_set",
    hdrs = ["flat_hash_set.h"],
    deps = [
        ":config",
        ":generic_flat_hash",
        "//cel-c:alloc",
    ],
)

cc_test(
    name = "flat_hash_set_test",
    srcs = ["flat_hash_set_test.cc"],
    deps = [
        ":config",
        ":flat_hash_set",
        "//cel-c:alloc",
        "@abseil-cpp//absl/hash",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "generic_array",
    srcs = ["generic_array.cc"],
    hdrs = ["generic_array.h"],
    deps = [
        ":asan",
        ":ckdint",
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:assert",
    ],
)

cc_library(
    name = "generic_deque",
    srcs = ["generic_deque.cc"],
    hdrs = ["generic_deque.h"],
    deps = [
        ":align",
        ":asan",
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:assert",
    ],
)

cc_library(
    name = "generic_flat_hash",
    srcs = ["generic_flat_hash.cc"],
    hdrs = ["generic_flat_hash.h"],
    deps = [
        ":align",
        ":bit",
        ":bitset",
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:assert",
    ],
)

cc_library(
    name = "generic_string",
    srcs = ["generic_string.cc"],
    hdrs = ["generic_string.h"],
    deps = [
        ":asan",
        ":ckdint",
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:cstring_view",
        "//cel-c:string_view",
    ],
)

cc_test(
    name = "generic_string_test",
    srcs = ["generic_string_test.cc"],
    deps = [
        ":generic_string",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "malloc",
    srcs = ["malloc.cc"],
    hdrs = ["malloc.h"],
    deps = [
        ":bit",
        ":ckdint",
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "malloc_test",
    srcs = ["malloc_test.cc"],
    deps = [
        ":align",
        ":config",
        ":malloc",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "memory",
    srcs = ["memory.cc"],
    hdrs = ["memory.h"],
    deps = [
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "memory_test",
    srcs = ["memory_test.cc"],
    deps = [
        ":memory",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "message_equality",
    srcs = ["message_equality.cc"],
    hdrs = ["message_equality.h"],
    deps = [
        ":any",
        ":config",
        ":number",
        ":setjmp",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:string_view",
        "//cel-c:well_known_types",
        "@protobuf//upb/base",
        "@protobuf//upb/message",
        "@protobuf//upb/message:compare",
        "@protobuf//upb/reflection",
    ],
)

cc_test(
    name = "message_equality_test",
    srcs = ["message_equality_test.cc"],
    deps = [
        ":config",
        ":message_equality",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:status",
        "//cel-c:well_known_types",
        "@abseil-cpp//absl/base",
        "@abseil-cpp//absl/functional:overload",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@abseil-cpp//absl/types:variant",
        "@cel-spec//proto/cel/expr/conformance/proto2:test_all_types_cc_proto",
        "@cel-spec//proto/cel/expr/conformance/proto2:test_all_types_upb_proto",
        "@cel-spec//proto/cel/expr/conformance/proto2:test_all_types_upb_proto_minitable",
        "@cel-spec//proto/cel/expr/conformance/proto2:test_all_types_upb_proto_reflection",
        "@cel-spec//proto/cel/expr/conformance/proto3:test_all_types_cc_proto",
        "@cel-spec//proto/cel/expr/conformance/proto3:test_all_types_upb_proto",
        "@cel-spec//proto/cel/expr/conformance/proto3:test_all_types_upb_proto_reflection",
        "@googletest//:gtest_main",
        "@protobuf",
        "@protobuf//:any_cc_proto",
        "@protobuf//:any_upb_reflection_proto",
        "@protobuf//:duration_cc_proto",
        "@protobuf//:duration_upb_reflection_proto",
        "@protobuf//:field_mask_cc_proto",
        "@protobuf//:field_mask_upb_reflection_proto",
        "@protobuf//:struct_cc_proto",
        "@protobuf//:struct_upb_reflection_proto",
        "@protobuf//:timestamp_cc_proto",
        "@protobuf//:timestamp_upb_reflection_proto",
        "@protobuf//:wrappers_cc_proto",
        "@protobuf//:wrappers_upb_reflection_proto",
        "@protobuf//upb/base",
        "@protobuf//upb/message",
        "@protobuf//upb/message:message_unknowns_testonly",
        "@protobuf//upb/mini_table",
        "@protobuf//upb/reflection",
        "@protobuf//upb/wire",
    ],
)

cc_library(
    name = "number",
    hdrs = ["number.h"],
    deps = [
        ":compare",
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "number_test",
    srcs = ["number_test.cc"],
    deps = [
        ":compare",
        ":number",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "rc",
    hdrs = ["rc.h"],
    deps = [
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "rc_test",
    srcs = ["rc_test.cc"],
    deps = [
        ":config",
        ":rc",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "ref",
    srcs = [
        "ref.cc",
    ],
    hdrs = [
        "ref.h",
    ],
    deps = [
        ":constant",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:string_view",
    ],
)

cc_library(
    name = "ref_proto",
    srcs = [
        "ref_proto.cc",
    ],
    hdrs = [
        "ref_proto.h",
    ],
    deps = [
        ":constant_proto",
        ":ref",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:status",
        "//cel-c:string_view",
        "@cel-spec//proto/cel/expr:checked_upb_proto",
        "@protobuf//upb/base",
    ],
)

cc_test(
    name = "ref_proto_test",
    srcs = ["ref_proto_test.cc"],
    deps = [
        ":constant",
        ":ref",
        ":ref_proto",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:status",
        "//cel-c:string_view",
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
        "ref_proto_v1alpha1.cc",
    ],
    hdrs = [
        "ref_proto_v1alpha1.h",
    ],
    deps = [
        ":constant_proto",
        ":constant_proto_v1alpha1",
        ":ref",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:config",
        "//cel-c:status",
        "//cel-c:string_view",
        "@googleapis//google/api/expr/v1alpha1:checked_upb_proto",
        "@protobuf//upb/base",
    ],
)

cc_test(
    name = "ref_proto_v1alpha1_test",
    srcs = ["ref_proto_v1alpha1_test.cc"],
    deps = [
        ":constant",
        ":ref",
        ":ref_proto_v1alpha1",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:status",
        "//cel-c:string_view",
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
    name = "regexp",
    srcs = ["regexp.cc"],
    hdrs = ["regexp.h"],
    deps = [
        ":alloca",
        ":config",
        ":malloc",
        "//cel-c:assert",
        "//cel-c:status",
        "//cel-c:status_code",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "@abseil-cpp//absl/strings:string_view",
        "@re2",
    ],
)

cc_test(
    name = "regexp_test",
    srcs = ["regexp_test.cc"],
    deps = [
        ":config",
        ":regexp",
        "//cel-c:status",
        "//cel-c:status_code",
        "//cel-c:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "runtime_test",
    srcs = ["runtime_test.cc"],
    deps = [
        ":config",
        ":value",
        "@googletest//:gtest_main",
        "@abseil-cpp//absl/cleanup",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:status_matchers",
        "@abseil-cpp//absl/strings:string_view",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:ast",
        "//cel-c:duration",
        "//cel-c:error",
        "//cel-c:error_code",
        "//cel-c:runtime",
        "//cel-c:status",
        "//cel-c:status_absl",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "//cel-c:timestamp",
        "//cel-c:trilean",
        "//cel-c:value",
        "//cel-c/internal/runtime",
        "//cel-c/internal/testing:compiler",
        "//cel-c/internal/testing:def_pool",
        "//cel-c/internal/testing:parser",
        "@cel-cpp//internal:testing_descriptor_pool",
        "@cel-cpp//internal:testing_message_factory",
        "@protobuf",
        # Embeds the full IANA time zone database so that `absl::LoadTimeZone`
        # works on platforms without a system zoneinfo database (e.g. Windows).
        "//third_party/tz:zoneinfo_embedded",
        "@protobuf//upb/message",
        "@protobuf//upb/reflection",
        "@protobuf//upb/wire",
    ],
)

cc_library(
    name = "setjmp",
    hdrs = ["setjmp.h"],
    deps = [
        ":config",
    ],
)

cc_test(
    name = "setjmp_test",
    srcs = ["setjmp_test.cc"],
    deps = [
        ":setjmp",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "sort",
    hdrs = ["sort.h"],
    deps = [
        ":alloca",
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "sort_test",
    srcs = ["sort_test.cc"],
    deps = [
        ":sort",
        "@abseil-cpp//absl/base",
        "@abseil-cpp//absl/random",
        "@abseil-cpp//absl/random:distributions",
        "@abseil-cpp//absl/strings",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "stable_sort",
    hdrs = ["stable_sort.h"],
    deps = [
        ":alloca",
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "stable_sort_test",
    srcs = ["stable_sort_test.cc"],
    deps = [
        ":stable_sort",
        "@abseil-cpp//absl/base",
        "@abseil-cpp//absl/random",
        "@abseil-cpp//absl/random:distributions",
        "@abseil-cpp//absl/strings",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "string",
    hdrs = ["string.h"],
    deps = [
        ":config",
        ":generic_string",
        ":trivial_relocation",
        "//cel-c:alloc",
        "//cel-c:assert",
        "//cel-c:cstring_view",
        "//cel-c:string_view",
    ],
)

cc_test(
    name = "string_test",
    srcs = ["string_test.cc"],
    deps = [
        ":config",
        ":generic_string",
        ":string",
        ":trivial_relocation",
        "//cel-c:alloc",
        "//cel-c:cstring_view",
        "//cel-c:string_view",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "trilean_test",
    srcs = ["trilean_test.cc"],
    deps = [
        ":config",
        "//cel-c:trilean",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "trivial_relocation",
    hdrs = ["trivial_relocation.hpp"],
    deps = [":config"],
)

cc_library(
    name = "unaligned",
    hdrs = ["unaligned.h"],
    deps = [
        ":config",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "unaligned_test",
    srcs = ["unaligned_test.cc"],
    deps = [
        ":unaligned",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "unicode",
    hdrs = ["unicode.h"],
    deps = [
        ":config",
    ],
)

cc_test(
    name = "unicode_test",
    srcs = ["unicode_test.cc"],
    deps = [
        ":unicode",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "utf8",
    srcs = ["utf8.cc"],
    hdrs = ["utf8.h"],
    deps = [
        ":config",
        ":unicode",
        "//cel-c:assert",
        "@protobuf//upb/base",
    ],
)

cc_test(
    name = "utf8_test",
    srcs = ["utf8_test.cc"],
    deps = [
        ":utf8",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
        "@protobuf//upb/base",
    ],
)

cc_library(
    name = "value",
    srcs = [
        "empty_list_value.cc",
        "empty_map_value.cc",
        "list_value.cc",
        "map_value.cc",
        "mutable_list_value.cc",
        "mutable_map_value.cc",
        "opaque_value.cc",
        "optional_value.cc",
        "parsed_map_field_value.cc",
        "parsed_message_value.cc",
        "parsed_repeated_field_value.cc",
        "struct_value.cc",
        "value.cc",
    ],
    hdrs = [
        "empty_list_value.h",
        "empty_map_value.h",
        "map_value.h",
        "mutable_list_value.h",
        "mutable_map_value.h",
        "parsed_map_field_value.h",
        "parsed_message_value.h",
        "parsed_repeated_field_value.h",
    ],
    visibility = ["//cel-c:internal"],
    deps = [
        ":any",
        ":array",
        ":bit",
        ":ckdint",
        ":config",
        ":message_equality",
        ":sort",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:duration",
        "//cel-c:error",
        "//cel-c:error_code",
        "//cel-c:status",
        "//cel-c:status_code",
        "//cel-c:string_view",
        "//cel-c:timestamp",
        "//cel-c:type",
        "//cel-c:value_headers",
        "//cel-c:value_kind",
        "@abseil-cpp//absl/hash",
        "@protobuf//upb/base",
        "@protobuf//upb/message",
        "@protobuf//upb/reflection",
    ],
)

cc_test(
    name = "value_test",
    srcs = [
        "empty_list_value_test.cc",
        "empty_map_value_test.cc",
        "list_value_test.cc",
        "map_value_test.cc",
        "mutable_list_value_test.cc",
        "mutable_map_value_test.cc",
        "opaque_value_test.cc",
        "optional_value_test.cc",
        "parsed_map_field_value_test.cc",
        "parsed_message_value_test.cc",
        "parsed_repeated_field_value_test.cc",
        "struct_value_test.cc",
        "value_test.cc",
    ],
    deps = [
        ":config",
        ":value",
        ":value_testing",
        "//cel-c:duration",
        "//cel-c:error",
        "//cel-c:error_code",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "//cel-c:timestamp",
        "//cel-c:value",
        "//cel-c:value_kind",
        "@abseil-cpp//absl/hash",
        "@abseil-cpp//absl/strings:string_view",
        "@googletest//:gtest_main",
        "@protobuf//upb/base",
        "@protobuf//upb/message",
    ],
)

cc_library(
    name = "value_testing",
    testonly = True,
    srcs = ["value_testing.cc"],
    hdrs = ["value_testing.h"],
    deps = [
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:duration",
        "//cel-c:error",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:timestamp",
        "//cel-c:type",
        "//cel-c:value",
        "//cel-c:value_kind",
        "//cel-c:well_known_types",
        "@abseil-cpp//absl/base",
        "@abseil-cpp//absl/functional:overload",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@abseil-cpp//absl/types:variant",
        "@cel-spec//proto/cel/expr/conformance/proto3:test_all_types_cc_proto",
        "@cel-spec//proto/cel/expr/conformance/proto3:test_all_types_upb_proto_reflection",
        "@googletest//:gtest",
        "@protobuf",
        "@protobuf//:any_cc_proto",
        "@protobuf//:any_upb_reflection_proto",
        "@protobuf//:duration_cc_proto",
        "@protobuf//:duration_upb_reflection_proto",
        "@protobuf//:field_mask_cc_proto",
        "@protobuf//:field_mask_upb_reflection_proto",
        "@protobuf//:struct_cc_proto",
        "@protobuf//:struct_upb_reflection_proto",
        "@protobuf//:timestamp_cc_proto",
        "@protobuf//:timestamp_upb_reflection_proto",
        "@protobuf//:wrappers_cc_proto",
        "@protobuf//:wrappers_upb_reflection_proto",
        "@protobuf//upb/message",
        "@protobuf//upb/reflection",
        "@protobuf//upb/wire",
    ],
)

cc_library(
    name = "timestampconv",
    srcs = ["timestampconv.cc"],
    hdrs = ["timestampconv.h"],
    deps = [
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "//cel-c:timestamp",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:string_view",
        "@abseil-cpp//absl/time",
    ],
)

cc_test(
    name = "timestampconv_test",
    srcs = ["timestampconv_test.cc"],
    deps = [
        ":timestampconv",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:string_view",
        "//cel-c:timestamp",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "durationconv",
    srcs = ["durationconv.cc"],
    hdrs = ["durationconv.h"],
    deps = [
        ":config",
        "//cel-c:arena",
        "//cel-c:assert",
        "//cel-c:duration",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@abseil-cpp//absl/strings:string_view",
        "@abseil-cpp//absl/time",
    ],
)

cc_test(
    name = "durationconv_test",
    srcs = ["durationconv_test.cc"],
    deps = [
        ":durationconv",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:duration",
        "//cel-c:string_view",
        "@googletest//:gtest_main",
    ],
)
