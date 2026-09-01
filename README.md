
# tree-sitter-caddyfile

A [Caddyfile](https://caddyserver.com) grammar for the [Tree-sitter](https://github.com/tree-sitter/tree-sitter) parser generator.

> [!NOTE]
> This project is not affiliated with or endorsed by the maintainers of caddy server.

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

### To do
- [ ] More comprehensive tests

### Disclaimer
To accommodate Caddyfile's highly contextual semantics, the parser is highly dependant on a handwritten custom scanner written in C11.

The complexity of the scanner increases the chance of fatal errors. While the parser currently tolerates fuzzed inputs, I urge caution until automated testing is implemented and an official release is posted.

### Scheme files 
The following scheme files[^8] are included in [./queries](queries/) to facilitate editor integrations and syntax highlight.

| File | Content |
| ---- | ------- |
| [highlights.scm](queries/highlights.scm)  | syntax highlighting & spell check  |
| [injections.scm](queries/injections.scm)  | language injection[^7]             |
| [folds.scm](queries/folds.scm)            | code folding                       |

### Language bindings
Bindings are available under [./bindings](bindings/) for C, Go, Java, JavaScript, Python, Rust, Swift, and Zig.

## Development

### Testing [^4]
The repository contains a several tests located within [./test/corpus](test/corpus/).

> [!TIP]
> Running the tests requires [tree-sitter-cli](https://github.com/tree-sitter/tree-sitter/blob/master/crates/cli/README.md).

```sh
# runs all tests in the corpus
tree-sitter test

# perform parser fuzzing
tree-sitter fuzz
```

| File | Content |
| ---- | ------- |
| [official.txt](test/corpus/official.txt) | adapted from the official tree-sitter-caddyfile repository [^9].

## Example
### Syntax highlighting
![Syntax highlighting example](docs/example.svg)

### Concrete syntax tree
```
0:0   - 95:0    caddyfile
0:0   - 5:1       global_block
0:0   - 0:1         "{"
1:1   - 1:20        statement
1:1   - 1:6           directive
1:1   - 1:6             identifier `email`
1:7   - 1:20          argument: address
1:7   - 1:10            user: literal_string `you`
1:10  - 1:11            "@"
1:11  - 1:20            host: domain_name
1:11  - 1:16              segment: literal_string `yours`
1:16  - 1:17              "."
1:17  - 1:20              segment: literal_string `com`
2:1   - 4:2         statement
2:1   - 2:8           directive
2:1   - 2:8             identifier `servers`
2:9   - 4:2           block
2:9   - 2:10            "{"
3:2   - 3:39            statement
3:2   - 3:17              directive
3:2   - 3:17                identifier `trusted_proxies`
3:18  - 3:24              argument: literal_string `static`
3:25  - 3:39              argument: shortcut `private_ranges`
4:1   - 4:2             "}"
5:0   - 5:1         "}"
7:0   - 95:0      multi_site
7:0   - 12:1        snippet_definition
7:0   - 7:1           "("
7:1   - 7:8           name: identifier `snippet`
7:8   - 7:9           ")"
7:10  - 12:1          block
7:10  - 7:11            "{"
8:1   - 9:0             comment
8:3   - 8:29              content: comment `this is a reusable snippet`
9:1   - 11:2            statement
9:1   - 9:4               directive
9:1   - 9:4                 identifier `log`
9:5   - 11:2              block
9:5   - 9:6                 "{"
10:2  - 10:43               statement
10:2  - 10:8                  directive
10:2  - 10:8                    identifier `output`
10:9  - 10:13                 argument: literal_string `file`
10:14 - 10:43                 argument: path
10:14 - 10:15                   "/"
10:15 - 10:18                   segment: identifier `var`
10:18 - 10:19                   "/"
10:19 - 10:22                   segment: identifier `log`
10:22 - 10:23                   "/"
10:23 - 10:43                   segment: identifier
10:23 - 10:43                     templated_identifier
10:23 - 10:30                       fragment: identifier `access_`
10:30 - 10:39                       fragment: substitution
10:30 - 10:31                         "{"
10:31 - 10:38                         parameter
10:31 - 10:35                           "args"
10:35 - 10:36                           "["
10:36 - 10:37                           index: integer `0`
10:37 - 10:38                           "]"
10:38 - 10:39                         "}"
10:39 - 10:43                       fragment: identifier `.log`
11:1  - 11:2                "}"
12:0  - 12:1            "}"
14:0  - 16:1        named_route_definition
14:0  - 14:1          "&"
14:1  - 14:2          "("
14:2  - 14:11         name: identifier `app-proxy`
14:11 - 14:12         ")"
14:13 - 16:1          block
14:13 - 14:14           "{"
15:1  - 15:42           statement
15:1  - 15:14             directive
15:1  - 15:14               identifier `reverse_proxy`
15:15 - 15:42             argument: templated_string
15:15 - 15:42               fragment: substitution
15:15 - 15:16                 "{"
15:16 - 15:41                 environment_variable
15:16 - 15:17                   "$"
15:17 - 15:26                   name: identifier `UPSTREAMS`
15:26 - 15:27                   ":"
15:27 - 15:41                   default: address
15:27 - 15:36                     host: domain_name
15:27 - 15:36                       segment: literal_string `localhost`
15:36 - 15:37                     ":"
15:37 - 15:41                     port: integer `9000`
15:41 - 15:42                 "}"
16:0  - 16:1            "}"
18:0  - 22:1        site_definition
18:0  - 18:16         site: address
18:0  - 18:4            scheme: literal_string `http`
18:4  - 18:7            "://"
18:7  - 18:16           host: domain_name
18:7  - 18:16             segment: literal_string `localhost`
18:16 - 18:21         site: address
18:16 - 18:17           ":"
18:17 - 18:21           port: integer `3000`
18:21 - 18:22         ","
19:0  - 19:18         site: address
19:0  - 19:4            scheme: literal_string `http`
19:4  - 19:7            "://"
19:7  - 19:18           host: ipv4 `192.168.1.1`
19:18 - 19:23         site: address
19:18 - 19:19           ":"
19:19 - 19:23           port: integer `3000`
19:23 - 19:24         ","
20:0  - 20:25         site: address
20:0  - 20:4            scheme: literal_string `http`
20:4  - 20:7            "://"
20:7  - 20:25           host: ipv6
20:7  - 20:8              "["
20:8  - 20:12             hextet: integer `2001`
20:12 - 20:13             ":"
20:13 - 20:16             hextet: literal_string `db8`
20:16 - 20:17             ":"
20:17 - 20:18             ":"
20:18 - 20:19             hextet: integer `1`
20:19 - 20:20             "%"
20:20 - 20:24             zone: literal_string `eth0`
20:24 - 20:25             "]"
20:25 - 20:30         site: address
20:25 - 20:26           ":"
20:26 - 20:30           port: integer `3000`
20:31 - 22:1          block
20:31 - 20:32           "{"
21:1  - 21:20           statement
21:1  - 21:12             directive
21:1  - 21:12               identifier `file_server`
21:13 - 21:20             argument: path
21:13 - 21:14               "/"
21:14 - 21:20               segment: identifier `static`
22:0  - 22:1            "}"
24:0  - 33:1        site_definition
24:0  - 24:13         site: address
24:0  - 24:13           host: domain_name
24:0  - 24:1              segment: literal_string `*`
24:1  - 24:2              "."
24:2  - 24:9              segment: literal_string `example`
24:9  - 24:10             "."
24:10 - 24:13             segment: literal_string `com`
24:14 - 33:1          block
24:14 - 24:15           "{"
25:1  - 32:2            statement
25:1  - 25:4              directive
25:1  - 25:4                identifier `tls`
25:5  - 32:2              block
25:5  - 25:6                "{"
26:2  - 30:3                statement
26:2  - 26:5                  directive
26:2  - 26:5                    identifier `dns`
26:6  - 26:17                 argument: literal_string `myregistrar`
26:18 - 30:3                  block
26:18 - 26:19                   "{"
27:3  - 27:45                   statement
27:3  - 27:10                     directive
27:3  - 27:10                       identifier `api_key`
27:11 - 27:45                     argument: templated_string
27:11 - 27:45                       fragment: substitution
27:11 - 27:12                         "{"
27:12 - 27:44                         placeholder
27:12 - 27:16                           module: identifier
27:12 - 27:16                             "file"
27:16 - 27:17                           "."
27:17 - 27:44                           member: path
27:17 - 27:18                             "/"
27:18 - 27:21                             segment: literal_string `run`
27:21 - 27:22                             "/"
27:22 - 27:29                             segment: literal_string `secrets`
27:29 - 27:30                             "/"
27:30 - 27:44                             segment: templated_string
27:30 - 27:44                               fragment: substitution
27:30 - 27:31                                 "{"
27:31 - 27:43                                 environment_variable
27:31 - 27:32                                   "$"
27:32 - 27:43                                   name: identifier `SECRET_FILE`
27:43 - 27:44                                 "}"
27:44 - 27:45                         "}"
28:3  - 28:10                   statement
28:3  - 28:7                      directive
28:3  - 28:7                        identifier `user`
28:8  - 28:10                     argument: literal_string `me`
29:3  - 29:50                   statement
29:3  - 29:15                     directive
29:3  - 29:15                       identifier `api_endpoint`
29:16 - 29:50                     argument: address
29:16 - 29:21                       scheme: literal_string `https`
29:21 - 29:24                       "://"
29:24 - 29:43                       host: domain_name
29:24 - 29:27                         segment: literal_string `api`
29:27 - 29:28                         "."
29:28 - 29:39                         segment: literal_string `myregistrar`
29:39 - 29:40                         "."
29:40 - 29:43                         segment: literal_string `com`
29:43 - 29:50                       path: path
29:43 - 29:44                         "/"
29:44 - 29:46                         segment: identifier `v1`
29:46 - 29:47                         "/"
29:47 - 29:50                         segment: identifier `api`
30:2  - 30:3                    "}"
31:2  - 31:62               statement
31:2  - 31:11                 directive
31:2  - 31:11                   identifier `resolvers`
31:12 - 31:20                 argument: literal_string `argument`
31:21 - 31:41                 argument: address
31:21 - 31:41                   host: domain_name
31:21 - 31:25                     segment: literal_string `dns1`
31:25 - 31:26                     "."
31:26 - 31:37                     segment: literal_string `myregistrar`
31:37 - 31:38                     "."
31:38 - 31:41                     segment: literal_string `com`
31:42 - 31:62                 argument: address
31:42 - 31:62                   host: domain_name
31:42 - 31:46                     segment: literal_string `dns2`
31:46 - 31:47                     "."
31:47 - 31:58                     segment: literal_string `myregistrar`
31:58 - 31:59                     "."
31:59 - 31:62                     segment: literal_string `com`
32:1  - 32:2                "}"
33:0  - 33:1            "}"
35:0  - 43:1        site_definition
35:0  - 35:22         site: address
35:0  - 35:22           host: domain_name
35:0  - 35:10             segment: templated_string
35:0  - 35:10               fragment: substitution
35:0  - 35:1                  "{"
35:1  - 35:9                  environment_variable
35:1  - 35:2                    "$"
35:2  - 35:5                    name: identifier `FOO`
35:5  - 35:6                    ":"
35:6  - 35:9                    default: literal_string `foo`
35:9  - 35:10                 "}"
35:10 - 35:11             "."
35:11 - 35:18             segment: literal_string `example`
35:18 - 35:19             "."
35:19 - 35:22             segment: literal_string `com`
35:23 - 43:1          block
35:23 - 35:24           "{"
36:1  - 36:30           statement
36:1  - 36:7              directive
36:1  - 36:7                identifier `header`
36:8  - 36:20             argument: literal_string `Content-Type`
36:21 - 36:30             argument: address
36:21 - 36:25               host: domain_name
36:21 - 36:25                 segment: literal_string `text`
36:25 - 36:30               path: path
36:25 - 36:26                 "/"
36:26 - 36:30                 segment: identifier `html`
37:1  - 42:10           statement
37:1  - 37:8              directive
37:1  - 37:8                identifier `respond`
37:9  - 42:6              argument: heredoc
37:9  - 37:11               "<<"
37:11 - 37:15               heredoc_tag `HTML`
37:15 - 42:2                heredoc_content
37:15 - 37:16                 `\n`
38:15 - 38:9                  `\t\t<html>\n`
39:15 - 39:35                 `\t\t\t<head><title>Foo</title></head>\n`
40:15 - 40:23                 `\t\t\t<body>Bar..?</body>\n`
41:15 - 41:10                 `\t\t</html>\n`
42:15 - 42:2                  `\t\t`
42:2  - 42:6                heredoc_suffix `HTML`
42:7  - 42:10             argument: integer `200`
43:0  - 43:1            "}"
46:0  - 63:1        site_definition
46:0  - 46:15         site: address
46:0  - 46:15           host: domain_name
46:0  - 46:3              segment: literal_string `bar`
46:3  - 46:4              "."
46:4  - 46:11             segment: literal_string `example`
46:11 - 46:12             "."
46:12 - 46:15             segment: literal_string `com`
46:16 - 63:1          block
46:16 - 46:17           "{"
47:1  - 50:2            named_matcher_definition
47:1  - 47:2              "@"
47:2  - 47:6              name: identifier `post`
47:7  - 50:2              request_matcher
47:7  - 50:2                block
47:7  - 47:8                  "{"
48:2  - 48:14                 request_matcher
48:2  - 48:3                    modifier: negative
48:2  - 48:3                      "!"
48:3  - 48:9                    matcher: identifier `method`
48:10 - 48:14                   argument: verb `POST`
49:2  - 49:44                 request_matcher
49:2  - 49:5                    modifier: negative
49:2  - 49:5                      "not"
49:6  - 49:17                   matcher: identifier `path_regexp`
49:18 - 49:44                   argument: regular_expression `\\.([a-f0-9]{6})\\.(css|js)$`
50:1  - 50:2                  "}"
51:1  - 53:2            statement
51:1  - 51:14             directive
51:1  - 51:14               identifier `reverse_proxy`
51:15 - 51:20             matcher: matcher
51:15 - 51:20               named_matcher_reference
51:15 - 51:16                 "@"
51:16 - 51:20                 name: identifier `post`
51:21 - 51:36             argument: address
51:21 - 51:31               host: domain_name
51:21 - 51:31                 segment: templated_string
51:21 - 51:31                   fragment: substitution
51:21 - 51:22                     "{"
51:22 - 51:30                     placeholder
51:22 - 51:25                       module: identifier
51:22 - 51:25                         "env"
51:25 - 51:26                       "."
51:26 - 51:30                       reference: identifier `SRV0`
51:30 - 51:31                     "}"
51:31 - 51:32               ":"
51:32 - 51:36               port: integer `9001`
51:37 - 51:57             argument: address
51:37 - 51:47               host: domain_name
51:37 - 51:47                 segment: templated_string
51:37 - 51:47                   fragment: substitution
51:37 - 51:38                     "{"
51:38 - 51:46                     placeholder
51:38 - 51:41                       module: identifier
51:38 - 51:41                         "env"
51:41 - 51:42                       "."
51:42 - 51:46                       reference: identifier `SRV1`
51:46 - 51:47                     "}"
51:47 - 51:48               ":"
51:48 - 51:57               port: range
51:48 - 51:52                 left: integer `9002`
51:52 - 51:53                 "-"
51:53 - 51:57                 right: integer `9010`
51:58 - 53:2              block
51:58 - 51:59               "{"
52:2  - 52:17               statement
52:2  - 52:11                 directive
52:2  - 52:11                   identifier `lb_policy`
52:12 - 52:17                 argument: literal_string `first`
53:1  - 53:2                "}"
55:1  - 55:44           statement
55:1  - 55:14             directive
55:1  - 55:14               identifier `reverse_proxy`
55:15 - 55:16             matcher: matcher
55:15 - 55:16               wildcard
55:15 - 55:16                 "*"
55:17 - 55:44             argument: network_address
55:17 - 55:21               network: protocol `unix`
55:21 - 55:22               "+"
55:22 - 55:25               network: protocol `h2c`
55:25 - 55:26               "/"
55:26 - 55:44               address: path
55:26 - 55:27                 "/"
55:27 - 55:31                 segment: identifier `path`
55:31 - 55:32                 "/"
55:32 - 55:34                 segment: identifier `to`
55:34 - 55:35                 "/"
55:35 - 55:37                 segment: identifier `my`
55:37 - 55:44                 segment: identifier `.socket`
57:1  - 60:2            statement
57:1  - 57:7              directive
57:1  - 57:7                identifier `handle`
57:8  - 57:12             argument: path
57:8  - 57:9                "/"
57:9  - 57:12               segment: identifier `srv`
57:13 - 60:2              block
57:13 - 57:14               "{"
58:2  - 58:11               statement
58:2  - 58:6                  directive
58:2  - 58:6                    identifier `root`
58:7  - 58:11                 argument: path
58:7  - 58:8                    "/"
58:8  - 58:11                   segment: identifier `srv`
58:11 - 59:0                comment
58:14 - 58:40                 content: comment `this is an inline comment!`
59:2  - 59:13               statement
59:2  - 59:13                 directive
59:2  - 59:13                   identifier `file_server`
60:1  - 60:2                "}"
61:1  - 61:24           statement
61:1  - 61:8              directive
61:1  - 61:8                identifier `respond`
61:9  - 61:24             argument: quoted_expression
61:9  - 61:10               "\""
61:10 - 61:23               content: literal_string `Hello, World!`
61:23 - 61:24               "\""
62:1  - 62:19           snippet_reference
62:1  - 62:7              "import"
62:8  - 62:15             snippet: identifier `snippet`
62:16 - 62:19             argument: literal_string `www`
63:0  - 63:1            "}"
65:0  - 76:1        site_definition
65:0  - 65:15         site: address
65:0  - 65:15           host: domain_name
65:0  - 65:3              segment: literal_string `wol`
65:3  - 65:4              "."
65:4  - 65:11             segment: literal_string `example`
65:11 - 65:12             "."
65:12 - 65:15             segment: literal_string `com`
65:16 - 76:1          block
65:16 - 65:17           "{"
66:8  - 66:40           statement
66:8  - 66:21             directive
66:8  - 66:21               identifier `reverse_proxy`
66:22 - 66:40             argument: address
66:22 - 66:35               host: ipv4 `192.168.0.100`
66:35 - 66:36               ":"
66:36 - 66:40               port: integer `8096`
67:8  - 75:9            statement
67:8  - 67:21             directive
67:8  - 67:21               identifier `handle_errors`
67:22 - 75:9              block
67:22 - 67:23               "{"
68:16 - 68:56               named_matcher_definition
68:16 - 68:17                 "@"
68:17 - 68:20                 name: identifier `502`
68:21 - 68:56                 request_matcher
68:21 - 68:31                   matcher: identifier `expression`
68:32 - 68:56                   argument: cel_expression `{err.status_code} == 502`
69:16 - 74:17               statement
69:16 - 69:22                 directive
69:16 - 69:22                   identifier `handle`
69:23 - 69:27                 matcher: matcher
69:23 - 69:27                   named_matcher_reference
69:23 - 69:24                     "@"
69:24 - 69:27                     name: identifier `502`
69:28 - 74:17                 block
69:28 - 69:29                   "{"
70:24 - 70:53                   statement
70:24 - 70:35                     directive
70:24 - 70:35                       identifier `wake_on_lan`
70:36 - 70:53                     argument: mac_address
70:36 - 70:38                       octet: byte `00`
70:38 - 70:39                       ":"
70:39 - 70:41                       octet: byte `11`
70:41 - 70:42                       ":"
70:42 - 70:44                       octet: byte `22`
70:44 - 70:45                       ":"
70:45 - 70:47                       octet: byte `33`
70:47 - 70:48                       ":"
70:48 - 70:50                       octet: byte `44`
70:50 - 70:51                       ":"
70:51 - 70:53                       octet: byte `55`
71:24 - 73:25                   statement
71:24 - 71:37                     directive
71:24 - 71:37                       identifier `reverse_proxy`
71:38 - 71:56                     argument: address
71:38 - 71:51                       host: ipv4 `192.168.0.100`
71:51 - 71:52                       ":"
71:52 - 71:56                       port: integer `8096`
71:57 - 73:25                     block
71:57 - 71:58                       "{"
72:32 - 72:52                       statement
72:32 - 72:47                         directive
72:32 - 72:47                           identifier `lb_try_duration`
72:48 - 72:52                         argument: amount
72:48 - 72:51                           quantity: integer `120`
72:51 - 72:52                           unit: duration `s`
73:24 - 73:25                       "}"
74:16 - 74:17                   "}"
75:8  - 75:9                "}"
76:0  - 76:1            "}"
78:0  - 87:1        site_definition
78:0  - 78:17         site: address
78:0  - 78:17           host: domain_name
78:0  - 78:5              segment: literal_string `test1`
78:5  - 78:6              "."
78:6  - 78:13             segment: literal_string `example`
78:13 - 78:14             "."
78:14 - 78:17             segment: literal_string `com`
78:18 - 78:35         site: address
78:18 - 78:35           host: domain_name
78:18 - 78:23             segment: literal_string `test2`
78:23 - 78:24             "."
78:24 - 78:31             segment: literal_string `example`
78:31 - 78:32             "."
78:32 - 78:35             segment: literal_string `com`
78:36 - 87:1          block
78:36 - 78:37           "{"
79:1  - 84:2            variable_declaration
79:1  - 79:5              "vars"
79:6  - 79:7              "{"
80:2  - 80:10             assignment
80:2  - 80:5                key: identifier `abc`
80:6  - 80:10               value: boolean `true`
81:2  - 81:7              assignment
81:2  - 81:5                key: identifier `def`
81:6  - 81:7                value: integer `1`
82:2  - 82:9              assignment
82:2  - 82:5                key: identifier `ghi`
82:6  - 82:9                value: decimal `2.3`
83:2  - 83:15             assignment
83:2  - 83:5                key: identifier `jkl`
83:6  - 83:15               value: quoted_expression
83:6  - 83:7                  "\""
83:7  - 83:14                 content: literal_string `example`
83:14 - 83:15                 "\""
84:1  - 84:2              "}"
85:1  - 85:69           statement
85:1  - 85:6              directive
85:1  - 85:6                identifier `redir`
85:7  - 85:69             argument: address
85:7  - 85:12               scheme: literal_string `https`
85:12 - 85:15               "://"
85:15 - 85:35               host: domain_name
85:15 - 85:18                 segment: literal_string `www`
85:18 - 85:19                 "."
85:19 - 85:26                 segment: literal_string `example`
85:26 - 85:27                 "."
85:27 - 85:35                 segment: templated_string
85:27 - 85:30                   fragment: literal_string `com`
85:30 - 85:35                   fragment: substitution
85:30 - 85:31                     "{"
85:31 - 85:34                     placeholder
85:31 - 85:34                       member: identifier `uri`
85:34 - 85:35                     "}"
85:35 - 85:40               path: path
85:35 - 85:36                 "/"
85:36 - 85:40                 segment: identifier `path`
85:40 - 85:60               query: query
85:40 - 85:41                 "?"
85:41 - 85:50                 mapping
85:41 - 85:44                   key: identifier `key`
85:44 - 85:45                   "="
85:45 - 85:50                   value: identifier `value`
85:50 - 85:51                 "&"
85:51 - 85:60                 mapping
85:51 - 85:54                   key: identifier `key`
85:54 - 85:55                   "="
85:55 - 85:60                   value: identifier `value`
85:60 - 85:61               "#"
85:61 - 85:69               fragment: literal_string `fragment`
86:1  - 86:21           snippet_reference
86:1  - 86:7              "import"
86:8  - 86:15             snippet: identifier `snippet`
86:16 - 86:21             argument: address
86:16 - 86:17               ":"
86:17 - 86:21               port: integer `3000`
87:0  - 87:1            "}"
89:0  - 94:1        site_definition
89:0  - 89:11         site: address
89:0  - 89:11           host: domain_name
89:0  - 89:7              segment: literal_string `example`
89:7  - 89:8              "."
89:8  - 89:11             segment: literal_string `com`
89:12 - 94:1          block
89:12 - 89:13           "{"
90:1  - 90:50           named_matcher_definition
90:1  - 90:2              "@"
90:2  - 90:8              name: identifier `denied`
90:9  - 90:50             request_matcher
90:9  - 90:50               embedded_content
90:9  - 90:10                 "\`"
90:10 - 90:49                 cel_expression `client_ip('12.23.34.45', '23.34.45.56')`
90:49 - 90:50                 "\`"
91:1  - 91:14           statement
91:1  - 91:6              directive
91:1  - 91:6                identifier `abort`
91:7  - 91:14             matcher: matcher
91:7  - 91:14               named_matcher_reference
91:7  - 91:8                  "@"
91:8  - 91:14                 name: identifier `denied`
92:1  - 92:17           invoke_statement
92:1  - 92:7              "invoke"
92:8  - 92:17             route: identifier `app-proxy`
93:1  - 93:19           snippet_reference
93:1  - 93:7              "import"
93:8  - 93:15             snippet: identifier `snippet`
93:16 - 93:19             argument: literal_string `app`
94:0  - 94:1            "}"
```

## References
[^5]: Language injection requires the associated tree-sitter grammar for that language.
[^6]: requires [tree-sitter-cel](https://github.com/bufbuild/tree-sitter-cel) or alternative.
[^9]: requires [tree-sitter-regex](https://github.com/tree-sitter/tree-sitter-regex) or alternative.
[^7]: [tree-sitter: Language injection](https://tree-sitter.github.io/tree-sitter/3-syntax-highlighting.html#language-injection)
[^8]: [tree-sitter: Query syntax](https://tree-sitter.github.io/tree-sitter/using-parsers/queries/1-syntax.html)
[^8]: [Official tree-sitter-caddyfile tests](https://github.com/caddyserver/tree-sitter-caddyfile/tree/8ee969d8fd68d67661016d890110e4cae18ed03c/test/corpus)
