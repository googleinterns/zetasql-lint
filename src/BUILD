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
    deps = [
        ":lint_errors",
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
    deps = ["@com_google_zetasql//zetasql/public:parse_helpers"],
)

cc_library(
    name = "execute_linter",
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
    deps = ["@com_google_zetasql//zetasql/public:parse_helpers"],
)
cc_test(
    name = "linter_test",
    size = "small",
    srcs = ["linter_test.cc"],
    deps = [
        ":linter",
        ":lint_errors",
        "@com_google_googletest//:gtest_main",
    ]
)