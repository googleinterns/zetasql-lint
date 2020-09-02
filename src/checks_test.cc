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

#include "src/checks.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/strings/match.h"
#include "gtest/gtest.h"
#include "src/checks_list.h"
#include "src/linter_options.h"

namespace zetasql::linter {

namespace {

TEST(LinterTest, StatementLineLengthCheck) {
  absl::string_view sql =
      "SELECT e, sum(f) FROM emp where b = a or c < d group by x\n";
  absl::string_view multiline_sql =
      "SELECT c\n"
      "some long invalid sql statement that shouldn't stop check\n"
      "SELECT t from G\n";
  LinterOptions option;
  EXPECT_TRUE(CheckLineLength(sql, option).ok());
  option.SetLineLimit(10);
  EXPECT_FALSE(CheckLineLength(sql, option).ok());
  option.SetLineDelimeter(' ');
  EXPECT_TRUE(CheckLineLength(sql, option).ok());

  option.SetLineLimit(100);
  option.SetLineDelimeter('\n');
  EXPECT_TRUE(CheckLineLength(multiline_sql, option).ok());

  option.SetLineLimit(30);
  EXPECT_FALSE(CheckLineLength(multiline_sql, option).ok());
}

TEST(LinterTest, StatementValidityCheck) {
  LinterOptions option;
  EXPECT_TRUE(CheckParserSucceeds("SELECT 5+2", option).ok());
  EXPECT_FALSE(CheckParserSucceeds("SELECT 5+2 sss ddd", option).ok());

  EXPECT_TRUE(CheckParserSucceeds(
                  "SELECT * FROM emp where b = a or c < d group by x", option)
                  .ok());

  EXPECT_TRUE(
      CheckParserSucceeds(
          "SELECT e, sum(f) FROM emp where b = a or c < d group by x", option)
          .ok());

  EXPECT_FALSE(
      CheckParserSucceeds("SELET A FROM B\nSELECT C FROM D", option).ok());

  EXPECT_FALSE(CheckParserSucceeds("SELECT 1; SELECT 2 3 4;", option).ok());
}

TEST(LinterTest, SemicolonCheck) {
  LinterOptions option;
  EXPECT_TRUE(CheckSemicolon("SELECT 3+5;\nSELECT 4+6;", option).ok());
  EXPECT_TRUE(CheckSemicolon("SELECT 3+5;   \n   SELECT 4+6;", option).ok());

  EXPECT_FALSE(CheckSemicolon("SELECT 3+5;  \nSELECT 4+6", option).ok());
  EXPECT_FALSE(CheckSemicolon("SELECT 3+5  ;  \nSELECT 4+6", option).ok());
}

TEST(LinterTest, UppercaseKeywordCheck) {
  LinterOptions option;
  EXPECT_TRUE(CheckUppercaseKeywords(
                  "SELECT * FROM emp WHERE b = a OR c < d GROUP BY x", option)
                  .ok());
  option.SetUpperKeyword(false);
  EXPECT_TRUE(CheckUppercaseKeywords(
                  "select * from emp where b = a or c < d group by x", option)
                  .ok());

  option.SetUpperKeyword(true);
  LinterResult result = CheckUppercaseKeywords(
      "SeLEct * frOM emp wHEre b = a OR c < d GROUP BY x", option);
  EXPECT_FALSE(result.ok());
  auto errors = result.GetErrors();

  EXPECT_EQ(errors.size(), 3);
  EXPECT_EQ(errors[0].GetPosition(), std::make_pair(1, 1));
  EXPECT_EQ(errors[1].GetPosition(), std::make_pair(1, 10));
  EXPECT_EQ(errors[2].GetPosition(), std::make_pair(1, 19));
}

TEST(LinterTest, CommentTypeCheck) {
  LinterOptions option;
  EXPECT_FALSE(
      CheckCommentType("# Comment 1\n-- Comment 2\nSELECT 3+5\n", option).ok());
  EXPECT_TRUE(
      CheckCommentType("-- Comment /* unfinished comment", option).ok());

  EXPECT_TRUE(
      CheckCommentType("//comment 1\nSELECT 3+5\n//comment 2", option).ok());
  EXPECT_FALSE(
      CheckCommentType("//comment 1\nSELECT 3+5\n--comment 2", option).ok());
  EXPECT_TRUE(
      CheckCommentType("--comment 1\nSELECT 3+5\n--comment 2", option).ok());

  // Check a sql containing a multiline comment.
  EXPECT_TRUE(
      CheckCommentType("/* here is // and -- */SELECT 1+2 -- comment 2", option)
          .ok());

  // Check multiline string literal
  EXPECT_TRUE(
      CheckCommentType(
          "SELECT \"\"\"multiline\nstring--\nliteral\nhaving//--//\"\"\"",
          option)
          .ok());
  LinterResult result = CheckCommentType(
      "# comment 1\nSELECT 3+5\n--comment 2//fake 3\n--comment 4", option);
  auto errors = result.GetErrors();

  EXPECT_EQ(errors.size(), 2);
  EXPECT_EQ(errors[0].GetPosition(), std::make_pair(3, 2));
  EXPECT_EQ(errors[1].GetPosition(), std::make_pair(4, 2));
}

TEST(LinterTest, AliasKeywordCheck) {
  LinterOptions option;
  EXPECT_FALSE(CheckAliasKeyword("SELECT 1 a", option).ok());
  EXPECT_TRUE(CheckAliasKeyword("SELECT * FROM emp AS X", option).ok());
  EXPECT_FALSE(CheckAliasKeyword("SELECT * FROM emp X", option).ok());
  EXPECT_TRUE(CheckAliasKeyword("SELECT 1 AS one", option).ok());

  LinterResult result = CheckAliasKeyword("SELECT A B FROM C D", option);
  auto errors = result.GetErrors();
  EXPECT_EQ(errors.size(), 2);
  EXPECT_EQ(errors[0].GetPosition(), std::make_pair(1, 10));
  EXPECT_EQ(errors[1].GetPosition(), std::make_pair(1, 19));
}

TEST(LinterTest, TabCharactersUniformCheck) {
  LinterOptions option_space;
  EXPECT_TRUE(
      CheckTabCharactersUniform("  SELECT 5;\n    SELECT 6;", option_space)
          .ok());
  LinterOptions option_tab;
  option_tab.SetAllowedIndent('\t');
  EXPECT_TRUE(
      CheckTabCharactersUniform("\tSELECT 5;\n\t\tSELECT 6;", option_tab).ok());
  EXPECT_TRUE(
      CheckTabCharactersUniform("SELECT 5;\n SELECT\t6;\t", option_space).ok());

  EXPECT_TRUE(
      CheckTabCharactersUniform("SELECT 5;\n \t SELECT 6;", option_space)
          .GetErrors()
          .back()
          .GetPosition() == std::make_pair(2, 2));
  EXPECT_TRUE(
      CheckTabCharactersUniform("  SELECT kek;\n\tSELECT lol;", option_space)
          .GetErrors()
          .back()
          .GetPosition() == std::make_pair(2, 1));
  EXPECT_TRUE(CheckTabCharactersUniform("SELECT 5;\n  SELECT 6;", option_tab)
                  .GetErrors()
                  .back()
                  .GetPosition() == std::make_pair(2, 1));
}

TEST(LinterTest, NoTabsBesidesIndentationsCheck) {
  LinterOptions option;
  EXPECT_TRUE(
      CheckNoTabsBesidesIndentations("\tSELECT 5;\n\tSELECT 6;", option).ok());
  EXPECT_TRUE(
      CheckNoTabsBesidesIndentations("\tSELECT   5;\n\t\tSELECT   6;", option)
          .ok());

  EXPECT_TRUE(
      CheckNoTabsBesidesIndentations("\tSELECT \t5;\n\t\tSELECT   6;", option)
          .GetErrors()
          .back()
          .GetPosition() == std::make_pair(1, 16));
  EXPECT_TRUE(CheckNoTabsBesidesIndentations("\tSELECT 5;\nS\tELECT 6;", option)
                  .GetErrors()
                  .back()
                  .GetPosition() == std::make_pair(2, 2));
}

TEST(LinterTest, CheckSingleQuotes) {
  LinterOptions option;
  EXPECT_TRUE(CheckSingleQuotes("SELECT 'a' \n SELECT 'b'", option).ok());
  EXPECT_FALSE(CheckSingleQuotes("SELECT \"a\" \n SELECT \"b\"", option).ok());

  EXPECT_TRUE(
      CheckSingleQuotes("SELECT 'string with \" (ok)' from b", option).ok());
  option.SetSingleQuote(false);
  EXPECT_FALSE(CheckSingleQuotes("SELECT 'a' \n SELECT 'b'", option).ok());
  EXPECT_TRUE(CheckSingleQuotes("SELECT \"a\" \n SELECT \"b\"", option).ok());
  LinterResult result = CheckSingleQuotes("SELECT 'a' \n SELECT 'b'", option);
  result.Sort();
  std::vector<LintError> errors = result.GetErrors();

  EXPECT_EQ(errors.size(), 2);
  EXPECT_EQ(errors[0].GetPosition(), std::make_pair(1, 8));
  EXPECT_EQ(errors[1].GetPosition(), std::make_pair(2, 9));
}

TEST(LinterTest, CheckTableNames) {
  LinterOptions option;
  EXPECT_FALSE(CheckNames("CREATE TABLE TABLE_NAME;", option).ok());
  EXPECT_TRUE(CheckNames("CREATE TABLE TableName;", option).ok());
  EXPECT_FALSE(CheckNames("CREATE TABLE table_name;", option).ok());
}

TEST(LinterTest, CheckWindowNames) {
  LinterOptions option;
  EXPECT_TRUE(
      CheckNames("SELECT a FROM B WINDOW WindowName AS T;", option).ok());
  EXPECT_FALSE(
      CheckNames("SELECT a FROM B WINDOW window_name AS T;", option).ok());
}

TEST(LinterTest, CheckFunctionNames) {
  LinterOptions option;
  EXPECT_TRUE(
      CheckNames("CREATE TEMP TABLE FUNCTION BugScore();", option).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP TABLE FUNCTION bugScore();", option).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP TABLE FUNCTION bug_score();", option).ok());
  EXPECT_TRUE(CheckNames("CREATE TEMP FUNCTION BugScore();", option).ok());
}

TEST(LinterTest, CheckSimpleTypeNames) {
  LinterOptions option;
  EXPECT_FALSE(CheckNames("CREATE TEMP FUNCTION A( s String );", option).ok());
  EXPECT_TRUE(CheckNames("CREATE TEMP FUNCTION A( s STRING );", option).ok());
  EXPECT_FALSE(CheckNames("CREATE TEMP FUNCTION A( s int64 );", option).ok());
  EXPECT_TRUE(CheckNames("CREATE TEMP FUNCTION A( s INT64 );", option).ok());
}

TEST(LinterTest, CheckColumnNames) {
  LinterOptions option;
  EXPECT_TRUE(CheckNames("SELECT column_name;", option).ok());
  EXPECT_TRUE(CheckNames("SELECT TableName.column_name;", option).ok());
  EXPECT_FALSE(CheckNames("SELECT columnName;", option).ok());
  EXPECT_FALSE(CheckNames("SELECT COLUMN_NAME;", option).ok());
}

TEST(LinterTest, CheckFunctionParameterNames) {
  LinterOptions option;
  EXPECT_TRUE(
      CheckNames("CREATE TEMP FUNCTION A(string_param STRING);", option).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP FUNCTION A(STRING_PARAM STRING);", option).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP FUNCTION A( stringParam STRING );", option).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP FUNCTION A( StringParam STRING );", option).ok());
}

TEST(LinterTest, CheckConstantNames) {
  LinterOptions option;
  EXPECT_FALSE(CheckNames("CREATE PUBLIC CONSTANT TwoPi = 6.28;", option).ok());
  EXPECT_TRUE(CheckNames("CREATE PUBLIC CONSTANT TWO_PI = 6.28;", option).ok());
}

TEST(LinterTest, ParserDependentChecks) {
  LinterOptions option;
  for (auto check : GetParserDependantChecks().GetList()) {
    // If there is no semicolon in between Parser fails and this check shouldn't
    // give extra errors aside from Parser Check.
    EXPECT_TRUE(check("SELECT 3+5\nSELECT 4+6;", option).GetErrors().empty());
    EXPECT_TRUE(check("SELECT 3+5\nSELECT 4+6;", option).GetStatus().empty());
  }
}

TEST(LinterTest, CheckJoin) {
  LinterOptions option;
  LinterResult result =
      CheckJoin("SELECT a FROM t JOIN x JOIN y JOIN z", option);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.GetErrors().size(), 3);

