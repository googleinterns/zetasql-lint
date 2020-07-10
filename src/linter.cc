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
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"

namespace zetasql {

namespace linter {


absl::Status printASTTree(absl::string_view sql) {
    absl::Status return_status;
    std::unique_ptr<zetasql::ParserOutput> output;

    return_status = zetasql::ParseStatement(sql,
        zetasql::ParserOptions(), &output);

    std::cout << "Status for sql \" " << sql << "\" = "
        << return_status.ToString() << std::endl;

    if ( return_status.ok() ) {
        std::cout << output -> statement() -> DebugString() << std::endl;
    }

    return return_status;
}

absl::Status checkStatement(absl::string_view sql) {
    std::unique_ptr<zetasql::ParserOutput> output;
    return zetasql::ParseStatement(sql,
        zetasql::ParserOptions(), &output);
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

void KeywordExtractor::extract() {
    // TODO(orhan_uysal): Implement a keyword extractor
    // that pushes all keyword nodes to vector "keywords"
}

bool allUpperCase(const zetasql::ASTNode* x) {
    // TODO(orhan_uysal): Implement a function checking
    // if all characters in ASTNode(keyword node) is uppercase
    return true;
}

absl::Status checkUppercaseKeywords(absl::string_view sql) {
    std::vector<const zetasql::ASTNode *> keywords{};

    std::unique_ptr<zetasql::ParserOutput> output;

    absl::Status parser_status = zetasql::ParseStatement(sql,
            zetasql::ParserOptions(), &output);

    if ( !parser_status.ok() )
        return parser_status;
    // TODO(orhan_uysal): Implement KeywordExtractor.

    for (const zetasql::ASTNode *keyword : keywords) {
        if ( !allUpperCase(keyword) ) {
            return absl::Status(
                absl::StatusCode::kFailedPrecondition, "");
        }
    }

    return absl::OkStatus();
}

}  // namespace linter
}  // namespace zetasql
