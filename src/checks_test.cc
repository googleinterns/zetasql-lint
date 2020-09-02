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
#include "src/checks_util.h"
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
  LinterOptions options;
  EXPECT_TRUE(CheckLineLength(sql, options).ok());
  options.SetLineLimit(10);
  EXPECT_FALSE(CheckLineLength(sql, options).ok());
  options.SetLineDelimeter(' ');
  EXPECT_TRUE(CheckLineLength(sql, options).ok());

  options.SetLineLimit(100);
  options.SetLineDelimeter('\n');
  EXPECT_TRUE(CheckLineLength(multiline_sql, options).ok());

  options.SetLineLimit(30);
  EXPECT_FALSE(CheckLineLength(multiline_sql, options).ok());
}

TEST(LinterTest, SemicolonCheck) {
  LinterOptions options;
  EXPECT_TRUE(CheckSemicolon("SELECT 3+5;\nSELECT 4+6;", options).ok());
  EXPECT_TRUE(CheckSemicolon("\nSELECT 4+6\n\n; -- comment", options).ok());
  EXPECT_TRUE(CheckSemicolon("SELECT 3+5 /*comment*/ ; \n", options).ok());
  EXPECT_TRUE(CheckSemicolon("SELECT 3+5 ; /*comment*/ \n", options).ok());
  EXPECT_FALSE(CheckSemicolon("SELECT 3+5 -- comment; \n", options).ok());

  EXPECT_FALSE(CheckSemicolon("SELECT 3+5;  \nSELECT 4+6", options).ok());
  EXPECT_FALSE(CheckSemicolon("SELECT 3+5  ;  \nSELECT 4+6", options).ok());
}

TEST(LinterTest, UppercaseKeywordCheck) {
  LinterOptions options;
  EXPECT_TRUE(CheckUppercaseKeywords(
                  "SELECT * FROM emp WHERE b = a OR c < d GROUP BY x", options)
                  .ok());
  options.SetUpperKeyword(false);
  EXPECT_TRUE(CheckUppercaseKeywords(
                  "select * from emp where b = a or c < d group by x", options)
                  .ok());

  options.SetUpperKeyword(true);
  LinterResult result = CheckUppercaseKeywords(
      "SeLEct * frOM emp wHEre b = a OR c < d GROUP BY x", options);
  EXPECT_FALSE(result.ok());
  auto errors = result.GetErrors();

  EXPECT_EQ(errors.size(), 3);
  EXPECT_EQ(errors[0].GetPosition(), std::make_pair(1, 1));
  EXPECT_EQ(errors[1].GetPosition(), std::make_pair(1, 10));
  EXPECT_EQ(errors[2].GetPosition(), std::make_pair(1, 19));
}

TEST(LinterTest, CommentTypeCheck) {
  LinterOptions options;
  EXPECT_FALSE(
      CheckCommentType("# Comment 1\n-- Comment 2\nSELECT 3+5\n", options)
          .ok());
  EXPECT_TRUE(
      CheckCommentType("-- Comment /* unfinished comment", options).ok());

  EXPECT_TRUE(
      CheckCommentType("//comment 1\nSELECT 3+5\n//comment 2", options).ok());
  EXPECT_FALSE(
      CheckCommentType("//comment 1\nSELECT 3+5\n--comment 2", options).ok());
  EXPECT_TRUE(
      CheckCommentType("--comment 1\nSELECT 3+5\n--comment 2", options).ok());

  // Check a sql containing a multiline comment.
  EXPECT_TRUE(CheckCommentType("/* here is // and -- */SELECT 1+2 -- comment 2",
                               options)
                  .ok());

  // Check multiline string literal
  EXPECT_TRUE(
      CheckCommentType(
          "SELECT \"\"\"multiline\nstring--\nliteral\nhaving//--//\"\"\"",
          options)
          .ok());
  LinterResult result = CheckCommentType(
      "# comment 1\nSELECT 3+5\n--comment 2//fake 3\n--comment 4", options);
  auto errors = result.GetErrors();

  EXPECT_EQ(errors.size(), 2);
  EXPECT_EQ(errors[0].GetPosition(), std::make_pair(3, 2));
  EXPECT_EQ(errors[1].GetPosition(), std::make_pair(4, 2));
}

