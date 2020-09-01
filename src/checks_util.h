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

#ifndef SRC_CHECKS_UTIL_H_
#define SRC_CHECKS_UTIL_H_

// This class is for all the helper functions that checks use.

#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "src/lint_error.h"
#include "src/linter_options.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_location.h"
#include "zetasql/public/parse_tokens.h"

namespace zetasql::linter {

// It gets rule and applies that rule to every ASTnode it visit.
class RuleVisitor : public NonRecursiveParseTreeVisitor {
 public:
  RuleVisitor(const std::function<LinterResult(const ASTNode *,
                                               const absl::string_view &,
                                               const LinterOptions &)> &rule,
              const absl::string_view &sql, const LinterOptions &options)
      : rule_(rule), sql_(sql), option_(options), result_(absl::OkStatus()) {}

  // It is a function that will be invoked each time a new
  // node is visited.
  zetasql_base::StatusOr<VisitResult> defaultVisit(
      const ASTNode *node) override;

  // Returns the cumulative result of all rules that applied.
  LinterResult GetResult() { return result_; }

 private:
  std::function<LinterResult(const ASTNode *, const absl::string_view &,
                             const LinterOptions &)>
      rule_;
  absl::string_view sql_;
  LinterResult result_;
  const LinterOptions &option_;
};

// Stores properties of a single rule and menages possible
// applications of this rule.
class ASTNodeRule {
 public:
  explicit ASTNodeRule(
      const std::function<LinterResult(
          const ASTNode *, const absl::string_view &, const LinterOptions &)>
          rule)
      : rule_(rule) {}

  // It applies the rule stored in this class to a sql statement.
  LinterResult ApplyTo(absl::string_view sql, const LinterOptions &options);

 private:
  std::function<LinterResult(const ASTNode *, const absl::string_view &,
                             const LinterOptions &)>
      rule_;
};

absl::string_view GetName(const ParseLocationRange &range,
                          const absl::string_view &sql);

// Given an ASTNode returns corresponding string for that node.
absl::string_view GetNodeString(const ASTNode *node,
                                const absl::string_view &sql);

// Checks if a character is uppercase.
bool IsUppercase(char c);

// Checks if a character is lowercase.
bool IsLowercase(char c);

// Converts and returns all uppercase version of a name.
std::string ConvertToUppercase(absl::string_view name);

// Checks if a name is written in UpperCamelCase.
bool IsUpperCamelCase(absl::string_view name);

// Checks if a name is written in lowerCamelCase.
bool IsLowerCamelCase(absl::string_view name);

// Checks if a name is written in ALLCAPS.
bool IsAllCaps(absl::string_view name);

// Checks if a name is written in CAPS_SNAKE_CASE.
bool IsCapsSnakeCase(absl::string_view name);

// Checks if a name is written in lower_snake_case.
bool IsLowerSnakeCase(absl::string_view name);

// Checks if, in a sql file, corresponding position of a node is smaller than
// corresponding position of a token.
bool IsBefore(const ASTNode *node, const ParseToken &token);

// Checks if a node and a token refer to the same part of the sql.
bool IsTheSame(const ASTNode *node, const ParseToken &token);

// Ignores forward spaces and increase the position until the first non-empty
// character. (empty characters: ' ', '\t', '\n')
// Returns true if position reaches to the end.
bool IgnoreSpacesForward(absl::string_view sql, int *position);

// Ignores backward spaces and decrease the position until the first non-empty
// character. (empty characters: ' ', '\t', '\n')
// Returns true if position reaches to the end.
bool IgnoreSpacesBackward(absl::string_view sql, int *position);

// Given a position in a sql file, checks if any comment
// starts from that position. If it is, sets position to the end of
// that comment(such that one character afterwards is unrelated to the comment),
// does nothing otherwise.
bool IgnoreComments(absl::string_view sql, const LinterOptions &options,
                    int *position, bool ignore_single_line = true);

// String version of 'IgnoreComments'.
// Given a position in a sql file, checks if any string
// starts from that position. If it is, sets position to the end of
// that string(such that one character afterwards is unrelated to the string),
// does nothing otherwise.
bool IgnoreStrings(absl::string_view sql, int *position);

// Given a position in a sql file, returns first word comes
// after that positon. The seperator characters in a sql file will be :
// ( ' ', '\t', '\n', ';', ',', '(' ).
std::string GetNextWord(absl::string_view sql, int *position);

// Prints AST tree of an sql statement.
LinterResult PrintASTTree(absl::string_view sql);

// There are several statements(or parts of them) that
// should be written in a single line (shouldn't be seperated).
// Checks if a given line can be seperated or not.
bool OneLineStatement(absl::string_view line);

// Checks if a string consists of either all uppercase letters
// or all lowercase letters.
bool ConsistentUppercaseLowercase(const absl::string_view &sql,
                                  const ParseLocationRange &range,
                                  const LinterOptions &options);

// Returns all tokenizer keywords of a sql query.
// They will be returned sorted in ascending position order.
std::vector<ParseToken> GetKeywords(absl::string_view sql);

// Helper function that adds all identifiers in subtree of a ASTNode
// to a list.
void GetIdentifiers(const ASTNode *node, std::vector<const ASTNode *> *list);

// Gets all identifiers from previously parsed AST.
// Return empty vector if options doesn't have previously parsed AST.
std::vector<const ASTNode *> GetIdentifiers(absl::string_view sql,
                                            const LinterOptions &options);

}  // namespace zetasql::linter

#endif  // SRC_CHECKS_UTIL_H_
