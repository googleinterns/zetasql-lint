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
#include "src/checks.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "src/checks_util.h"
#include "src/lint_error.h"
#include "src/linter_options.h"
#include "zetasql/base/status.h"
#include "zetasql/base/status_macros.h"
#include "zetasql/base/statusor.h"
#include "zetasql/parser/parse_tree.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include "zetasql/parser/parser.h"
#include "zetasql/public/parse_helpers.h"
#include "zetasql/public/parse_location.h"
#include "zetasql/public/parse_resume_location.h"
#include "zetasql/public/parse_tokens.h"

// Implemented rules in the same order with
// rules in the documention in 'docs/checks.md'.
namespace zetasql::linter {

LinterResult CheckLineLength(absl::string_view sql,
                             const LinterOptions &option) {
  int line_size = 0;
  LinterResult result;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == option.LineDelimeter()) {
      if (line_size > option.LineLimit() &&
          !OneLineStatement(sql.substr(i - line_size, i))) {
        if (option.IsActive(ErrorCode::kLineLimit, i))
          result.Add(ErrorCode::kLineLimit, sql, i,
                     absl::StrCat("Lines should be <= ", option.LineLimit(),
                                  " characters long."));
      }
      line_size = 0;
    } else {
      ++line_size;
    }
  }
  return result;
}

LinterResult CheckSemicolon(absl::string_view sql,
                            const LinterOptions &option) {
  // If parser is not active from config this check won't work.
  if (!option.IsActive(ErrorCode::kParseFailed, -1)) return LinterResult();
  LinterResult result;
  std::unique_ptr<ParserOutput> output;

  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  bool is_the_end = false;

  while (!is_the_end) {
    absl::Status status = ParseNextScriptStatement(&location, ParserOptions(),
                                                   &output, &is_the_end);
    if (!status.ok()) return LinterResult();
    int location =
        output->statement()->GetParseLocationRange().end().GetByteOffset();
    while (location < sql.size() &&
           (sql[location] == ' ' || sql[location] == option.LineDelimeter()))
      location++;
    if (location >= sql.size() || sql[location] != ';') {
      if (option.IsActive(ErrorCode::kSemicolon, location))
        result.Add(ErrorCode::kSemicolon, sql, location,
                   "Each statement should end with a "
                   "semicolon ';'.");
    }
  }
  return result;
}

LinterResult CheckUppercaseKeywords(absl::string_view sql,
                                    const LinterOptions &option) {
  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  std::vector<ParseToken> parse_tokens;
  LinterResult result;

  absl::Status status =
      GetParseTokens(ParseTokenOptions(), &location, &parse_tokens);

  if (!status.ok()) return LinterResult(status);

  // Some special words in tokenizer is defined as "Keyword",
  // but linter shouldn't complain for them. The list can change easily, and
  // every word in the list should be uppercase.
  std::vector<std::string> nolint_words{"TIME", "OFFSET", "DATE", "TRUE",
                                        "FALSE"};

  // Keyword definition in tokenizer is very wide,
  // it include some special characters like ';', '*', etc.
  // Keyword Uppercase check will simply ignore characters
  // outside of english lowercase letters.
  for (auto &token : parse_tokens) {
    if (token.kind() == ParseToken::KEYWORD) {
      if (!ConsistentUppercaseLowercase(sql, token.GetLocationRange(),
                                        option)) {
        int position = token.GetLocationRange().start().GetByteOffset();
        bool nolint = false;
        for (auto word : nolint_words)
          if (token.GetSQL() == word) nolint = true;
        if (nolint) continue;
        if (option.IsActive(ErrorCode::kLetterCase, position))
          result.Add(
              ErrorCode::kLetterCase, sql, position,
              absl::StrCat("All keywords should be ",
                           option.UpperKeyword() ? "uppercase" : "lowercase"));
      }
    }
  }
  return result;
}

LinterResult CheckCommentType(absl::string_view sql,
                              const LinterOptions &option) {
  LinterResult result;
  bool dash_comment = false;
  bool slash_comment = false;
  bool hash_comment = false;
  absl::string_view first_type = "";

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (IgnoreStrings(sql, &i)) continue;
    if (IgnoreComments(sql, option, &i, false)) continue;
    absl::string_view type = "";
    if (i > 0 && sql[i - 1] == '-' && sql[i] == '-') type = "--";
    if (i > 0 && sql[i - 1] == '/' && sql[i] == '/') type = "//";
    if (sql[i] == '#') type = "#";

    if (type != "") {
      if (type == "--") {
        dash_comment = true;
      } else if (type == "//") {
        slash_comment = true;
      } else {
        hash_comment = true;
      }

      if (dash_comment + slash_comment + hash_comment == 1)
        first_type = type;
      else if (type != first_type &&
               option.IsActive(ErrorCode::kCommentStyle, i))
        result.Add(
            ErrorCode::kCommentStyle, sql, i,
            absl::StrCat("One line comments should be consistent, expected: ",
                         first_type, ", found: ", type));

      // Ignore the line.
      while (i < static_cast<int>(sql.size()) &&
             sql[i] != option.LineDelimeter()) {
        ++i;
      }
      continue;
    }
  }

  return result;
}

