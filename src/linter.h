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
#include "src/lint_errors.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_resume_location.h"

namespace zetasql::linter {

// It gets rule and applies that rule to every ASTnode it visit.
class RuleVisitor : public NonRecursiveParseTreeVisitor {
 public:
  RuleVisitor(const std::function<LinterResult(
                  const ASTNode *, const absl::string_view &)> &rule,
              const absl::string_view &sql)
      : rule_(rule), sql_(sql), result_(absl::OkStatus()) {}

  // It is a function that will be invoked each time a new
  // node is visited.
  zetasql_base::StatusOr<VisitResult> defaultVisit(
      const ASTNode *node) override;

  // Returns the cumulative result of all rules that applied.
  LinterResult GetResult() { return result_; }

 private:
  std::function<LinterResult(const ASTNode *, const absl::string_view &)> rule_;
  absl::string_view sql_;
  LinterResult result_;
};

// Stores properties of a single rule and menages possible
// applications of this rule.
class ASTNodeRule {
 public:
  explicit ASTNodeRule(const std::function<
                       LinterResult(const ASTNode *, const absl::string_view &)>
                           rule)
      : rule_(rule) {}

  // It applies the rule stored in this class to a sql statement.
  LinterResult ApplyTo(absl::string_view sql);

 private:
  std::function<LinterResult(const ASTNode *, const absl::string_view &)> rule_;
};

// Debugger that will be erased later.
LinterResult PrintASTTree(absl::string_view sql);

// Checks if the number of characters in any line
// exceed a certain treshold.
LinterResult CheckLineLength(absl::string_view sql, int lineLimit = 100,
                             const char delimeter = '\n');

// Checks whether input can be parsed with ZetaSQL parser.
LinterResult CheckParserSucceeds(absl::string_view sql);

// Checks whether every statement ends with a semicolon ';'.
LinterResult CheckSemicolon(absl::string_view sql);

// Checks whether all keywords are uppercase.
LinterResult CheckUppercaseKeywords(absl::string_view sql);

// Check if comment style is uniform (either -- or //, not both).
LinterResult CheckCommentType(absl::string_view sql,
                              const char delimeter = '\n');

// Checks whether all aliases denoted by 'AS' keyword.
LinterResult CheckAliasKeyword(absl::string_view sql);

// Checks whether all tab characters in indentations are equal to
// <allowed_indent>.
LinterResult CheckTabCharactersUniform(absl::string_view sql,
                                       const char allowed_indent = ' ',
                                       const char line_delimeter = '\n');

// Checks whether there are no tabs in the code except indents.
LinterResult CheckNoTabsBesidesIndentations(absl::string_view sql,
                                            const char line_delimeter = '\n');

}  // namespace zetasql::linter

#endif  // SRC_LINTER_H_
