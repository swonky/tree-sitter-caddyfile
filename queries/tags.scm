; Named matchers
(named_matcher_definition name: (_) @definition.matcher)
(named_matcher_reference name: (_) @reference.matcher)

; Snippets
(snippet_declaration name: (_) @definition.snippet)
(import_statement pattern: (identifier) @reference.snippet)

; Named routes
(named_route_declaration name: (_) @definition.route)
(invoke_statement route: (_) @reference.route)

; Variables
(variable_declaration (assignment key: (_) @definition.variable))
(
	(generic_placeholder 
		(namespace_expression 
			module: (_) @module-name
			member: (_) @reference.variable
		)
	)
	(#match? @module-name "vars")
)

; Directives
(directive name: (_) @reference.directive))
