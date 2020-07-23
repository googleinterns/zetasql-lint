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

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "absl/strings/match.h"
#include "gtest/gtest.h"
#include "src/linter_options.h"

namespace zetasql::linter {

namespace {

TEST(LinterTest, StatementLineLengthCheck) {
  absl::string_view sql =
      "SELECT e, sum(f) FROM emp where b = a or c < d group by x";
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
  EXPECT_FALSE(CheckSemicolon("SELECT 3+5\nSELECT 4+6;", option).ok());
  EXPECT_FALSE(CheckSemicolon("SELECT 3+5;  \nSELECT 4+6", option).ok());
  EXPECT_FALSE(CheckSemicolon("SELECT 3+5  ;  \nSELECT 4+6", option).ok());
}

TEST(LinterTest, UppercaseKeywordCheck) {
  LinterOptions option;
  EXPECT_TRUE(CheckUppercaseKeywords(
                  "SELECT * FROM emp WHERE b = a OR c < d GROUP BY x", option)
                  .ok());
  EXPECT_TRUE(CheckUppercaseKeywords(
                  "SELECT * FROM emp where b = a or c < d GROUP by x", option)
                  .ok());
  EXPECT_FALSE(CheckUppercaseKeywords(
                   "SeLEct * frOM emp wHEre b = a or c < d GROUP by x", option)
                   .ok());
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
}

TEST(LinterTest, AliasKeywordCheck) {
  LinterOptions option;
  EXPECT_FALSE(CheckAliasKeyword("SELECT 1 a", option).ok());
  EXPECT_TRUE(CheckAliasKeyword("SELECT * FROM emp AS X", option).ok());
  EXPECT_FALSE(CheckAliasKeyword("SELECT * FROM emp X", option).ok());
  EXPECT_TRUE(CheckAliasKeyword("SELECT 1 AS one", option).ok());
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

}  // namespace
}  // namespace zetasql::linter
