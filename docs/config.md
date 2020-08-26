# Config
Some checks in ZetaSQL linter have configurable options which can be specified in a file contains config proto in text format. Disabling checks can also be done using configuration file.

**Example File**
```protobuf
tab_size: 2;
line_limit: 80;
allowed_indent: ' ';
nolint: ["parser-failed", "alias"];
```
**Configurable Options**
|Type | Name | Default Value | Description|
|--------|-------|-------------------|--------------|
|int32|tab_size| 4 |Number of spaces one 't' character corresponds to|
|string|end_line|'\n'|End line character used to separate lines|
|int32|line_limit|100|Maximum number of characters one line should contain|
|string|allowed_indent|' '(whitespace)|Allowed indentation character(usually ' ' or '\t')|
|bool|single_quote|true|Whether single or double quote will used in sql file, checked by this [rule](config.md#single-or-double-quote)|
|string*|nolint|[]|List of [check names](checks.md) that will be disabled|
