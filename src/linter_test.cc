//
// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "src/linter.h"

#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "gtest/gtest.h"

namespace zetasql::linter {

namespace {

TEST(LinterTest, StatementLineLengthCheck) {
    const absl::string_view sql =
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x";

    EXPECT_TRUE(CheckLineLength(sql).ok());
    EXPECT_FALSE(CheckLineLength(sql, 10).ok());
    EXPECT_TRUE(CheckLineLength(sql, 10, ' ').ok());
}

TEST(LinterTest, StatementValidityCheck) {
    const absl::string_view valid_sql = "SELECT 5+2";

    const absl::string_view invalid_sql = "SELECT 5+2 sss ddd";

    EXPECT_TRUE(CheckStatement(valid_sql).ok());
    EXPECT_FALSE(CheckStatement(invalid_sql).ok());

    EXPECT_TRUE(CheckStatement(
        "SELECT * FROM emp where b = a or c < d group by x").ok());

    EXPECT_TRUE(CheckStatement(
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x").ok());

    EXPECT_FALSE(CheckStatement(
        "SELET A FROM B\nSELECT C FROM D").ok());
}

TEST(LinterTest, SemicolonCheck) {
    const absl::string_view valid_sql =
        "SELECT 3+5;\nSELECT 4+6;";
    const absl::string_view invalid_sql =
        "SELECT 3+5\nSELECT 4+6;";
    const absl::string_view valid_sql2 =
        "SELECT 3+5;  \nSELECT 4+6";
    const absl::string_view invalid_sql2 =
        "SELECT 3+5  ;  \nSELECT 4+6";

    EXPECT_TRUE(CheckSemicolon(valid_sql).ok());
    EXPECT_FALSE(CheckSemicolon(invalid_sql).ok());
    EXPECT_TRUE(CheckSemicolon(valid_sql2).ok());
    EXPECT_FALSE(CheckSemicolon(invalid_sql2).ok());
}

TEST(LinterTest, UppercaseKeywordCheck) {
    const absl::string_view valid_sql =
        "SELECT * FROM emp WHERE b = a OR c < d GROUP BY x";
    const absl::string_view invalid_sql =
        "SELECT * FROM emp where b = a or c < d GROUP by x";

    EXPECT_TRUE(CheckUppercaseKeywords(valid_sql).ok());
    EXPECT_FALSE(CheckUppercaseKeywords(invalid_sql).ok());
}

TEST(LinterTest, CommentTypeCheck) {
    const absl::string_view valid_sql =
        "//comment 1\nSELECT 3+5\n//comment 2";
    const absl::string_view invalid_sql =
        "//comment 1\nSELECT 3+5\n--comment 2";
    const absl::string_view valid_sql2 =
        "--comment 1\nSELECT 3+5\n--comment 2";

    const absl::string_view multiline_comment_sql =
        "/* here is // and -- */SELECT 1+2 -- comment 2";

    const absl::string_view multiline_string_sql =
        "SELECT \"\"\"multiline\nstring--\nliteral\nhaving//--//\"\"\"";

    EXPECT_TRUE(CheckCommentType(valid_sql).ok());
    EXPECT_FALSE(CheckCommentType(invalid_sql).ok());
    EXPECT_TRUE(CheckCommentType(valid_sql2).ok());
    EXPECT_TRUE(CheckCommentType(multiline_comment_sql).ok());
    EXPECT_TRUE(CheckCommentType(multiline_string_sql).ok());
}

TEST(LinterTest, AliasKeywordCheck) {
    const absl::string_view valid_sql =
        "SELECT * FROM emp AS X";
    const absl::string_view invalid_sql =
        "SELECT * FROM emp X";

    EXPECT_TRUE(CheckAliasKeyword(valid_sql).ok());
    EXPECT_FALSE(CheckAliasKeyword(invalid_sql).ok());
}

}  // namespace
}  // namespace zetasql::linter
