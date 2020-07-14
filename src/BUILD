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

licenses(["notice"])  # Apache v2.0

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "linter",
    srcs = [
        "linter.cc",
    ],
    hdrs = [
        "linter.h",
    ],
    copts = [
        "-Wno-return-type",
        "-Wno-sign-compare",
        "-Wno-switch",
    ],
    deps = ["@com_google_zetasql//zetasql/public:parse_helpers"],
)

cc_test(
    name = "linter_test",
    size = "small",
    srcs = ["linter_test.cc"],
    deps = [
        ":linter",
        "@com_google_googletest//:gtest_main",
    ]
)
