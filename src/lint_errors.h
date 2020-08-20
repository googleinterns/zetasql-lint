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

#ifndef SRC_LINT_ERRORS_H_
#define SRC_LINT_ERRORS_H_

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "zetasql/base/status.h"

namespace zetasql::linter {

enum class ErrorCode : int {
  kParseFailed = 0,
  kLineLimit,
  kSemicolon,
  kLetterCase,
  kCommentStyle,
  kAlias,
  kJoin,
  kImport,
  kSingleQuote,
  kUniformIndent,
  kNotIndentTab,
  kTableName,
  kWindowName,
  kFunctionName,
  kDataTypeName,
  kColumnName,
  kParameterName,
  kConstantName,
  kStatus,
  kNoLint,
  COUNT,
};

std::ostream& operator<<(std::ostream& os, const ErrorCode& obj);

// Returns string mapping of each ErrorCode
std::map<std::string, ErrorCode> GetErrorMap();

// Stores properties of a single lint error.
class LintError {
 public:
  LintError(ErrorCode type, absl::string_view filename, int line, int column,
            absl::string_view message)
      : type_(type),
        filename_(filename),
        line_(line),
        column_(column),
        message_(message) {}

  // Returns the raw form of error message, (without position information).
  std::string GetErrorMessage();

  // Returns the position where the error occured,
  // in a pair with <line, column> format.
  std::pair<int, int> GetPosition() const {
    return std::make_pair(line_, column_);
  }

  // Constructs a text message with code position info.
  std::string ConstructPositionMessage();

  // Returns mapped string that corresponds to the error type.
  std::string ErrorCodeToString();

  // This function outputs lint errors of successful checks
  // and status messages of failed checks.
  void PrintError();

  // Returns the line number where the error occured.
  int GetLineNumber() const { return line_; }

  // Returns the type of the lint error
  ErrorCode GetType() const { return type_; }

 private:
  // Holds type of the lint error. Type of an error is a number
  // that corresponds to a specific linter check.
  ErrorCode type_;

  // Name of the file where the lint error occured.
  absl::string_view filename_ = "";

  // Line number where the lint error occured.
  int line_;

  // Column number where the lint error occured.
  int column_;

  // Error message that will be printed.
  std::string message_ = "";
};

// It is the result of a linter run.
// Result of a linter run is cumilative results of
// linter check. Linter checks can fail checking on the querry
// and return status, or it can successfully work and return list
// of lint errors.
class LinterResult {
 public:
  LinterResult()
      : errors_(std::vector<LintError>()),
        status_(std::vector<absl::Status>()) {}

  explicit LinterResult(absl::string_view filename) : filename_(filename) {}

  explicit LinterResult(const absl::Status& status);

  // This function adds a new lint error that occured in 'sql' in
  // location 'character_location', and 'type' refers to
  // the type of linter check that is failed.
  absl::Status Add(absl::string_view filename, ErrorCode type,
                   absl::string_view sql, int character_location,
                   std::string message);

  // Basicly does the same with above function without a
  // specific filename.
  void Add(ErrorCode type, absl::string_view sql, int character_location,
           std::string message);

  // This function adds all errors in 'result' to this
  // It basicly combines two result.
  void Add(LinterResult result);

  // Returns if any lint error occured.
  bool ok();

  // Clears all errors.
  void Clear();

  // Sorts all errors.
  void Sort();

  // Returns all Lint Errors that are detected.
  std::vector<LintError> GetErrors() { return errors_; }

  // Returns all Status Errors that are occured.
  std::vector<absl::Status> GetStatus() { return status_; }

  // Output the result in a user-readable format. This function
  // will be used to inform user about lint errors in their sql file.
  void PrintResult();

  // Sets if status messages will be shown to the user.
  void SetShowStatus(bool show_status) { show_status_ = show_status; }

  // Sets if status messages will be shown to the user.
  void SetFilename(absl::string_view filename) { filename_ = filename; }

 private:
  // All linter errors occured in various lint checks.
  std::vector<LintError> errors_;

  // All status occured in various lint checks.
  std::vector<absl::Status> status_;

  // Whenever a lint check fails status message occurs. This variable
  // determines if status messages should be shown to the user.
  bool show_status_ = true;

  // Name of the sql file.
  absl::string_view filename_;
};

}  // namespace zetasql::linter

#endif  // SRC_LINT_ERRORS_H_
