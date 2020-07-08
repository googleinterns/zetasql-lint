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
#include "zetasql/base/status.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_tokens.h"
#include "zetasql/public/parse_resume_location.h"

// Implemented rules in the same order with rules in the documention
namespace zetasql {

namespace linter {

absl::Status printASTTree(absl::string_view sql) {
    absl::Status return_status;
    std::unique_ptr<zetasql::ParserOutput> output;

    zetasql::ParseResumeLocation location =
        zetasql::ParseResumeLocation::FromStringView(sql);
    bool isTheEnd = false;
    int cnt = 0;
    while ( !isTheEnd ) {
        return_status = zetasql::ParseNextScriptStatement(
            &location, zetasql::ParserOptions(), &output, &isTheEnd);

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

absl::Status checkLineLength(absl::string_view sql, int lineLimit,
    const char delimeter) {
    int lineSize = 0;
    int lineNumber = 1;
    for (int i=0; i<static_cast<int>(sql.size()); i++) {
        if ( sql[i] == delimeter ) {
            lineSize = 0;
            lineNumber++;
        } else {
            lineSize++;
        }
        if ( lineSize > lineLimit ) {
            return absl::Status(
                absl::StatusCode::kFailedPrecondition,
                absl::StrCat("Lines should be <= ", std::to_string(lineLimit),
                " characters long [", std::to_string(lineNumber), ",1]") );
        }
    }
    return absl::OkStatus();
}

absl::Status checkStatement(absl::string_view sql) {
    absl::Status return_status = absl::OkStatus();
    std::unique_ptr<zetasql::ParserOutput> output;

    zetasql::ParseResumeLocation location =
        zetasql::ParseResumeLocation::FromStringView(sql);
    bool isTheEnd = false;
    while ( !isTheEnd && return_status.ok() ) {
        return_status = zetasql::ParseNextScriptStatement(
            &location, zetasql::ParserOptions(), &output, &isTheEnd);
    }
    return return_status;
}

absl::Status checkSemicolon(absl::string_view sql) {
    absl::Status return_status = absl::OkStatus();
    std::unique_ptr<zetasql::ParserOutput> output;

    zetasql::ParseResumeLocation location =
        zetasql::ParseResumeLocation::FromStringView(sql);
    bool isTheEnd = false;
    while ( !isTheEnd && return_status.ok() ) {
        return_status = zetasql::ParseNextScriptStatement(
            &location, zetasql::ParserOptions(), &output, &isTheEnd);
        if ( !isTheEnd && return_status.ok() ) {
            int endLocation = output -> statement() -> GetParseLocationRange()
                .end().GetByteOffset();

            if ( endLocation < sql.size() && sql[endLocation] != ';' ) {
                return absl::Status(
                        absl::StatusCode::kFailedPrecondition,
                        absl::StrCat(
                            "Each statemnt should end with a consequtive",
                            "semicolon ';'"));
            }
        }
    }
    return return_status;
}

bool allUpperCase(const absl::string_view &sql,
    const zetasql::ParseLocationRange &range) {
    for (int i = range.start().GetByteOffset();
                i < range.end().GetByteOffset(); i++) {
        if ( 'a' <= sql[i] && sql[i] <= 'z' )
            return false;
    }
    return true;
}

absl::Status checkUppercaseKeywords(absl::string_view sql) {
    ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
    std::vector<zetasql::ParseToken> parse_tokens;
    absl::Status tokenizer_status =
        GetParseTokens(ParseTokenOptions(), &location, &parse_tokens);

    // Keyword definition in tokenizer is very wide,
    // it include some special characters like ';', '*', etc.
    // Keyword Uppercase check will simply ignore characters
    // outside of english lowercase letters.
    for ( auto &token : parse_tokens ) {
      if ( token.kind() == zetasql::ParseToken::KEYWORD ) {
        if (!allUpperCase(sql, token.GetLocationRange())) {
          return absl::Status(
            absl::StatusCode::kFailedPrecondition,
            absl::StrCat("All keywords should be Uppercase, In character ",
            std::to_string(token.GetLocationRange().start().GetByteOffset()),
            " string should be: ", token.GetSQL()));
        }
      }
    }

    return absl::OkStatus();
}

absl::Status checkCommentType(absl::string_view sql) {
    bool includesType1 = false;
    bool includesType2 = false;
    bool insideString = false;
    for (int i = 1; i<static_cast<int>(sql.size()); i++) {
        if (!insideString && sql[i-1] == '-' && sql[i] == '-')
            includesType1 = true;
        if (!insideString && sql[i-1] == '/' && sql[i] == '/')
            includesType2 = true;

        if (sql[i] == '\'' || sql[i] == '"')
            insideString = !insideString;
    }
    if ( includesType1 && includesType2 )
        return absl::Status(
                absl::StatusCode::kFailedPrecondition,
                absl::StrCat("either '//' or '--' should be used to ",
                            "specify a comment"));
    return absl::OkStatus();
}

absl::Status ASTNodeRule::applyTo(absl::string_view sql) {
    RuleVisitor visitor(rule, sql);
    absl::Status return_status = absl::OkStatus();
    std::unique_ptr<zetasql::ParserOutput> output;

    zetasql::ParseResumeLocation location =
        zetasql::ParseResumeLocation::FromStringView(sql);
    bool isTheEnd = false;
    while ( !isTheEnd && return_status.ok() ) {
        return_status = zetasql::ParseNextScriptStatement(
            &location, zetasql::ParserOptions(), &output, &isTheEnd);
        if ( return_status.ok() ) {
            return_status = output->statement()->TraverseNonRecursive(&visitor);
        }
    }
    if ( return_status.ok() ) {
        return_status = visitor.getResult();
    }
    return return_status;
}

absl::Status checkAliasKeyword(absl::string_view sql) {
    return ASTNodeRule([](const zetasql::ASTNode* node,
        absl::string_view sql) -> absl::Status {
        if (node->node_kind() == zetasql::AST_ALIAS) {
            int position = node->GetParseLocationRange()
                .start().GetByteOffset();
            if ( sql[position] != 'A' || sql[position+1] != 'S' ) {
                return absl::Status(
                    absl::StatusCode::kFailedPrecondition,
                    absl::StrCat(
                    "Always use AS keyword for referencing aliases, ",
                    "In position: ", std::to_string(position)));
            }
        }
        return absl::OkStatus();
    }).applyTo(sql);
}

}  // namespace linter
}  // namespace zetasql
