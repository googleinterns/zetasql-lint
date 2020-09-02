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

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/match.h"
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
                             const LinterOptions &options) {
  int line_size = 0;
  LinterResult result;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == options.LineDelimeter()) {
      if (line_size > options.LineLimit() &&
          !OneLineStatement(sql.substr(i - line_size, i))) {
        if (options.IsActive(ErrorCode::kLineLimit, i))
          result.Add(ErrorCode::kLineLimit, sql, i,
                     absl::StrCat("Lines should be <= ", options.LineLimit(),
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
                            const LinterOptions &options) {
  LinterResult result;
  bool last = false;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (IgnoreStrings(sql, &i)) continue;
    if (IgnoreComments(sql, options, &i)) continue;
    if (sql[i] == ' ' || sql[i] == '\t' || sql[i] == options.LineDelimeter())
      continue;
    if (sql[i] == ';') {
      last = 1;
    } else {
      last = 0;
    }
  }
  if (!last)
    if (options.IsActive(ErrorCode::kSemicolon, sql.size() - 1))
      result.Add(ErrorCode::kSemicolon, sql, sql.size() - 1,
                 "Each statement should end with a "
                 "semicolon ';'.");
  return result;
}

LinterResult CheckUppercaseKeywords(absl::string_view sql,
                                    const LinterOptions &options) {
  std::vector<ParseToken> keywords = GetKeywords(sql, ErrorCode::kLetterCase);
  std::vector<const ASTNode *> identifiers = GetIdentifiers(sql, options);
  LinterResult result;
  int index = 0;
  for (auto &token : keywords) {
    // Two pointer algorithm to reduce complexity O(N^2) to O(N)
    while (index < identifiers.size() && IsBefore(identifiers[index], token))
      index++;

    // Ignore the keyword token if it is an identifier.
    if (index < identifiers.size() && IsTheSame(identifiers[index], token))
      continue;

    if (!ConsistentUppercaseLowercase(sql, token.GetLocationRange(), options)) {
      int position = token.GetLocationRange().start().GetByteOffset();
      if (options.IsActive(ErrorCode::kLetterCase, position))
        result.Add(
            ErrorCode::kLetterCase, sql, position,
            absl::StrCat("Keyword '", token.GetImage(), "' should be all ",
                         options.UpperKeyword() ? "uppercase" : "lowercase"));
    }
  }
  return result;
}

LinterResult CheckCommentType(absl::string_view sql,
                              const LinterOptions &options) {
  LinterResult result;
  bool dash_comment = false;
  bool slash_comment = false;
  bool hash_comment = false;
  absl::string_view first_type = "";

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (IgnoreStrings(sql, &i)) continue;
    if (IgnoreComments(sql, options, &i, false)) continue;
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
               options.IsActive(ErrorCode::kCommentStyle, i))
        result.Add(
            ErrorCode::kCommentStyle, sql, i,
            absl::StrCat("One line comments should be consistent, expected: ",
                         first_type, ", found: ", type));

      // Ignore the line.
      while (i < static_cast<int>(sql.size()) &&
             sql[i] != options.LineDelimeter()) {
        ++i;
      }
      continue;
    }
  }

  return result;
}

LinterResult CheckAliasKeyword(absl::string_view sql,
                               const LinterOptions &options) {
  // If parser is not active from config this check won't work.
  if (!options.IsActive(ErrorCode::kParseFailed, -1)) return LinterResult();
  return ASTNodeRule([](const ASTNode *node, const absl::string_view &sql,
                        const LinterOptions &options) -> LinterResult {
           LinterResult result;
           if (node->node_kind() == AST_ALIAS) {
             int position = GetStartPosition(*node);
             const std::string name =
                 ConvertToUppercase(GetNodeString(node, sql));
             if (name.size() < 2 || !absl::StartsWith(name, "AS")) {
               if (options.IsActive(ErrorCode::kAlias, position))
                 result.Add(ErrorCode::kAlias, sql, position,
                            "Always use AS keyword before aliases");
             }
           }
           return result;
         })
      .ApplyTo(sql, options);
}

