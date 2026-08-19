
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
    - [Benchmarking](#benchmarking)
- [Example](#example)
    - [Syntax highlighting](#syntax-highlighting)
    - [Concrete syntax tree](#concrete-syntax-tree)
- [References](#references)

## Features
### Base syntax
The grammar supports the following Caddyfile language syntax structures:

- **Global block**
- **Site blocks**
- **Snippets**
- **Named routes**
- **Directives**
- **Negations**
- **Comments**
- **Language injection** incl. cel expressions and heredocs.

# TODO
- [ ] Durations
- [ ] Decimals
- [ ] Address/path/CIDR capture groups in directive args
- [ ] Folds
- [ ] !-prefix
