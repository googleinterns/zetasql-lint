# Checks
Each linter check is associated with a check name. Check names are listed as below:

1. [parser-failed](checks.md#parser-failed)
2. [line-limit-exceed](checks.md#line-limit-exceed)
3. [statement-semicolon](checks.md#statement-semicolon)
4. [consistent-letter-case](checks.md#consistent-letter-case)
5. [consistent-comment-style](checks.md#consistent-comment-style)
6. [alias](checks.md#alias)
7. [uniform-indent](checks.md#uniform-indent)
8. [not-indent-tab](checks.md#not-indent-tab)


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
