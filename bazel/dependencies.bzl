load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def zetasql_repo():
    if not native.existing_rule("com_google_zetasql"):
        http_archive(
            name = "com_google_zetasql",

            #From Zetasql repository 29/06/2020
            urls = [
                "https://github.com/google/zetasql/archive/1aefaa7c62fc7a50def879bb7c4225ec6974b7ef.tar.gz",
            ],
            sha256 = "86f9b4f71ecf6f2ca6a7153f349c577ac1eb2f8bd8df2ce680a975ebe5ba1eda",
            strip_prefix = "zetasql-1aefaa7c62fc7a50def879bb7c4225ec6974b7ef",
        )
