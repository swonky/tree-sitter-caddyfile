; (environment_variable
; 	reference: (identifier) @keyword.directive
; )

; (comment) @spell @comment

; (identifier) @constant
(string) @string 
(cel_expression) @string.special
(verb) @string.special
(numeric) @number
(identifier) @variable
(snippet
	name: (_) @constant.macro
)

"import" @keyword

["{" "}"] @punctuation.bracket
["$" "*" "@"] @punctuation.special
["/"] @punctuation.delimiter

(statement
	directive: (_) @function.call
)

(matcher_definition
	name: (_) @type.definition
)

(named_matcher
	name: (_) @type
)


; (path
; 	segment: (string) @string.special.url
; )
; (site_block (string) @string.special.url ) 



(ERROR) @error-node
