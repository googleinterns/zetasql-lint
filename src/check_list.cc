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
#include "src/check_list.h"

#include "absl/strings/string_view.h"
#include "src/linter.h"
#include "src/linter_options.h"

namespace zetasql::linter {

const std::vector<
    std::function<LinterResult(absl::string_view, const LinterOptions&)>>
CheckList::GetList() {
  return list_;
}

void CheckList::Add(
    std::function<LinterResult(absl::string_view, const LinterOptions&)>
        check) {
  list_.push_back(check);
}

CheckList GetParserDependantChecks() {
  CheckList list;
  list.Add(CheckSemicolon);
  list.Add(CheckAliasKeyword);
  list.Add(CheckNames);
  list.Add(CheckJoin);
  return list;
}

CheckList GetAllChecks() {
  CheckList list;
  list.Add(CheckLineLength);
  list.Add(CheckParserSucceeds);
  list.Add(CheckSemicolon);
  list.Add(CheckUppercaseKeywords);
  list.Add(CheckCommentType);
  list.Add(CheckAliasKeyword);
  list.Add(CheckTabCharactersUniform);
  list.Add(CheckNoTabsBesidesIndentations);
  list.Add(CheckSingleQuotes);
  list.Add(CheckNames);
  list.Add(CheckJoin);
  list.Add(CheckImports);
  return list;
}

}  // namespace zetasql::linter
