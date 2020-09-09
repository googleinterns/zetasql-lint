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

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <streambuf>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "re2/re2.h"
#include "src/checks.h"
#include "src/checks_list.h"
#include "src/config.pb.h"
#include "src/lint_error.h"
#include "src/linter_options.h"
#include "zetasql/base/status.h"
#include "zetasql/base/status_macros.h"
#include "zetasql/public/error_helpers.h"

namespace zetasql::linter {

const LazyRE2 kLintCommentRegex = {
    "\\s*(NOLINT|LINT)\\s*\\(([a-z ,\"-]*)\\)\\s*(.*)\\s*"};

LinterResult ParseNoLintSingleComment(absl::string_view line,
                                      const absl::string_view& sql,
                                      int position, LinterOptions* options) {
  LinterResult result;
  std::map<std::string, ErrorCode> error_map = GetErrorMap();

  std::string type = "";
  std::string check_names = "";
  std::string lint_comment = "";
  bool matched = RE2::FullMatch(line, *kLintCommentRegex, &type, &check_names,
                                &lint_comment);

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
        result.Add(
            ErrorCode::kNoLint, sql, position,
            absl::StrCat("Unknown NOLINT error category: '", check_name, "'"));
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

// TODO(orhanuysal): Extract the code so that 'CheckCommentType'
// implementation and this implementation won't have the same parts.
LinterResult ParseNoLintComments(absl::string_view sql,
                                 LinterOptions* options) {
  LinterResult result;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (IgnoreComments(sql, *options, &i, false)) continue;
    if (IgnoreStrings(sql, &i)) continue;

    bool is_line_comment = false;
    if (i > 0 && sql[i - 1] == '-' && sql[i] == '-') is_line_comment = true;
    if (i > 0 && sql[i - 1] == '/' && sql[i] == '/') is_line_comment = true;
    if (sql[i] == '#') is_line_comment = true;

    if (is_line_comment) {
      std::string line = "";
      while (i + 1 < static_cast<int>(sql.size()) &&
             sql[i] != options->LineDelimeter())
        line += sql[++i];
      result.Add(ParseNoLintSingleComment(line, sql, i, options));
      continue;
    }
  }

  return result;
}

LinterResult CheckParserSucceeds(absl::string_view sql,
                                 LinterOptions* options) {
  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  bool is_the_end = false;
  int byte_position = -1;
  LinterResult result;

  while (!is_the_end) {
    std::unique_ptr<ParserOutput> output;
    byte_position = location.byte_position();
    absl::Status status = ParseNextScriptStatement(&location, ParserOptions(),
                                                   &output, &is_the_end);
    if (!status.ok()) {
      if (options->IsActive(ErrorCode::kParseFailed, byte_position)) {
        ErrorLocation position;
        // TODO(nastaran): Check return value and propagate it.
        GetErrorLocation(status, &position);
        result.Add(ErrorCode::kParseFailed, position.line(), position.column(),
                   status.message());
        return result;
      }
    }
    options->AddParserOutput(std::move(output));
  }

  options->SetRememberParser(true);
  return result;
}

void GetOptionsFromConfig(Config config, LinterOptions* options) {
  if (config.has_tab_size()) options->SetTabSize(config.tab_size());

  if (config.has_end_line()) options->SetLineDelimeter(config.end_line()[0]);

  if (config.has_line_limit()) options->SetLineLimit(config.line_limit());

  if (config.has_allowed_indent())
    options->SetAllowedIndent(config.allowed_indent()[0]);

  if (config.has_single_quote()) options->SetSingleQuote(config.single_quote());

  if (config.has_upper_keyword())
    options->SetUpperKeyword(config.upper_keyword());

  std::map<std::string, ErrorCode> error_map = GetErrorMap();

  for (std::string check_name : config.nolint()) {
    if (error_map.count(check_name)) {
      options->DisableCheck(error_map[check_name]);
    }
  }
}

LinterResult RunChecks(absl::string_view sql, LinterOptions* options) {
  ChecksList list = GetAllChecks();
  LinterResult result = ParseNoLintComments(sql, options);
  result.SetFilename(options->Filename());

  // This check should come strictly before others, and able to
  // change options.
  result.Add(CheckParserSucceeds(sql, options));

  for (auto check : list.GetList()) {
    result.Add(check(sql, *options));
  }
  return result;
}

LinterResult RunChecks(absl::string_view sql, Config config,
                       absl::string_view filename) {
  LinterOptions options(filename);
  GetOptionsFromConfig(config, &options);
  return RunChecks(sql, &options);
}

LinterResult RunChecks(absl::string_view sql, absl::string_view filename) {
  LinterOptions options(filename);
  return RunChecks(sql, &options);
}

LinterResult RunChecks(absl::string_view sql) {
  LinterOptions options;
  return RunChecks(sql, &options);
}

}  // namespace zetasql::linter
