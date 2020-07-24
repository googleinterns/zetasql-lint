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
#include <cctype>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "re2/re2.h"
#include "src/execute_linter.h"

int main(int argc, char* argv[]) {
  if (argc < 1) {
    std::cout << "Usage: runner < <file_name>\n" << std::endl;
    return 1;
  }
  std::string str;
  for (std::string line; std::getline(std::cin, line);) {
    str += line + "\n";
  }
  zetasql::linter::LinterResult result =
      zetasql::linter::RunChecks(absl::string_view(str));

  result.PrintResult();

  return 0;
}
