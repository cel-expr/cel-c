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
    name = "arena_array",
    hdrs = ["arena_array.h"],
    deps = [
        ":config",
        ":generic_array",
        "//cel-c:arena",
    ],
)

cc_test(
    name = "arena_array_test",
    srcs = ["arena_array_test.cc"],
    deps = [
        ":arena_array",
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "arena_deque",
    hdrs = ["arena_deque.h"],
    deps = [
        ":config",
        ":generic_deque",
        "//cel-c:arena",
    ],
)

cc_test(
    name = "arena_deque_test",
    srcs = ["arena_deque_test.cc"],
    deps = [
        ":arena_deque",
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "arena_flat_hash_map",
    hdrs = ["arena_flat_hash_map.h"],
    deps = [
        ":config",
        ":generic_flat_hash",
        "//cel-c:arena",
    ],
)

cc_test(
    name = "arena_flat_hash_map_test",
    srcs = ["arena_flat_hash_map_test.cc"],
    deps = [
        ":arena_flat_hash_map",
        ":config",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:hash",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "arena_string",
    hdrs = ["arena_string.h"],
    deps = [
        ":config",
        ":generic_string",
        "//cel-c:arena",
        "//cel-c:cstring_view",
        "//cel-c:string_view",
    ],
)

cc_test(
    name = "arena_string_test",
    srcs = ["arena_string_test.cc"],
    deps = [
        ":arena_string",
        ":config",
        ":generic_string",
        "//cel-c:alloc",
        "//cel-c:arena",
        "//cel-c:cstring_view",
        "//cel-c:string_view",
        "@abseil-cpp//absl/log:die_if_null",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "array",
    hdrs = ["array.h"],
    deps = [
        ":config",
        ":generic_array",
        "//cel-c:alloc",
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
    srcs = ["bit.cc"],
    hdrs = ["bit.h"],
    deps = [
        ":config",
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
    name = "container",
    hdrs = ["container.h"],
    deps = [
        ":config",
        ":string",
        "//cel-c:alloc",
        "//cel-c:assert",
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
    srcs = ["ctype.cc"],
    hdrs = ["ctype.h"],
    deps = [
        ":config",
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
    name = "endian",
    hdrs = ["endian.h"],
    deps = [
        ":config",
        ":unaligned",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "endian_test",
    srcs = ["endian_test.cc"],
    deps = [
        ":endian",
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
        "//cel-c:hash",
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
        "//cel-c:hash",
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
        "@abseil-cpp//absl/cleanup",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/log:absl_check",
        "@abseil-cpp//absl/log:die_if_null",
        "@abseil-cpp//absl/strings:string_view",
        "@cel-cpp//internal:testing_descriptor_pool",
        "@cel-cpp//internal:testing_message_factory",
        "@googletest//:gtest_main",
        "@protobuf",
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
        "//cel-c:alloc",
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
    name = "uchar",
    hdrs = ["uchar.h"],
    deps = [
        ":config",
    ],
)

cc_library(
    name = "uint128",
    hdrs = ["uint128.h"],
    deps = [
        ":config",
        ":endian",
        "//cel-c:assert",
    ],
)

cc_test(
    name = "uint128_test",
    srcs = ["uint128_test.cc"],
    deps = [
        ":uint128",
        "@googletest//:gtest_main",
    ],
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
        ":uchar",
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
        ":uchar",
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
        "//cel-c:hash",
        "//cel-c:status",
        "//cel-c:status_code",
        "//cel-c:string_view",
        "//cel-c:timestamp",
        "//cel-c:type",
        "//cel-c:value_headers",
        "//cel-c:value_kind",
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
        "//cel-c:hash",
        "//cel-c:status",
        "//cel-c:string_view",
        "//cel-c:string_view_absl",
        "//cel-c:timestamp",
        "//cel-c:type",
        "//cel-c:value",
        "//cel-c:value_kind",
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