LinterResult CheckAliasKeyword(absl::string_view sql,
                               const LinterOptions &option) {
  // If parser is not active from config this check won't work.
  if (!option.IsActive(ErrorCode::kParseFailed, -1)) return LinterResult();
  return ASTNodeRule([](const ASTNode *node, const absl::string_view &sql,
                        const LinterOptions &option) -> LinterResult {
           LinterResult result;
           if (node->node_kind() == AST_ALIAS) {
             int position =
                 node->GetParseLocationRange().start().GetByteOffset();
             std::string name = ConvertToUppercase(GetNodeString(node, sql));
             if (name.substr(0, 2) != "AS") {
               if (option.IsActive(ErrorCode::kAlias, position))
                 result.Add(ErrorCode::kAlias, sql, position,
                            "Always use AS keyword before aliases");
             }
           }
           return result;
         })
      .ApplyTo(sql, option);
}

LinterResult CheckTabCharactersUniform(absl::string_view sql,
                                       const LinterOptions &option) {
  bool is_indent = true;
  const char kSpace = ' ', kTab = '\t';
  LinterResult result;

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == option.LineDelimeter()) {
      is_indent = true;
    } else if (is_indent && sql[i] != option.AllowedIndent()) {
      if (sql[i] == kTab || sql[i] == kSpace) {
        if (option.IsActive(ErrorCode::kUniformIndent, i))
          result.Add(
              ErrorCode::kUniformIndent, sql, i,
              absl::StrCat("Inconsistent use of indentation symbols, "
                           "expected: ",
                           (sql[i] == kTab ? "whitespace" : "tab character")));
      }
      is_indent = false;
    }
  }

  return result;
}

LinterResult CheckNoTabsBesidesIndentations(absl::string_view sql,
                                            const LinterOptions &option) {
  const char kSpace = ' ', kTab = '\t';

  bool is_indent = true;
  LinterResult result;

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == option.LineDelimeter()) {
      is_indent = true;
    } else if (sql[i] != kSpace && sql[i] != kTab) {
      is_indent = false;
    } else if (sql[i] == kTab && !is_indent) {
      if (option.IsActive(ErrorCode::kNotIndentTab, i))
        result.Add(ErrorCode::kNotIndentTab, sql, i,
                   "Tab is not in the indentation");
    }
  }
  return result;
}

LinterResult CheckSingleQuotes(absl::string_view sql,
                               const LinterOptions &option) {
  LinterResult result;

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (IgnoreComments(sql, option, &i)) continue;

    if (sql[i] == '\'' || sql[i] == '"') {
      if ((option.SingleQuote() && sql[i] == '"')) {
        result.Add(ErrorCode::kSingleQuote, sql, i,
                   "Use single quotes(') instead of double quotes(\")");
      } else if ((!option.SingleQuote() && sql[i] == '\'')) {
        result.Add(ErrorCode::kSingleQuote, sql, i,
                   "Use double quotes(\") instead of single quotes(')");
      }
      IgnoreStrings(sql, &i);
    }
  }
  return result;
}

