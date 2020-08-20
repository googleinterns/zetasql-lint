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
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "src/config.pb.h"
#include "src/execute_linter.h"

ABSL_FLAG(std::string, config, "",
          "A prototxt file having configuration options.");

namespace zetasql::linter {
namespace {

Config ReadFromConfigFile(std::string filename) {
  Config config;
  // TODO(orhanuysal): Add prototxt parser here. Expected something
  // like: config = google::protobuf::ParseFromPrototxt(filename);
  return config;
}

}  // namespace
}  // namespace zetasql::linter

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: ./runner --config=<config_file> <file_names>\n"
              << std::endl;
    return 1;
  }

  std::vector<char*> sql_files = absl::ParseCommandLine(argc, argv);

  std::string config_file = absl::GetFlag(FLAGS_config);

  zetasql::linter::Config config =
      zetasql::linter::ReadFromConfigFile(config_file);

  std::vector<std::string> supported_extensions{".sql", ".sqlm", ".sqlp",
                                                ".sqlt", ".gsql"};

  std::string extension_str = absl::StrJoin(supported_extensions.begin(),
                                            supported_extensions.end(), ", ");

  bool runner = true;
  for (char* filename : sql_files) {
    // The first argument is 'runner'.
    if (runner) {
      runner = false;
      continue;
    }
    bool ok = false;
    std::string extension;
    for (int i = std::strlen(filename) - 1; i >= 0 && filename[i] != '.'; i--)
      extension = filename[i] + extension;
    extension = '.' + extension;
    for (std::string supported_extension : supported_extensions)
      if (supported_extension == extension) ok = true;
    if (!ok) {
      std::cout << "Ignoring " << filename << ";  not have a valid extension ("
                << extension_str << ")" << std::endl;
      continue;
    }
    std::ifstream file(filename);
    std::string str = "";
    for (std::string line; std::getline(file, line);) {
      str += line + "\n";
    }
    std::cout << filename << std::endl;
    // zetasql::linter::PrintASTTree(str);

    zetasql::linter::LinterResult result =
        zetasql::linter::RunChecks(absl::string_view(str), config, filename);

    result.PrintResult();
  }

  return 0;
}
