((cel_expression) @injection.content
  (#set! injection.language "cel"))

((regular_expression) @injection.content
  (#set! injection.language "regex"))

(heredoc
  (heredoc_content) @injection.content
  (heredoc_suffix) @injection.language)
