# Example concrete syntax tree

This document provides an example concrete syntax tree structure for a caddyfile sample.

> [!NOTE]
> The following sample is note highlighted using this parser, but rather the official tree-sitter-caddyfile parser.
>
> See [example.svg](docs/example.svg) for a more accurate showcase of syntax highlighting.

## Source code
```caddyfile
{
	email you@yours.com
	servers {
		trusted_proxies static private_ranges
	}
	admin caddy:2019 {
		origins 172.16.2.2:2019
	}
}

(snippet) {
	# this is a reusable snippet
	log {
		output file /var/log/access_{args[0]}.log
	}
}

# an example named route
&(app-proxy) {
	reverse_proxy {$UPSTREAMS:localhost:9000}
}

http://localhost:3000,
http://192.168.1.1:3000,
http://[2001:db8::1%eth0]:3000 {
	file_server /static
}

*.example.com {
	tls {
		dns myregistrar {
			api_key {file./run/secrets/{$SECRET_FILE}}
			user me
			api_endpoint https://api.myregistrar.com/v1/api
		}
		resolvers argument dns1.myregistrar.com dns2.myregistrar.com
	}
}

{$FOO:foo}.example.com {
	header Content-Type text/html

	respond <<HTML
		<html>
			<head><title>Foo</title></head>
			<body>Bar..?</body>
		</html>
		HTML 200
}
bar.example.com {
	@post { 
		!method POST
		not path_regexp \.([a-f0-9]{6})\.(css|js)$
	}
	reverse_proxy @post {env.SRV0}:9001 {env.SRV1}:9002-9010 {
		lb_policy first
	}

	reverse_proxy * unix+h2c//path/to/my.socket

	handle /srv {
		root /srv # this is an inline comment!
		file_server
	}

	respond "Hello, World!"
}

wol.example.com {
        reverse_proxy 192.168.0.100:8096

        handle_errors {
                @502 expression {err.status_code} == 502
                handle @502 {
                        wake_on_lan 00:11:22:33:44:55
                        reverse_proxy 192.168.0.100:8096 {
                                lb_try_duration 120s
                        }
                }
        }
}

test1.example.com test2.example.com {
	vars {
		abc true
		def {time.now.unix}
		ghi 2.3
	}
	redir https://www.example.com{uri}/path?key=value&key=value#fragment
	import snippet :3000
}

app.example.com {
	@denied `client_ip('12.23.34.45', '23.34.45.56')`
	abort @denied
	invoke app-proxy
	import snippet app
}
```

