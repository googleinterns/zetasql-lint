# Checks
Each linter check is associated with a check name. Any check can be disabled by changing [configuration](config.md), or [specifying inside of the code](disabling_checks.md).
Check names are listed as below:

1. [parser-failed](checks.md#parser-failed)
2. [line-limit-exceed](checks.md#line-limit-exceed)
3. [statement-semicolon](checks.md#statement-semicolon)
4. [consistent-letter-case](checks.md#consistent-letter-case)
5. [consistent-comment-style](checks.md#consistent-comment-style)
6. [alias](checks.md#alias)
7. [uniform-indent](checks.md#uniform-indent)
8. [not-indent-tab](checks.md#not-indent-tab)
9. [single-or-double-quote](checks.md#single-or-double-quote)
10. [table-name](checks.md#naming)
11. [window-name](checks.md#naming)
13. [function-name](checks.md#naming)
14. [data-type-name](checks.md#naming)
15. [column-name](checks.md#naming)
16. [parameter-name](checks.md#naming)
17. [constant-name](checks.md#naming)
18. [join](checks.md#join)
19. [imports](checks.md#imports)

## parser-failed
Checks if ZetaSQL parser succeeds to parse your sql statements. 

**Example**
```sql
-- Here is an OK statement.
SELECT *;
-- Here is an invalid statement.
SELECT A FROM B C D;
```
**Linter Output**
```
In line 4, column 19: Syntax error: Expected end of input but got identifier "D" [parser-failed]
```

## line-limit-exceed
Each line in an SQL file should contain less characters than a configurable [line limit](config.md#config).

**Example**
```sql
-- Here is a very long comment that will exceed line limit and should be seperated into different lines.
SELECT *;
```
**Linter Output**
```
In line 1, column 101: Lines should be <= 100 characters long [line-limit-exceed]
```

## statement-semicolon
Checks if every SQL statement ends with a semicolon (';')

**Example**
```sql
-- Here is a normal statement that ends with a semicolon.
SELECT *;
-- Here is also valid statement but it is missing a semicolon.
SELECT A FROM B
```
**Linter Output**
```
In line 4, column 16: Each statement should end with a semicolon ';' [statement-semicolon]
```
## consistent-letter-case
Checks if every keyword is all uppercase or all lowercase accoring to [configuration](config.md).

**Example**
```sql
-- The 'SELECT' keyword written as all uppercase, so there is no problem.
SELECT *;

-- Here, 'FROM' keyword consists of uppercase and lowercase letters,
-- which will cause a lint error.
SELECT a From B;
```
**Linter Output**
```
In line 6, column 10: Keyword 'From' should be all uppercase [consistent-letter-case]
```
## consistent-comment-style
SQL engines support different styles of single-line comments: '#', '//', and '--'. Only one of them should be used consistently.

**Example**
```sql
-- The first comment used '--'
SELECT *;
# The second comment used '#', which is different than the first comment
# and it will cause lint error.
```

**Linter Output**
```
In line 3, column 1: One line comments should be consistent, expected: --, found: # [consistent-comment-style]
In line 4, column 1: One line comments should be consistent, expected: --, found: # [consistent-comment-style]
```
## alias
Every alias should use keyword 'AS'. 

**Example**
```sql
-- Here is a valid usage.
SELECT A AS a;

-- Here 'AS' keyword is omitted. Still a valid SQL statement, but linter will complain.
SELECT A a;
```

**Linter Output**
```
In line 5, column 10: Always use AS keyword before aliases [alias]
```
## uniform-indent
Indentation character should be uniform. It could be either tab('\t') or space(' '). It is configurable with variable [allowed_indent](config.md#config). By default, spaces are expected to be used as the indentation character.

**Example**
```sql
-- No configuration changes are made. So, default indentation character is space.
-- Here, spaces are used, and it is valid.
SELECT
    A
FROM B;

-- Here, tabs('\t') are used, and it is invalid.
SELECT
	A
FROM B;
```

**Linter Output**
```
In line 9, column 1: Inconsistent use of indentation symbols, expected: whitespace [uniform-indent]
```

## not-indent-tab
There shouldn't be any tab characters besides from indentation.

**Example**
```sql
-- Indentation character is set as tabs in configuration.
-- So, here is a valid usage.
SELECT
	A
FROM B;

-- There is an extra tab before 'A'
SELECT 	A FROM B;
```

**Linter Output**
```
In line 8, column 8: Tab is not in the indentation [not-indent-tab]
```


## single-or-double-quote
In a sql file either single or double quote should be used. By default single quotes should be used, but it can be [configured](config.md)

**Example**
```sql
-- Here is a valid quoting with default configuration.
SELECT 'a' FROM TableName;

-- and also an invalid one.
SELECT "a" FROM TableName;
```

**Linter Output**
```
In line 5, column 8: Use single quotes(') instead of double quotes(") [single-or-double-quote]
```

**Example 2**
```sql
-- Here is an invalid quoting with a specific configuration
SELECT 'a' FROM TableName;

-- and also a valid one.
SELECT "a" FROM TableName;
```

**Config File**
```protobuf
message Config {
	single_quote = false
}
```

**Linter Output 2**
```
In line 2, column 8: Use double quotes(") instead of single quotes(') [single-or-double-quote]
```


## naming
In a sql file each name should follow a specific convention.

| Type | Check Names | Recommendation |
|--------|----------------------------------------|
|Table Names|table-name| UpperCamelCase, e.g. `CREATE TABLE MyTable`|
|Window Names|window-name| UpperCamelCase, e.g. `OrderedCategory` |
|User-Defined Functions|function-name|UpperCamelCase, e.g. `CREATE PUBLIC FUNCTION MyFunction`|
|Built-In SQL Data Types|data-type-name|All caps, e.g. `BOOL`, `STRING`, `INT64`|
|Column Names|column-name|snake_case, e.g. `SELECT MyTable.my_rowkey` or UpperCamelCase|
|User-Defined Function Parameters|parameter-name|snake_case, e.g. `my_param`|
|User-Defined Constants|constant-name|CAPS_SNAKE_CASE, e.g. `CREATE PUBLIC CONSTANT TWO_PI = 6.28`|


**Example**
```sql
CREATE TABLE TABLE_NAME; -- False
CREATE TABLE TableName; -- True
CREATE TABLE table_name; -- False, line 3

SELECT a FROM B WINDOW WindowName AS T; -- True
SELECT a FROM B WINDOW window_name AS T; -- False, line 6

CREATE TEMP TABLE FUNCTION BugScore(); -- True
CREATE TEMP TABLE FUNCTION bugScore(); -- False
CREATE TEMP TABLE FUNCTION bug_score(); -- false
CREATE TEMP FUNCTION BugScore(); -- True, line 11

CREATE TEMP FUNCTION A( s String ); -- False
CREATE TEMP FUNCTION A( s STRING ); -- True
CREATE TEMP FUNCTION A( s int64 ); -- False
CREATE TEMP FUNCTION A( s INT64 ); -- True, line 16

SELECT column_name; -- True
SELECT TableName.column_name; -- True
SELECT ColumnName; -- False
SELECT COLUMN_NAME; -- False, line 21

CREATE TEMPORARY FUNCTION A( string_param STRING ); -- True
CREATE TEMPORARY FUNCTION A( STRING_PARAM STRING ); -- False
CREATE TEMPORARY FUNCTION A( stringParam STRING ); -- False
CREATE TEMPORARY FUNCTION A( StringParam STRING ); -- False, line 26

CREATE PUBLIC CONSTANT TwoPi = 6.28; -- False
CREATE PUBLIC CONSTANT TWO_PI = 6.28; -- True, line 29
```

**Linter Output**
```
In line 1, column 14: Table names or table aliases should be UpperCamelCase. [table-name]
In line 3, column 14: Table names or table aliases should be UpperCamelCase. [table-name]
In line 6, column 24: Window names should be UpperCamelCase. [window-name]
In line 9, column 28: Function names should be UpperCamelCase. [function-name]
In line 10, column 28: Function names should be UpperCamelCase. [function-name]
In line 13, column 27: Simple SQL data types should be all caps. [data-type-name]
In line 15, column 27: Simple SQL data types should be all caps. [data-type-name]
In line 20, column 8: Column names should be lower_snake_case. [column-name]
In line 21, column 8: Column names should be lower_snake_case. [column-name]
In line 24, column 30: Function parameters should be lower_snake_case. [parameter-name]
In line 25, column 30: Function parameters should be lower_snake_case. [parameter-name]
In line 26, column 30: Function parameters should be lower_snake_case. [parameter-name]
In line 28, column 24: Constant names should be CAPS_SNAKE_CASE. [constant-name]
```

## join
Always explicitly indicate the type of join. Do not use just "JOIN", try to indicate the type like "INNER", "LEFT", etc.

**Example**
```sql
-- Here is some valid JOIN usage.
SELECT a FROM (t INNER JOIN x);
SELECT a FROM (t LEFT JOIN x);
SELECT a FROM (t RIGHT OUTER JOIN x);

-- and also an invalid one.
SELECT a FROM (t JOIN x); -- line 7

```

**Linter Output**
```
In line 7, column 18: Always explicitly indicate the type of join. [join]
```

## imports
Imports should be on the top. 
There shouldn't be any redundant(dublicate) imports.
All imports should order as follows:

1. Proto imports
2. Module imports

or

1. Module imports
2. Proto imports

**Example**
```sql
IMPORT MODULE random;
IMPORT PROTO 'random.proto';
IMPORT MODULE random; -- Here is a duplicate order, which also is in the wrong place.
```

**Linter Output**
```
In line 3, column 14: PROTO and MODULE inputs should be in seperate groups. [imports]
In line 3, column 21: "random" is already defined. [imports]
```

## expression-parantheses
Use parathenthesis to highlight the order when there are multiple Boolean operators of different types.

**Example**
```sql
-- Here is ok because they are the same type.
SELECT D WHERE a AND b AND c;

-- Here is an invalid usage of operators.
SELECT D WHERE a AND b OR c; -- line 5

-- It can be fixed by adding paratheses.
SELECT D WHERE a AND (b OR c);
```

**Linter Output**
```
In line 5, column 16: Use parantheses between consequtive AND and OR operators. [expression-parantheses]
```

## count-star
Prefer COUNT(*) over COUNT(1). They have the same functionality.

**Example**
```sql
-- Simple Example
SELECT COUNT(1); -- line 2
SELECT COUNT(*); -- line 3
```

**Linter Output**
```
In line 2, column 15: Use COUNT(*) instead of COUNT(1) [count-star]
```

## keyword-identifier
Do not use ZetaSQL keywords as identifiers (aliases, function arguments, etc.). Change the name or escape with backticks (`).

**Example**
```sql
-- Alias name is 'date' which is a SQL keyword.
SELECT MyTable.travel_date AS Date;
SELECT MyTable.travel_date AS t_date; -- line 7

-- Escape characters can also be used.
SELECT type FROM A;
SELECT `type` FROM A; -- line 7
```

**Linter Output**
```
In line 2, column 31: Identifier `Date` is an SQL keyword. Change the name or escape with backticks (`) [keyword-identifier]
In line 6, column 8: Identifier `type` is an SQL keyword. Change the name or escape with backticks (`) [keyword-identifier]
```
