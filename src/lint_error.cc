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
#include "src/lint_error.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "zetasql/base/status.h"
#include "zetasql/base/status_macros.h"
#include "zetasql/base/statusor.h"
#include "zetasql/public/error_helpers.h"
#include "zetasql/public/error_location.pb.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_location.h"

namespace zetasql::linter {

std::ostream& operator<<(std::ostream& os, const ErrorCode& obj) {
  std::string s = "No such ErrorCode";
  for (auto& it : GetErrorMap())
    if (it.second == obj) s = it.first;
  os << s;
  return os;
}

std::map<std::string, ErrorCode> GetErrorMap() {
  static std::map<std::string, ErrorCode> error_map{
      {"parser-failed", ErrorCode::kParseFailed},
      {"nolint", ErrorCode::kNoLint},
      {"line-limit-exceed", ErrorCode::kLineLimit},
      {"statement-semicolon", ErrorCode::kSemicolon},
      {"consistent-letter-case", ErrorCode::kLetterCase},
      {"consistent-comment-style", ErrorCode::kCommentStyle},
      {"alias", ErrorCode::kAlias},
      {"uniform-indent", ErrorCode::kUniformIndent},
      {"not-indent-tab", ErrorCode::kNotIndentTab},
      {"single-or-double-quote", ErrorCode::kSingleQuote},
      {"table-name", ErrorCode::kTableName},
      {"window-name", ErrorCode::kWindowName},
      {"function-name", ErrorCode::kFunctionName},
      {"data-type-name", ErrorCode::kDataTypeName},
      {"column-name", ErrorCode::kColumnName},
      {"parameter-name", ErrorCode::kParameterName},
      {"constant-name", ErrorCode::kConstantName},
      {"join", ErrorCode::kJoin},
      {"imports", ErrorCode::kImport},
      {"expression-parantheses", ErrorCode::kExpressionParanteses},
      {"count-star", ErrorCode::kCountStar},
      {"keyword-identifier", ErrorCode::kKeywordIdentifier},
      {"specify-table", ErrorCode::kSpecifyTable},
      {"status", ErrorCode::kStatus}};
  return error_map;
}
std::string LintError::GetErrorMessage() { return message_; }

std::string LintError::ConstructPositionMessage() {
  return absl::StrCat("In line ", line_, ", column ", column_, ": ");
}

std::string LintError::ErrorCodeToString() {
  std::map<std::string, ErrorCode> error_map = GetErrorMap();
  for (auto& it : error_map)
    if (type_ == it.second) return it.first;
  // It should NEVER come here, but program shouldn't crash for it.
  // Add error to error_map to fix the bug.
  return "";
}

void LintError::PrintError() {
  if (filename_ == "") {
    std::cout << ConstructPositionMessage() << GetErrorMessage() << " ["
              << ErrorCodeToString() << "]" << std::endl;
  } else {
    std::cout << filename_ << ":" << ConstructPositionMessage()
              << GetErrorMessage() << " [" << ErrorCodeToString() << "]"
              << std::endl;
  }
}

void LinterResult::PrintResult() {
  Sort();
  for (LintError error : errors_) error.PrintError();
  if (filename_ == "") {
    std::cerr << "Linter results are printed" << std::endl;
  } else {
    std::cerr << "Linter is done processing file: " << filename_ << std::endl;
  }
}

LinterResult::LinterResult(const absl::Status& status) {
  if (!status.ok()) {
    if (show_status_) {
      ErrorLocation location;
      // TODO(nastaran): Check return value and propagate it.
      GetErrorLocation(status, &location);
      LintError t(ErrorCode::kStatus, filename_, location.line(),
                  location.column(), status.message());
      errors_.push_back(t);
    }
  }
}

absl::Status LinterResult::Add(absl::string_view filename, ErrorCode type,
                               absl::string_view sql, int character_location,
                               std::string message) {
  ParseLocationPoint lp =
      ParseLocationPoint::FromByteOffset(character_location);
  ParseLocationTranslator lt(sql);
  std::pair<int, int> error_pos;
  ZETASQL_ASSIGN_OR_RETURN(error_pos, lt.GetLineAndColumnAfterTabExpansion(lp));
  LintError t(type, filename, error_pos.first, error_pos.second, message);
  errors_.push_back(t);
  return absl::OkStatus();
}

void LinterResult::Add(ErrorCode type, absl::string_view sql,
                       int character_location, std::string message) {
  // TODO(nastaran): Propagate status.
  Add(filename_, type, sql, character_location, message).IgnoreError();
}

void LinterResult::Add(ErrorCode type, int line, int column,
                       absl::string_view message) {
  errors_.push_back(LintError(type, filename_, line, column, message));
}

void LinterResult::Add(LinterResult result) {
  for (const LintError error : result.GetErrors()) errors_.push_back(error);
  for (const absl::Status status : result.GetStatus()) status_.push_back(status);
}

bool LinterResult::ok() { return errors_.empty() && status_.empty(); }

void LinterResult::Clear() { errors_.clear(); }

void LinterResult::Sort() {
  sort(errors_.begin(), errors_.end(),
       [&](const LintError& a, const LintError& b) {
         return a.GetPosition() < b.GetPosition();
       });
}

}  // namespace zetasql::linter