## Concrete syntax tree
```
0:0   - 98:0    caddyfile
0:0   - 8:1       global_options
0:0   - 0:1         "{"
1:1   - 1:20        statement
1:1   - 1:6           directive
1:1   - 1:6             name: identifier `email`
1:7   - 1:20          argument: address
1:7   - 1:10            user: literal_string `you`
1:10  - 1:11            "@"
1:11  - 1:20            authority
1:11  - 1:20              host: domain_name
1:11  - 1:16                segment: literal_string `yours`
1:16  - 1:17                "."
1:17  - 1:20                segment: literal_string `com`
2:1   - 4:2         statement
2:1   - 2:8           directive
2:1   - 2:8             name: identifier `servers`
2:9   - 4:2           block
2:9   - 2:10            "{"
3:2   - 3:39            statement
3:2   - 3:17              directive
3:2   - 3:17                name: identifier `trusted_proxies`
3:18  - 3:24              argument: literal_string `static`
3:25  - 3:39              argument: shortcut `private_ranges`
4:1   - 4:2             "}"
5:1   - 7:2         statement
5:1   - 5:6           directive
5:1   - 5:6             name: identifier `admin`
5:7   - 5:17          argument: address
5:7   - 5:17            authority
5:7   - 5:12              host: domain_name
5:7   - 5:12                segment: literal_string `caddy`
5:12  - 5:13              ":"
5:13  - 5:17              port: integer `2019`
5:18  - 7:2           block
5:18  - 5:19            "{"
6:2   - 6:25            statement
6:2   - 6:9               directive
6:2   - 6:9                 name: identifier `origins`
6:10  - 6:25              argument: address
6:10  - 6:25                authority
6:10  - 6:20                  host: ipv4 `172.16.2.2`
6:20  - 6:21                  ":"
6:21  - 6:25                  port: integer `2019`
7:1   - 7:2             "}"
8:0   - 8:1         "}"
10:0  - 98:0      multi_site
10:0  - 15:1        snippet_definition
10:0  - 10:1          "("
10:1  - 10:8          name: identifier `snippet`
10:8  - 10:9          ")"
10:10 - 15:1          block
10:10 - 10:11           "{"
11:1  - 12:0            comment
11:3  - 11:29             content: comment `this is a reusable snippet`
12:1  - 14:2            statement
12:1  - 12:4              directive
12:1  - 12:4                name: identifier `log`
12:5  - 14:2              block
12:5  - 12:6                "{"
13:2  - 13:43               statement
13:2  - 13:8                  directive
13:2  - 13:8                    name: identifier `output`
13:9  - 13:13                 argument: literal_string `file`
13:14 - 13:43                 argument: path
13:14 - 13:15                   "/"
13:15 - 13:18                   segment: identifier `var`
13:18 - 13:19                   "/"
13:19 - 13:22                   segment: identifier `log`
13:22 - 13:23                   "/"
13:23 - 13:43                   segment: identifier
13:23 - 13:43                     templated_identifier
13:23 - 13:30                       fragment: identifier `access_`
13:30 - 13:39                       fragment: substitution
13:30 - 13:31                         "{"
13:31 - 13:38                         parameter
13:31 - 13:35                           "args"
13:35 - 13:36                           "["
13:36 - 13:37                           index: integer `0`
13:37 - 13:38                           "]"
13:38 - 13:39                         "}"
13:39 - 13:43                       fragment: identifier `.log`
14:1  - 14:2                "}"
15:0  - 15:1            "}"
17:0  - 18:0        comment
17:2  - 17:24         content: comment `an example named route`
18:0  - 20:1        named_route_definition
18:0  - 18:1          "&"
18:1  - 18:2          "("
18:2  - 18:11         name: identifier `app-proxy`
18:11 - 18:12         ")"
18:13 - 20:1          block
18:13 - 18:14           "{"
19:1  - 19:42           statement
19:1  - 19:14             directive
19:1  - 19:14               name: identifier `reverse_proxy`
19:15 - 19:42             argument: templated_string
19:15 - 19:42               fragment: substitution
19:15 - 19:16                 "{"
19:16 - 19:41                 environment_variable
19:16 - 19:17                   "$"
19:17 - 19:26                   name: identifier `UPSTREAMS`
19:26 - 19:27                   ":"
19:27 - 19:41                   default: address
19:27 - 19:41                     authority
19:27 - 19:36                       host: domain_name
19:27 - 19:36                         segment: literal_string `localhost`
19:36 - 19:37                       ":"
19:37 - 19:41                       port: integer `9000`
19:41 - 19:42                 "}"
20:0  - 20:1            "}"
22:0  - 26:1        site_definition
22:0  - 22:21         site: address
22:0  - 22:4            scheme: literal_string `http`
22:4  - 22:7            "://"
22:7  - 22:21           authority
22:7  - 22:16             host: domain_name
22:7  - 22:16               segment: literal_string `localhost`
22:16 - 22:17             ":"
22:17 - 22:21             port: integer `3000`
22:21 - 22:22         ","
23:0  - 23:23         site: address
23:0  - 23:4            scheme: literal_string `http`
23:4  - 23:7            "://"
23:7  - 23:23           authority
23:7  - 23:18             host: ipv4 `192.168.1.1`
23:18 - 23:19             ":"
23:19 - 23:23             port: integer `3000`
23:23 - 23:24         ","
24:0  - 24:30         site: address
24:0  - 24:4            scheme: literal_string `http`
24:4  - 24:7            "://"
24:7  - 24:30           authority
24:7  - 24:25             host: ipv6
24:7  - 24:8                "["
24:8  - 24:12               hextet: integer `2001`
24:12 - 24:13               ":"
24:13 - 24:16               hextet: literal_string `db8`
24:16 - 24:17               ":"
24:17 - 24:18               ":"
24:18 - 24:19               hextet: integer `1`
24:19 - 24:20               "%"
24:20 - 24:24               zone: literal_string `eth0`
24:24 - 24:25               "]"
24:25 - 24:26             ":"
24:26 - 24:30             port: integer `3000`
24:31 - 26:1          block
24:31 - 24:32           "{"
25:1  - 25:20           statement
25:1  - 25:12             directive
25:1  - 25:12               name: identifier `file_server`
25:13 - 25:20             argument: path
25:13 - 25:14               "/"
25:14 - 25:20               segment: identifier `static`
26:0  - 26:1            "}"
28:0  - 37:1        site_definition
28:0  - 28:13         site: address
28:0  - 28:13           authority
28:0  - 28:13             host: domain_name
28:0  - 28:1                segment: literal_string `*`
28:1  - 28:2                "."
28:2  - 28:9                segment: literal_string `example`
28:9  - 28:10               "."
28:10 - 28:13               segment: literal_string `com`
28:14 - 37:1          block
28:14 - 28:15           "{"
29:1  - 36:2            statement
29:1  - 29:4              directive
29:1  - 29:4                name: identifier `tls`
29:5  - 36:2              block
29:5  - 29:6                "{"
30:2  - 34:3                statement
30:2  - 30:5                  directive
30:2  - 30:5                    name: identifier `dns`
30:6  - 30:17                 argument: literal_string `myregistrar`
30:18 - 34:3                  block
30:18 - 30:19                   "{"
31:3  - 31:45                   statement
31:3  - 31:10                     directive
31:3  - 31:10                       name: identifier `api_key`
31:11 - 31:45                     argument: templated_string
31:11 - 31:45                       fragment: substitution
31:11 - 31:12                         "{"
31:12 - 31:44                         file_placeholder
31:12 - 31:16                           "file"
31:16 - 31:17                           "."
31:17 - 31:44                           member: path
31:17 - 31:18                             "/"
31:18 - 31:21                             segment: literal_string `run`
31:21 - 31:22                             "/"
31:22 - 31:29                             segment: literal_string `secrets`
31:29 - 31:30                             "/"
31:30 - 31:44                             segment: templated_string
31:30 - 31:44                               fragment: substitution
31:30 - 31:31                                 "{"
31:31 - 31:43                                 environment_variable
31:31 - 31:32                                   "$"
31:32 - 31:43                                   name: identifier `SECRET_FILE`
31:43 - 31:44                                 "}"
31:44 - 31:45                         "}"
32:3  - 32:10                   statement
32:3  - 32:7                      directive
32:3  - 32:7                        name: identifier `user`
32:8  - 32:10                     argument: literal_string `me`
33:3  - 33:50                   statement
33:3  - 33:15                     directive
33:3  - 33:15                       name: identifier `api_endpoint`
33:16 - 33:50                     argument: address
33:16 - 33:21                       scheme: literal_string `https`
33:21 - 33:24                       "://"
33:24 - 33:43                       authority
33:24 - 33:43                         host: domain_name
33:24 - 33:27                           segment: literal_string `api`
33:27 - 33:28                           "."
33:28 - 33:39                           segment: literal_string `myregistrar`
33:39 - 33:40                           "."
33:40 - 33:43                           segment: literal_string `com`
33:43 - 33:50                       path: path
33:43 - 33:44                         "/"
33:44 - 33:46                         segment: identifier `v1`
33:46 - 33:47                         "/"
33:47 - 33:50                         segment: identifier `api`
34:2  - 34:3                    "}"
35:2  - 35:62               statement
35:2  - 35:11                 directive
35:2  - 35:11                   name: identifier `resolvers`
35:12 - 35:20                 argument: literal_string `argument`
35:21 - 35:41                 argument: address
35:21 - 35:41                   authority
35:21 - 35:41                     host: domain_name
35:21 - 35:25                       segment: literal_string `dns1`
35:25 - 35:26                       "."
35:26 - 35:37                       segment: literal_string `myregistrar`
35:37 - 35:38                       "."
35:38 - 35:41                       segment: literal_string `com`
35:42 - 35:62                 argument: address
35:42 - 35:62                   authority
35:42 - 35:62                     host: domain_name
35:42 - 35:46                       segment: literal_string `dns2`
35:46 - 35:47                       "."
35:47 - 35:58                       segment: literal_string `myregistrar`
35:58 - 35:59                       "."
35:59 - 35:62                       segment: literal_string `com`
36:1  - 36:2                "}"
37:0  - 37:1            "}"
39:0  - 48:1        site_definition
39:0  - 39:22         site: address
39:0  - 39:22           authority
39:0  - 39:22             host: domain_name
39:0  - 39:10               segment: templated_string
39:0  - 39:10                 fragment: substitution
39:0  - 39:1                    "{"
39:1  - 39:9                    environment_variable
39:1  - 39:2                      "$"
39:2  - 39:5                      name: identifier `FOO`
39:5  - 39:6                      ":"
39:6  - 39:9                      default: literal_string `foo`
39:9  - 39:10                   "}"
39:10 - 39:11               "."
39:11 - 39:18               segment: literal_string `example`
39:18 - 39:19               "."
39:19 - 39:22               segment: literal_string `com`
39:23 - 48:1          block
39:23 - 39:24           "{"
40:1  - 40:30           statement
40:1  - 40:7              directive
40:1  - 40:7                name: identifier `header`
40:8  - 40:20             argument: literal_string `Content-Type`
40:21 - 40:30             argument: address
40:21 - 40:25               authority
40:21 - 40:25                 host: domain_name
40:21 - 40:25                   segment: literal_string `text`
40:25 - 40:30               path: path
40:25 - 40:26                 "/"
40:26 - 40:30                 segment: identifier `html`
42:1  - 47:10           statement
42:1  - 42:8              directive
42:1  - 42:8                name: identifier `respond`
42:9  - 47:6              argument: heredoc
42:9  - 42:11               "<<"
42:11 - 42:15               heredoc_tag `HTML`
42:15 - 47:2                heredoc_content
42:15 - 42:16                 `\n`
43:15 - 43:9                  `\t\t<html>\n`
44:15 - 44:35                 `\t\t\t<head><title>Foo</title></head>\n`
45:15 - 45:23                 `\t\t\t<body>Bar..?</body>\n`
46:15 - 46:10                 `\t\t</html>\n`
47:15 - 47:2                  `\t\t`
47:2  - 47:6                heredoc_suffix `HTML`
47:7  - 47:10             argument: integer `200`
48:0  - 48:1            "}"
49:0  - 66:1        site_definition
49:0  - 49:15         site: address
49:0  - 49:15           authority
49:0  - 49:15             host: domain_name
49:0  - 49:3                segment: literal_string `bar`
49:3  - 49:4                "."
49:4  - 49:11               segment: literal_string `example`
49:11 - 49:12               "."
49:12 - 49:15               segment: literal_string `com`
49:16 - 66:1          block
49:16 - 49:17           "{"
50:1  - 53:2            named_matcher_definition
50:1  - 50:2              "@"
50:2  - 50:6              name: identifier `post`
50:7  - 53:2              request_matcher
50:7  - 53:2                block
50:7  - 50:8                  "{"
51:2  - 51:14                 request_matcher
51:2  - 51:3                    modifier: negative
51:2  - 51:3                      "!"
51:3  - 51:9                    matcher: identifier `method`
51:10 - 51:14                   argument: verb `POST`
52:2  - 52:44                 request_matcher
52:2  - 52:5                    modifier: negative
52:2  - 52:5                      "not"
52:6  - 52:17                   matcher: identifier `path_regexp`
52:18 - 52:44                   argument: regular_expression `\\.([a-f0-9]{6})\\.(css|js)$`
53:1  - 53:2                  "}"
54:1  - 56:2            statement
54:1  - 54:14             directive
54:1  - 54:14               name: identifier `reverse_proxy`
54:15 - 54:20             matcher: matcher
54:15 - 54:20               named_matcher_reference
54:15 - 54:16                 "@"
54:16 - 54:20                 name: identifier `post`
54:21 - 54:36             argument: address
54:21 - 54:36               authority
54:21 - 54:31                 host: domain_name
54:21 - 54:31                   segment: templated_string
54:21 - 54:31                     fragment: substitution
54:21 - 54:22                       "{"
54:22 - 54:30                       env_placeholder
54:22 - 54:25                         "env"
54:25 - 54:26                         "."
54:26 - 54:30                         name: identifier `SRV0`
54:30 - 54:31                       "}"
54:31 - 54:32                 ":"
54:32 - 54:36                 port: integer `9001`
54:37 - 54:57             argument: address
54:37 - 54:57               authority
54:37 - 54:47                 host: domain_name
54:37 - 54:47                   segment: templated_string
54:37 - 54:47                     fragment: substitution
54:37 - 54:38                       "{"
54:38 - 54:46                       env_placeholder
54:38 - 54:41                         "env"
54:41 - 54:42                         "."
54:42 - 54:46                         name: identifier `SRV1`
54:46 - 54:47                       "}"
54:47 - 54:48                 ":"
54:48 - 54:57                 port: range
54:48 - 54:52                   left: integer `9002`
54:52 - 54:53                   "-"
54:53 - 54:57                   right: integer `9010`
54:58 - 56:2              block
54:58 - 54:59               "{"
55:2  - 55:17               statement
55:2  - 55:11                 directive
55:2  - 55:11                   name: identifier `lb_policy`
55:12 - 55:17                 argument: literal_string `first`
56:1  - 56:2                "}"
58:1  - 58:44           statement
58:1  - 58:14             directive
58:1  - 58:14               name: identifier `reverse_proxy`
58:15 - 58:16             matcher: matcher
58:15 - 58:16               wildcard
58:15 - 58:16                 "*"
58:17 - 58:44             argument: network_address
58:17 - 58:21               network: protocol `unix`
58:21 - 58:22               "+"
58:22 - 58:25               network: protocol `h2c`
58:25 - 58:26               "/"
58:26 - 58:44               address: path
58:26 - 58:27                 "/"
58:27 - 58:31                 segment: identifier `path`
58:31 - 58:32                 "/"
58:32 - 58:34                 segment: identifier `to`
58:34 - 58:35                 "/"
58:35 - 58:37                 segment: identifier `my`
58:37 - 58:44                 segment: identifier `.socket`
60:1  - 63:2            statement
60:1  - 60:7              directive
60:1  - 60:7                name: identifier `handle`
60:8  - 60:12             argument: path
60:8  - 60:9                "/"
60:9  - 60:12               segment: identifier `srv`
60:13 - 63:2              block
60:13 - 60:14               "{"
61:2  - 61:11               statement
61:2  - 61:6                  directive
61:2  - 61:6                    name: identifier `root`
61:7  - 61:11                 argument: path
61:7  - 61:8                    "/"
61:8  - 61:11                   segment: identifier `srv`
61:11 - 62:0                comment
61:14 - 61:40                 content: comment `this is an inline comment!`
62:2  - 62:13               statement
62:2  - 62:13                 directive
62:2  - 62:13                   name: identifier `file_server`
63:1  - 63:2                "}"
65:1  - 65:24           statement
65:1  - 65:8              directive
65:1  - 65:8                name: identifier `respond`
65:9  - 65:24             argument: quoted_expression
65:9  - 65:10               "\""
65:10 - 65:23               content: literal_string `Hello, World!`
65:23 - 65:24               "\""
66:0  - 66:1            "}"
68:0  - 80:1        site_definition
68:0  - 68:15         site: address
68:0  - 68:15           authority
68:0  - 68:15             host: domain_name
68:0  - 68:3                segment: literal_string `wol`
68:3  - 68:4                "."
68:4  - 68:11               segment: literal_string `example`
68:11 - 68:12               "."
68:12 - 68:15               segment: literal_string `com`
68:16 - 80:1          block
68:16 - 68:17           "{"
69:8  - 69:40           statement
69:8  - 69:21             directive
69:8  - 69:21               name: identifier `reverse_proxy`
69:22 - 69:40             argument: address
69:22 - 69:40               authority
69:22 - 69:35                 host: ipv4 `192.168.0.100`
69:35 - 69:36                 ":"
69:36 - 69:40                 port: integer `8096`
71:8  - 79:9            statement
71:8  - 71:21             directive
71:8  - 71:21               name: identifier `handle_errors`
71:22 - 79:9              block
71:22 - 71:23               "{"
72:16 - 72:56               named_matcher_definition
72:16 - 72:17                 "@"
72:17 - 72:20                 name: identifier `502`
72:21 - 72:56                 request_matcher
72:21 - 72:31                   matcher: identifier `expression`
72:32 - 72:56                   argument: cel_expression `{err.status_code} == 502`
73:16 - 78:17               statement
73:16 - 73:22                 directive
73:16 - 73:22                   name: identifier `handle`
73:23 - 73:27                 matcher: matcher
73:23 - 73:27                   named_matcher_reference
73:23 - 73:24                     "@"
73:24 - 73:27                     name: identifier `502`
73:28 - 78:17                 block
73:28 - 73:29                   "{"
74:24 - 74:53                   statement
74:24 - 74:35                     directive
74:24 - 74:35                       name: identifier `wake_on_lan`
74:36 - 74:53                     argument: mac_address
74:36 - 74:38                       octet: byte `00`
74:38 - 74:39                       ":"
74:39 - 74:41                       octet: byte `11`
74:41 - 74:42                       ":"
74:42 - 74:44                       octet: byte `22`
74:44 - 74:45                       ":"
74:45 - 74:47                       octet: byte `33`
74:47 - 74:48                       ":"
74:48 - 74:50                       octet: byte `44`
74:50 - 74:51                       ":"
74:51 - 74:53                       octet: byte `55`
75:24 - 77:25                   statement
75:24 - 75:37                     directive
75:24 - 75:37                       name: identifier `reverse_proxy`
75:38 - 75:56                     argument: address
75:38 - 75:56                       authority
75:38 - 75:51                         host: ipv4 `192.168.0.100`
75:51 - 75:52                         ":"
75:52 - 75:56                         port: integer `8096`
75:57 - 77:25                     block
75:57 - 75:58                       "{"
76:32 - 76:52                       statement
76:32 - 76:47                         directive
76:32 - 76:47                           name: identifier `lb_try_duration`
76:48 - 76:52                         argument: amount
76:48 - 76:51                           quantity: integer `120`
76:51 - 76:52                           unit: duration `s`
77:24 - 77:25                       "}"
78:16 - 78:17                   "}"
79:8  - 79:9                "}"
80:0  - 80:1            "}"
82:0  - 90:1        site_definition
82:0  - 82:17         site: address
82:0  - 82:17           authority
82:0  - 82:17             host: domain_name
82:0  - 82:5                segment: literal_string `test1`
82:5  - 82:6                "."
82:6  - 82:13               segment: literal_string `example`
82:13 - 82:14               "."
82:14 - 82:17               segment: literal_string `com`
82:18 - 82:35         site: address
82:18 - 82:35           authority
82:18 - 82:35             host: domain_name
82:18 - 82:23               segment: literal_string `test2`
82:23 - 82:24               "."
82:24 - 82:31               segment: literal_string `example`
82:31 - 82:32               "."
82:32 - 82:35               segment: literal_string `com`
82:36 - 90:1          block
82:36 - 82:37           "{"
83:1  - 87:2            variable_declaration
83:1  - 83:5              "vars"
83:6  - 83:7              "{"
84:2  - 84:10             assignment
84:2  - 84:5                key: identifier `abc`
84:6  - 84:10               value: boolean `true`
85:2  - 85:21             assignment
85:2  - 85:5                key: identifier `def`
85:6  - 85:21               value: templated_string
85:6  - 85:21                 fragment: substitution
85:6  - 85:7                    "{"
85:7  - 85:20                   time_placeholder
85:7  - 85:11                     "time"
85:11 - 85:12                     "."
85:12 - 85:15                     "now"
85:15 - 85:16                     "."
85:16 - 85:20                     name: identifier `unix`
85:20 - 85:21                   "}"
86:2  - 86:9              assignment
86:2  - 86:5                key: identifier `ghi`
86:6  - 86:9                value: decimal `2.3`
87:1  - 87:2              "}"
88:1  - 88:69           statement
88:1  - 88:6              directive
88:1  - 88:6                name: identifier `redir`
88:7  - 88:69             argument: address
88:7  - 88:12               scheme: literal_string `https`
88:12 - 88:15               "://"
88:15 - 88:35               authority
88:15 - 88:35                 host: domain_name
88:15 - 88:18                   segment: literal_string `www`
88:18 - 88:19                   "."
88:19 - 88:26                   segment: literal_string `example`
88:26 - 88:27                   "."
88:27 - 88:35                   segment: templated_string
88:27 - 88:30                     fragment: literal_string `com`
88:30 - 88:35                     fragment: substitution
88:30 - 88:31                       "{"
88:31 - 88:34                       generic_placeholder
88:31 - 88:34                         identifier `uri`
88:34 - 88:35                       "}"
88:35 - 88:40               path: path
88:35 - 88:36                 "/"
88:36 - 88:40                 segment: identifier `path`
88:40 - 88:60               query: query
88:40 - 88:41                 "?"
88:41 - 88:50                 mapping
88:41 - 88:44                   key: identifier `key`
88:44 - 88:45                   "="
88:45 - 88:50                   value: identifier `value`
88:50 - 88:51                 "&"
88:51 - 88:60                 mapping
88:51 - 88:54                   key: identifier `key`
88:54 - 88:55                   "="
88:55 - 88:60                   value: identifier `value`
88:60 - 88:61               "#"
88:61 - 88:69               fragment: literal_string `fragment`
89:1  - 89:21           snippet_reference
89:1  - 89:7              "import"
89:8  - 89:15             snippet: identifier `snippet`
89:16 - 89:21             argument: address
89:16 - 89:21               authority
89:16 - 89:17                 ":"
89:17 - 89:21                 port: integer `3000`
90:0  - 90:1            "}"
92:0  - 97:1        site_definition
92:0  - 92:15         site: address
92:0  - 92:15           authority
92:0  - 92:15             host: domain_name
92:0  - 92:3                segment: literal_string `app`
92:3  - 92:4                "."
92:4  - 92:11               segment: literal_string `example`
92:11 - 92:12               "."
92:12 - 92:15               segment: literal_string `com`
92:16 - 97:1          block
92:16 - 92:17           "{"
93:1  - 93:50           named_matcher_definition
93:1  - 93:2              "@"
93:2  - 93:8              name: identifier `denied`
93:9  - 93:50             request_matcher
93:9  - 93:50               embedded_content
93:9  - 93:10                 "\`"
93:10 - 93:49                 cel_expression `client_ip('12.23.34.45', '23.34.45.56')`
93:49 - 93:50                 "\`"
94:1  - 94:14           statement
94:1  - 94:6              directive
94:1  - 94:6                name: identifier `abort`
94:7  - 94:14             matcher: matcher
94:7  - 94:14               named_matcher_reference
94:7  - 94:8                  "@"
94:8  - 94:14                 name: identifier `denied`
95:1  - 95:17           invoke_statement
95:1  - 95:7              "invoke"
95:8  - 95:17             route: identifier `app-proxy`
96:1  - 96:19           snippet_reference
96:1  - 96:7              "import"
96:8  - 96:15             snippet: identifier `snippet`
96:16 - 96:19             argument: literal_string `app`
97:0  - 97:1            "}"
```
