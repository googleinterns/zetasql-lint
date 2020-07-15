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
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_location.h"

namespace zetasql::linter {

// Copy constructor for Lint Error
LintError::LintError(const LintError &result) {
  type_ = result.type_;
  filename_ = result.filename_;
  line_ = result.line_;
  column_ = result.column_;
  message_ = result.message_;
}

absl::string_view LintError::GetErrorMessage() { return message_; }

std::string LintError::ConstructPositionMessage() {
  return absl::StrCat("In line ", line_, ", column ", column_, ": ");
}

void LintError::PrintError() {
  if (filename_ == "") {
    std::cout << ConstructPositionMessage() << GetErrorMessage() << std::endl;
  } else {
    std::cout << filename_ << ":" << ConstructPositionMessage()
              << GetErrorMessage() << std::endl;
  }
}

void LinterResult::PrintResult() {
  for (LintError error : errors_) error.PrintError();
  // Need to sort according to increasing line number
  sort(errors_.begin(), errors_.end(),
       [&](const LintError &a, const LintError &b) {
         return a.getLineNumber() < b.getLineNumber();
       });
  for (LintError error : errors_) error.PrintError();

  for (absl::Status status : status_) std::cerr << status << std::endl;

  std::cout << "Linter results are printed" << std::endl;
}

LinterResult::LinterResult(const LinterResult &result) { Add(result); }
LinterResult::LinterResult(const absl::Status &status) {
  if (!status.ok()) status_.push_back(status);
}

void LinterResult::Add(ErrorCode type, absl::string_view filename,
                       absl::string_view sql, int character_location,
                       absl::string_view message) {
  int line_number = 0;
  int column_number = 1;
  // When LinterOptions(getting config from user) is implemented,
  // this '\n' will be replaced with 'options.delimeter'.
  for (int i = 0; i < character_location; ++i)
    if (sql[i] == '\n') {
      ++line_number;
      column_number = 1;
    } else {
      ++column_number;
    }
  LintError t(type, filename, line_number, column_number, message);
  errors_.push_back(t);
}

void LinterResult::Add(ErrorCode type, absl::string_view sql,
                       int character_location, absl::string_view message) {
  Add(type, "", sql, character_location, message);
}

void LinterResult::Add(const LinterResult &result) {
  std::vector<LintError> errors = result.GetErrors();
  for (LintError error : errors) errors_.push_back(error);

  std::vector<absl::Status> allStatus = result.GetStatus();
  for (absl::Status status : allStatus) status_.push_back(status);
}

bool LinterResult::ok() { return errors_.empty() && status_.empty(); }

void LinterResult::clear() { errors_.clear(); }

}  // namespace zetasql::linter