LinterResult CheckTabCharactersUniform(absl::string_view sql,
                                       const LinterOptions &options) {
  bool is_indent = true;
  const char kSpace = ' ', kTab = '\t';
  LinterResult result;

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == options.LineDelimeter()) {
      is_indent = true;
    } else if (is_indent && sql[i] != options.AllowedIndent()) {
      if (sql[i] == kTab || sql[i] == kSpace) {
        if (options.IsActive(ErrorCode::kUniformIndent, i))
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
                                            const LinterOptions &options) {
  const char kSpace = ' ', kTab = '\t';

  bool is_indent = true;
  LinterResult result;

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == options.LineDelimeter()) {
      is_indent = true;
    } else if (sql[i] != kSpace && sql[i] != kTab) {
      is_indent = false;
    } else if (sql[i] == kTab && !is_indent) {
      if (options.IsActive(ErrorCode::kNotIndentTab, i))
        result.Add(ErrorCode::kNotIndentTab, sql, i,
                   "Tab is not in the indentation");
    }
  }
  return result;
}

LinterResult CheckSingleQuotes(absl::string_view sql,
                               const LinterOptions &options) {
  LinterResult result;

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (IgnoreComments(sql, options, &i)) continue;

    if (sql[i] == '\'' || sql[i] == '"') {
      if ((options.SingleQuote() && sql[i] == '"')) {
        result.Add(ErrorCode::kSingleQuote, sql, i,
                   "Use single quotes(') instead of double quotes(\")");
      } else if ((!options.SingleQuote() && sql[i] == '\'')) {
        result.Add(ErrorCode::kSingleQuote, sql, i,
                   "Use double quotes(\") instead of single quotes(')");
      }
      IgnoreStrings(sql, &i);
    }
  }
  return result;
}

LinterResult CheckNames(absl::string_view sql, const LinterOptions &options) {
  // If parser is not active from config this check won't work.
  if (!options.IsActive(ErrorCode::kParseFailed, -1)) return LinterResult();

  return ASTNodeRule([](const ASTNode *node, const absl::string_view &sql,
                        const LinterOptions &options) -> LinterResult {
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
                   options.IsActive(ErrorCode::kTableName, position))
                 result.Add(ErrorCode::kTableName, sql, position,
                            "Table names or"
                            " table aliases should be UpperCamelCase.");

             } else if (kind == AST_WINDOW_CLAUSE) {
               if (!IsUpperCamelCase(name) &&
                   options.IsActive(ErrorCode::kWindowName, position))
                 result.Add(ErrorCode::kWindowName, sql, position,
                            "Window names should be UpperCamelCase.");

             } else if (kind == AST_FUNCTION_DECLARATION) {
               if (!IsUpperCamelCase(name) &&
                   options.IsActive(ErrorCode::kFunctionName, position))
                 result.Add(ErrorCode::kFunctionName, sql, position,
                            "Function names should be UpperCamelCase.");

             } else if (kind == AST_SIMPLE_TYPE) {
               if (!IsAllCaps(name) &&
                   options.IsActive(ErrorCode::kDataTypeName, position))
                 result.Add(ErrorCode::kDataTypeName, sql, position,
                            "Simple SQL data types should be all caps.");

             } else if (kind == AST_SELECT_COLUMN) {
               if (parent->node_kind() != AST_ALIAS) return result;
               if (!IsLowerSnakeCase(name) && !IsUpperCamelCase(name) &&
                   options.IsActive(ErrorCode::kColumnName, position))
                 result.Add(ErrorCode::kColumnName, sql, position,
                            "Column names should be lower_snake_case.");

             } else if (kind == AST_FUNCTION_PARAMETERS) {
               // For a function parameter child(0) is identifier, and child(1)
               // is the type.
               bool isTable = parent->child(1)->node_kind() == AST_TVF_SCHEMA;

               if (!isTable && !IsLowerSnakeCase(name) &&
                   options.IsActive(ErrorCode::kParameterName, position))
                 result.Add(ErrorCode::kParameterName, sql, position,
                            "Non-table function parameters should be "
                            "lower_snake_case.");

               if (isTable && !IsUpperCamelCase(name) &&
                   options.IsActive(ErrorCode::kParameterName, position))
                 result.Add(ErrorCode::kParameterName, sql, position,
                            "Table or proto function parameters should be "
                            "UpperCamelCase.");

             } else if (kind == AST_CREATE_CONSTANT_STATEMENT) {
               if (!IsCapsSnakeCase(name) &&
                   options.IsActive(ErrorCode::kConstantName, position))
                 result.Add(ErrorCode::kConstantName, sql, position,
                            "Constant names should be CAPS_SNAKE_CASE.");
             }
           }
           return result;
         })
      .ApplyTo(sql, options);
}

