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
#include "src/linter.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/strings/str_cat.h"
#include "src/lint_errors.h"
#include "zetasql/base/status.h"
#include "zetasql/base/status_macros.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_tokens.h"
#include "zetasql/public/parse_resume_location.h"

// Implemented rules in the same order with rules in the documention.
namespace zetasql::linter {

// This will eventually be erased.
absl::Status PrintASTTree(absl::string_view sql, LinterResult* result) {
    absl::Status return_status;
    std::unique_ptr<ParserOutput> output;

    ParseResumeLocation location =
        ParseResumeLocation::FromStringView(sql);
    bool is_the_end = false;
    int cnt = 0;
    while ( !is_the_end ) {
        return_status = ParseNextScriptStatement(
            &location, ParserOptions(), &output, &is_the_end);

        std::cout << "Status for sql#" << ++cnt << ": \"" << sql << "\" = "
            << return_status.ToString() << std::endl;

        if ( return_status.ok() ) {
            std::cout << output -> statement() -> DebugString() << std::endl;
        } else {
            break;
        }
    }
    return return_status;
}

absl::Status CheckLineLength(absl::string_view sql,  LinterResult* result,
    int line_limit, const char delimeter) {
    int lineSize = 0;
    int line_number = 1;
    for (int i=0; i<static_cast<int>(sql.size()); ++i) {
        if ( sql[i] == delimeter ) {
            lineSize = 0;
            ++line_number;
        } else {
            ++lineSize;
        }
        if ( lineSize > line_limit ) {
            // TODO(orhanuysal): add proper error handling.
            result->Add(ErrorCode::kLineLimit, sql, i);
        }
    }
    return absl::OkStatus();
}

absl::Status CheckParserSucceeds(absl::string_view sql, LinterResult* result) {
    std::unique_ptr<ParserOutput> output;

    ParseResumeLocation location =
        ParseResumeLocation::FromStringView(sql);

    bool is_the_end = false;
    int byte_position = 1;
    while ( !is_the_end ) {
        byte_position = location.byte_position();
        absl::Status status = ParseNextScriptStatement(
            &location, ParserOptions(), &output, &is_the_end);
        if ( !status.ok() ) {
            // TODO(orhanuysal): Implement a token parser to seperate statements
            // currently, when parser fails, it is unable to determine
            // the end of the statement.
            result -> Add(ErrorCode::kParseFailed, sql, byte_position);
            break;
        }
    }
    return absl::OkStatus();
}

absl::Status CheckSemicolon(absl::string_view sql, LinterResult* result) {
    std::unique_ptr<ParserOutput> output;

    ParseResumeLocation location =
        ParseResumeLocation::FromStringView(sql);
    bool is_the_end = false;

    while ( !is_the_end ) {
        ZETASQL_RETURN_IF_ERROR(ParseNextScriptStatement(
            &location, ParserOptions(), &output, &is_the_end));

        int location = output -> statement() -> GetParseLocationRange()
            .end().GetByteOffset();

        if ( location >= sql.size() || sql[location] != ';' ) {
            result->Add(ErrorCode::kSemicolon, sql, location-1);
        }
    }
    return absl::OkStatus();
}

bool ConsistentUppercaseLowercase(const absl::string_view &sql,
    const ParseLocationRange &range) {
    bool uppercase = false;
    bool lowercase = false;
    for (int i = range.start().GetByteOffset();
                i < range.end().GetByteOffset(); ++i) {
        if ( 'a' <= sql[i] && sql[i] <= 'z' )
            lowercase = true;
        if ( 'A' <= sql[i] && sql[i] <= 'Z' )
            uppercase = true;
    }
    // There shouldn't be any case any Keyword
    // contains both uppercase and lowercase characters
    return !(lowercase && uppercase);
}

absl::Status CheckUppercaseKeywords(absl::string_view sql,
                                    LinterResult* result) {
    ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
    std::vector<ParseToken> parse_tokens;

    ZETASQL_RETURN_IF_ERROR(
        GetParseTokens(ParseTokenOptions(), &location, &parse_tokens));

    // Keyword definition in tokenizer is very wide,
    // it include some special characters like ';', '*', etc.
    // Keyword Uppercase check will simply ignore characters
    // outside of english lowercase letters.
    for ( auto &token : parse_tokens ) {
      if ( token.kind() == ParseToken::KEYWORD ) {
        if (!ConsistentUppercaseLowercase(sql, token.GetLocationRange())) {
          result->Add(ErrorCode::kUppercase, sql,
            token.GetLocationRange().start().GetByteOffset());
        }
      }
    }

    return absl::OkStatus();
}

absl::Status CheckCommentType(absl::string_view sql, LinterResult* result) {
    bool includes_type1 = false;
    bool includes_type2 = false;
    bool includes_type3 = false;
    bool inside_string = false;
    int location = 0;

    for (int i = 1; i<static_cast<int>(sql.size()); ++i) {
        if (!inside_string && sql[i-1] == '-' && sql[i] == '-') {
            includes_type1 = true;
            location = i;
            // ignore the line.
            while ( i < static_cast<int>(sql.size()) &&
                    sql[i] != '\n' ) {
                ++i;
            }
        }

        if (!inside_string && sql[i-1] == '/' && sql[i] == '/') {
            includes_type2 = true;
            location = i;
            // ignore the line.
            while ( i < static_cast<int>(sql.size()) &&
                    sql[i] != '\n' ) {
                ++i;
            }
        }

        if (!inside_string && sql[i] == '#') {
            includes_type3 = true;
            location = i;
            // ignore the line.
            while ( i < static_cast<int>(sql.size()) &&
                    sql[i] != '\n' ) {
                ++i;
            }
        }

        // ignore multiline comments.
        if (!inside_string && sql[i-1] == '/' && sql[i] == '*') {
            // it will start checking after '/*' and after the iteration
            // finished, the pointer 'i' will be just after '*/' (incrementation
            // from the for statement is included).
            i += 2;
            while ( i < static_cast<int>(sql.size()) &&
                    !(sql[i-1] == '*' && sql[i] == '/') ) {
                ++i;
            }
        }

        if (sql[i] == '\'' || sql[i] == '"')
            inside_string = !inside_string;
    }

    if ( includes_type1 + includes_type2 + includes_type3 > 1 ) {
        result->Add(ErrorCode::kCommentStyle, sql, location);
    }
    return absl::OkStatus();
}

absl::Status ASTNodeRule::ApplyTo(absl::string_view sql) {
    RuleVisitor visitor(rule_, error_code_, sql, result_);

    std::unique_ptr<ParserOutput> output;
    ParseResumeLocation location =
        ParseResumeLocation::FromStringView(sql);

    bool is_the_end = false;
    while (!is_the_end) {
        ZETASQL_RETURN_IF_ERROR(ParseNextScriptStatement(
            &location, ParserOptions(), &output, &is_the_end));
        ZETASQL_RETURN_IF_ERROR(
            output->statement()->TraverseNonRecursive(&visitor));
    }

    return absl::OkStatus();
}

absl::Status CheckAliasKeyword(absl::string_view sql, LinterResult* result) {
    return ASTNodeRule(result, kAlias, [](const ASTNode* node,
        absl::string_view sql) -> absl::Status {
        if (node->node_kind() == AST_ALIAS) {
            int position = node->GetParseLocationRange()
                .start().GetByteOffset();
            if ( sql[position] != 'A' || sql[position+1] != 'S' ) {
                return absl::Status(absl::StatusCode::kFailedPrecondition, "");
            }
        }
        return absl::OkStatus();
    }).ApplyTo(sql);
}

zetasql_base::StatusOr<VisitResult> RuleVisitor::defaultVisit(
        const ASTNode* node) {
    absl::Status rule_result = rule_(node, sql_);
    if ( !rule_result.ok() ) {
        result_->Add(error_code_, sql_,
            node->GetParseLocationRange().start().GetByteOffset());
    }
    return VisitResult::VisitChildren(node);
}

}  // namespace zetasql::linter
