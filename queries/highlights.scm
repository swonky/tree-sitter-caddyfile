(heredoc_tag) @label
(heredoc_suffix) @label

; Injected strings
; Should be replaced by injected language capture groups if installed.
(heredoc_content) @string.special
(cel_expression) @string.special
(regular_expression) @string.special

; Lexer atomic types
[(string) (literal_string)] @string 
(verb) @string.special
(integer) @number
(boolean) @boolean
(byte) @character
(decimal) @number.float
(ipv4) @number.float

; Definitions
(snippet_definition
	name: (_) @constant.macro
)

(named_route_definition
	name: (_) @constant.macro
)

; Keywords
"import" @keyword.directive
"invoke" @keyword.directive
"vars" @keyword
"args" @variable.builtin
"env" @keyword
"file" @keyword

(negative) @keyword.operator

["(" ")" "{" "}" "[" "]"] @punctuation.bracket
["&" "$"  "@" "<<"] @punctuation.special
["*"] @character.special
["/" "." ":" "#" "?" "%" "|" "-" "://"] @punctuation.delimiter
["`"] @punctuation.special
["\""] @string

; Substitution
(substitution ["{" "}"] @punctuation.special)


(namespace_expression
	module: (_) @module
	member: (_) @function.call
)

(assignment
	key: (_) @variable
)

; Address

(address "@" @punctuation.delimiter )
(query "&" @punctuation.delimiter )
(mapping "=" @punctuation.delimiter)
(mapping key: (_) @property )
(mapping value: (_) @string )

(path
	segment: (_) @string.special.path
)


(protocol) @constant.builtin

(network_address
	"+" @punctuation.delimiter
)


(request_matcher
	matcher: (_) @type.builtin
)

(named_matcher_definition
	name: (_) @type
)

(named_matcher_reference
	name: (_) @type
)


(amount
	quantity: (integer) @number
	unit: (_) @number
)

(amount
	quantity: (decimal) @number.float
	unit: (_) @number.float
)

(ipv6
	hextet: (_) @character
)

(comment
	"@" @type
	doc: (_) @type
) @comment.documentation

(path
	segment: (_) @string.special.url
)

(environment_variable
	name: (_) @constant.macro
)

(global_options 
	(statement (directive name: (_) @property)))

(site_definition
	(block (statement (directive name: (_) @function.builtin))))
(snippet_definition
	(block (statement (directive name: (_) @function.builtin))))
(named_route_definition
	(block (statement (directive name: (_) @function.builtin))))

(statement (block (statement (directive name: (_) @function.method))))

(shortcut) @constant.builtin

(comment) @spell @comment


(ERROR) @error-node
