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
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "src/lint_errors.h"
#include "src/linter.h"
#include "zetasql/base/status.h"
#include "zetasql/base/status_macros.h"

namespace zetasql::linter {

namespace {

// It runs all specified checks
// "LinterOptions" parameter will be added in future.
absl::Status RunChecks(absl::string_view sql) {
  LinterResult result, output;
  // ZETASQL_RETURN_IF_ERROR(CheckLineLength(sql, &output));
  // result.Add(output);
  // output.clear();
  // ZETASQL_RETURN_IF_ERROR(CheckParserSucceeds(sql, &output));
  // result.Add(output);
  // output.clear();
  // ZETASQL_RETURN_IF_ERROR(CheckSemicolon(sql, &output));
  // result.Add(output);
  // output.clear();
  // ZETASQL_RETURN_IF_ERROR(CheckUppercaseKeywords(sql, &output));
  // result.Add(output);
  // output.clear();
  // ZETASQL_RETURN_IF_ERROR(CheckCommentType(sql, &output));
  // result.Add(output);
  // output.clear();
  // ZETASQL_RETURN_IF_ERROR(CheckAliasKeyword(sql, &output));
  // result.Add(output);
  std::cout << "Linter completed running." << std::endl;
  result.PrintResult();
  return absl::OkStatus();
}

}  // namespace
}  // namespace zetasql::linter

int main(int argc, char* argv[]) {
  if (argc <= 2) {
    std::cout << "Usage: execute_linter < <file_name>\n" << std::endl;
  }
  char* filename = argv[argc - 1];

  std::cout << filename << std::endl;

  std::ifstream t(filename);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  absl::string_view sql(str);

  absl::Status status = zetasql::linter::RunChecks(sql);

  if (status.ok()) {
    return 0;
  } else {
    std::cerr << "ERROR: " << status << std::endl;
    return 1;
  }
}
