(heredoc_tag) @label
(heredoc_suffix) @label


; (identifier) @constant
[(string) (literal_string)] @string 
; (comment) @comment 
(cel_expression) @string.special
(verb) @string.special
(integer) @number
(boolean) @boolean
(byte) @character
(decimal) @number.float
(identifier) @variable
(snippet_definition
	name: (_) @constant.macro
)

(ipv4) @number.float

(named_route_definition
	name: (_) @constant.macro
)

"import" @keyword.directive
"invoke" @keyword.directive
"vars" @keyword
"args" @variable.builtin

(negative) @keyword.modifier

(placeholder
	module: (_) @module
)

(placeholder
	member: (identifier) @variable.member
)

(placeholder
	reference: (identifier) @constant
)

(assignment
	key: (_) @variable
)

["(" ")" "{" "}" "[" "]"] @punctuation.bracket
["&" "$"  "@" "<<"] @punctuation.special
["*"] @character.special
["/" "." ":" "#" "?" "%" "|" "-" "://"] @punctuation.delimiter
["`"] @punctuation.special
["\""] @string

(address "@" @punctuation.delimiter )
(query "&" @punctuation.delimiter )
(mapping "=" @punctuation.delimiter)
(mapping key: (_) @variable.member )
(mapping value: (_) @string )

(path
	"/" @string.special.path
)

(path
	segment: (_) @string.special.path
)

; (address) @string.special.url

(substitution
	["{" "}"] @punctuation.special
)

(protocol) @constant.builtin

(network_address
	"+" @punctuation.delimiter
)

(directive (identifier)) @function.call

(namespace_expression
	module: (_) @module
	member: (_) @function.call
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


(shortcut) @constant.builtin

(comment) @spell @comment

(ERROR) @error-node
