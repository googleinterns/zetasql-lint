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

// This class is for any options that checks will use.
//
// Adding configurable option have these steps:
//    1. Create the variable, along with its UpperCamelCase
//       getter and setter functions. (Follow the convention.)
//    2. Add the variable to the 'config.proto' file.
//    3. Connect proto and option from linter.cc/GetOptionsFromConfig()
//    4. Update the documentation.
//
// This class can also contain other helper varibles that are used in checks.

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/lint_error.h"
#include "zetasql/public/parse_helpers.h"

namespace zetasql::linter {

class LinterOptions {
  class CheckOptions;

 public:
  LinterOptions() {}

  explicit LinterOptions(absl::string_view filename) : filename_(filename) {}

  // Setter for filename_.
  void SetFilename(absl::string_view filename) { filename_ = filename; }

  // Getter for filename_.
  absl::string_view Filename() { return filename_; }

  // Returns if the linter check should be active
  // in <position>.
  bool IsActive(ErrorCode code, int position) const;

  // Disables linter check after <position>.
  // Enabling/Disabling positions should always come in
  // INCREASING order.
  void Disable(ErrorCode code, int position);

  // Enables linter check after <position>.
  // Enabling/Disabling positions should always come in
  // INCREASING order.
  void Enable(ErrorCode code, int position);

  // Adds a single parser output to parset_output_
  void AddParserOutput(ParserOutput *output);

  // Changes if any lint is active from the start.
  void DisactivateCheck(ErrorCode code);

  // ---------------------------------- GETTER/SETTER functions

  const std::vector<ParserOutput *> &ParserOutputs() const {
    return parser_outputs_;
  }

  bool RememberParser() const { return remember_parser_; }
  void SetRememberParser(bool val) { remember_parser_ = val; }

  int TabSize() const { return tab_size_; }
  void SetTabSize(char val) { tab_size_ = val; }

  int LineDelimeter() const { return line_delimeter_; }
  void SetLineDelimeter(char val) { line_delimeter_ = val; }

  int LineLimit() const { return line_limit_; }
  void SetLineLimit(int val) { line_limit_ = val; }

  char AllowedIndent() const { return allowed_indent_; }
  void SetAllowedIndent(char val) { allowed_indent_ = val; }

  bool SingleQuote() const { return single_quote_; }
  void SetSingleQuote(bool val) { single_quote_ = val; }

  bool UpperKeyword() const { return upper_keyword_; }
  void SetUpperKeyword(bool val) { upper_keyword_ = val; }

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

  // True if all keywords should be all uppercase, false for all lowercase.
  bool upper_keyword_ = true;

  // Whenever a lint check fails status message occurs. This variable
  // determines if status messages should be shown to the user.
  bool show_status_ = true;

  // For each ErrorCode that correspond to a check, it stores
  // options for that check.
  std::map<ErrorCode, CheckOptions> option_map_;

  // Stores whether at least one parser call is made.
  // It will optimize linter to make only one parser call.
  bool remember_parser_ = false;

  // If remember_parser_ is enabled, this will hold parser output.
  std::vector<ParserOutput *> parser_outputs_;

  // Name of the sql file.
  absl::string_view filename_ = "";

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
