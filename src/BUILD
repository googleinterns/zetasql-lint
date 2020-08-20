#
# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_proto_library", "cc_test")

licenses(["notice"])  # Apache v2.0

cc_library(
    name = "check_list",
    srcs = [
        "check_list.cc",
    ],
    hdrs = [
        "check_list.h",
    ],
    copts = [
        "-Wno-return-type",
        "-Wno-sign-compare",
        "-Wno-switch",
    ],
    deps = [
        ":linter",
        ":linter_options",
        "@com_google_zetasql//zetasql/public:parse_helpers",
    ],
)

cc_library(
    name = "linter",
    srcs = [
        "linter.cc",
    ],
    hdrs = [
        "linter.h",
    ],
    deps = [
        ":checks_util",
        ":lint_errors",
        ":linter_options",
        "@com_google_zetasql//zetasql/public:parse_helpers",
    ],
)

cc_library(
    name = "lint_errors",
    srcs = [
        "lint_errors.cc",
    ],
    hdrs = [
        "lint_errors.h",
    ],
    copts = [
        "-Wno-return-type",
        "-Wno-sign-compare",
        "-Wno-switch",
    ],
    deps = [
        "@com_google_zetasql//zetasql/public:parse_helpers",
    ],
)

cc_library(
    name = "linter_options",
    srcs = [
        "linter_options.cc",
    ],
    hdrs = [
        "linter_options.h",
    ],
    deps = [
        ":lint_errors",
    ],
)

cc_library(
    name = "execute_linter",
    srcs = [
        "execute_linter.cc",
    ],
    hdrs = [
        "execute_linter.h",
    ],
    deps = [
        ":check_list",
        ":config",
        ":lint_errors",
        ":linter",
        "@com_google_zetasql//zetasql/public:parse_helpers",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "checks_util",
    srcs = [
        "checks_util.cc",
    ],
    hdrs = [
        "checks_util.h",
    ],
    deps = [
        ":lint_errors",
        ":linter_options",
        "@com_google_zetasql//zetasql/public:parse_helpers",
    ],
)

cc_library(
    name = "config",
    deps = [":config_cc_proto"],
)

cc_proto_library(
    name = "config_cc_proto",
    deps = [":config_proto"],
)

proto_library(
    name = "config_proto",
    srcs = ["config.proto"],
)

# ---------------------------- Binary

cc_binary(
    name = "runner",
    srcs = [
        "runner.cc",
    ],
    copts = [
        "-Wno-return-type",
        "-Wno-sign-compare",
        "-Wno-switch",
    ],
    deps = [
        ":config",
        ":execute_linter",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)

# ---------------------------- TEST

cc_test(
    name = "linter_test",
    size = "small",
    srcs = ["linter_test.cc"],
    deps = [
        ":check_list",
        ":lint_errors",
        ":linter",
        ":linter_options",
        "@com_google_googletest//:gtest_main",
        "@com_google_zetasql//zetasql/public:parse_helpers",
    ],
)

cc_test(
    name = "execute_linter_test",
    size = "small",
    srcs = ["execute_linter_test.cc"],
    deps = [
        ":execute_linter",
        ":lint_errors",
        ":linter_options",
        "@com_google_googletest//:gtest_main",
        "@com_google_zetasql//zetasql/public:parse_helpers",
    ],
)

cc_test(
    name = "checks_util_test",
    size = "small",
    srcs = ["checks_util_test.cc"],
    deps = [
        ":checks_util",
        ":lint_errors",
        "@com_google_googletest//:gtest_main",
        "@com_google_zetasql//zetasql/public:parse_helpers",
    ],
)
