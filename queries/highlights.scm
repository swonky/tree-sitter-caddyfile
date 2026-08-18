; (environment_variable
; 	reference: (identifier) @keyword.directive
; )
(heredoc_tag) @label
(heredoc_suffix) @label

(comment) @spell @comment

; (identifier) @constant
(string) @string 
; (comment) @comment 
(cel_expression) @string.special
(verb) @string.special
(numeric) @number
(identifier) @variable
(snippet_definition
	name: (_) @constant.macro
)

(named_route_definition
	name: (_) @constant.macro
)

(keyword_import) @keyword.directive
(keyword_invoke) @keyword.directive
(keyword_vars) @keyword
(keyword_not) @keyword.operator


(variable_declaration
	name: (_) @variable
)

["(" ")" "{" "}"] @punctuation.bracket
["&" "$" "*" "@" "<<"] @punctuation.special
["/" "." ":"] @punctuation.delimiter


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
(keyword_expression) @type.builtin

(named_matcher
	name: (_) @type
)
;
; (named_matcher
; 	name: (_) @type
; )


; (path
; 	segment: (string) @string.special.url
; )
; (site_block (string) @string.special.url ) 
(keyword_private_ranges) @constant.builtin


(ERROR) @error-node
