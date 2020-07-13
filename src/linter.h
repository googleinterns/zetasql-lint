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

#include <vector>
#include <cstdio>
#include <string>

#include "absl/strings/string_view.h"
#include "src/lint_errors.h"
#include "zetasql/base/status.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_resume_location.h"

namespace zetasql::linter {

// It gets rule and applies that rule to every ASTnode it visit.
class RuleVisitor : public NonRecursiveParseTreeVisitor {
 public:
    RuleVisitor(const std::function
        <absl::Status(const ASTNode*, absl::string_view)> &rule,
            ErrorCode error_code,
            const absl::string_view &sql, LinterResult* result)
        :   rule_(rule), error_code_(error_code),
            sql_(sql), result_(result) {}

    zetasql_base::StatusOr<VisitResult> defaultVisit(
        const ASTNode* node) override;

    LinterResult* GetResult() { return result_; }

 private:
    std::function
        <absl::Status(const ASTNode*, absl::string_view)> rule_;
    ErrorCode error_code_;
    absl::string_view sql_;
    LinterResult* result_;
};

class ASTNodeRule {
 public:
    explicit ASTNodeRule(LinterResult* result, ErrorCode error_code,
        const std::function
        <absl::Status(const ASTNode*, absl::string_view)> rule)
        : result_(result), error_code_(error_code), rule_(rule) {}
    absl::Status ApplyTo(absl::string_view sql);

 private:
    LinterResult* result_;
    ErrorCode error_code_;
    std::function
        <absl::Status(const ASTNode*, absl::string_view)> rule_;
};

// Debugger that will be erased later.
absl::Status PrintASTTree(absl::string_view sql, LinterResult* result);

// Checks if the number of characters in any line
// exceed a certain treshold.
absl::Status CheckLineLength(absl::string_view sql,  LinterResult* result,
    int lineLimit = 100, const char delimeter = '\n');

// Checks whether input can be parsed with ZetaSQL parser.
absl::Status CheckParserSucceeds(absl::string_view sql, LinterResult* result);

// Checks whether every statement ends with a semicolon ';'.
absl::Status CheckSemicolon(absl::string_view sql, LinterResult* result);

// Checks whether all keywords are either uppercase or lowercase.
absl::Status CheckUppercaseKeywords(absl::string_view sql,
    LinterResult* result);

// Check if comment style is uniform (either -- or //, not both).
absl::Status CheckCommentType(absl::string_view sql, LinterResult* result);

// Checks whether all aliases denoted by 'AS' keyword.
absl::Status CheckAliasKeyword(absl::string_view sql, LinterResult* result);

}  // namespace zetasql::linter

#endif  // SRC_LINTER_H_
