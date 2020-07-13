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
    absl::string_view sql =
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x";
    absl::string_view multiline_sql =
    "SELECT c\n"
    "some long invalid sql statement that shouldn't stop check\n"
    "SELECT t from G\n";

    EXPECT_TRUE(CheckLineLength(sql).ok());
    EXPECT_FALSE(CheckLineLength(sql, 10).ok());
    EXPECT_TRUE(CheckLineLength(sql, 10, ' ').ok());

    EXPECT_TRUE(CheckLineLength(multiline_sql).ok());
    EXPECT_FALSE(CheckLineLength(multiline_sql, 30).ok());
}

TEST(LinterTest, StatementValidityCheck) {
    EXPECT_TRUE(CheckParserSucceeds("SELECT 5+2").ok());
    EXPECT_FALSE(CheckParserSucceeds("SELECT 5+2 sss ddd").ok());

    EXPECT_TRUE(CheckParserSucceeds(
        "SELECT * FROM emp where b = a or c < d group by x").ok());

    EXPECT_TRUE(CheckParserSucceeds(
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x").ok());

    EXPECT_FALSE(CheckParserSucceeds(
        "SELET A FROM B\nSELECT C FROM D").ok());

    EXPECT_FALSE(CheckParserSucceeds(
        "SELECT 1; SELECT 2 3 4;").ok());
}

TEST(LinterTest, SemicolonCheck) {
    EXPECT_TRUE(CheckSemicolon("SELECT 3+5;\nSELECT 4+6;").ok());
    EXPECT_TRUE(CheckSemicolon("SELECT 3+5;   \n   SELECT 4+6;").ok());
    EXPECT_FALSE(CheckSemicolon("SELECT 3+5\nSELECT 4+6;").ok());
    EXPECT_FALSE(CheckSemicolon("SELECT 3+5;  \nSELECT 4+6").ok());
    EXPECT_FALSE(CheckSemicolon("SELECT 3+5  ;  \nSELECT 4+6").ok());
}

TEST(LinterTest, UppercaseKeywordCheck) {
    EXPECT_TRUE(CheckUppercaseKeywords(
        "SELECT * FROM emp WHERE b = a OR c < d GROUP BY x").ok());
    EXPECT_FALSE(CheckUppercaseKeywords(
        "SELECT * FROM emp where b = a or c < d GROUP by x").ok());
}

TEST(LinterTest, CommentTypeCheck) {
    EXPECT_FALSE(CheckCommentType(
        "# Comment 1\n-- Comment 2\nSELECT 3+5\n").ok());
    EXPECT_TRUE(CheckCommentType(
        "-- Comment /* unfinished comment").ok());

    EXPECT_TRUE(CheckCommentType("//comment 1\nSELECT 3+5\n//comment 2").ok());
    EXPECT_FALSE(CheckCommentType("//comment 1\nSELECT 3+5\n--comment 2").ok());
    EXPECT_TRUE(CheckCommentType("--comment 1\nSELECT 3+5\n--comment 2").ok());

    // Check a sql containing a multiline comment.
    EXPECT_TRUE(CheckCommentType(
        "/* here is // and -- */SELECT 1+2 -- comment 2").ok());

    // Check multiline string literal
    EXPECT_TRUE(CheckCommentType(
        "SELECT \"\"\"multiline\nstring--\nliteral\nhaving//--//\"\"\"").ok());
}

TEST(LinterTest, AliasKeywordCheck) {
    EXPECT_FALSE(CheckAliasKeyword("SELECT 1 a").ok());
    EXPECT_TRUE(CheckAliasKeyword("SELECT * FROM emp AS X").ok());
    EXPECT_FALSE(CheckAliasKeyword("SELECT * FROM emp X").ok());
    EXPECT_TRUE(CheckAliasKeyword("SELECT 1 AS one").ok());
}

}  // namespace
}  // namespace zetasql::linter
