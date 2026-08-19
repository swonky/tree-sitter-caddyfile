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



(assignment
	key: (_) @variable
)

["(" ")" "{" "}" "[" "]"] @punctuation.bracket
["&" "$" "*" "@" "<<"] @punctuation.special
["/" "." ":"] @punctuation.delimiter

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

; (matcher_clause
; 	matcher (_) @type
; )
;
(matcher_definition
	name: (_) @type.definition
)

(generic_matcher
	matcher: (_) @type.builtin
)
; special matchers

(named_matcher
	name: (_) @type
)

(environment_variable
	reference: (_) @constant.macro
)
; (named_matcher
; 	name: (_) @type
; )

(ipv6
	hextet: (_) @number
)

; (path
; 	segment: (string) @string.special.url
; )
; (site_block (string) @string.special.url ) 

"expression" @type.builtin
"private_ranges" @constant.builtin

(ERROR) @error-node
