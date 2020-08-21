# ZetaSQL linter

**This is not an officially supported Google product.**

ZetaSQL linter is tool that analyzes ZetaSQL queries to flag errors, bugs, and
stylistic errors.

This is an internship project for Summer 2020.

## Usage

In order to run linter on some sql files, you can use this command to compile and run current implementation:
    
    `bazel run //src:runner <absolute_path_of_sql_files>`

or this command to run latest binary:

    `./sqllint <path_of_sql_files>`

Each file should be separated by space. Example:

    `./sqllint example.sql example2.sql`

or

    `./sqllint *.sql`

## Flags

ZetaSQL Linter uses the Abseil [Flags](https://abseil.io/blog/20190509-flags) library to handle commandline flags. There are currently two flags implemented.

### config

It will get a configuration file defined in [config documentation](docs/config.md). Example:

    `./sqllint --config=my_config.textproto example.sql`

Convention for configuration file it to have `.textproto` extension.

### quick

It will read and lint a single statement, until a semicolon(';') comes. Example:

    `./sqllint --quick`