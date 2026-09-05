## [0.5.0] - 2026-09-05
### Features

- Improved parity with official Caddyfile parser:
    - Added node `named_route_declaration`
    - Added node `snippet_declaration`
- Added `tags.scm` query file for code navigation
- Added `additive_sequence` token for sequences of duration expressions like '2h30m10s' (as per Go's `time.ParseDuration` syntax).

### Bug Fixes

- Included `@` prefix in matcher definition names for consistency
- Import statement arguments are now separate argument fields.
- Fixed issue with using absolute Windows pathnames with `import_statement` while inside blocks.

### Refactor

- Tidied up tag queries

### Testing

- Added Go-based test that compared parser capture groups to the tokenizer output of the official Caddyfile parser.
