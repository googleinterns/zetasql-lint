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

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <streambuf>
#include <string>
#include <vector>

#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "re2/re2.h"
#include "src/lint_errors.h"
#include "src/linter.h"
#include "src/linter_options.h"
#include "zetasql/base/status.h"
#include "zetasql/base/status_macros.h"

namespace zetasql::linter {

// This function will parse "NOLINT"(<CheckName>) syntax from
// a single line of comment.
// If NOLINT usage errors occur, it counts as a lint
// error and it will be returned in a result.
LinterResult ParseNoLintSingleComment(absl::string_view line,
                                      const absl::string_view& sql,
                                      LinterOptions* options, int position) {
  LinterResult result;
  std::map<std::string, ErrorCode> error_map = GetErrorMap();

  const LazyRE2 kRegex = {"\\s*(NOLINT|LINT)\\s*\\(([a-z ,-]*)\\)\\s*(.*)\\s*"};
  std::string type = "";
  std::string check_names = "";
  std::string lint_comment = "";
  bool matched =
      RE2::FullMatch(line, *kRegex, &type, &check_names, &lint_comment);

  // Found enabling or disabling type of comment.
  if (matched) {
    check_names.erase(
        std::remove_if(check_names.begin(), check_names.end(), ::isspace),
        check_names.end());

    std::vector<std::string> names = absl::StrSplit(check_names, ',');
    for (std::string check_name : names) {
      // The name inside of parantheses is stored in 'check_name'
      // If it is not valid add error, otherwise enable/disable position
      if (!error_map.count(check_name)) {
        result.Add(ErrorCode::kNoLint, sql, position,
                   absl::StrCat("Unkown NOLINT error category: ", check_name));
      } else {
        const ErrorCode& code = error_map[check_name];
        if (type == "NOLINT")
          options->Disable(code, position);
        else
          options->Enable(code, position);
      }
    }
  }
  return result;
}

// This function will parse "NOLINT"(<CheckName>) syntax from a sql file.
// If NOLINT usage errors occur, it counts as a lint
// error and it will be returned in a result.
LinterResult ParseNoLintComments(absl::string_view sql,
                                 LinterOptions* options) {
  LinterResult result;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == '\'' || sql[i] == '"') {
      // The sql is inside string. This will ignore sql part until
      // the same type(' or ") of character will occur
      // without \ in the beginning. For example 'a"b' is a valid string.
      for (int j = i + 1; j < static_cast<int>(sql.size()); ++j) {
        if (sql[j - 1] == '\\' && sql[j] == sql[i]) continue;
        if (sql[j] == sql[i] || j + 1 == static_cast<int>(sql.size())) {
          i = j;
          break;
        }
      }
      continue;
    }

    // Ignore multiline comments.
    if (i > 0 && sql[i - 1] == '/' && sql[i] == '*') {
      // It will start checking after '/*' and after the iteration
      // finished, the pointer 'i' will be just after '*/' (incrementation
      // from the for statement is included).
      i += 2;
      while (i < static_cast<int>(sql.size()) &&
             !(sql[i - 1] == '*' && sql[i] == '/')) {
        ++i;
      }
    }

    bool is_line_comment = false;
    if (i > 0 && sql[i - 1] == '-' && sql[i] == '-') is_line_comment = true;
    if (i > 0 && sql[i - 1] == '/' && sql[i] == '/') is_line_comment = true;
    if (sql[i] == '#') is_line_comment = true;

    if (is_line_comment) {
      std::string line = "";
      while (i + 1 < static_cast<int>(sql.size()) &&
             sql[i] != options->LineDelimeter())
        line += sql[++i];
      result.Add(ParseNoLintSingleComment(line, sql, options, i));
      continue;
    }
  }

  return result;
}

LinterOptions GetOptionsFromConfig() {
  // TODO(orhanuysal): Connect here with a config file.
  return LinterOptions();
}

// This function is the main function to get all the checks.
// Whenever a new check is added this should be
// the first place to update.
CheckList GetAllChecks() {
  CheckList list;
  list.Add(CheckLineLength);
  list.Add(CheckParserSucceeds);
  list.Add(CheckSemicolon);
  list.Add(CheckUppercaseKeywords);
  list.Add(CheckCommentType);
  list.Add(CheckAliasKeyword);
  list.Add(CheckTabCharactersUniform);
  list.Add(CheckNoTabsBesidesIndentations);
  return list;
}

LinterResult RunChecks(absl::string_view sql, LinterOptions options) {
  CheckList list = GetAllChecks();
  LinterResult result = ParseNoLintComments(sql, &options);

  for (auto check : list.GetList()) {
    result.Add(check(sql, options));
  }
  return result;
}

LinterResult RunChecks(absl::string_view sql) {
  LinterOptions options = GetOptionsFromConfig();
  return RunChecks(sql, options);
}

}  // namespace zetasql::linter
