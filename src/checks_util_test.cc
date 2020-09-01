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

#include "src/checks_util.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/match.h"
#include "gtest/gtest.h"
#include "src/lint_error.h"
#include "src/linter_options.h"

namespace zetasql::linter {

namespace {

TEST(CheckUtilTest, LetterCaseFunctionsCheck) {
  EXPECT_EQ(ConvertToUppercase("mOdule"), "MODULE");
  EXPECT_EQ(ConvertToUppercase("Here19_m"), "HERE19_M");

  EXPECT_TRUE(IsUpperCamelCase("LongName"));
  EXPECT_FALSE(IsUpperCamelCase("LONG_NAME"));
  EXPECT_TRUE(IsUpperCamelCase("LONGNAME"));

  EXPECT_FALSE(IsAllCaps("LongName"));
  EXPECT_TRUE(IsAllCaps("LONG_NAME"));
  EXPECT_TRUE(IsAllCaps("LONGNAME"));

  EXPECT_FALSE(IsCapsSnakeCase("LongName"));
  EXPECT_TRUE(IsCapsSnakeCase("LONG_NAME"));
  EXPECT_TRUE(IsCapsSnakeCase("LONGNAME"));

  EXPECT_FALSE(IsLowerSnakeCase("LongName"));
  EXPECT_TRUE(IsLowerSnakeCase("long_name"));
  EXPECT_FALSE(IsLowerSnakeCase("LONG_NAME"));
}

TEST(CheckUtilTest, IgnoreCommentsCheck) {
  LinterOptions options;
  int position = 0;
  absl::string_view str = "A /*comment*/\nsecond line";
  EXPECT_FALSE(IgnoreComments(str, options, &position));
  position = 3;
  EXPECT_TRUE(IgnoreComments(str, options, &position));
  EXPECT_EQ(position, 12);
  EXPECT_EQ(str[position + 1], '\n');

  position = 0;
  str = "A --comment--\nsecond line";
  EXPECT_FALSE(IgnoreComments(str, options, &position));
  position = 3;
  EXPECT_FALSE(IgnoreComments(str, options, &position, false));
  EXPECT_TRUE(IgnoreComments(str, options, &position));
  EXPECT_EQ(position, 13);
  EXPECT_EQ(str[position + 1], 's');
}

TEST(CheckUtilTest, IgnoreStringsCheck) {
  int position = 0;
  absl::string_view str = "A \"st'r'ing\"\nsecond line";
  EXPECT_FALSE(IgnoreStrings(str, &position));
  position = 2;
  EXPECT_TRUE(IgnoreStrings(str, &position));
  EXPECT_EQ(position, 11);
  EXPECT_EQ(str[position + 1], '\n');

  position = 0;
  str = "A 'st\"r\"ing'\nsecond line";
  EXPECT_FALSE(IgnoreStrings(str, &position));
  position = 2;
  EXPECT_TRUE(IgnoreStrings(str, &position));
  EXPECT_EQ(position, 11);
  EXPECT_EQ(str[position + 1], '\n');
}

TEST(CheckUtilTest, OneLineStatementCheck) {
  EXPECT_TRUE(OneLineStatement("IMPORT MODULE asd;"));
  EXPECT_TRUE(OneLineStatement("IMPORT PROTO asd;"));

  EXPECT_FALSE(OneLineStatement("CREATE PUBLIC CONSTANT TwoPi = 6.28;"));
  EXPECT_TRUE(OneLineStatement("CREATE PUBLIC CONSTANT TwoPi = "));

  EXPECT_FALSE(
      OneLineStatement("CREATE TEMPORARY FUNCTION A( string_param STRING ); "));
  EXPECT_TRUE(OneLineStatement("CREATE TEMPORARY FUNCTION A("));
}

}  // namespace
}  // namespace zetasql::linter
