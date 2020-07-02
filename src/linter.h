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

#include "absl/strings/string_view.h"
#include "zetasql/base/status.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"

namespace zetasql {

namespace linter {

class KeywordExtractor {
 public:
    KeywordExtractor(const zetasql::ASTNode *_node,
                     std::vector<const zetasql::ASTNode *> *_keywords)
        : node(_node),
          keywords(_keywords) {}
    void extract();

 private:
    const zetasql::ASTNode *node;
    std::vector<const zetasql::ASTNode *> *keywords;
};

// Checks whether given sql statement is a valid
// GoogleSql statement
absl::Status checkStatement(absl::string_view sql);

// Checks if the number of characters in any line
// exceed a certain treshold
absl::Status checkLineLength(absl::string_view sql, int lineLimit = 100,
    const char delimeter = '\n');

}  // namespace linter
}  // namespace zetasql

#endif  // SRC_LINTER_H_
