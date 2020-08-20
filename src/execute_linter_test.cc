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

#include "src/execute_linter.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/match.h"
#include "gtest/gtest.h"
#include "src/lint_errors.h"
#include "src/linter_options.h"

namespace zetasql::linter {

namespace {

TEST(CheckListTest, SizeMatch) {
  std::map<std::string, ErrorCode> error_map = GetErrorMap();
  EXPECT_TRUE(error_map.size() == static_cast<int>(ErrorCode::COUNT));
}

TEST(NolintTest, MultipleDisabling) {
  LinterOptions option;
  absl::string_view sql =
      "Select 3 a;\n-- NOLINT ( alias, consistent-letter-case)\n"  // Both fail.
      "SELEcT 3 a;\n--LINT(alias)\n"                               // Both ok.
      " Select 3 a;\n--NOLINT  (  alias)\n"  // 'alias' fail.
      "--   LINT (consistent-letter-case)\n"
      " Select 3 a;";  // 'letter-case' fail.
  LinterResult result = RunChecks(sql);
  result.Sort();
  std::vector<LintError> errors = result.GetErrors();

  EXPECT_EQ(errors.size(), 4);

  EXPECT_TRUE(errors[0].GetType() == ErrorCode::kLetterCase);
  EXPECT_TRUE(errors[1].GetType() == ErrorCode::kAlias);
  EXPECT_TRUE(errors[2].GetType() == ErrorCode::kAlias);
  EXPECT_TRUE(errors[3].GetType() == ErrorCode::kLetterCase);

  EXPECT_TRUE(errors[0].GetPosition() == std::make_pair(1, 1));
  EXPECT_TRUE(errors[1].GetPosition() == std::make_pair(1, 10));
  EXPECT_TRUE(errors[2].GetPosition() == std::make_pair(5, 11));
  EXPECT_TRUE(errors[3].GetPosition() == std::make_pair(8, 2));
}

}  // namespace
}  // namespace zetasql::linter
