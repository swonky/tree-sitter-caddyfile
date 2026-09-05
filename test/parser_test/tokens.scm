(statement argument: [
	(mac_address)
	(string)
	(unary_expression)
	(integer)
	(decimal)
	(boolean)
	(address)
	(network_address)
	(pathname)
	(shortcut)
	(verb)
	(heredoc)
	(amount)
	(additive_sequence)
	(ipv4)
	(cidr)
] @argument)

(request_matcher 
	matcher: (_) @token)
(request_matcher 
	(not_operator) @token)
(request_matcher 
	argument: [
		(regular_expression)
		(mac_address)
		(string)
		(unary_expression)
		(integer)
		(decimal)
		(boolean)
		(address)
		(network_address)
		(pathname)
		(shortcut)
		(verb)
		(heredoc)
		(amount)
		(additive_sequence)
		(ipv4)
		(cidr)
	] @token)

(embedded_content (cel_expression) @token)
(quoted_expression content: (_) @argument)
(statement (directive) @directive)
(statement (conditional_directive 
	(directive) @directive
	matcher: (_) @matcher))
(statement (conditional_directive 
	modifier: (_) @matcher))


(site_definition site: (_) @token)

(named_matcher_definition name: (_) @token)

(snippet_declaration) @token
(named_route_declaration) @token

(invoke_statement "invoke" @keyword)
(invoke_statement matcher: (_) @matcher)
(invoke_statement route: (_) @key)

(import_statement "import" @keyword)
(import_statement (_) @token)

(block (substitution) @macro)
(variable_declaration
  "vars" @keyword)
(variable_declaration
  (assignment (_) @token))

