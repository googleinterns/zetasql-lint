# Disabling Checks Inside The Code

Any check in Zetasql-linter can be disabled or enabled for a specified code interval.
-- NOLINT(<CheckNames>) will disable specified names for the rest of the file. In order to
re-enable it, //LINT(<CheckNames>) can be added. Every check is associated with a string name
indicated in [Checks](checks.md) documentation. Any lint error also gives the check name at the end, 
in [<CheckName>] format.

### Usage

Lets say we have a lint error that we want to ignore.

SQL:
```sql
Select a;
```
Linter Output:
```
In line 1, column 1: All keywords should be either uppercase or lowercase. [consistent-letter-case]
```
We see the check is named 'consistent-letter-case', it can be disabled by

SQL:
```sql
-- NOLINT (consistent-letter-case) this is a special case that should be ignored
Select a;

Select b;
```
Linter Output:
```
No problems have been detected.
```

In order to continue lint it should be: 

SQL:
```sql
-- NOLINT (consistent-letter-case) this is a special case that should be ignored
Select a;
-- LINT(consistent-letter-case) from this point special case is finished.
Select b;
```
Linter Output:
```
In line 4, column 1: All keywords should be either uppercase or lowercase. [consistent-letter-case]
```

### Rules

 - Check names will be seperated by comma(,). i.e. `--NOLINT(join,imports)` is ok.
 - `LINT` or `NOLINT` should always be uppercase. i.e. `Lint` or `nolint` will be ignored.
 - It should be specified with one line comment. i.e. `//NOLINT(...)`, `--NOLINT(...)`, `#NOLINT(...)` are ok, but `/*NOLINT(...)*/` is not.
 - There shouldn't be any characters except spaces in the beginning . i.e. `-- comment NOLINT(...)` will be ignored.
 - There can be spaces in statement. i.e. `--    NOLINT   (parser-failed    )  ` is ok but not recommended.
 - There can be comments after `NOLINT`, usually it should be the explanation of `NOLINT`. i.e. `-- NOLINT(alias) ignore there` is ok.

