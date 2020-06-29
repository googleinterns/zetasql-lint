#include <cstdio>
#include <iostream>
#include "zetasql/public/parse_helpers.h"
#include "zetasql/parser/parser.h"
#include "absl/strings/string_view.h"
#include "zetasql/base/status.h"
#include "zetasql/parser/parse_tree_visitor.h"

// Structre:
// absl::Status ParseStatement(absl::string_view statement_string,
//                             const ParserOptions& parser_options_in,
//                             std::unique_ptr<ParserOutput>* output);

void check( std::string sql ) {
    absl::Status return_status;
    std::unique_ptr<zetasql::ParserOutput> output;

    return_status = zetasql::ParseStatement( sql, zetasql::ParserOptions(), &output );

    std::cout << "Status for sql \" " << sql << "\" = " << return_status.ToString() << std::endl;
}

int main() {
    
    const std::string valid_sql = "SELECT 5+2";

    const std::string invalid_sql = "SELECT 5+2 ssss dddd";

    check( valid_sql );
    check( invalid_sql );

    return 0;
}