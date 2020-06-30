#include "zetasql/parser/parser.h"

#include <cstdio>
#include <iostream>

#include "zetasql/public/parse_helpers.h"
#include "absl/strings/string_view.h"
#include "zetasql/base/status.h"
#include "zetasql/parser/parse_tree_visitor.h"
#include <google/protobuf/text_format.h>


void check( absl::string_view sql ) {
    absl::Status return_status;
    std::unique_ptr<zetasql::ParserOutput> output;

    return_status = zetasql::ParseStatement( sql,
        zetasql::ParserOptions(), &output );

    std::cout << "Status for sql \" " << sql << "\" = "
        << return_status.ToString() << std::endl;
    
    if( return_status.ok() ) {
        std::cout << output -> statement() -> DebugString() << std::endl;
    }
}

int main() {
    
    const absl::string_view valid_sql = "SELECT 5+2";

    const absl::string_view invalid_sql = "SELECT 5+2 ssss dddd";

    check( valid_sql );
    check( invalid_sql );

    return 0;
}
