# tree-sitter-caddyfile

An alternative [Caddyfile](https://caddyserver.com) grammar for the [Tree-sitter](https://github.com/tree-sitter/tree-sitter) parser generator.

The grammar places greater emphasis on identifying syntactic boundaries and preserving the smaller nested elements that make up the language. The aim is to provide a more permissive parser with deterministic boundaries and a granular syntax tree, with the hope that this approach will be better suited to editor integrations and language tooling.

> [!NOTE]
> This project is an unofficial alternative grammar and is not affiliated with or endorsed by the maintainers of caddy server.
>
> Please see [caddyserver/tree-sitter-caddyfile](https://github.com/caddyserver/tree-sitter-caddyfile) if you are searching for the official grammar maintained by caddy.

## Links
- [Documentation](./docs/usage.md)
- [Playground](https://swonky.github.io/tree-sitter-caddyfile)
- [Third party notices](./NOTICE.md)

## Features

### Base syntax
The grammar supports the Caddyfile language syntax.

- **High-level structural parity with the reference parser**
    - High-level structural boundaries between nodes are tested against the caddy reference parser[^1].
- **Hierarchical nesting**
    - **Global block**
    - **Site blocks**
    - **Snippets** (w/ `{block}` and `{blocks.*}` placeholders)
    - **Named matchers**
    - **Named routes**
    - **Subdirective blocks**
    - **Variable definitions**
- **Additional nested structure**
    - **Primitive value types**: `literal_string`, `integer`, `decimal`, 
    - **Compound value types**: 
        - Templatable strings.
        - Complex header syntax.
        - Go-style durations and byte sizes (eg. `2m` `50MiB`).
        - URL and network addresses (eg. domains, ipv4, ipv6, mac addresses, unix sockets)
    - **Substitution**: 
        - `environment_variable` with default values.
        - `placeholder` with namespacing and module-specific highlighting.
        - `arguments` with Go-style slice indexing. 
- **Language injection**
    - Heredocs will highlight according to the label term (eg. "<<HTML" or "<<JSON")[^5]
    - Grave (`\``) quoted cel expressions[^6]
    - `expression` matcher cel expressions[^6]
    - regular expressions[^9]
- **Code navigation**
    - Query files for definitions and references.

## Examples
<p align="center">
  <img src="docs/example0.svg" width="48%">
  <img src="docs/example1.svg" width="48%">
</p>

## References
[^1]: [Caddyfile Go Module](https://pkg.go.dev/github.com/caddyserver/caddy/v2/caddyconfig/caddyfile)
[^5]: Language injection requires the associated tree-sitter grammar for that language.
[^6]: requires [tree-sitter-cel](https://github.com/bufbuild/tree-sitter-cel) or alternative.
[^9]: requires [tree-sitter-regex](https://github.com/tree-sitter/tree-sitter-regex) or alternative.