TEST(LinterTest, AliasKeywordCheck) {
  LinterOptions options;
  EXPECT_FALSE(CheckAliasKeyword("SELECT 1 a", options).ok());
  EXPECT_TRUE(CheckAliasKeyword("SELECT * FROM emp AS X", options).ok());
  EXPECT_FALSE(CheckAliasKeyword("SELECT * FROM emp X", options).ok());
  EXPECT_TRUE(CheckAliasKeyword("SELECT 1 AS one", options).ok());

  LinterResult result = CheckAliasKeyword("SELECT A B FROM C D", options);
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
  LinterOptions options;
  EXPECT_TRUE(
      CheckNoTabsBesidesIndentations("\tSELECT 5;\n\tSELECT 6;", options).ok());
  EXPECT_TRUE(
      CheckNoTabsBesidesIndentations("\tSELECT   5;\n\t\tSELECT   6;", options)
          .ok());

  EXPECT_TRUE(
      CheckNoTabsBesidesIndentations("\tSELECT \t5;\n\t\tSELECT   6;", options)
          .GetErrors()
          .back()
          .GetPosition() == std::make_pair(1, 16));
  EXPECT_TRUE(
      CheckNoTabsBesidesIndentations("\tSELECT 5;\nS\tELECT 6;", options)
          .GetErrors()
          .back()
          .GetPosition() == std::make_pair(2, 2));
}

TEST(LinterTest, CheckSingleQuotes) {
  LinterOptions options;
  EXPECT_TRUE(CheckSingleQuotes("SELECT 'a' \n SELECT 'b'", options).ok());
  EXPECT_FALSE(CheckSingleQuotes("SELECT \"a\" \n SELECT \"b\"", options).ok());

  EXPECT_TRUE(
      CheckSingleQuotes("SELECT 'string with \" (ok)' from b", options).ok());
  options.SetSingleQuote(false);
  EXPECT_FALSE(CheckSingleQuotes("SELECT 'a' \n SELECT 'b'", options).ok());
  EXPECT_TRUE(CheckSingleQuotes("SELECT \"a\" \n SELECT \"b\"", options).ok());
  LinterResult result = CheckSingleQuotes("SELECT 'a' \n SELECT 'b'", options);
  result.Sort();
  std::vector<LintError> errors = result.GetErrors();

  EXPECT_EQ(errors.size(), 2);
  EXPECT_EQ(errors[0].GetPosition(), std::make_pair(1, 8));
  EXPECT_EQ(errors[1].GetPosition(), std::make_pair(2, 9));
}

TEST(LinterTest, CheckTableNames) {
  LinterOptions options;
  EXPECT_FALSE(CheckNames("CREATE TABLE TABLE_NAME;", options).ok());
  EXPECT_TRUE(CheckNames("CREATE TABLE TableName;", options).ok());
  EXPECT_FALSE(CheckNames("CREATE TABLE table_name;", options).ok());
}

TEST(LinterTest, CheckWindowNames) {
  LinterOptions options;
  EXPECT_TRUE(
      CheckNames("SELECT a FROM B WINDOW WindowName AS T;", options).ok());
  EXPECT_FALSE(
      CheckNames("SELECT a FROM B WINDOW window_name AS T;", options).ok());
}

TEST(LinterTest, CheckFunctionNames) {
  LinterOptions options;
  EXPECT_TRUE(
      CheckNames("CREATE TEMP TABLE FUNCTION BugScore();", options).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP TABLE FUNCTION bugScore();", options).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP TABLE FUNCTION bug_score();", options).ok());
  EXPECT_TRUE(CheckNames("CREATE TEMP FUNCTION BugScore();", options).ok());
}

TEST(LinterTest, CheckSimpleTypeNames) {
  LinterOptions options;
  EXPECT_FALSE(CheckNames("CREATE TEMP FUNCTION A( s String );", options).ok());
  EXPECT_TRUE(CheckNames("CREATE TEMP FUNCTION A( s STRING );", options).ok());
  EXPECT_FALSE(CheckNames("CREATE TEMP FUNCTION A( s int64 );", options).ok());
  EXPECT_TRUE(CheckNames("CREATE TEMP FUNCTION A( s INT64 );", options).ok());
}

TEST(LinterTest, CheckColumnNames) {
  LinterOptions options;
  EXPECT_TRUE(CheckNames("SELECT a AS column_name;", options).ok());
  EXPECT_TRUE(CheckNames("SELECT a AS TableName.column_name;", options).ok());
  EXPECT_FALSE(CheckNames("SELECT a AS columnName;", options).ok());
  EXPECT_FALSE(CheckNames("SELECT a AS COLUMN_NAME;", options).ok());
}

TEST(LinterTest, CheckFunctionParameterNames) {
  LinterOptions options;
  EXPECT_TRUE(
      CheckNames("CREATE TEMP FUNCTION A(string_param STRING);", options).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP FUNCTION A(STRING_PARAM STRING);", options).ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP FUNCTION A( stringParam STRING );", options)
          .ok());
  EXPECT_FALSE(
      CheckNames("CREATE TEMP FUNCTION A( StringParam STRING );", options)
          .ok());
}

TEST(LinterTest, CheckConstantNames) {
  LinterOptions options;
  EXPECT_FALSE(
      CheckNames("CREATE PUBLIC CONSTANT TwoPi = 6.28;", options).ok());
  EXPECT_TRUE(
      CheckNames("CREATE PUBLIC CONSTANT TWO_PI = 6.28;", options).ok());
}

