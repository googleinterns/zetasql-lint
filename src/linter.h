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
<<<<<<< HEAD
=======
#include <utility>
>>>>>>> df57be8ba9111411d0d4dd9804477d4dad632273
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
<<<<<<< HEAD
#include "src/lint_errors.h"
#include "zetasql/base/status.h"
=======
>>>>>>> df57be8ba9111411d0d4dd9804477d4dad632273
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_resume_location.h"

namespace zetasql::linter {

// It gets rule and applies that rule to every ASTnode it visit.
class RuleVisitor : public NonRecursiveParseTreeVisitor {
 public:
<<<<<<< HEAD
  RuleVisitor(const std::function<absl::Status(const ASTNode*,
                                               absl::string_view)>& rule,
              ErrorCode error_code, const absl::string_view& sql,
              LinterResult* result)
      : rule_(rule), error_code_(error_code), sql_(sql), result_(result) {}

  zetasql_base::StatusOr<VisitResult> defaultVisit(
      const ASTNode* node) override;

  LinterResult* GetResult() { return result_; }

 private:
  std::function<absl::Status(const ASTNode*, absl::string_view)> rule_;
  ErrorCode error_code_;
  absl::string_view sql_;
  LinterResult* result_;
=======
  RuleVisitor(const std::function<absl::Status(const ASTNode *,
                                               absl::string_view)> &rule,
              const absl::string_view &sql)
      : rule_(rule), sql_(sql), result_(absl::OkStatus()) {}

  zetasql_base::StatusOr<VisitResult> defaultVisit(
      const ASTNode *node) override;

  absl::Status GetResult() { return result_; }

 private:
  std::function<absl::Status(const ASTNode *, absl::string_view)> rule_;
  absl::string_view sql_;
  absl::Status result_;
>>>>>>> df57be8ba9111411d0d4dd9804477d4dad632273
};

class ASTNodeRule {
 public:
  explicit ASTNodeRule(
<<<<<<< HEAD
      LinterResult* result, ErrorCode error_code,
      const std::function<absl::Status(const ASTNode*, absl::string_view)> rule)
      : result_(result), error_code_(error_code), rule_(rule) {}
  absl::Status ApplyTo(absl::string_view sql);

 private:
  LinterResult* result_;
  ErrorCode error_code_;
  std::function<absl::Status(const ASTNode*, absl::string_view)> rule_;
=======
      const std::function<absl::Status(const ASTNode *, absl::string_view)>
          rule)
      : rule_(rule) {}
  absl::Status ApplyTo(absl::string_view sql);

 private:
  std::function<absl::Status(const ASTNode *, absl::string_view)> rule_;
>>>>>>> df57be8ba9111411d0d4dd9804477d4dad632273
};

// Debugger that will be erased later.
absl::Status PrintASTTree(absl::string_view sql, LinterResult* result);

// Checks if the number of characters in any line
// exceed a certain treshold.
<<<<<<< HEAD
absl::Status CheckLineLength(absl::string_view sql, LinterResult* result,
                             int lineLimit = 100, const char delimeter = '\n');
=======
absl::Status CheckLineLength(absl::string_view sql, int lineLimit = 100,
                             const char delimeter = '\n');
>>>>>>> df57be8ba9111411d0d4dd9804477d4dad632273

// Checks whether input can be parsed with ZetaSQL parser.
absl::Status CheckParserSucceeds(absl::string_view sql, LinterResult* result);

// Checks whether every statement ends with a semicolon ';'.
absl::Status CheckSemicolon(absl::string_view sql, LinterResult* result);

// Checks whether all keywords are either uppercase or lowercase.
absl::Status CheckUppercaseKeywords(absl::string_view sql,
                                    LinterResult* result);

// Check if comment style is uniform (either -- or //, not both).
<<<<<<< HEAD
absl::Status CheckCommentType(absl::string_view sql, LinterResult* result);
=======
absl::Status CheckCommentType(absl::string_view sql,
                              const char delimeter = '\n');
>>>>>>> df57be8ba9111411d0d4dd9804477d4dad632273

// Checks whether all aliases denoted by 'AS' keyword.
absl::Status CheckAliasKeyword(absl::string_view sql, LinterResult* result);

// Checks whether all tab characters in indentations are equal to
// <allowed_indent>.
absl::Status CheckTabCharactersUniform(absl::string_view sql,
                                       const char allowed_indent = ' ',
                                       const char line_delimeter = '\n');

// Checks whether there are no tabs in the code except indents.
absl::Status CheckNoTabsBesidesIndentations(absl::string_view sql,
                                            const char line_delimeter = '\n');

// Constructs a text message with code position info.
// <pos> represents the (line, position) in the code.
std::string ConstructPositionMessage(std::pair<int, int> pos);

// Constructs an absl::Status with <error_msg> message specified at
// the <sql[index]> position.
absl::Status ConstructErrorWithPosition(absl::string_view sql,
                                        int index,
                                        absl::string_view error_msg);

}  // namespace zetasql::linter

#endif  // SRC_LINTER_H_