  result = CheckJoin("SELECT a FROM t INNER JOIN x LEFT JOIN y RIGHT JOIN z",
                     option);
  EXPECT_TRUE(result.ok());

  result = CheckJoin("SELECT a FROM t LEFT OUTER JOIN x", option);
  EXPECT_TRUE(result.ok());
}

TEST(LinterTest, CheckImports) {
  LinterOptions option;

  EXPECT_TRUE(CheckImports("IMPORT PROTO 'random';\n", option).ok());
  EXPECT_TRUE(CheckImports("IMPORT MODULE random;\n", option).ok());

  absl::string_view sql =
      "IMPORT PROTO 'random';\n"
      "IMPORT MODULE random;\n"
      "IMPORT PROTO 'CONFIG';\n";
  LinterResult result = CheckImports(sql, option);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.GetErrors().size(), 1);
  EXPECT_EQ(result.GetErrors()[0].GetErrorMessage(),
            "PROTO and MODULE inputs should be in seperate groups.");

  result = CheckImports("IMPORT random", option);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.GetErrors().size(), 1);
  EXPECT_EQ(result.GetErrors()[0].GetErrorMessage(),
            "Imports should specify the type 'MODULE' or 'PROTO'.");
}

}  // namespace
}  // namespace zetasql::linter
