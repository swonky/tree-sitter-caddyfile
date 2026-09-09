(heredoc_tag) @label
(heredoc_suffix) @label

; Injected strings
; Should be replaced by injected language capture groups if installed.
(heredoc_content) @string.special
(cel_expression) @string.special
(regular_expression) @string.special

; Lexer atomic types
(statement argument: [(string) (literal_string)] @string)
(statement argument: [(integer) (decimal)] @number)
(address (string) @constant)

(verb) @label
(boolean) @boolean
(byte) @character
(octal) @character
(ipv4) @character
(amount) @label

; Definitions
(snippet_declaration
	name: (_) @module)

(named_route_declaration
	name: (_) @module)

(regular_expression) @string.special.regex

; Keywords
["import" "invoke" "vars"] @label

["env"
"file"
"system" 
"time" 
"now"] @label

(not_operator) @label

["(" ")" "{" "}" "[" "]"] @punctuation.bracket
["&" "$"  "<<" "*"] @character.special
["/" "." ":" "#" "?" "%" "|" "-" "://" "\\"] @punctuation.delimiter
["`"] @string.special
["\""] @string

(unary_expression (["?" "!" "+" "-" "<" ">"] @operator))

(current_directory) @string.special.symbol
(parent_directory) @string.special.symbol


(system_placeholder 
	name: (_) @keyword.coroutine)
(time_placeholder 
	name: (_) @keyword.coroutine)
(env_placeholder 
	name: (_) @constant)

(namespace_expression
	module: (_) @label
	member: (_) @constant)


; Address
(address "@" @punctuation.delimiter )
(query "&" @punctuation.delimiter )
(mapping "=" @punctuation.delimiter)
(mapping key: (_) @property )
(mapping value: (_) @string )


(path segment: (_) @module)

(protocol) @constant.builtin

(network_address "+" @punctuation.operator)

(request_matcher matcher: (_) @module)

(named_matcher_definition
	name: (_) @module)

(named_matcher_reference) @module

((ipv6
	hextet: (_) @character)
    (#set! "priority" 110)
)

(comment
	"@" @type
	doc: (_) @type
) @comment.documentation


(path
	segment: (_) @string.special.url)

(windows_relative_pathname
	segment: (_) @path)
(windows_relative_pathname
	segment: (templated_string fragment: (literal_string) @path ))

(windows_absolute_pathname
	drive: (_) @path)
(windows_absolute_pathname
	segment: (_) @path)
(windows_absolute_pathname
	segment: (templated_string fragment: (literal_string) @path ))

(posix_relative_pathname
	segment: (_) @path)
(posix_relative_pathname
	segment: (templated_string fragment: (literal_string) @path ))

(posix_absolute_pathname
	segment: (_) @path)
(posix_absolute_pathname
	segment: (templated_string fragment: (literal_string) @path ))

(generic_pathname 
	segment: (_) @path)
(generic_pathname 
	segment: (templated_string fragment: (literal_string) @path ))

(environment_variable
	name: (_) @type
) @type

(domain_name segment: (_) @constant)

(block (substitution (generic_placeholder (identifier) @type)))
(block (substitution (generic_placeholder (namespace_expression ((_) @type)))))

(global_options 
	(statement (directive name: (_) @function)))
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
(index_expression operand: (_) @type)
(statement (block (statement (directive name: (_) @function.method))))
(statement (block (statement (conditional_directive (directive name: (_) @function.method)))))

(shortcut) @label

(substitution ["{" "}"] @type)
(generic_placeholder (identifier) @type)

(comment) @spell @comment

(string) @string
(integer) @number
(decimal) @number
(identifier) @module
(amount quantity: (_) @character unit: (_) @character) @character


