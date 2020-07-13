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

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "gtest/gtest.h"
#include "absl/strings/string_view.h"
#include "src/lint_errors.h"
#include "zetasql/base/status.h"
#include "zetasql/base/status_macros.h"

namespace zetasql::linter {

namespace {

// TODO(orhanuysal): add advanced checks.
// currently, these tests just check if the result is ok
// number of lint errors or lint error itself is not checked

void Test_if(bool expect,
    absl::Status (*lint_check)(absl::string_view, LinterResult*),
    const absl::string_view sql ) {
    LinterResult result;
    absl::Status status = (*lint_check)(sql, &result);
    EXPECT_TRUE(status.ok());
    if ( expect ) {
        EXPECT_TRUE(result.ok());
    } else {
        EXPECT_FALSE(result.ok());
    }
}

TEST(LinterTest, StatementLineLengthCheck) {
    absl::string_view sql =
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x";
    absl::string_view multiline_sql =
    "SELECT c\n"
    "some long invalid sql statement that shouldn't stop check\n"
    "SELECT t from G\n";

    // CheckLineLength is special and it will not use Test(..) function
    LinterResult result;
    absl::Status status;

    status = CheckLineLength(sql, &result);
    EXPECT_TRUE(result.ok());
    result.clear();
    status = CheckLineLength(sql, &result, 10);
    EXPECT_FALSE(result.ok());
    result.clear();
    status = CheckLineLength(sql, &result, 10, ' ');
    EXPECT_TRUE(result.ok());
    result.clear();

    status = CheckLineLength(multiline_sql, &result);
    EXPECT_TRUE(result.ok());
    result.clear();
    status = CheckLineLength(multiline_sql, &result, 30);
    EXPECT_FALSE(result.ok());

    // Just to erase warnings
    EXPECT_TRUE(status.ok());
}

TEST(LinterTest, StatementValidityCheck) {
    Test_if(true, CheckParserSucceeds, "SELECT 5+2");
    Test_if(false, CheckParserSucceeds, "SELECT 5+2 sss ddd");

    Test_if(true, CheckParserSucceeds,
        "SELECT * FROM emp where b = a or c < d group by x");

    Test_if(true, CheckParserSucceeds,
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x");

    Test_if(false, CheckParserSucceeds,
        "SELET A FROM B\nSELECT C FROM D");
}

TEST(LinterTest, SemicolonCheck) {
    Test_if(true, CheckSemicolon, "SELECT 3+5;\nSELECT 4+6;");
    Test_if(false, CheckSemicolon, "SELECT 3+5;\nSELECT 4+6");
    Test_if(true, CheckSemicolon, "SELECT 3+5;  \nSELECT 4+6;");
    Test_if(false, CheckSemicolon, "SELECT 3+5  ;  \nSELECT 4+6;");
}

TEST(LinterTest, UppercaseKeywordCheck) {
    Test_if(true, CheckUppercaseKeywords,
        "SELECT * FROM emp WHERE b = a OR c < d GROUP BY x");
    Test_if(true, CheckUppercaseKeywords,
        "SELECT * FROM emp where b = a or c < d GROUP by x");
    Test_if(false, CheckUppercaseKeywords,
        "SeLEct * frOM emp wHEre b = a or c < d GROUP by x");
}

TEST(LinterTest, CommentTypeCheck) {
    Test_if(true, CheckCommentType, "//comment 1\nSELECT 3+5\n//comment 2");
    Test_if(false, CheckCommentType, "//comment 1\nSELECT 3+5\n--comment 2");
    Test_if(true, CheckCommentType, "--comment 1\nSELECT 3+5\n--comment 2");

    // Check a sql containing a multiline comment.
    Test_if(true, CheckCommentType,
        "/* here is // and -- */SELECT 1+2 -- comment 2");

    // Check multiline string literal
    Test_if(true, CheckCommentType,
        "SELECT \"\"\"multiline\nstring--\nliteral\nhaving//--//\"\"\"");
}

TEST(LinterTest, AliasKeywordCheck) {
    Test_if(true, CheckAliasKeyword, "SELECT * FROM emp AS X");
    Test_if(false, CheckAliasKeyword, "SELECT * FROM emp X");
    Test_if(true, CheckAliasKeyword, "SELECT 1 AS one");
}

}  // namespace
}  // namespace zetasql::linter
