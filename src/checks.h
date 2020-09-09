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

#ifndef SRC_CHECKS_H_
#define SRC_CHECKS_H_

// This class is only for main implementation of checks.
// Helper functions can be added to 'check_utils.h'
// All checks should follow the same signature, extra options
// can be included in 'LinterOptions' and it is recommended to make
// it configurable if possible.
//
// In order to add a new check you should follow these steps:
//    1. Create an ErrorCode(code of the lint error).
//    2. Map name of the check and ErrorCode by adding an element to
//       error_map in lint_error.cc->GetErrorMap();
//    3. Implement a check function in checks.cc file and add it to checks.h
//    4. Add it to the checks_list. (Linter will run it after this step).
//    5. Add unit tests.
//    6. Update the documentation /docs/checks.md with examples.

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "src/checks_util.h"
#include "src/lint_error.h"
#include "src/linter_options.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_resume_location.h"

namespace zetasql::linter {

// Checks if the number of characters in any line
// exceed a certain threshold.
LinterResult CheckLineLength(absl::string_view sql,
                             const LinterOptions &options);

// Checks whether every statement ends with a semicolon ';'.
LinterResult CheckSemicolon(absl::string_view sql,
                            const LinterOptions &options);

// Checks whether all keywords are uppercase.
LinterResult CheckUppercaseKeywords(absl::string_view sql,
                                    const LinterOptions &options);

// Check if comment style is uniform (either -- or //, not both).
LinterResult CheckCommentType(absl::string_view sql,
                              const LinterOptions &options);

// Checks whether all aliases denoted by 'AS' keyword.
LinterResult CheckAliasKeyword(absl::string_view sql,
                               const LinterOptions &options);

// Checks whether all tab characters in indentations are equal to
// <allowed_indent>.
LinterResult CheckTabCharactersUniform(absl::string_view sql,
                                       const LinterOptions &options);

// Checks whether there are no tabs in the code except indents.
LinterResult CheckNoTabsBesidesIndentations(absl::string_view sql,
                                            const LinterOptions &options);

// Checks if single/double quote usage conflicts with configuration.
LinterResult CheckSingleQuotes(absl::string_view sql,
                               const LinterOptions &options);

// Checks if any of naming conventions is not satisfied. Details of naming
// convenstion can be found in docs/checks.md#naming
LinterResult CheckNames(absl::string_view sql, const LinterOptions &options);

// Checks if any of Join statement has missing indicator(LEFT, INNER, etc.)
LinterResult CheckJoin(absl::string_view sql, const LinterOptions &options);

// Checks if PROTO imports and MODULE imports are consequtive among themselves.
// Also checks if there is a dublicate import.
LinterResult CheckImports(absl::string_view sql, const LinterOptions &options);

// Checks if any complex expression is without parantheses.
LinterResult CheckExpressionParantheses(absl::string_view sql,
                                        const LinterOptions &options);

// Checks if count(1) is used instead of count(*)
LinterResult CheckCountStar(absl::string_view sql,
                            const LinterOptions &options);

// Checks if any identifier is named as a keyword (date, type, language, etc.)
LinterResult CheckKeywordNamedIdentifier(absl::string_view sql,
                                         const LinterOptions &options);

// Checks if table names are specified in a query containing "JOIN".
LinterResult CheckSpecifyTable(absl::string_view sql,
                               const LinterOptions &options);
}  // namespace zetasql::linter

#endif  // SRC_CHECKS_H_
