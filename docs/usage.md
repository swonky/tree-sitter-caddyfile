# Usage

> [!CAUTION]
> To accommodate Caddyfile's contextual syntax quirks, the parser is highly dependant on a handwritten custom scanner written in C11. The complexity of the scanner increases the chance of fatal errors. While the parser currently tolerates fuzzed inputs, I urge caution before integrating the parser into tooling until automated testing is implemented and an official release is posted.

## Installation

### Go
```sh
go get github.com/swonky/tree-sitter-caddyfile
```

```go
import github.com/swonky/tree-sitter-caddyfile
```

### Rust
```sh
cargo add tree-sitter-caddyfile-swonky
```

```rust
use tree_sitter_caddyfile
```

# Repository structure

## Scheme files 
The following scheme files[^8] are included in [./queries](queries/) to facilitate editor integrations and syntax highlight.

| File | Content |
| ---- | ------- |
| [highlights.scm](queries/highlights.scm)  | syntax highlighting & spell check  |
| [injections.scm](queries/injections.scm)  | language injection[^7]             |
| [folds.scm](queries/folds.scm)            | code folding                       |

| File | Content |
| ---- | ------- |
| [official.txt](test/corpus/official.txt) | adapted from the official tree-sitter-caddyfile repository [^9].

## Language bindings
Bindings are available under [./bindings](bindings/) for C, Go, Java, JavaScript, Python, Rust, Swift, and Zig.

# Development

## Testing [^4]
The repository contains a several tests located within [./test/corpus](test/corpus/).

> [!TIP]
> Running the tests requires [tree-sitter-cli](https://github.com/tree-sitter/tree-sitter/blob/master/crates/cli/README.md).

```sh
# runs all tests in the corpus
tree-sitter test

# perform parser fuzzing
tree-sitter fuzz
```

[^4]: [tree-sitter: Writing tests](https://tree-sitter.github.io/tree-sitter/creating-parsers/5-writing-tests.html)
[^7]: [tree-sitter: Language injection](https://tree-sitter.github.io/tree-sitter/3-syntax-highlighting.html#language-injection)
[^8]: [tree-sitter: Query syntax](https://tree-sitter.github.io/tree-sitter/using-parsers/queries/1-syntax.html)
[^9]: [Official tree-sitter-caddyfile tests](https://github.com/caddyserver/tree-sitter-caddyfile/tree/8ee969d8fd68d67661016d890110e4cae18ed03c/test/corpus)
