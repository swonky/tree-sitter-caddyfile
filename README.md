
# tree-sitter-caddyfile

A [Caddyfile](https://caddyserver.com) grammar for the [Tree-sitter](https://github.com/tree-sitter/tree-sitter) parser generator.

> [!NOTE]
> This project is not affiliated with or endorsed by the ZeroSSL Project or the maintainers of caddy.


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
    - Heredocs will highlight according to the label term (eg. "<<HTML" or "<<JSON")[^5]
    - Grave (`\``) quoted cel expressions[^6]
    - `expression` matcher cel expressions[^6]

### To do
- [ ] Fold scheme file
- [ ] !-prefix
- [ ] Tests
- [ ] Benchmarks

### Scheme files 
The following scheme files[^8] are included in [./queries](queries/) to facilitate editor integrations and syntax highlight.

| File | Content |
| ---- | ------- |
| [highlights.scm](queries/highlights.scm)  | syntax highlighting & spell check  |
| [injections.scm](queries/injections.scm)  | language injection[^7]       |
<!-- | [folds.scm](queries/folds.scm)            | code folding                       | -->

### Language bindings
Bindings are available under [./bindings](bindings/) for C, Go, Java, JavaScript, Python, Rust, Swift, and Zig.

## Development

### Testing
> [!WARNING] Yet to do

## Example
### Syntax highlighting
> [!WARNING] Yet to do
### Concrete syntax tree
> [!WARNING] Yet to do

## References
[^5]: Language injection requires the associated tree-sitter grammar for that language.
[^6]: tree-sitter-cel
[^7]: [tree-sitter: Language injection](https://tree-sitter.github.io/tree-sitter/3-syntax-highlighting.html#language-injection)
[^8]: [tree-sitter: Query syntax](https://tree-sitter.github.io/tree-sitter/using-parsers/queries/1-syntax.html)
