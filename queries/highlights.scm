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

"import" @keyword

["{" "}"] @punctuation.bracket
["$" "*" "@" "<<"] @punctuation.special
["/"] @punctuation.delimiter

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
;
; (named_matcher
; 	name: (_) @type
; )


; (path
; 	segment: (string) @string.special.url
; )
; (site_block (string) @string.special.url ) 



(ERROR) @error-node
