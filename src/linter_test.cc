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


namespace zetasql {

namespace linter {

TEST(LinterTest, DumperTest) {
    const absl::string_view sql =
        " -- comment 1\nSELECT 3+5;\n--comment 2\nSELECT 4+6";

    const absl::string_view sql2 =
        "SELECT memory From xpreia X;\nSELECT 4+6";

    // printASTTree(sql);
    // printASTTree(sql2);
    // printASTTree(
    //     "SELET A FROM B\nSELECT C FROM D");
}

TEST(LinterTest, StatementLineLengthCheck) {
    const absl::string_view sql =
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x";

    EXPECT_TRUE(checkLineLength(sql).ok());
    EXPECT_FALSE(checkLineLength(sql, 10).ok());
    EXPECT_TRUE(checkLineLength(sql, 10, ' ').ok());
}

TEST(LinterTest, StatementValidityCheck) {
    const absl::string_view valid_sql = "SELECT 5+2";

    const absl::string_view invalid_sql = "SELECT 5+2 sss ddd";

    EXPECT_TRUE(checkStatement(valid_sql).ok());
    EXPECT_FALSE(checkStatement(invalid_sql).ok());

    EXPECT_TRUE(checkStatement(
        "SELECT * FROM emp where b = a or c < d group by x").ok());

    EXPECT_TRUE(checkStatement(
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x").ok());

    EXPECT_FALSE(checkStatement(
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

    EXPECT_TRUE(checkSemicolon(valid_sql).ok());
    EXPECT_FALSE(checkSemicolon(invalid_sql).ok());
    EXPECT_TRUE(checkSemicolon(valid_sql2).ok());
    EXPECT_FALSE(checkSemicolon(invalid_sql2).ok());
}

TEST(LinterTest, UppercaseKeywordCheck) {
    const absl::string_view valid_sql =
        "SELECT * FROM emp WHERE b = a OR c < d GROUP BY x";
    const absl::string_view invalid_sql =
        "SELECT * FROM emp where b = a or c < d GROUP by x";

    EXPECT_TRUE(checkUppercaseKeywords(valid_sql).ok());
    EXPECT_FALSE(checkUppercaseKeywords(invalid_sql).ok());
}

TEST(LinterTest, CommentTypeCheck) {
    const absl::string_view valid_sql =
        "//comment 1\nSELECT 3+5\n//comment 2";
    const absl::string_view invalid_sql =
        "//comment 1\nSELECT 3+5\n--comment 2";
    const absl::string_view valid_sql2 =
        "--comment 1\nSELECT 3+5\n--comment 2";

    EXPECT_TRUE(checkCommentType(valid_sql).ok());
    EXPECT_FALSE(checkCommentType(invalid_sql).ok());
    EXPECT_TRUE(checkCommentType(valid_sql2).ok());
}

TEST(LinterTest, AliasKeywordCheck) {
    const absl::string_view valid_sql =
        "SELECT * FROM emp AS X";
    const absl::string_view invalid_sql =
        "SELECT * FROM emp X";

    EXPECT_TRUE(checkAliasKeyword(valid_sql).ok());
    EXPECT_FALSE(checkAliasKeyword(invalid_sql).ok());
}

}  // namespace linter
}  // namespace zetasql
