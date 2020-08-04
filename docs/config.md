# Config
Some checks in ZetaSQL linter has configurable options. Users can provide a proto-txt file in order to change these options as well as disabling some linter checks.

**Example Usage**
```protobuf
message Config {
	tab_size = 2;
	end_line = '\r\n';
	line_limit = 80;
	allowed_indent = '\t';
	nolint = {"parser-failed", "alias" };
}
```
**Configurable Options**
|Type | Name | Default Value | Description|
|--------|-------|-------------------|--------------|
|int32|tab_size| 4 |Number of spaces one 't' character corresponds to|
|int32|end_line|'\n'|End line character used to separate lines|
|int32|line_limit|100|Maximum number of characters one line should contain|
|int32|allowed_indent|' '(whitespace)|Allowed indentation character(usually ' ' or '\t')|
|string*|nolint|{}|List of [check names](https://github.com/googleinterns/zetasql-lint/blob/proto/docs/checks.md) that will be disabled|
