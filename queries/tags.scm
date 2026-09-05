(named_matcher_definition name: (_) @definition.matcher)
(named_matcher_reference name: (_) @reference.matcher)

(snippet_declaration name: (_) @definition.snippet)

(named_route_declaration name: (_) @definition.route)

(invoke_statement route: (_) @reference.route)
(import_statement pattern: (identifier) @reference.snippet)

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
