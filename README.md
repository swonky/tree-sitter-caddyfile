
# tree-sitter-caddyfile

A [Caddyfile](https://caddyserver.com) grammar for the [Tree-sitter](https://github.com/tree-sitter/tree-sitter) parser generator.

> [!NOTE]
> This project is not affiliated with or endorsed by the ZeroSSL Project or the maintainers of caddy.

# TODO
- [ ] Fold scheme file
- [ ] !-prefix
- [ ] Tests
- [ ] Benchmarks

## Contents
- [Features](#features)
    - [Syntax](#base-syntax)
    - [Scheme files](#scheme-files)
    - [Language bindings](#language-bindings)
- [Development](#development)
    - [Testing](#testing)
- [Example](#example)
    - [Syntax highlighting](#syntax-highlighting)
    - [Concrete syntax tree](#concrete-syntax-tree)
- [References](#references)

## Features
### Base syntax
The grammar supports the following Caddyfile language syntax structures:

- **Primitive value types**: `literal_string`, `integer`, `decimal`, `ipv4`
- **Compound value types**: `templated_string`, `duration`, `address`, `ipv6`
- **Global block**
- **Site blocks**
- **Snippet** definitions and references
- **Named routes**
- **Directives** with arguments and/or subdirective blocks
- **Negations** ("not" keyword)
- **Comments**
- **Language injection**
    - Heredocs will highlight according to the label term (eg. "<<HTML" or "<<JSON")
    - Grave (`\``) quoted cel expressions
    - `expression` matcher cel expressions

### Scheme files 
The following scheme files[^8] are included in [./queries](queries/) to facilitate editor integrations and syntax highlight.

| File | Content |
| ---- | ------- |
| [highlights.scm](queries/highlights.scm)  | syntax highlighting & spell check  |
| [injections.scm](queries/injections.scm)  | YAML[^7] language injection        |
<!-- | [folds.scm](queries/folds.scm)            | code folding                       | -->

### Language bindings
Bindings are available under [./bindings](bindings/) for C, Go, Java, JavaScript, Python, Rust, Swift, and Zig.

## Development

### Testing
> [!TODO]

## Example
### Syntax highlighting
> [!TODO]
### Concrete syntax tree
> [!TODO]

## References
> [!TODO]