LinterResult CheckNames(absl::string_view sql, const LinterOptions &option) {
  // If parser is not active from config this check won't work.
  if (!option.IsActive(ErrorCode::kParseFailed, -1)) return LinterResult();

  return ASTNodeRule([](const ASTNode *node, const absl::string_view &sql,
                        const LinterOptions &option) -> LinterResult {
           LinterResult result;
           if (node->node_kind() == AST_IDENTIFIER) {
             if (node->parent() == nullptr ||
                 node->parent()->parent() == nullptr)
               return result;
             const ASTNodeKind kind = node->parent()->parent()->node_kind();
             ASTNode *parent = node->parent();
             int position =
                 node->GetParseLocationRange().start().GetByteOffset();
             absl::string_view name = GetNodeString(node, sql);

             if (parent->node_kind() == AST_PATH_EXPRESSION)
               if (parent->child(parent->num_children() - 1) != node)
                 return result;

             if (kind == AST_CREATE_TABLE_STATEMENT) {
               if (!IsUpperCamelCase(name) &&
                   option.IsActive(ErrorCode::kTableName, position))
                 result.Add(ErrorCode::kTableName, sql, position,
                            "Table names or"
                            " table aliases should be UpperCamelCase.");

             } else if (kind == AST_WINDOW_CLAUSE) {
               if (!IsUpperCamelCase(name) &&
                   option.IsActive(ErrorCode::kWindowName, position))
                 result.Add(ErrorCode::kWindowName, sql, position,
                            "Window names should be UpperCamelCase.");

             } else if (kind == AST_FUNCTION_DECLARATION) {
               if (!IsUpperCamelCase(name) &&
                   option.IsActive(ErrorCode::kFunctionName, position))
                 result.Add(ErrorCode::kFunctionName, sql, position,
                            "Function names should be UpperCamelCase.");

             } else if (kind == AST_SIMPLE_TYPE) {
               if (!IsAllCaps(name) &&
                   option.IsActive(ErrorCode::kDataTypeName, position))
                 result.Add(ErrorCode::kDataTypeName, sql, position,
                            "Simple SQL data types should be all caps.");

             } else if (kind == AST_SELECT_COLUMN) {
               if (parent->node_kind() != AST_ALIAS) return result;
               if (!IsLowerSnakeCase(name) && !IsUpperCamelCase(name) &&
                   option.IsActive(ErrorCode::kColumnName, position))
                 result.Add(ErrorCode::kColumnName, sql, position,
                            "Column names should be lower_snake_case.");

             } else if (kind == AST_FUNCTION_PARAMETERS) {
               // For a function parameter child(0) is identifier, and child(1)
               // is the type.
               bool isTable = parent->child(1)->node_kind() == AST_TVF_SCHEMA;

               if (!isTable && !IsLowerSnakeCase(name) &&
                   option.IsActive(ErrorCode::kParameterName, position))
                 result.Add(ErrorCode::kParameterName, sql, position,
                            "Non-table function parameters should be "
                            "lower_snake_case.");

               if (isTable && !IsUpperCamelCase(name) &&
                   option.IsActive(ErrorCode::kParameterName, position))
                 result.Add(ErrorCode::kParameterName, sql, position,
                            "Table or proto function parameters should be "
                            "UpperCamelCase.");

             } else if (kind == AST_CREATE_CONSTANT_STATEMENT) {
               if (!IsCapsSnakeCase(name) &&
                   option.IsActive(ErrorCode::kConstantName, position))
                 result.Add(ErrorCode::kConstantName, sql, position,
                            "Constant names should be CAPS_SNAKE_CASE.");
             }
           }
           return result;
         })
      .ApplyTo(sql, option);
}

LinterResult CheckJoin(absl::string_view sql, const LinterOptions &option) {
  // If parser is not active from config this check won't work.
  if (!option.IsActive(ErrorCode::kParseFailed, -1)) return LinterResult();

  return ASTNodeRule([](const ASTNode *node, const absl::string_view &sql,
                        const LinterOptions &option) -> LinterResult {
           LinterResult result;
           // SingleNodeDebugString also returns the type if there are any.
           // If it is equal to normal kind string, this means join is typeless.
           if (node->node_kind() == AST_JOIN &&
               node->SingleNodeDebugString() == node->GetNodeKindString()) {
             result.Add(ErrorCode::kJoin, sql,
                        node->GetParseLocationRange().start().GetByteOffset(),
                        "Always explicitly indicate the type of join.");
           }
           return result;
         })
      .ApplyTo(sql, option);
}

LinterResult CheckImports(absl::string_view sql, const LinterOptions &option) {
  std::vector<std::string> imports;
  LinterResult result;
  int first_type = 0, second_type = 0;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (IgnoreStrings(sql, &i)) continue;
    if (IgnoreComments(sql, option, &i)) continue;

    if (i + 6 <= sql.size() && sql.substr(i, 6) == "IMPORT") {
      i += 6;
      std::string word = ConvertToUppercase(GetNextWord(sql, &i));
      if (word == "PROTO" || word == "MODULE") {
        int type = (word == "PROTO" ? 1 : 2);
        // Mixed check, There will be no PROTO-MODULE-PROTO
        // or MODULE-PROTO-MODULE.
        if (first_type == type && second_type != 0)
          result.Add(ErrorCode::kImport, sql, i,
                     "PROTO and MODULE inputs should be in seperate groups.");
        if (first_type == 0)
          first_type = type;
        else if (second_type == 0 && type != first_type)
          second_type = type;
        std::string name = GetNextWord(sql, &i);
        for (std::string prev_name : imports)
          if (prev_name == name) {
            result.Add(ErrorCode::kImport, sql, i,
                       absl::StrCat("\"", name, "\" is already defined."));
            break;
          }
        imports.push_back(name);
      } else {
        result.Add(ErrorCode::kImport, sql, i,
                   "Imports should specify the type 'MODULE' or 'PROTO'.");
      }
    }
  }
  return result;
}

LinterResult CheckExpressionParantheses(absl::string_view sql,
                                        const LinterOptions &option) {
  LinterResult result;
  return result;
}

LinterResult CheckCountStar(absl::string_view sql,
                            const LinterOptions &option) {
  LinterResult result;

  return result;
}

LinterResult CheckKeywordNamedIdentifier(absl::string_view sql,
                                         const LinterOptions &option) {
  LinterResult result;

  return result;
}

LinterResult CheckSpecifyTable(absl::string_view sql,
                               const LinterOptions &option) {
  LinterResult result;

  return result;
}

}  // namespace zetasql::linter