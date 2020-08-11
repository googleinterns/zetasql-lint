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

#ifndef SRC_LINTER_OPTIONS_H_
#define SRC_LINTER_OPTIONS_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "src/lint_errors.h"

namespace zetasql::linter {

class LinterOptions {
  class CheckOptions;

 public:
  // Returns if the linter check should be active
  // in <position>.
  bool IsActive(ErrorCode code, int position) const;

  // Disables linter check after <position>.
  // Enabling/Disabling positions should always come in
  // increasing order.
  void Disable(ErrorCode code, int position);

  // Enables linter check after <position>.
  // Enabling/Disabling positions should always come in
  // increasing order.
  void Enable(ErrorCode code, int position);

  // Getter for tab_size_.
  int TabSize() const { return tab_size_; }

  // Setter for tab_size_.
  int SetTabSize(char tab_size) { tab_size_ = tab_size; }

  // Getter for line_delimeter_.
  int LineDelimeter() const { return line_delimeter_; }

  // Setter for line_delimeter_.
  void SetLineDelimeter(int line_delimeter) {
    line_delimeter_ = line_delimeter;
  }

  // Getter for line_limit_.
  int LineLimit() const { return line_limit_; }

  // Setter for line_limit_.
  void SetLineLimit(int line_limit) { line_limit_ = line_limit; }

  // Getter for allowed_indent_.
  char AllowedIndent() const { return allowed_indent_; }

  // Setter for allowed_indent_.
  void SetAllowedIndent(char allowed_indent) {
    allowed_indent_ = allowed_indent;
  }

  // Getter for single_quote_.
  char SingleQuote() const { return single_quote_; }

  // Setter for single_quote_.
  void SetSingleQuote(char single_quote) {
    single_quote_ = single_quote;
  }

  // Changes if any lint is active from the start.
  void DisactivateCheck(ErrorCode code);

 private:
  // Number of characters one tab character(\t) counts.
  int tab_size_ = 4;

  // Delimeter that seperates lines.
  char line_delimeter_ = '\n';

  // Maximum number of characters one line should contain.
  int line_limit_ = 100;

  // Allowed character type of indentation. It should be either
  // '\t' or ' '.
  char allowed_indent_ = ' ';

  // True if user should use single quotes, false for double quotes.
  bool single_quote_ = true;

  // Whenever a lint check fails status message occurs. This variable
  // determines if status messages should be shown to the user.
  bool show_status_ = true;

  // For each ErrorCode that correspond to a check, it stores
  // options for that check.
  std::map<ErrorCode, CheckOptions> option_map_;

  // This class is options specified for a check.
  class CheckOptions {
   public:
    // Returns if the linter check should be active
    // in <position>.
    bool IsActive(int position) const;

    // Disables linter check after <position>.
    // Enabling/Disabling positions should always come in
    // increasing order.
    void Disable(int position);

    // Enables linter check after <position>.
    // Enabling/Disabling positions should always come in
    // increasing order.
    void Enable(int position);

    // Setter for active_start.
    void SetActiveStart(bool active_start) { active_start_ = active_start; }

   private:
    // Stores switching points between enabling and disabling.
    std::vector<int> switchs_ = std::vector<int>();

    // Stores if the linter check is active from the start.
    bool active_start_ = true;
  };
};

}  // namespace zetasql::linter

#endif  // SRC_LINTER_OPTIONS_H_
