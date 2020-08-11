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

#ifndef SRC_EXECUTE_LINTER_H_
#define SRC_EXECUTE_LINTER_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "src/lint_errors.h"
#include "src/linter.h"
#include "src/check_list.h"
#include "src/linter_options.h"

namespace zetasql::linter {

// This function will parse "NOLINT"(<Name1>, <Name2>, ...) syntax from
// a single line of comment.
// If NOLINT usage errors occur, it counts as a lint
// error and it will be returned in a result.
LinterResult ParseNoLintSingleComment(absl::string_view line,
                                      const absl::string_view& sql,
                                      int position, LinterOptions* options);

// This function will parse "NOLINT"(<CheckName>) syntax from a sql file.
// If NOLINT usage errors occur, it counts as a lint
// error and it will be returned in a result.
// The main purpose of this function is parsing single line comments
// and getting(then combining) results from 'ParseNoLintSingleComment'
LinterResult ParseNoLintComments(absl::string_view sql, LinterOptions* options);

// This function is the main function to get all the checks.
// Whenever a new check is added this should be
// the first place to update.
CheckList GetAllChecks();

// This function gets LinterOptions from a specified
// configuration file.
LinterOptions GetOptionsFromConfig();

// It runs all specified checks
LinterResult RunChecks(absl::string_view sql, LinterOptions);

// It runs all specified checks
LinterResult RunChecks(absl::string_view sql);

}  // namespace zetasql::linter

#endif  // SRC_EXECUTE_LINTER_H_
