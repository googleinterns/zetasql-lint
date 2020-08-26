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

#ifndef SRC_CHECKS_LIST_H_
#define SRC_CHECKS_LIST_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "src/checks.h"
#include "src/linter_options.h"

namespace zetasql::linter {

// It is the general list of the linter checks. It can be used to
// verify if a place is controlling all of the checks and not missing any.
class ChecksList {
 public:
  // Getter function for the list
  const std::vector<
      std::function<LinterResult(absl::string_view, const LinterOptions&)>>
  GetList();

  // Add a linter check to the list
  void Add(std::function<LinterResult(absl::string_view, const LinterOptions&)>
               check);

 private:
  std::vector<
      std::function<LinterResult(absl::string_view, const LinterOptions&)>>
      list_;
};

// This function gives all Checks that are using ZetaSQL parser.
ChecksList GetParserDependantChecks();

// This function is the main function to get all the checks.
// Whenever a new check is added this should be
// the first place to update.
ChecksList GetAllChecks();

}  // namespace zetasql::linter

#endif  // SRC_CHECKS_LIST_H_
