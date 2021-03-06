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

#ifndef SRC_LINTER_H_
#define SRC_LINTER_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "src/checks.h"
#include "src/checks_list.h"
#include "src/config.pb.h"
#include "src/lint_error.h"
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

// Checks whether input can be parsed with ZetaSQL parser.
LinterResult CheckParserSucceeds(absl::string_view sql, LinterOptions* options);

// This function gets LinterOptions from a specified
// configuration file.
void GetOptionsFromConfig(Config config, LinterOptions* options);

// It runs all linter checks
LinterResult RunChecks(absl::string_view sql, LinterOptions* options);

// It runs all linter checks
LinterResult RunChecks(absl::string_view sql, Config config,
                       absl::string_view filename);

// It runs all linter checks
LinterResult RunChecks(absl::string_view sql, absl::string_view filename);

// It runs all linter checks
LinterResult RunChecks(absl::string_view sql);

}  // namespace zetasql::linter

#endif  // SRC_LINTER_H_
