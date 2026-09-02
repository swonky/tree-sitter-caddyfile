# tree-sitter-caddyfile

A [Caddyfile](https://caddyserver.com) grammar for the [Tree-sitter](https://github.com/tree-sitter/tree-sitter) parser generator.

> [!NOTE]
> This project is an unofficial alternative grammar and is not affiliated with or endorsed by the maintainers of caddy server.
>
> Please see [caddyserver/tree-sitter-caddyfile](https://github.com/caddyserver/tree-sitter-caddyfile) if you are searching for the official grammar maintained by caddy.

## Links
- [Documentation](./docs/usage.md)
- [Concrete syntax tree](./docs/cst.md)
- [Third party notices](./NOTICE.md)

## Features

### Base syntax
The grammar supports the following Caddyfile language syntax structures:

- **Primitive value types**: `literal_string`, `integer`, `decimal`, 
- **Compound value types**: 
    - Templatable strings
    - Go-style strings for durations and byte sizes. 
    - URL and network addresses incl. domains, ipv4, ipv6, mac addresses, unix sockets.
    - Port ranges.
- **Substitution**: 
    - `environment_variable` with default values.
    - `placeholder` with namespacing and module-specific highlighting.
    - `arguments` with Go-style slice indexing. 
- **Global block**
- **Site blocks**
- **Snippet** definitions and references
- **Named matchers**
- **Named routes**
- **Directives** with namespace support, with arguments and/or subdirective blocks
- **Negations** (`not` keyword and `!` operator)
- **Comments**
- **Language injection**
    - Heredocs will highlight according to the label term (eg. "<<HTML" or "<<JSON")[^5]
    - Grave (`\``) quoted cel expressions[^6]
    - `expression` matcher cel expressions[^6]
    - regular expressions[^9]

## Examples
<p align="center">
  <img src="docs/example0.svg" width="48%">
  <img src="docs/example1.svg" width="48%">
</p>

## References
[^5]: Language injection requires the associated tree-sitter grammar for that language.
[^6]: requires [tree-sitter-cel](https://github.com/bufbuild/tree-sitter-cel) or alternative.
[^9]: requires [tree-sitter-regex](https://github.com/tree-sitter/tree-sitter-regex) or alternative.
