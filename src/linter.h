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

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "src/checks_util.h"
#include "src/lint_errors.h"
#include "src/linter_options.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_resume_location.h"

namespace zetasql::linter {

// Checks whether input can be parsed with ZetaSQL parser.
LinterResult CheckParserSucceeds(absl::string_view sql,
                                 const LinterOptions &option);

// Checks if the number of characters in any line
// exceed a certain treshold.
LinterResult CheckLineLength(absl::string_view sql,
                             const LinterOptions &option);

// Checks whether every statement ends with a semicolon ';'.
LinterResult CheckSemicolon(absl::string_view sql, const LinterOptions &option);

// Checks whether all keywords are uppercase.
LinterResult CheckUppercaseKeywords(absl::string_view sql,
                                    const LinterOptions &option);

// Check if comment style is uniform (either -- or //, not both).
LinterResult CheckCommentType(absl::string_view sql,
                              const LinterOptions &option);

// Checks whether all aliases denoted by 'AS' keyword.
LinterResult CheckAliasKeyword(absl::string_view sql,
                               const LinterOptions &option);

// Checks whether all tab characters in indentations are equal to
// <allowed_indent>.
LinterResult CheckTabCharactersUniform(absl::string_view sql,
                                       const LinterOptions &option);

// Checks whether there are no tabs in the code except indents.
LinterResult CheckNoTabsBesidesIndentations(absl::string_view sql,
                                            const LinterOptions &option);

// Checks if single/double quote usage conflicts with configuration.
LinterResult CheckSingleQuotes(absl::string_view sql,
                               const LinterOptions &option);

// Checks if any of naming conventions is not satisfied. Details of naming
// convenstion can be found in docs/checks.md#naming
LinterResult CheckNames(absl::string_view sql, const LinterOptions &option);

// Checks if any of Join statement has missing indicator(LEFT, INNER, etc.)
LinterResult CheckJoin(absl::string_view sql, const LinterOptions &option);

//
LinterResult CheckImports(absl::string_view sql, const LinterOptions &option);

// Checks if any
LinterResult CheckExpressionParantheses(absl::string_view sql,
                                        const LinterOptions &option);
}  // namespace zetasql::linter

#endif  // SRC_LINTER_H_
