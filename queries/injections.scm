((cel_expression) @injection.content
  (#set! injection.language "cel"))

(heredoc
  (heredoc_content) @injection.content
  (heredoc_suffix) @injection.language)
