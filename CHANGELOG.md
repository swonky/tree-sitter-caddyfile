## [0.4.0] - 2026-09-04

### Features

- Added support for `{block}` and `{blocks.*}` style snippet substitutions as top-level block statements.
- Replaced `paramater` node with generic `index_expression` and `slice_expression` like tree-sitter-go.
- Added `cidr` and `unary expressions` for complex headers.
- Added highlights queries for symbolic header operators.
- Renamed `snippet_reference` to `import_statement`.

### Bug Fixes

- Allow windows absolute pathnames to omit a subpath following the drive letter.
- Corrected `operator` behaviour

### Refactor

- Removed vestigial `args` keyword
