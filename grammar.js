/**
 * @file A grammar for Caddyfile
 * @author Tom Spencer
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
	name: 'caddyfile',

	supertypes: $ => [
		$.argument,
		$.string,
		$.host,
		$.definition,
		$.global_placeholder,
		$.placeholder,
		$.reference,
		$.posix_pathname,
		$.windows_pathname,
	],

	externals: $ => [
		$._ext_unspecified,

		// heredoc
		$._ext_heredoc_operator,
		$.heredoc_tag,
		$.heredoc_content,
		$.heredoc_suffix,

		// plain text content
		$._ext_str_word,
		$._ext_str_bare,
		$._ext_str_upper,
		$._ext_str_num,
		$._ext_str_decimal,
		$._ext_str_hex_byte,
		$._ext_str_ipv4,
		$._ext_str_cel,
		$._ext_str_cel_inline,
		$._ext_str_comment,
		$._ext_str_qty_integer,
		$._ext_str_qty_decimal,

		// whitespace
		$._ext_eol,
		$._ext_ws,

		// keywords
		$.boolean,
		$._key_import,
		$._key_invoke,
		$._key_private_ranges,
		$._key_expression,
		$._key_vars,
		$._key_env,
		$._key_file,
		$._key_system,
		$._key_time,
		$._key_now,
		$._key_not,
		$._key_site,

		// classes
		$._ext_cls_protocol,
		$._ext_cls_regexp,
		$._ext_str_unit_duration,
		$._ext_str_unit_size,

		// symbols
		$._ext_sym_paren_o,
		$._ext_sym_paren_c,
		$._ext_sym_brace_o,
		$._ext_sym_brace_c,
		$._ext_sym_bracket_o,
		$._ext_sym_bracket_c,
		$._ext_sym_chevron_o,
		$._ext_sym_chevron_c,
		$._ext_sym_colon,
		$._ext_sym_solidus,
		$._ext_sym_bsolidus,
		$._ext_sym_hyphen,
		$._ext_sym_at,
		$._ext_sym_comma,
		$._ext_sym_period,
		$._ext_sym_ampersand,
		$._ext_sym_plus,
		$._ext_sym_num,
		$._ext_sym_dollar,
		$._ext_sym_grave,
		$._ext_sym_quote,
		$._ext_sym_asterisk,
		$._ext_sym_exclaim,
		$._ext_sym_question,
		$._ext_sym_percent,
		$._ext_sym_bar,
		$._ext_sym_equal,

		$._ext_sym_block_start,
		$._ext_sym_scheme,
		$._ext_sym_comment,
		$._ext_sym_dot_slash,
		$._ext_sym_dot_dot_slash,
		$._ext_sym_colon_backslash,
		$._error_sentinel,
	],

	extras: $ => [],

	rules: {
		caddyfile: $ =>
			seq(
				repeat($._eol),
				optional(seq($.global_options, repeat($._eol))),
				optional(choice($.single_site, $.multi_site)),
			),

		single_site: $ => seq($._site, repeat1($._eol), optional($._block_body)),
		multi_site: $ => seq($._content, repeat(choice($._content, $._eol))),

		definition: $ =>
			choice($.site_definition, $.snippet_definition, $.named_route_definition),

		_content: $ => choice($.definition, $.import_statement),

		global_options: $ => $._block,
		site_definition: $ => seq($._site, $.block),

		_site_field: $ => field('site', $.address),
		_site_list: $ =>
			seq(optional($._site_field), repeat1(seq($._site_delim, optional($._site_field)))),
		_site_delim: $ => prec.right(choice($._ws, $._sym_comma, $._site_break)),
		_site_break: $ => seq($._sym_comma, $._eol),
		_site: $ => choice($._site_field, $._site_list),

		_nested_string: $ => alias($._ext_str_word, $.literal_string),

		_nested_identifier: $ => alias($._ext_str_word, $.identifier),
		_bare_identifier: $ => alias($._ext_str_bare, $.identifier),

		verb: $ => $._ext_str_upper,
		integer: $ => $._ext_str_num,

		substitution: $ => seq($._sym_brace_o, optional($.reference), $._sym_brace_c),
		reference: $ =>
			choice(
				$.environment_variable,
				$.placeholder,
				$.index_expression,
				$.slice_expression,
			),
		identifier: $ => choice($.templated_identifier, $._ext_str_bare),

		string: $ => prec.right(choice($.templated_string, $.literal_string)),
		_string: $ => choice($.templated_string, $.literal_string),
		literal_string: $ => $._ext_str_bare,

		templated_identifier: $ => prec.right(seq(repeat1($._tmpl_identifier_fragment))),
		templated_string: $ => prec.right(seq(repeat1($._tmpl_string_fragment))),

		_tmpl_string_fragment: $ => field('fragment', choice($._nested_string, $.substitution)),
		_tmpl_identifier_fragment: $ =>
			field('fragment', choice($._nested_identifier, $.substitution)),

		_operand: $ => field('operand', $.identifier),
		index_expression: $ =>
			prec.right(
				seq(
					optional($._operand),
					$._sym_bracket_o,
					optional(field('index', $.integer)),
					$._sym_bracket_c,
				),
			),

		slice_expression: $ =>
			prec.right(
				seq(
					optional($._operand),
					$._sym_bracket_o,
					optional(field('start', $.integer)),
					$._sym_colon,
					optional(field('end', $.integer)),
					$._sym_bracket_c,
				),
			),

		namespace_expression: $ =>
			seq(
				optional(field('module', $._bare_identifier)),
				repeat1(seq($._sym_period, optional(field('member', $._bare_identifier)))),
			),

		placeholder: $ => choice($.generic_placeholder, $.global_placeholder),

		generic_placeholder: $ => seq(choice($.identifier, $.namespace_expression)),

		global_placeholder: $ =>
			choice(
				$.system_placeholder,
				$.time_placeholder,
				$.env_placeholder,
				$.file_placeholder,
			),

		system_placeholder: $ =>
			seq(
				optional($._sym_period),
				$._keyword_system,
				$._sym_period,
				optional(field('name', $.identifier)),
			),

		time_placeholder: $ =>
			seq(
				optional($._sym_period),
				$._keyword_time,
				$._sym_period,
				optional(seq($._keyword_now, $._sym_period)),
				optional(field('name', $.identifier)),
			),

		env_placeholder: $ =>
			seq(
				optional($._sym_period),
				$._keyword_env,
				$._sym_period,
				optional(field('name', $.identifier)),
			),

		file_placeholder: $ =>
			seq(
				optional($._sym_period),
				$._keyword_file,
				$._sym_period,
				optional(field('member', prec.right(choice($.string, $.pathname)))),
			),

		environment_variable: $ =>
			prec.right(
				seq($._sym_dollar, optional(choice($._env_var_name, $._env_var_name_default))),
			),

		_env_var_name: $ => field('name', $._bare_identifier),
		_env_var_name_default: $ =>
			seq($._env_var_name, $._sym_colon, optional(field('default', $.argument))),

		ipv6: $ =>
			seq(
				$._sym_bracket_o,
				repeat(choice(field('hextet', $._primitive), $._sym_colon)),
				optional(seq($._sym_percent, optional(field('zone', $.string)))),
				$._sym_bracket_c,
			),
		_path: $ => repeat1(choice(field('segment', $.string), $._sym_solidus)),

		_fragment: $ => prec.right(seq($._sym_num, optional($.string))),

		_network: $ => seq($.protocol, optional(seq($._sym_plus, $.protocol))),

		network_address: $ =>
			seq(
				field('network', $._network),
				$._sym_solidus,
				field('address', choice($.pathname, $.authority)),
			),

		protocol: $ => $._ext_cls_protocol,

		byte: $ => $._ext_str_hex_byte,
		mac_address: $ =>
			prec.right(
				seq(field('octet', $.byte), repeat1(seq($._sym_colon, field('octet', $.byte)))),
			),

		address: $ =>
			prec.right(
				seq(
					optional(field('scheme', seq($.string, $._sym_scheme))),
					optional(seq(field('user', $.string), $._sym_at)),
					$.authority,
					optional(field('path', $.path)),
					optional(field('query', $.query)),
					optional(field('fragment', $._fragment)),
				),
			),

		host: $ => prec.right(choice($.ipv6, $.ipv4, $.domain_name)),

		authority: $ =>
			prec.right(repeat1(choice(field('host', $.host), field('port', $._port)))),

		_port: $ => prec.right(seq($._sym_colon, optional($._primitive))),

		domain_name: $ =>
			prec.right(
				seq(
					field('segment', $.string),
					repeat(choice(field('segment', $.string), $._sym_period)),
				),
			),

		range: $ =>
			seq(
				optional(field('left', $._primitive)),
				$._sym_hyphen,
				field('right', $._primitive),
			),

		mapping: $ =>
			prec.right(
				seq(
					optional(field('key', $._bare_identifier)),
					repeat1(seq($._sym_equal, optional(field('value', $._bare_identifier)))),
				),
			),
		query: $ =>
			prec.right(
				seq(
					$._sym_question,
					optional($.mapping),
					repeat(seq($._sym_ampersand, $.mapping)),
				),
			),
		_block: $ =>
			seq($._sym_block_start, repeat1($._eol), optional($._block_body), $._sym_brace_c),

		block: $ => $._block,

		_block_body: $ =>
			seq($._expression, repeat(seq(repeat1($._eol), $._expression)), repeat1($._eol)),

		_vars_block: $ =>
			seq(
				$._sym_block_start,
				repeat1($._eol),
				repeat(seq($.assignment, repeat1($._eol))),
				$._sym_brace_c,
			),

		_snippet_name: $ =>
			seq($._sym_paren_o, optional(field('name', $._bare_identifier)), $._sym_paren_c),

		snippet_definition: $ => seq($._snippet_name, $.block),

		_expression: $ =>
			choice(
				$.statement,
				$.named_matcher_definition,
				$.import_statement,
				$.invoke_statement,
				$.variable_declaration,
				$.substitution,
			),

		_nested_site_block: $ => seq($._doc_comment_site_aliased, $.site_definition),

		_named_route_name: $ =>
			seq(
				$._sym_ampersand,
				$._sym_paren_o,
				optional(field('name', $._bare_identifier)),
				$._sym_paren_c,
			),

		variable_declaration: $ =>
			seq(
				$._keyword_vars,
				optional($._matcher_field),
				choice($.assignment, $._vars_block),
			),

		assignment: $ => seq(field('key', $._bare_identifier), repeat1($._value_field)),

		named_route_definition: $ => seq($._named_route_name, $.block),

		named_matcher_reference: $ => $._matcher_name,

		pathname: $ =>
			seq(choice($.posix_pathname, $.windows_pathname), optional($.permission)),

		posix_pathname: $ => choice($.posix_absolute_pathname, $.posix_relative_pathname),
		windows_pathname: $ => choice($.windows_absolute_pathname, $.windows_relative_pathname),

		posix_absolute_pathname: $ => prec(-1, $._posix_path),

		windows_absolute_pathname: $ =>
			prec.right(
				seq(
					field('drive', $.string),
					$._drive_letter_delimiter,
					optional($._windows_path),
				),
			),

		posix_relative_pathname: $ =>
			prec(
				-1,
				seq(
					choice(
						$.current_directory,
						seq(
							$.parent_directory,
							repeat(seq($._sym_solidus, $.parent_directory)),
						),
					),
					$._posix_path,
				),
			),
		windows_relative_pathname: $ =>
			prec(
				-1,
				seq(
					choice(
						$.current_directory,
						seq(
							$.parent_directory,
							repeat(seq($._sym_bsolidus, $.parent_directory)),
						),
					),
					$._windows_path,
				),
			),

		_drive_letter_delimiter: $ => alias($._ext_sym_colon_backslash, ':'),

		current_directory: $ => $._ext_sym_dot_slash,
		parent_directory: $ => $._ext_sym_dot_dot_slash,

		permission: $ => prec.right(seq($._sym_bar, optional(alias($.integer, $.octal)))),

		_posix_path: $ =>
			prec.right(
				seq($._sym_solidus, repeat(choice($._sym_solidus, field('segment', $.string)))),
			),

		_windows_path: $ =>
			prec.right(
				seq(
					$._sym_bsolidus,
					repeat(choice($._sym_bsolidus, field('segment', $.string))),
				),
			),

		path: $ => $._posix_path,

		matcher: $ => prec.right(choice($.wildcard, $.named_matcher_reference, $.path)),
		wildcard: $ => $._sym_asterisk,

		_matcher_name: $ => seq($._sym_at, field('name', $.identifier)),

		named_matcher_definition: $ => seq($._matcher_name, optional($._ws), $.request_matcher),

		_matcher_block: $ =>
			seq(
				$._sym_block_start,
				repeat1($._eol),
				repeat(seq($.request_matcher, repeat1($._eol))),
				$._sym_brace_c,
			),

		not_operator: $ => $._key_not,

		request_matcher: $ =>
			prec.right(
				seq(
					optional(field('modifier', $.not_operator)),
					choice(
						$._regexp_matcher,
						$._generic_matcher,
						$._expression_matcher,
						$.embedded_content,
						$.heredoc,
						$.not_operator,
						alias($._matcher_block, $.block),
					),
				),
			),

		_generic_matcher: $ =>
			prec.right(
				seq(
					field('matcher', $.identifier),
					repeat(seq(repeat1($._ws), $._arguments_field)),
					alias(optional($._matcher_block), $.block),
				),
			),

		_expression_matcher: $ =>
			seq(
				field('matcher', $._keyword_expression),
				repeat($._ws),
				field('argument', choice($._implied_cel_expression, $.embedded_content)),
			),

		regular_expression: $ => repeat1($._ext_str_bare),
		_regexp_matcher: $ =>
			seq(
				field('matcher', alias($._ext_cls_regexp, $.identifier)),
				repeat($._ws),
				choice(
					seq(
						field('argument', $.literal_string),
						$._ws,
						field('argument', $.regular_expression),
					),
					field('argument', $.regular_expression),
				),
			),

		_implied_cel_expression: $ => alias($._ext_str_cel_inline, $.cel_expression),

		directive: $ => field('name', $._bare_identifier),
		conditional_directive: $ => seq($.directive, repeat1($._ws), $._matcher_field),
		statement: $ =>
			prec.left(
				seq(
					choice($.directive, $.conditional_directive),
					repeat(seq(repeat1($._ws), $._arguments_field)),
					optional(seq(repeat1($._ws), $.block)),
				),
			),

		_matcher_field: $ =>
			seq(optional(field('modifier', $.not_operator)), field('matcher', $.matcher)),
		_arguments_field: $ => field('argument', $.argument),
		_value_field: $ => seq(field('value', $.argument), repeat($._ws)),

		_primitive: $ => prec.right(choice($.string, $.integer)),

		_amount_decimal: $ => alias($._ext_str_qty_decimal, $.decimal),
		_amount_integer: $ => alias($._ext_str_qty_integer, $.integer),
		duration: $ => $._ext_str_unit_duration,
		size: $ => $._ext_str_unit_size,
		amount: $ =>
			seq(
				field('quantity', choice($._amount_integer, $._amount_decimal)),
				field('unit', choice($.duration, $.size)),
			),

		ipv4: $ => $._ext_str_ipv4,
		cidr: $ => seq(choice($.ipv4, $.ipv6), $._sym_solidus, $.integer),
		decimal: $ => $._ext_str_decimal,

		_modifier: $ =>
			choice(
				$._sym_exclaim,
				$._sym_question,
				$._sym_hyphen,
				$._sym_plus,
				$._sym_chevron_o,
				$._sym_chevron_c,
			),
		unary_expression: $ =>
			seq(field('operator', $._modifier), field('operand', $.literal_string)),

		argument: $ =>
			prec.right(
				1,
				choice(
					$.mac_address,
					$.string,
					$.unary_expression,
					$.integer,
					$.decimal,
					$.boolean,
					$.address,
					$.network_address,
					$.pathname,
					$.quoted_expression,
					$.embedded_content,
					$.verb,
					$.heredoc,
					$._keyword_private_ranges,
					$.amount,
					$.ipv4,
					$.cidr,
				),
			),

		heredoc: $ => seq($._sym_heredoc, $.heredoc_tag, $.heredoc_content, $.heredoc_suffix),
		_sym_heredoc: $ => alias($._ext_heredoc_operator, '<<'),

		invoke_statement: $ =>
			prec.right(
				seq(
					$._keyword_invoke,
					repeat($._ws),
					optional($._matcher_field),
					field('route', $._bare_identifier),
				),
			),

		import_statement: $ =>
			prec.right(
				seq(
					$._keyword_import,
					repeat($._ws),
					field(
						'pattern',
						prec.right(choice(alias($.literal_string, $.identifier), $.pathname)),
					),
					repeat($._ws),
					repeat($._arguments_field),
					optional($.block),
				),
			),

		quoted_expression: $ =>
			seq($._sym_quote, optional(field('content', $.string)), $._sym_quote),

		embedded_content: $ => seq($._sym_grave, $.cel_expression, $._sym_grave),
		cel_expression: $ => $._ext_str_cel,

		// symbols
		_sym_paren_o: $ => alias($._ext_sym_paren_o, '('),
		_sym_paren_c: $ => alias($._ext_sym_paren_c, ')'),
		_sym_brace_o: $ => alias($._ext_sym_brace_o, '{'),
		_sym_brace_c: $ => alias($._ext_sym_brace_c, '}'),
		_sym_bracket_o: $ => alias($._ext_sym_bracket_o, '['),
		_sym_bracket_c: $ => alias($._ext_sym_bracket_c, ']'),
		_sym_chevron_o: $ => alias($._ext_sym_chevron_o, '<'),
		_sym_chevron_c: $ => alias($._ext_sym_chevron_c, '>'),
		_sym_colon: $ => alias($._ext_sym_colon, ':'),
		_sym_solidus: $ => alias($._ext_sym_solidus, '/'),
		_sym_bsolidus: $ => alias($._ext_sym_bsolidus, '\\'),
		_sym_hyphen: $ => alias($._ext_sym_hyphen, '-'),
		_sym_at: $ => alias($._ext_sym_at, '@'),
		_sym_comma: $ => alias($._ext_sym_comma, ','),
		_sym_period: $ => alias($._ext_sym_period, '.'),
		_sym_ampersand: $ => alias($._ext_sym_ampersand, '&'),
		_sym_plus: $ => alias($._ext_sym_plus, '+'),
		_sym_num: $ => alias($._ext_sym_num, '#'),
		_sym_dollar: $ => alias($._ext_sym_dollar, '$'),
		_sym_grave: $ => alias($._ext_sym_grave, '`'),
		_sym_quote: $ => alias($._ext_sym_quote, '"'),
		_sym_asterisk: $ => alias($._ext_sym_asterisk, '*'),
		_sym_exclaim: $ => alias($._ext_sym_exclaim, '!'),
		_sym_question: $ => alias($._ext_sym_question, '?'),
		_sym_percent: $ => alias($._ext_sym_percent, '%'),
		_sym_bar: $ => alias($._ext_sym_bar, '|'),
		_sym_equal: $ => alias($._ext_sym_equal, '='),

		_keyword_import: $ => alias($._key_import, 'import'),
		_keyword_invoke: $ => alias($._key_invoke, 'invoke'),
		_keyword_private_ranges: $ => alias($._key_private_ranges, $.shortcut),
		_keyword_vars: $ => alias($._key_vars, 'vars'),
		_keyword_env: $ => alias($._key_env, 'env'),
		_keyword_file: $ => alias($._key_file, 'file'),
		_keyword_system: $ => alias($._key_system, 'system'),
		_keyword_time: $ => alias($._key_time, 'time'),
		_keyword_now: $ => alias($._key_now, 'now'),

		_keyword_expression: $ => alias($._key_expression, $.identifier),

		_sym_block_start: $ => alias($._ext_sym_block_start, '{'),
		_sym_scheme: $ => alias($._ext_sym_scheme, '://'),

		_ws: $ => $._ext_ws,
		_eol: $ => choice($.comment, $._ext_eol),

		_comment_content: $ => field('content', alias($._ext_str_comment, $.comment)),
		_doc_generic: $ => seq($._sym_at, field('doc', $.identifier)),
		_doc_site: $ => seq($._sym_at, field('doc', alias($._key_site, $.identifier))),

		comment: $ =>
			prec.left(
				seq(
					$._ext_sym_comment,
					optional($._doc_generic),
					optional($._comment_content),
					$._ext_eol,
				),
			),

		_doc_comment_site: $ =>
			prec.left(
				seq($._ext_sym_comment, $._doc_site, optional($._comment_content), $._ext_eol),
			),

		_doc_comment_site_aliased: $ => alias($._doc_comment_site, $.comment),
	},
});
