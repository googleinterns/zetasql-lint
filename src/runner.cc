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
#include "google/protobuf/text_format.h"
#include "src/config.pb.h"
#include "src/linter.h"

ABSL_FLAG(std::string, config, "",
          "A prototxt file having configuration options.");

ABSL_FLAG(bool, quick, false,
          "Read from standart input. It will read one"
          "statement and continue until reading semicolon ';'");

ABSL_FLAG(bool, parsed_ast, false, "Print parsed AST for the input queries.");

namespace zetasql::linter {
namespace {

std::string ReadFile(std::string filename) {
  std::ifstream file(filename.c_str());
  std::string str = "";
  for (std::string line; std::getline(file, line);) {
    str += line + "\n";
  }
  return str;
}

Config ReadFromConfigFile(std::string filename) {
  Config config;
  std::string str = ReadFile(filename);
  if (!google::protobuf::TextFormat::ParseFromString(str, &config)) {
    std::cerr << "Configuration file couldn't be parsed." << std::endl;
    config = Config();
  }
  return config;
}

bool HasValidExtension(std::string filename) {
  std::vector<std::string> supported_extensions{".sql", ".sqlm", ".sqlp",
                                                ".sqlt", ".gsql"};

  std::string extension_str = absl::StrJoin(supported_extensions.begin(),
                                            supported_extensions.end(), ", ");

  bool ok = false;
  std::string extension = "";
  // Find until last '.';
  for (int i = filename.size() - 1; i >= 0; i--) {
    extension = filename[i] + extension;
    if (filename[i] == '.') break;
  }

  for (std::string supported_extension : supported_extensions)
    if (supported_extension == extension) ok = true;
  if (!ok) {
    std::cerr << "Ignoring " << filename << ";  not have a valid extension ("
              << extension_str << ")" << std::endl;
    return 0;
  }
  return 1;
}

void quick_run(Config config) {
  std::string str = "";
  for (std::string line; std::getline(std::cin, line);) {
    bool end = false;
    for (char c : line) {
      str += c;
      if (c == ';') {
        end = true;
        break;
      }
    }
    str += "\n";
    if (end) break;
  }
  zetasql::linter::LinterResult result =
      zetasql::linter::RunChecks(absl::string_view(str), config, "");

  result.PrintResult();
}

void run(std::vector<std::string> sql_files, Config config) {
  bool debug = absl::GetFlag(FLAGS_parsed_ast);
  bool runner = true;
  for (std::string filename : sql_files) {
    // The first argument is './runner'.
    if (runner || !HasValidExtension(filename)) {
      runner = false;
      continue;
    }
    std::string str = ReadFile(filename);
    if (debug) PrintASTTree(str);
    LinterResult result = RunChecks(absl::string_view(str), config, filename);

    result.PrintResult();
  }
}

}  // namespace
}  // namespace zetasql::linter

class B {
 public:
 private:
 int a;
  std::vector<std::unique_ptr<bool>> v;
};

void ff(const B& a) {}

int main(int argc, char* argv[]) {
  B a;
  
  if (argc < 2) {
    std::cerr << "Usage: ./runner --config=<config_file> <file_names>\n"
              << std::endl;
    return 1;
  }

  std::vector<std::string> sql_files;
  for (char* file : absl::ParseCommandLine(argc, argv))
    sql_files.push_back(std::string(file));
  std::string config_file = absl::GetFlag(FLAGS_config);
  bool quick = absl::GetFlag(FLAGS_quick);

  zetasql::linter::Config config =
      zetasql::linter::ReadFromConfigFile(config_file);

  if (quick)
    zetasql::linter::quick_run(config);
  else
    zetasql::linter::run(sql_files, config);

  return 0;
}
