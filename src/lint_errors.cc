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
#include "src/lint_errors.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "zetasql/base/status.h"

namespace zetasql::linter {

absl::string_view LintError::GetErrorMessage() {
  // Refers to the error type
  switch (type_) {
    case kLineLimit:
      return "Lines should be <= 100 characters long";
    case kParseFailed:
      return "ZetaSQL parser failed";
    case kSemicolon:
      return "Each statement should end with a consequtive semicolon ';'";
    case kUppercase:
      return "All keywords should be consist of uppercase letters";
    case kCommentStyle:
      return "Either '//' or '--' should be used to specify a comment";
    case kAlias:
      return "Always use AS keyword for referencing aliases";
      break;
    default:
      break;
  }
  return "";
}

void LintError::PrintError() {
  if (filename_ == "") {
    std::cout << "In line " << line_ << ": " << GetErrorMessage() << std::endl;
  } else {
    std::cout << filename_ << ":" << line_ << ": " << GetErrorMessage()
              << std::endl;
  }
}

void LinterResult::PrintResult() {
  // Need to sort according to increasing line number
  sort(errors_.begin(), errors_.end(),
       [&](const LintError &a, const LintError &b) {
         return a.getLineNumber() < b.getLineNumber();
       });
  for (LintError error : errors_) error.PrintError();
}

LinterResult::LinterResult(const LinterResult &result) {
  errors_ = result.GetErrors();
}

void LinterResult::Add(ErrorCode type, absl::string_view filename,
                       absl::string_view sql, int character_location) {
  int line_number = 1;
  for (int i = 0; i < character_location; ++i)
    if (sql[i] == '\n') ++line_number;
  errors_.push_back(LintError(type, filename, line_number));
}

void LinterResult::Add(ErrorCode type, absl::string_view sql,
                       int character_location) {
  Add(type, "", sql, character_location);
}

void LinterResult::Add(const LinterResult &result) {
  std::vector<LintError> errors = result.GetErrors();
  for (LintError error : errors) errors_.push_back(error);
}

bool LinterResult::ok() { return errors_.empty(); }

void LinterResult::clear() { errors_.clear(); }

}  // namespace zetasql::linter
