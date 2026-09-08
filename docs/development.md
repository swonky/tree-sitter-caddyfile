# Development

> [!CAUTION]
> To accommodate Caddyfile's contextual syntax quirks, the parser is highly dependant on a handwritten custom scanner written in C11. 
> The complexity of the scanner increases the chance of fatal errors. 
> While the parser currently tolerates fuzzed inputs, I urge caution before integrating the parser into tooling until automated testing is implemented and an official release is posted.

# Repository structure
Tree-sitter repositories contain a number of generated files. Most of the parser's behaviour is determined by a small number of source files, while the remaining files are generated from them.

## Parser
The parser itself is generated from these files.

| File | Content |
| ---- | ------- |
| [grammar.js](grammar.js)          | Defines the compositional grammar rules.       |
| [src/scanner.c](src/scanner.c)    | Custom scanner that handles lexical behaviour. |

## Queries
The following query files[^8] are used by editor integrations and other tooling.

| File | Content |
| ---- | ------- |
| [highlights.scm](queries/highlights.scm)  | Syntax highlighting and spell checking  |
| [injections.scm](queries/injections.scm)  | Language injection[^7]                  |
| [folds.scm](queries/folds.scm)            | Code folding                            |

## Tests

| File | Content |
| ---- | ------- |
| [official.txt](test/corpus/official.txt) | Adapted from the official tree-sitter-caddyfile repository [^9].

# Development

## Testing [^4]
The repository contains a several tests located within [./test/corpus](test/corpus/).

> [!TIP]
> Running the tests requires [tree-sitter-cli](https://github.com/tree-sitter/tree-sitter/blob/master/crates/cli/README.md) and [Go 1.27+](https://go.dev/doc/install).

```sh
# runs all tests in the corpus
tree-sitter test

# perform parser fuzzing
tree-sitter fuzz

# perform token boundary test against the reference parser (required Go 1.27+).
cd test
go clean -cache # if the grammar has been altered
go test .
```

[^1]: [tree-sitter: Documentation](https://tree-sitter.github.io/tree-sitter/)
[^4]: [tree-sitter: Writing tests](https://tree-sitter.github.io/tree-sitter/creating-parsers/5-writing-tests.html)
[^7]: [tree-sitter: Language injection](https://tree-sitter.github.io/tree-sitter/3-syntax-highlighting.html#language-injection)
[^8]: [tree-sitter: Query syntax](https://tree-sitter.github.io/tree-sitter/using-parsers/queries/1-syntax.html)
[^9]: [Official tree-sitter-caddyfile tests](https://github.com/caddyserver/tree-sitter-caddyfile/tree/8ee969d8fd68d67661016d890110e4cae18ed03c/test/corpus)
