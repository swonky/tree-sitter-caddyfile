(heredoc_tag) @label
(heredoc_suffix) @label

(comment) @spell @comment

; (identifier) @constant
(string) @string 
; (comment) @comment 
(cel_expression) @string.special
(verb) @string.special
(integer) @number
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
["/" "." ":"] @punctuation.delimiter
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


(construction
	keyword: (_) @function.call
)


(request_matcher
	matcher: (_) @type.builtin
)

(named_matcher_definition
	name: (_) @type
)

(environment_variable
	reference: (_) @constant.macro
)

(duration
	quantity: (integer) @number
	unit: (_) @number
)

(duration
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

"private_ranges" @constant.builtin

(ERROR) @error-node
