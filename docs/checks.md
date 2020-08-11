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
10. [naming](checks#naming)

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
In line 4, column 19: Syntax error: Expected end of input but got identifier "D"
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
In line 1, column 101: Lines should be <= 100 characters long
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
In line 4, column 16: Each statement should end with a semicolon ';'
```
## consistent-letter-case
Checks if every keyword, either all uppercase or all lowercase.

**Example**
```sql
-- The 'SELECT' keyword written as all uppercase, so there is no problem.
SELECT *;

-- Here, 'FROM' keyword consists of uppercase and lowercase letters,
-- which will cause a lint error.
SELECT A From B;
```
**Linter Output**
```
In line 6, column 10: All keywords should be either uppercase or lowercase
```
## consistent-comment-style
ZetaSQL linter supports different kind of comment styles ('#', '//', or '--'). Only one of them should be used consistently,

**Example**
```sql
-- The first comment used '--'
SELECT *;
# The second comment used '#', which is different than the first comment
# and it will cause lint error.
```

**Linter Output**
```
In line 3, column 1: One line comments should be consistent, expected: --, found: #
In line 4, column 1: One line comments should be consistent, expected: --, found: #
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
In line 5, column 10: Always use AS keyword before aliases
```
## uniform-indent
Indentation character should be uniform. Generally, either tabs or spaces are used. It is configurable with variable [allowed_indent](config.md#config). By default, spaces are expected to be used as the indentation character.

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
In line 9, column 1: Inconsistent use of indentation symbols, expected: whitespace
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
In line 8, column 8: Tab is not in the indentation
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
In line 5, column 8: Use single quotes(') instead of double quotes(")
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
In line 2, column 8: Use double quotes(") instead of single quotes(')
```


## naming
In a sql file each name should follow a specific convention.

| Type | Recommendation |
|--------|----------------------------------------|
|Table Names| UpperCamelCase, e.g. `CREATE TABLE MyTable`|
|Window Names| UpperCamelCase, e.g. `OrderedCategory` |
|Built-In SQL Data Types|All caps, e.g. `BOOL`, `STRING`, `INT64`|
|User-Defined Functions|UpperCamelCase, e.g. `CREATE PUBLIC FUNCTION MyFunction`|
|User-Defined Function Parameters|snake_case, e.g. `my_param`|
|User-Defined Constants|CAPS_SNAKE_CASE, e.g. `CREATE PUBLIC CONSTANT TWO_PI = 6.28`|
|Column Names|snake_case, e.g. `SELECT MyTable.my_rowkey AS account_id`|


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
In line 1, column 14: Table names or table aliases should be UpperCamelCase.
In line 3, column 14: Table names or table aliases should be UpperCamelCase.
In line 6, column 24: Window names should be UpperCamelCase.
In line 9, column 28: Function names should be UpperCamelCase.
In line 10, column 28: Function names should be UpperCamelCase.
In line 13, column 27: Simple SQL data types should be all caps.
In line 15, column 27: Simple SQL data types should be all caps.
In line 20, column 8: Column names should be lower_snake_case.
In line 21, column 8: Column names should be lower_snake_case.
In line 24, column 30: Function parameters should be lower_snake_case.
In line 25, column 30: Function parameters should be lower_snake_case.
In line 26, column 30: Function parameters should be lower_snake_case.
In line 28, column 24: Constant names should be CAPS_SNAKE_CASE.
```