TEST(LinterTest, ParserDependentChecks) {
  LinterOptions options;
  for (auto check : GetParserDependantChecks().GetList()) {
    // If there is no semicolon in between Parser fails and this check shouldn't
    // give extra errors aside from Parser Check.
    EXPECT_TRUE(check("SELECT 3+5\nSELECT 4+6;", options).GetErrors().empty());
    EXPECT_TRUE(check("SELECT 3+5\nSELECT 4+6;", options).GetStatus().empty());
  }
}

TEST(LinterTest, CheckJoin) {
  LinterOptions options;
  LinterResult result =
      CheckJoin("SELECT a FROM t JOIN x JOIN y JOIN z", options);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.GetErrors().size(), 3);

  result = CheckJoin("SELECT a FROM t INNER JOIN x LEFT JOIN y RIGHT JOIN z",
                     options);
  EXPECT_TRUE(result.ok());

  result = CheckJoin("SELECT a FROM t LEFT OUTER JOIN x", options);
  EXPECT_TRUE(result.ok());
}

TEST(LinterTest, CheckImports) {
  LinterOptions options;

  EXPECT_TRUE(CheckImports("IMPORT PROTO 'random';\n", options).ok());
  EXPECT_TRUE(CheckImports("IMPORT MODULE random;\n", options).ok());

  absl::string_view sql =
      "IMPORT PROTO 'random';\n"
      "IMPORT MODULE random;\n"
      "IMPORT PROTO 'CONFIG';\n";
  LinterResult result = CheckImports(sql, options);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.GetErrors().size(), 1);
  EXPECT_EQ(result.GetErrors()[0].GetErrorMessage(),
            "PROTO and MODULE inputs should be in seperate groups.");

  result = CheckImports("IMPORT random", options);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.GetErrors().size(), 1);
  EXPECT_EQ(result.GetErrors()[0].GetErrorMessage(),
            "Imports should specify the type 'MODULE' or 'PROTO'.");
}

TEST(LinterTest, CheckExpressionParantheses) {
  LinterOptions options;

  EXPECT_EQ(CheckExpressionParantheses(
                "SELECT D where a AND b*2 OR (c AND x OR T);", options)
                .GetErrors()
                .size(),
            2);

  EXPECT_TRUE(
      CheckExpressionParantheses("SELECT D where a or b or c;", options).ok());

  EXPECT_TRUE(
      CheckExpressionParantheses("SELECT D where a and b and c;", options)
          .ok());
  EXPECT_FALSE(
      CheckExpressionParantheses("SELECT D where a and b or c;", options).ok());
  EXPECT_FALSE(
      CheckExpressionParantheses("SELECT D where a or b and c;", options).ok());
  EXPECT_TRUE(CheckExpressionParantheses(
                  "SELECT D where (       a or b   ) and c;", options)
                  .ok());
}

TEST(LinterTest, CheckCountStar) {
  LinterOptions options;
  EXPECT_TRUE(CheckCountStar("SELECT COUNT", options).ok());
  EXPECT_TRUE(CheckCountStar("SELECT COUNT(0)", options).ok());
  EXPECT_FALSE(CheckCountStar("SELECT COUNT ( 1  )", options).ok());
  EXPECT_FALSE(CheckCountStar("SELECT count(1)", options).ok());
  EXPECT_TRUE(CheckCountStar("SELECT COUNT(*)", options).ok());
  EXPECT_EQ(CheckCountStar("SELECT count ( 1 );\n"
                           "/* count(1) */ SELECT COUNT(*) -- count 1",
                           options)
                .GetErrors()
                .size(),
            1);
  EXPECT_EQ(CheckCountStar("SELECT count ( 1 );\n"
                           "/* count(1) */ SELECT COUNT(*) -- count 1",
                           options)
                .GetErrors()
                .back()
                .GetLineNumber(),
            1);

  EXPECT_TRUE(CheckCountStar("SELECT \"COUNT(1)\"", options).ok());
}

TEST(LinterTest, CheckKeywordNamedIdentifier) {
  LinterOptions options;
  LinterResult result = CheckKeywordNamedIdentifier("SELECT Date", options);
  EXPECT_FALSE(CheckKeywordNamedIdentifier("SELECT Date", options).ok());
  EXPECT_TRUE(CheckKeywordNamedIdentifier("SELECT `Date`", options).ok());
  EXPECT_FALSE(CheckKeywordNamedIdentifier("SELECT Type", options).ok());
  EXPECT_TRUE(CheckKeywordNamedIdentifier("SELECT `Type`", options).ok());
  EXPECT_FALSE(
      CheckKeywordNamedIdentifier("SELECT Table.column1", options).ok());
  EXPECT_TRUE(
      CheckKeywordNamedIdentifier("SELECT `Table`.column1", options).ok());
}

TEST(LinterTest, CheckSpecifyTable) { LinterOptions options; }

}  // namespace
}  // namespace zetasql::linter
