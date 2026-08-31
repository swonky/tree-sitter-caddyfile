(heredoc_tag) @label
(heredoc_suffix) @label


; (identifier) @constant
[(string) (literal_string)] @string 
; (comment) @comment 
(cel_expression) @string.special
(verb) @string.special
(integer) @number
(octal) @number
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
"not" @keyword.operator

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
["&" "$" "*" "@" "<<"] @punctuation.special
["/" "." ":" "#" "?" "%" "|" "://"] @punctuation.delimiter
["`"] @punctuation.special
["\""] @string

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

(environment_variable
	reference: (_) @constant.macro
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
	hextet: (_) @number
)

(comment
	"@" @type
	doc: (_) @type
) @comment.documentation

(path
	segment: (_) @string.special.url
)

(site_block (string) @string.special.url ) 

(shortcut) @constant.builtin

(comment) @spell @comment

(ERROR) @error-node
