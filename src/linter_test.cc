//
// Copyright 2019 ZetaSQL Authors
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

#include "gtest/gtest.h"

TEST(LinterTest, StatementLineLengthCheck) {
    const absl::string_view sql =
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x";

    EXPECT_EQ(checkLineLength(sql).ok(), 1);
    EXPECT_EQ(checkLineLength(sql, 10).ok(), 0);
    EXPECT_EQ(checkLineLength(sql, 10, ' ').ok(), 1);
}


TEST(LinterTest, StatementValidityCheck) {
    const absl::string_view valid_sql = "SELECT 5+2";

    const absl::string_view invalid_sql = "SELECT 5+2 sss ddd";

    EXPECT_EQ(checkStatement(valid_sql).ok(), 1);
    EXPECT_EQ(checkStatement(invalid_sql).ok(), 0);

    EXPECT_EQ(checkStatement(
        "SELECT * FROM emp where b = a or c < d group by x").ok(), 1);

    EXPECT_EQ(checkStatement(
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x").ok(), 1);

    EXPECT_EQ(checkStatement(
        "SELET A FROM B\nSELECT C FROM D").ok(), 0);
}

TEST(LinterTest, StatementLineLengthCheck) {
    const absl::string_view sql =
        "SELECT e, sum(f) FROM emp where b = a or c < d group by x";

    EXPECT_EQ(checkLineLength(sql).ok(), 1);
    EXPECT_EQ(checkLineLength(sql, 10).ok(), 0);
    EXPECT_EQ(checkLineLength(sql, 10, ' ').ok(), 1);
}
