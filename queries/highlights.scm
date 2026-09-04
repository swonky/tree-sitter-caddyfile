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
(octal) @character
(decimal) @number.float
(ipv4) @number.float

; Definitions
(snippet_definition
	name: (_) @constant.macro)

(named_route_definition
	name: (_) @constant.macro)

(regular_expression) @string.special.regex

; Keywords
"import" @keyword.directive
"invoke" @keyword.directive
"vars" @keyword
; "args" @variable.builtin

"env" @keyword
"file" @keyword
"system" @keyword
"time" @keyword
"now" @keyword

(negative) @keyword.operator

["(" ")" "{" "}" "[" "]"] @punctuation.bracket
["&" "$"  "@" "<<"] @punctuation.special
["*"] @character.special
["/" "." ":" "#" "?" "%" "|" "-" "://" "\\"] @punctuation.delimiter
["`"] @punctuation.special
["\""] @string

(current_directory) @string.special.symbol
(parent_directory) @string.special.symbol

; Substitutions
(substitution ["{" "}"] @punctuation.special)
(generic_placeholder (identifier) @variable.member)

(system_placeholder 
	name: (_) @keyword.coroutine)
(time_placeholder 
	name: (_) @keyword.coroutine)

(namespace_expression
	module: (_) @module
	member: (_) @function.call)

(assignment
	key: (_) @variable)

; Address

(address "@" @punctuation.delimiter )
(query "&" @punctuation.delimiter )
(mapping "=" @punctuation.delimiter)
(mapping key: (_) @property )
(mapping value: (_) @string )

(path
	segment: (_) @string.special.path)

(path
	segment: (_) @string.special.path)

(protocol) @constant.builtin

(network_address
	"+" @punctuation.operator
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
	segment: (_) @string.special.url)
(windows_relative_pathname
	segment: (_) @string.special.path)
(windows_absolute_pathname
	drive: (_) @string.special.path)
(windows_absolute_pathname
	segment: (_) @string.special.path)
(posix_relative_pathname
	segment: (_) @string.special.path)
(posix_absolute_pathname
	segment: (_) @string.special.path)

(environment_variable
	name: (_) @constant.macro
)

(block (substitution (generic_placeholder (identifier) @label)))
(block (substitution (generic_placeholder (namespace_expression ((_) @label)))))

(global_options 
	(statement (directive name: (_) @property)))
(site_definition
	(block (statement (directive name: (_) @function.builtin))))
(snippet_definition
	(block (statement (directive name: (_) @function.builtin))))
(named_route_definition
	(block (statement (directive name: (_) @function.builtin))))
(site_definition
	(block (statement (conditional_directive (directive name: (_) @function.builtin)))))
(snippet_definition
	(block (statement (conditional_directive (directive name: (_) @function.builtin)))))
(named_route_definition
	(block (statement (conditional_directive (directive name: (_) @function.builtin)))))

(statement (block (statement (directive name: (_) @function.method))))
(statement (block (statement (conditional_directive (directive name: (_) @function.method)))))

(shortcut) @constant.builtin

(comment) @spell @comment


(ERROR) @error-node
