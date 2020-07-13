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

#include <vector>

#include "absl/strings/string_view.h"
#include "zetasql/base/status.h"

namespace zetasql {

namespace linter {

enum ErrorCode : int {
    kLineLimit = 1,
    kParseFailed = 2,
    kSemicolon = 3,
    kUppercase = 4,
    kCommentStyle = 6,
    kAlias = 7
};

// Its a class for saving properties of a one lint error.
class LintError {
 public:
    LintError(ErrorCode type, absl::string_view filename,
        int line )
        : type_(type), filename_(filename),
        line_(line) {}

    LintError(ErrorCode type, int line)
        : type_(type), filename_(""), line_(line) {}

    absl::string_view GetErrorMessage();

    void PrintError();

 private:
    // Holds type of the lint error. Type of an error is a number
    // that corresponds to a specific linter check.
    ErrorCode type_;

    // Name of the file where the lint error occured.
    absl::string_view filename_ = "";

    // Line number where the lint error occured.
    int line_;
};

class LinterResult {
 public:
    LinterResult() {}

    explicit LinterResult(const LinterResult &result);

    // This function adds a new lint error that occured in 'sql' in
    // location 'character_location', and 'type' refers to
    // the type of linter check that is failed.

    void Add(ErrorCode type, absl::string_view filename,
        absl::string_view sql, int character_location);

    // Basicly does the same with above function without a
    // specific filename.
    void Add(ErrorCode type, absl::string_view sql, int character_location);

    // This function adds all errors in 'result' to this
    // It basicly combines two result.
    void Add(const LinterResult &result);

    // Returns if any lint error occured
    bool ok();

    // Clears all errors
    void clear();

    std::vector<LintError> GetErrors() const {
        return errors_;
    }

    void PrintResult();

 private:
    std::vector<LintError> errors_;
};

}  // namespace linter
}  // namespace zetasql

#endif  // SRC_LINTER_H_
