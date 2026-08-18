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

(keyword_import) @function.macro
(keyword_invoke) @function.macro

["(" ")" "{" "}"] @punctuation.bracket
["&" "$" "*" "@" "<<"] @punctuation.special
["/"] @punctuation.delimiter


(substitution
	["{" "}"] @punctuation.special
)


(directive
	keyword: (_) @function.call
)

(subdirective
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