LinterResult CheckJoin(absl::string_view sql, const LinterOptions &options) {
  // If parser is not active from config this check won't work.
  if (!options.IsActive(ErrorCode::kParseFailed, -1)) return LinterResult();

  return ASTNodeRule([](const ASTNode *node, const absl::string_view &sql,
                        const LinterOptions &options) -> LinterResult {
           LinterResult result;
           // SingleNodeDebugString also returns the type if there are any.
           // If it is equal to normal kind string, this means join is typeless.
           int position = GetStartPosition(*node);
           if (node->node_kind() == AST_JOIN &&
               options.IsActive(ErrorCode::kImport, position) &&
               node->SingleNodeDebugString() == node->GetNodeKindString()) {
             result.Add(ErrorCode::kJoin, sql, position,
                        "Always explicitly indicate the type of join.");
           }
           return result;
         })
      .ApplyTo(sql, options);
}

LinterResult CheckImports(absl::string_view sql, const LinterOptions &options) {
  std::vector<std::string> imports;
  LinterResult result;
  int first_type = 0, second_type = 0;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (IgnoreStrings(sql, &i)) continue;
    if (IgnoreComments(sql, options, &i)) continue;

    if (i + 6 <= sql.size() && sql.substr(i, 6) == "IMPORT") {
      if (!options.IsActive(ErrorCode::kImport, i)) continue;
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
                                        const LinterOptions &options) {
  return ASTNodeRule([](const ASTNode *node, const absl::string_view &sql,
                        const LinterOptions &options) -> LinterResult {
           LinterResult result;
           if (node->node_kind() == AST_OR_EXPR ||
               node->node_kind() == AST_AND_EXPR) {
             if (node->parent() == nullptr) return result;
             const ASTNode *parent = node->parent();

             if (parent->node_kind() == AST_OR_EXPR ||
                 parent->node_kind() == AST_AND_EXPR) {
               if (parent->node_kind() == node->node_kind()) return result;
               int pos = node->GetParseLocationRange().start().GetByteOffset();
               int start = pos - 1;
               int end = node->GetParseLocationRange().end().GetByteOffset();

               if (IgnoreSpacesBackward(sql, &start) ||
                   IgnoreSpacesForward(sql, &end) || sql[start] != '(' ||
                   sql[end] != ')')
                 if (options.IsActive(ErrorCode::kExpressionParanteses, pos))
                   result.Add(ErrorCode::kExpressionParanteses, sql, pos,
                              "Use parantheses between consequtive AND and OR "
                              "operators");
             }
           }
           return result;
         })
      .ApplyTo(sql, options);
}

LinterResult CheckCountStar(absl::string_view sql,
                            const LinterOptions &options) {
  LinterResult result;
  for (int i = 0; i < static_cast<int>(sql.size()); i++) {
    IgnoreComments(sql, options, &i);
    IgnoreStrings(sql, &i);
    if (i + 5 < static_cast<int>(sql.size()) &&
        absl::EqualsIgnoreCase(sql.substr(i, 5), "COUNT")) {
      i += 5;
      if (IgnoreSpacesForward(sql, &i)) continue;
      if (sql[i] != '(') continue;
      i++;
      if (IgnoreSpacesForward(sql, &i)) continue;
      if (sql[i] != '1') continue;
      i++;
      if (IgnoreSpacesForward(sql, &i)) continue;
      if (sql[i] != ')') continue;

      if (options.IsActive(ErrorCode::kCountStar, i))
        result.Add(ErrorCode::kCountStar, sql, i,
                   "Use COUNT(*) instead of COUNT(1)");
    }
  }
  return result;
}

LinterResult CheckKeywordNamedIdentifier(absl::string_view sql,
                                         const LinterOptions &options) {
  LinterResult result;
  std::vector<ParseToken> keywords =
      GetKeywords(sql, ErrorCode::kKeywordIdentifier);
  std::vector<const ASTNode *> identifiers = GetIdentifiers(sql, options);
  int index = 0;
  for (auto &token : keywords) {
    // Two pointer algorithm to reduce complexity O(N^2) to O(N)
    while (index < identifiers.size() && IsBefore(identifiers[index], token))
      index++;

    // The Identifier is also a keyword
    if (index < identifiers.size() && IsTheSame(identifiers[index], token)) {
      int position = token.GetLocationRange().start().GetByteOffset();
      if (options.IsActive(ErrorCode::kKeywordIdentifier, position))
        result.Add(
            ErrorCode::kKeywordIdentifier, sql, position,
            absl::StrCat("Identifier `", token.GetImage(),
                         "` is an SQL keyword. Change the name or escape with "
                         "backticks (`)"));
    }
  }
  return result;
}

LinterResult CheckSpecifyTable(absl::string_view sql,
                               const LinterOptions &options) {
  LinterResult result;

  return result;
}

}  // namespace zetasql::linter
