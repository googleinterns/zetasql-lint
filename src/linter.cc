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
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "src/lint_errors.h"
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

// Implemented rules in the same order with rules in the documention.
namespace zetasql::linter {

// This will eventually be erased.
LinterResult PrintASTTree(absl::string_view sql) {
  absl::Status return_status;
  std::unique_ptr<ParserOutput> output;

  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  bool is_the_end = false;
  int cnt = 0;
  while (!is_the_end) {
    return_status = ParseNextScriptStatement(&location, ParserOptions(),
                                             &output, &is_the_end);

    std::cout << "Status for sql#" << ++cnt << " : \"" << sql
              << "\" = " << return_status.ToString() << std::endl;

    if (return_status.ok()) {
      std::cout << output->statement()->DebugString() << std::endl;
    } else {
      break;
    }
  }
  return LinterResult(return_status);
}

LinterResult CheckLineLength(absl::string_view sql,
                             const LinterOptions &option) {
  int lineSize = 0;
  int line_number = 1;
  int last_added = 0;
  LinterResult result;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == option.LineDelimeter()) {
      lineSize = 0;
      ++line_number;
    } else {
      ++lineSize;
    }

    if (lineSize > option.LineLimit() && line_number > last_added) {
      last_added = line_number;
      if (option.IsActive(ErrorCode::kLineLimit, i))
        result.Add(ErrorCode::kLineLimit, sql, i,
                   absl::StrCat("Lines should be <= ", option.LineLimit(),
                                " characters long"));
    }
  }
  return result;
}

LinterResult CheckParserSucceeds(absl::string_view sql,
                                 const LinterOptions &option) {
  std::unique_ptr<ParserOutput> output;

  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);

  bool is_the_end = false;
  int byte_position = 1;
  LinterResult result;
  while (!is_the_end) {
    byte_position = location.byte_position();
    absl::Status status = ParseNextScriptStatement(&location, ParserOptions(),
                                                   &output, &is_the_end);
    if (!status.ok()) {
      // TODO(orhanuysal): Implement a token parser to seperate statements
      // currently, when parser fails, it is unable to determine
      // the end of the statement.
      if (option.IsActive(ErrorCode::kParseFailed, byte_position)) {
        return LinterResult(status);
        // ErrorLocation location = internal::GetPayload<ErrorLocation>(status);
        result.Add(ErrorCode::kParseFailed, sql, byte_position,
                   absl::StrCat("Parser Failed, with message ", status.message()));
      }
      break;
    }
  }
  return result;
}

LinterResult CheckSemicolon(absl::string_view sql,
                            const LinterOptions &option) {
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

    if (location >= sql.size() || sql[location] != ';') {
      if (option.IsActive(ErrorCode::kSemicolon, location))
        result.Add(ErrorCode::kSemicolon, sql, location,
                   "Each statement should end with a "
                   "semicolon ';'");
    }
  }
  return result;
}

bool ConsistentUppercaseLowercase(const absl::string_view &sql,
                                  const ParseLocationRange &range) {
  bool uppercase = false;
  bool lowercase = false;
  for (int i = range.start().GetByteOffset(); i < range.end().GetByteOffset();
       ++i) {
    if ('a' <= sql[i] && sql[i] <= 'z') lowercase = true;
    if ('A' <= sql[i] && sql[i] <= 'Z') uppercase = true;
  }
  // There shouldn't be any case any Keyword
  // contains both uppercase and lowercase characters
  return !(lowercase && uppercase);
}

LinterResult CheckUppercaseKeywords(absl::string_view sql,
                                    const LinterOptions &option) {
  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  std::vector<ParseToken> parse_tokens;
  LinterResult result;

  absl::Status status =
      GetParseTokens(ParseTokenOptions(), &location, &parse_tokens);

  if (!status.ok()) return LinterResult(status);

  // Keyword definition in tokenizer is very wide,
  // it include some special characters like ';', '*', etc.
  // Keyword Uppercase check will simply ignore characters
  // outside of english lowercase letters.
  for (auto &token : parse_tokens) {
    if (token.kind() == ParseToken::KEYWORD) {
      if (!ConsistentUppercaseLowercase(sql, token.GetLocationRange())) {
        int position = token.GetLocationRange().start().GetByteOffset();
        if (option.IsActive(ErrorCode::kLetterCase, position))
          result.Add(ErrorCode::kLetterCase, sql, position,
                     "All keywords should be either uppercase or lowercase");
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
    if (sql[i] == '\'' || sql[i] == '"') {
      // The sql is inside string. This will ignore sql part until
      // the same type(' or ") of character will occur
      // without \ in the beginning. For example 'a"b' is a valid string.
      for (int j = i + 1; j < static_cast<int>(sql.size()); ++j) {
        if (sql[j - 1] == '\\' && sql[j] == sql[i]) continue;
        if (sql[j] == sql[i] || j + 1 == static_cast<int>(sql.size())) {
          i = j;
          break;
        }
      }
      continue;
    }

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

    // Ignore multiline comments.
    if (i > 0 && sql[i - 1] == '/' && sql[i] == '*') {
      // It will start checking after '/*' and after the iteration
      // finished, the pointer 'i' will be just after '*/' (incrementation
      // from the for statement is included).
      i += 2;
      while (i < static_cast<int>(sql.size()) &&
             !(sql[i - 1] == '*' && sql[i] == '/')) {
        ++i;
      }
    }
  }

  return result;
}

LinterResult ASTNodeRule::ApplyTo(absl::string_view sql,
                                  const LinterOptions &option) {
  RuleVisitor visitor(rule_, sql, option);

  std::unique_ptr<ParserOutput> output;
  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  absl::Status status;
  LinterResult result;

  bool is_the_end = false;
  while (!is_the_end) {
    status = ParseNextScriptStatement(&location, ParserOptions(), &output,
                                      &is_the_end);
    if (!status.ok()) return LinterResult();

    status = output->statement()->TraverseNonRecursive(&visitor);
    if (!status.ok()) return LinterResult(status);
  }

  return visitor.GetResult();
}

zetasql_base::StatusOr<VisitResult> RuleVisitor::defaultVisit(
    const ASTNode *node) {
  result_.Add(rule_(node, sql_, option_));
  return VisitResult::VisitChildren(node);
}

LinterResult CheckAliasKeyword(absl::string_view sql,
                               const LinterOptions &option) {
  return ASTNodeRule([](const ASTNode *node, const absl::string_view &sql,
                        const LinterOptions &option) -> LinterResult {
           LinterResult result;
           if (node->node_kind() == AST_ALIAS) {
             int position =
                 node->GetParseLocationRange().start().GetByteOffset();
             if (sql[position] != 'A' || sql[position + 1] != 'S') {
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

}  // namespace zetasql::linter
