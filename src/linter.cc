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
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
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
absl::Status PrintASTTree(absl::string_view sql) {
  absl::Status return_status;
  std::unique_ptr<ParserOutput> output;

  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  bool is_the_end = false;
  int cnt = 0;
  while (!is_the_end) {
    return_status = ParseNextScriptStatement(&location, ParserOptions(),
                                             &output, &is_the_end);

    std::cout << "Status for sql#" << ++cnt << ": \"" << sql
              << "\" = " << return_status.ToString() << std::endl;

    if (return_status.ok()) {
      std::cout << output->statement()->DebugString() << std::endl;
    } else {
      break;
    }
  }
  return return_status;
}

absl::Status CheckLineLength(absl::string_view sql, int line_limit,
                             const char delimeter) {
  int lineSize = 0;
  int line_number = 1;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == delimeter) {
      lineSize = 0;
      ++line_number;
    } else {
      ++lineSize;
    }
    if (lineSize > line_limit) {
      // TODO(orhanuysal): add proper error handling.
      return absl::Status(
          absl::StatusCode::kFailedPrecondition,
          absl::StrCat("Lines should be <= ", std::to_string(line_limit),
                       " characters long [", std::to_string(line_number),
                       ",1]"));
    }
  }
  return absl::OkStatus();
}

absl::Status CheckParserSucceeds(absl::string_view sql) {
  std::unique_ptr<ParserOutput> output;

  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);

  bool is_the_end = false;
  while (!is_the_end) {
    ZETASQL_RETURN_IF_ERROR(ParseNextScriptStatement(&location, ParserOptions(),
                                                     &output, &is_the_end));
  }
  return absl::OkStatus();
}

absl::Status CheckSemicolon(absl::string_view sql) {
  std::unique_ptr<ParserOutput> output;

  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  bool is_the_end = false;

  while (!is_the_end) {
    ZETASQL_RETURN_IF_ERROR(ParseNextScriptStatement(&location, ParserOptions(),
                                                     &output, &is_the_end));

    int location =
        output->statement()->GetParseLocationRange().end().GetByteOffset();

    if (location >= sql.size() || sql[location] != ';') {
      return absl::Status(absl::StatusCode::kFailedPrecondition,
                          "Each statemnt should end with a consequtive"
                          "semicolon ';'");
    }
  }
  return absl::OkStatus();
}

bool AllUpperCase(const absl::string_view &sql,
                  const ParseLocationRange &range) {
  for (int i = range.start().GetByteOffset(); i < range.end().GetByteOffset();
       ++i) {
    if ('a' <= sql[i] && sql[i] <= 'z') return false;
  }
  return true;
}

absl::Status CheckUppercaseKeywords(absl::string_view sql) {
  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);
  std::vector<ParseToken> parse_tokens;
  absl::Status tokenizer_status =
      GetParseTokens(ParseTokenOptions(), &location, &parse_tokens);

  // Keyword definition in tokenizer is very wide,
  // it include some special characters like ';', '*', etc.
  // Keyword Uppercase check will simply ignore characters
  // outside of english lowercase letters.
  for (auto &token : parse_tokens) {
    if (token.kind() == ParseToken::KEYWORD) {
      if (!AllUpperCase(sql, token.GetLocationRange())) {
        return absl::Status(
            absl::StatusCode::kFailedPrecondition,
            absl::StrCat("All keywords should be Uppercase, In character ",
                         std::to_string(
                             token.GetLocationRange().start().GetByteOffset()),
                         " string should be: ", token.GetSQL()));
      }
    }
  }

  return absl::OkStatus();
}

absl::Status CheckCommentType(absl::string_view sql, char delimeter) {
  bool dash_comment = false;
  bool slash_comment = false;
  bool hash_comment = false;
  bool inside_string = false;

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == '\'' || sql[i] == '"') inside_string = !inside_string;

    if (inside_string) continue;

    if (i > 0 && sql[i - 1] == '-' && sql[i] == '-') {
      dash_comment = true;
      // ignore the line.
      while (i < static_cast<int>(sql.size()) && sql[i] != delimeter) {
        ++i;
      }
    }

    if (i > 0 && sql[i - 1] == '/' && sql[i] == '/') {
      slash_comment = true;
      // ignore the line.
      while (i < static_cast<int>(sql.size()) && sql[i] != delimeter) {
        ++i;
      }
    }

    if (sql[i] == '#') {
      hash_comment = true;
      // ignore the line.
      while (i < static_cast<int>(sql.size()) && sql[i] != delimeter) {
        ++i;
      }
    }

    // ignore multiline comments.
    if (i > 0 && sql[i - 1] == '/' && sql[i] == '*') {
      // it will start checking after '/*' and after the iteration
      // finished, the pointer 'i' will be just after '*/' (incrementation
      // from the for statement is included).
      i += 2;
      while (i < static_cast<int>(sql.size()) &&
             !(sql[i - 1] == '*' && sql[i] == '/')) {
        ++i;
      }
    }
  }

  if (dash_comment + slash_comment + hash_comment > 1)
    return absl::Status(absl::StatusCode::kFailedPrecondition,
                        "either '//' or '--' should be used to "
                        "specify a comment");
  return absl::OkStatus();
}

