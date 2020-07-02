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
        "-Wno-unused-but-set-parameter",
        "-Wno-unused-function",
        #"-std=c++17",
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