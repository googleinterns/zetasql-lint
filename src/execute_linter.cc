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
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <streambuf>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "src/lint_errors.h"
#include "src/linter.h"
#include "src/linter_options.h"
#include "zetasql/base/status.h"
#include "zetasql/base/status_macros.h"

namespace zetasql::linter {

namespace {

// This function will parse "NOLINT"(<CheckName>) syntax and adjust enability
// options for each check. If NOLINT usage errors occur, it counts as a lint
// error and it will be returned in a result.
// This works similar to consistent comment type check.
LinterResult ParseNoLintComments(absl::string_view sql, LinterOptions options,
                                 std::map<ErrorCode, LinterOptions>* nolint) {
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

    absl::string_view type = "";
    if (i > 0 && sql[i - 1] == '-' && sql[i] == '-') type = "--";
    if (i > 0 && sql[i - 1] == '/' && sql[i] == '/') type = "//";
    if (sql[i] == '#') type = "#";

    if (type != "") {
      ++i;
      while (i < static_cast<int>(sql.size()) && sql[i] == ' ') ++i;

      if (sql.substr(i, 6) == "NOLINT" || sql.substr(i, 4) == "LINT") {
        bool disabler = (sql.substr(i, 6) == "NOLINT");
        while (i < static_cast<int>(sql.size()) && sql[i] != '(' &&
               sql[i] != options.LineDelimeter())
          ++i;
        ++i;
        std::string s = "";
        while (i < static_cast<int>(sql.size()) && sql[i - 1] != ')' &&
               sql[i] != options.LineDelimeter()) {
          if (sql[i] == ' ') continue;
          if (sql[i] == ',' || sql[i] == ')') {
            std::map<std::string, ErrorCode> error_map = GetErrorMap();
            if (!error_map.count(s)) {
              result.Add(ErrorCode::kNoLint, sql, i,
                         absl::StrCat("Unkown NOLINT error category: ", s));
            } else {
              const ErrorCode& code = error_map[s];
              if (!nolint->count(code)) (*nolint)[code] = options;

              (*nolint)[code].IsActive(i + 1);
              if (disabler)
                (*nolint)[code].Disable(i);
              else
                (*nolint)[code].Enable(i);
              (*nolint)[code].IsActive(i + 1);
            }
            s = "";
          } else {
            s += sql[i];
          }
          ++i;
        }
      }
      while (i < static_cast<int>(sql.size()) &&
             sql[i] != options.LineDelimeter())
        ++i;
      continue;
    }
  }

  return result;
}

LinterOptions GetOptionsFromConfig() {
  // TODO(orhanuysal): Connect here with a config file.
  return LinterOptions();
}

// TODO(orhanuysal): Move these classes to proper .h file
// when a good file name is chosen.

// It is an helper class for CheckList
class CheckListObject {
 public:
  CheckListObject(
      std::function<LinterResult(absl::string_view, LinterOptions)> check,
      ErrorCode code)
      : check_(check), code_(code) {}
  std::function<LinterResult(absl::string_view, LinterOptions)> GetCheck() {
    return check_;
  }
  ErrorCode GetCode() { return code_; }

 private:
  std::function<LinterResult(absl::string_view, LinterOptions)> check_;
  ErrorCode code_;
};

// It is the general list of the linter checks. It can be used to
// verify if a place is controlling all of the checks and not missing any.
class CheckList {
 public:
  std::vector<CheckListObject> GetList() { return list_; }
  void Add(std::function<LinterResult(absl::string_view, LinterOptions)> check,
           ErrorCode code) {
    list_.push_back(CheckListObject(check, code));
  }

 private:
  std::vector<CheckListObject> list_;
};

// This function is the main function to get all the checks.
// Whenever a new check is added this should be
// the first place to update.
CheckList GetAllChecks() {
  CheckList list;
  list.Add(CheckLineLength, ErrorCode::kLineLimit);
  list.Add(CheckParserSucceeds, ErrorCode::kParseFailed);
  list.Add(CheckSemicolon, ErrorCode::kSemicolon);
  list.Add(CheckUppercaseKeywords, ErrorCode::kLetterCase);
  list.Add(CheckCommentType, ErrorCode::kCommentStyle);
  list.Add(CheckAliasKeyword, ErrorCode::kAlias);
  list.Add(CheckTabCharactersUniform, ErrorCode::kUniformIndent);
  list.Add(CheckNoTabsBesidesIndentations, ErrorCode::kNotIndentTab);
  return list;
}

// It runs all specified checks
// "LinterOptions" parameter will be added in future.
LinterResult RunChecks(absl::string_view sql) {
  // Variable 'config_options' disregards in-file configuration changes,
  // it is only to construct a basis for option_map.
  LinterOptions config_options = GetOptionsFromConfig();
  CheckList list = GetAllChecks();
  std::map<ErrorCode, LinterOptions> option_map;
  LinterResult result = ParseNoLintComments(sql, config_options, &option_map);

  for (CheckListObject object : list.GetList()) {
    std::function<LinterResult(absl::string_view, LinterOptions)> check =
        object.GetCheck();
    ErrorCode code = object.GetCode();
    if (!option_map.count(code)) option_map[code] = config_options;
    result.Add(check(sql, option_map[code]));
  }
  return result;
}

}  // namespace
}  // namespace zetasql::linter

int main(int argc, char* argv[]) {
  if (argc < 1) {
    std::cout << "Usage: execute_linter < <file_name>\n" << std::endl;
    return 1;
  }

  std::string str;
  for (std::string line; std::getline(std::cin, line);) {
    str += line + "\n";
  }
  zetasql::linter::LinterResult result =
      zetasql::linter::RunChecks(absl::string_view(str));

  result.PrintResult();

  return 0;
}
