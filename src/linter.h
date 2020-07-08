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
#include "zetasql/base/status.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_resume_location.h"

namespace zetasql {

namespace linter {

// It gets rule and applies that rule to every ASTnode it visit
class RuleVisitor : public zetasql::NonRecursiveParseTreeVisitor {
 public:
    RuleVisitor(const std::function
        <absl::Status(const zetasql::ASTNode*, absl::string_view)> &_rule,
                const absl::string_view &_sql)
        : rule(_rule), sql(_sql), result(absl::OkStatus()) {}

    zetasql_base::StatusOr<VisitResult> defaultVisit(
        const ASTNode* node) override {
        absl::Status rule_result = rule(node, sql);
        if ( !rule_result.ok() ) {
            // There may be multiple rule failures for now
            // only the last failure will be shown.
            result = rule_result;
        }
        return VisitResult::VisitChildren(node);
    }

    absl::Status getResult() { return result; }

 private:
    std::function
        <absl::Status(const zetasql::ASTNode*, absl::string_view)> rule;
    absl::string_view sql;
    absl::Status result;
};

//
class ASTNodeRule {
 public:
    explicit ASTNodeRule(const std::function
        <absl::Status(const zetasql::ASTNode*, absl::string_view)> _rule)
        : rule(_rule) {}
    absl::Status applyTo(absl::string_view sql);
 private:
    std::function
        <absl::Status(const zetasql::ASTNode*, absl::string_view)> rule;
};

// Debugger that will be erased later.
absl::Status printASTTree(absl::string_view sql);

// Checks if the number of characters in any line
// exceed a certain treshold
absl::Status checkLineLength(absl::string_view sql, int lineLimit = 100,
    const char delimeter = '\n');

// Checks whether given sql statement is a valid
// GoogleSql statement
absl::Status checkStatement(absl::string_view sql);

// Checks whether every statement ends with a semicolon ';'
absl::Status checkSemicolon(absl::string_view sql);

// Checks whether all keywords are uppercase
absl::Status checkUppercaseKeywords(absl::string_view sql);

// Check if comment style is uniform (either -- or //, not both)
absl::Status checkCommentType(absl::string_view sql);

// Checks whether all aliases denoted by 'AS' keyword
absl::Status checkAliasKeyword(absl::string_view sql);

}  // namespace linter
}  // namespace zetasql

#endif  // SRC_LINTER_H_
