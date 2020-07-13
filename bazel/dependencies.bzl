load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def zetasql_repo():
    if not native.existing_rule("com_google_zetasql"):
        http_archive(
            name = "com_google_zetasql",

            #From Zetasql repository 29/06/2020
            urls = [
                "https://github.com/google/zetasql/archive/3e5d33d707919df42d1083665113ef05dd78ffff.tar.gz",
            ],
            sha256 = "52ccd7f2e4f5865a1d664b2bf06e6dee948a643347ed23b56249185649c7200b",
            strip_prefix = "zetasql-3e5d33d707919df42d1083665113ef05dd78ffff",
        )