absl::Status ASTNodeRule::ApplyTo(absl::string_view sql) {
  RuleVisitor visitor(rule_, sql);

  std::unique_ptr<ParserOutput> output;
  ParseResumeLocation location = ParseResumeLocation::FromStringView(sql);

  bool is_the_end = false;
  while (!is_the_end) {
    ZETASQL_RETURN_IF_ERROR(ParseNextScriptStatement(&location, ParserOptions(),
                                                     &output, &is_the_end));
    ZETASQL_RETURN_IF_ERROR(
        output->statement()->TraverseNonRecursive(&visitor));
  }

  ZETASQL_RETURN_IF_ERROR(visitor.GetResult());

  return absl::OkStatus();
}

absl::Status CheckAliasKeyword(absl::string_view sql) {
  return ASTNodeRule(
             [](const ASTNode *node, absl::string_view sql) -> absl::Status {
               if (node->node_kind() == AST_ALIAS) {
                 int position =
                     node->GetParseLocationRange().start().GetByteOffset();
                 if (sql[position] != 'A' || sql[position + 1] != 'S') {
                   return absl::Status(
                       absl::StatusCode::kFailedPrecondition,
                       absl::StrCat("Always use AS keyword for referencing "
                                    "aliases, ",
                                    "In position: ", std::to_string(position)));
                 }
               }
               return absl::OkStatus();
             })
      .ApplyTo(sql);
}

std::string ConstructPositionMessage(std::pair<int, int> pos) {
  return absl::StrCat("line ", pos.first, " position ", pos.second);
}

absl::Status ConstructErrorWithPosition(absl::string_view sql,
                                        int index,
                                        absl::string_view error_msg) {
  ParseLocationPoint lp =
    ParseLocationPoint::FromByteOffset(index);
  ParseLocationTranslator lt(sql);
  std::pair <int, int> error_pos;
  ZETASQL_ASSIGN_OR_RETURN(error_pos,
    lt.GetLineAndColumnAfterTabExpansion(lp));
  return absl::Status(
           absl::StatusCode::kFailedPrecondition,
           absl::StrCat(
           error_msg, " in ", ConstructPositionMessage(error_pos)));
}

absl::Status CheckTabCharactersUniform(absl::string_view sql,
                                       const char allowed_indent,
                                       const char line_delimeter) {
  bool is_indent = true;
  const char kSpace = ' ', kTab = '\t';

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == line_delimeter) {
      is_indent = true;
    } else if (is_indent && sql[i] != allowed_indent) {
      if (sql[i] == kTab || sql[i] == kSpace) {
        return ConstructErrorWithPosition(sql, i,
                 absl::StrCat(
                 "Inconsistent use of indentation symbols: ",
                 "expected \"", std::string(1, allowed_indent),
                 "\""));
      }
      is_indent = false;
    }
  }

  return absl::OkStatus();
}

absl::Status CheckNoTabsBesidesIndentations(absl::string_view sql,
                                            const char line_delimeter) {
  const char kSpace = ' ', kTab = '\t';

  bool is_indent = true;

  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (sql[i] == line_delimeter) {
      is_indent = true;
    } else if (sql[i] != kSpace && sql[i] != kTab) {
      is_indent = false;
    } else if (sql[i] == kTab && !is_indent) {
      return ConstructErrorWithPosition(sql, i,
               absl::string_view(
               "Tab not in the indentation, expected space"));
    }
  }

  return absl::OkStatus();
}

zetasql_base::StatusOr<VisitResult> RuleVisitor::defaultVisit(
    const ASTNode *node) {
  absl::Status rule_result = rule_(node, sql_);
  if (!rule_result.ok()) {
    // There may be multiple rule failures for now
    // only the last failure will be shown.
    result_ = rule_result;
  }
  return VisitResult::VisitChildren(node);
}

}  // namespace zetasql::linter
