/**
 * @file A grammar for Caddyfile
 * @author Tom Spencer
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
	name: 'caddyfile',

	supertypes: $ => [$.reference, $.argument, $.string, $.host],

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
		$._ext_str_ipv4,
		$._ext_str_cel,
		$._ext_str_cel_inline,
		$._ext_str_comment,
		$._ext_str_qty_integer,
		$._ext_str_qty_decimal,
		$._ext_str_unit_duration,
		$._ext_str_unit_size,

		// whitespace
		$._ext_eol,
		$._ext_ws,

		// keywords
		$._key_import,
		$._key_invoke,
		$._key_private_ranges,
		$._key_expression,
		$._key_vars,
		$._key_args,
		$._key_env,
		$._key_file,
		$._key_not,
		$._key_site,
		$._key_path_regexp,
		$._key_host_regexp,
		$._key_header_regexp,
		$._key_cookie_regexp,
		$._key_vars_regexp,

		$._key_protocol,

		// symbols
		$._ext_sym_paren_o,
		$._ext_sym_paren_c,
		$._ext_sym_brace_o,
		$._ext_sym_brace_c,
		$._ext_sym_bracket_o,
		$._ext_sym_bracket_c,
		$._ext_sym_colon,
		$._ext_sym_solidus,
		$._ext_sym_hyphen,
		$._ext_sym_at,
		$._ext_sym_comma,
		$._ext_sym_period,
		$._ext_sym_ampersand,
		$._ext_sym_plus,
		$._ext_sym_num,
		$._ext_sym_dollar,
		$._ext_sym_gt,
		$._ext_sym_grave,
		$._ext_sym_quote,
		$._ext_sym_asterisk,
		$._ext_sym_exclaim,
		$._ext_sym_question,
		$._ext_sym_percent,
		$._ext_sym_bar,

		$._ext_sym_block_start,
		$._ext_sym_scheme,
		// error recovery indicator
		$._error_sentinel,
	],

	extras: $ => [],

	rules: {
		caddyfile: $ =>
			seq(
				repeat($._eol),
				optional(seq($.global_block, repeat($._eol))),
				choice($.single_site, $.multi_site),
			),

		single_site: $ => seq($._site_list, repeat1($._eol), optional($._block_body)),
		multi_site: $ => seq($._content, repeat(choice($._content, $._eol))),
		_content: $ =>
			choice(
				$.site_block,
				$.snippet_definition,
				$.named_route_definition,
				$.snippet_reference,
			),

		global_block: $ => $._block,
		site_block: $ => seq($._site_list, $._block),
		_site_list: $ => repeat1(choice($._sd, $._site_field)),
		_sd: $ => prec.right(seq(repeat1($._d), optional(seq($._eol, repeat($._d))))),
		_d: $ => choice($._ws, $._sym_comma),

		_site_field: $ => field('site', $.address),

		_word: $ => $._ext_str_word,

		_nested_string: $ => alias($._ext_str_word, $.literal_string),
		_bare_string: $ => alias($._ext_str_bare, $.literal_string),

		_nested_identifier: $ => alias($._ext_str_word, $.identifier),
		_bare_identifier: $ => alias($._ext_str_bare, $.identifier),

		verb: $ => $._ext_str_upper,
		integer: $ => $._ext_str_num,

		substitution: $ => seq($._sym_brace_o, optional($.reference), $._sym_brace_c),

		identifier: $ => choice($.templated_identifier, $._ext_str_bare),

		string: $ => choice($.templated_string, $.literal_string),
		_string: $ => choice($.templated_string, $.literal_string),
		literal_string: $ => $._ext_str_bare,

		templated_identifier: $ => prec.right(seq(repeat1($._tmpl_identifier_fragment))),
		templated_string: $ => prec.right(seq(repeat1($._tmpl_string_fragment))),

		_tmpl_string_fragment: $ => field('fragment', choice($._nested_string, $.substitution)),
		_tmpl_identifier_fragment: $ =>
			field('fragment', choice($._nested_identifier, $.substitution)),

		reference: $ => choice($.environment_variable, $.placeholder, $.parameter),

		_index: $ =>
			field(
				'index',
				seq($._sym_bracket_o, optional(choice($.integer, $.range)), $._sym_bracket_c),
			),

		range: $ =>
			prec.left(
				seq(
					optional(field('left', $.integer)),
					$._sym_colon,
					optional(field('right', $.integer)),
				),
			),
		placeholder: $ =>
			choice(
				$._placeholder_shorthand,
				$._placeholder_env,
				$._placeholder_file,
				$._placeholder_namespaced,
			),

		_placeholder_env: $ =>
			seq(
				optional($._sym_period),
				field('module', alias($._keyword_env, $.identifier)),
				$._sym_period,
				optional(field('reference', $.identifier)),
			),

		_placeholder_file: $ =>
			seq(
				optional($._sym_period),
				field('module', alias($._keyword_file, $.identifier)),
				$._sym_period,
				optional(field('member', alias($._path, $.path))),
			),

		_placeholder_shorthand: $ => field('member', $.identifier),
		_placeholder_namespaced: $ =>
			seq(
				// optional($._sym_period),
				optional(field('module', $.identifier)),
				repeat1(seq($._sym_period, optional(field('member', $.identifier)))),
			),

		parameter: $ => seq(optional($._keyword_args), $._index),
		environment_variable: $ =>
			seq($._sym_dollar, optional(field('reference', $.identifier))),

		_ipv4_octet: $ => field('octet', $.integer),
		ipv6: $ =>
			seq(
				$._sym_bracket_o,
				repeat(choice(field('hextet', $._primitive), $._sym_colon)),
				optional(seq($._sym_percent, optional(field('zone', $.string)))),
				$._sym_bracket_c,
			),
		_path: $ => repeat1(choice(field('segment', $.string), $._sym_solidus)),
		inode: $ => prec.right(repeat1(choice($.string, $._sym_period))),

		_query: $ => prec.right(seq($._sym_question, optional($.string))),
		_fragment: $ => prec.right(seq($._sym_num, optional($.string))),

		_network: $ => seq($.protocol, optional(seq($._sym_plus, $.protocol))),

		network_address: $ =>
			seq(
				field('network', $._network),
				$._sym_solidus,
				field('address', choice($.path, $._host_port)),
			),

		protocol: $ => $._key_protocol,

		_host_port: $ =>
			prec.left(repeat1(choice(field('host', $.host), field('port', $._port)))),

		_port: $ => seq($._sym_colon, choice($._primitive, $.range_expression)),

		range_expression: $ =>
			seq(
				optional(field('left', $._primitive)),
				$._sym_hyphen,
				field('right', $._primitive),
			),

		address: $ =>
			prec.right(
				seq(
					repeat1(
						choice(field('scheme', seq($.string, $._sym_scheme)), $._host_port),
					),
					optional(field('path', $.path)),
					optional(field('query', $._query)),
					optional(field('fragment', $._fragment)),
				),
			),

		host: $ => choice($.ipv6, $.ipv4, $.domain_name),

		domain_name: $ => $._dot_delimited,

		_dot_delimited: $ =>
			prec.right(repeat1(choice(field('segment', $.string), $._sym_period))),

		_block: $ =>
			seq($._sym_block_start, repeat1($._eol), optional($._block_body), $._sym_brace_c),

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

		snippet_definition: $ => seq($._snippet_name, $._block),

		_expression: $ =>
			choice(
				$.construction,
				$.named_matcher_definition,
				$.snippet_reference,
				$.invoke_statement,
				$.variable_declaration,
				$._nested_site_block,
			),

		_nested_site_block: $ => seq($._doc_comment_site_aliased, $.site_block),

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

		named_route_definition: $ => seq($._named_route_name, $._block),

		named_matcher_reference: $ => $._matcher_name,
		path_matcher: $ => prec.left(seq(field('path', $.path), optional($._ws))),

		path: $ =>
			prec.right(
				seq(
					$._sym_solidus,
					repeat(field('segment', choice($.wildcard, $.identifier, $._sym_solidus))),
					optional(
						seq(
							$._sym_bar,
							field('permission', optional(alias($.integer, $.octal))),
						),
					),
				),
			),

		matcher: $ => choice($.wildcard, $.named_matcher_reference, $.path_matcher),
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

		request_matcher: $ =>
			seq(
				optional(field('modifier', $._keyword_not)),
				choice(
					$._regexp_matcher,
					$._generic_matcher,
					$._expression_matcher,
					$.embedded_content,
					$.heredoc,
					$._matcher_block,
				),
			),

		_generic_matcher: $ =>
			prec.left(
				seq(
					field('matcher', $.identifier),
					optional($._ws),
					repeat($._arguments_field),
					optional($._matcher_block),
				),
			),

		_expression_matcher: $ =>
			seq(
				field('matcher', $._keyword_expression),
				repeat($._ws),
				field('argument', choice($._implied_cel_expression, $.embedded_content)),
			),

		_keyword_regex_matcher: $ =>
			choice(
				$._key_path_regexp,
				$._key_host_regexp,
				$._key_header_regexp,
				$._key_cookie_regexp,
				$._key_vars_regexp,
			),
		regular_expression: $ => repeat1($._ext_str_bare),
		_regexp_matcher: $ =>
			seq(
				field('matcher', alias($._keyword_regex_matcher, $.identifier)),
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

		construction: $ =>
			prec.right(
				seq(
					$.directive,
					optional(seq(repeat1($._ws), $._matcher_field)),
					repeat(seq(repeat1($._ws), $._arguments_field)),
					optional(seq(repeat1($._ws), $._block)),
				),
			),

		// _identifier_shorthand: $ => field('member', $.identifier),
		namespace_expression: $ =>
			seq(
				optional(field('module', $._bare_identifier)),
				repeat1(seq($._sym_period, optional(field('member', $._bare_identifier)))),
			),
		directive: $ => choice($.namespace_expression, $._bare_identifier),
		_matcher_field: $ => field('matcher', $.matcher),
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
		decimal: $ => $._ext_str_decimal,
		add_expression: $ => seq($._sym_plus, $.string),

		argument: $ =>
			prec.left(
				1,
				choice(
					$.add_expression,
					$.string,
					$.integer,
					$.decimal,
					$.address,
					$.network_address,
					$.path,
					$.quoted_expression,
					$.embedded_content,
					$.verb,
					$.heredoc,
					$._keyword_private_ranges,
					$.amount,
					$.ipv4,
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
					// repeat($._ws),
				),
			),

		snippet_reference: $ =>
			prec.right(
				seq(
					$._keyword_import,
					repeat($._ws),
					field('snippet', $._bare_identifier),
					repeat($._ws),
					repeat($._arguments_field),
					optional($._block),
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
		_sym_colon: $ => alias($._ext_sym_colon, ':'),
		_sym_solidus: $ => alias($._ext_sym_solidus, '/'),
		_sym_hyphen: $ => alias($._ext_sym_hyphen, '-'),
		_sym_at: $ => alias($._ext_sym_at, '@'),
		_sym_comma: $ => alias($._ext_sym_comma, ','),
		_sym_period: $ => alias($._ext_sym_period, '.'),
		_sym_ampersand: $ => alias($._ext_sym_ampersand, '&'),
		_sym_plus: $ => alias($._ext_sym_plus, '+'),
		_sym_num: $ => alias($._ext_sym_num, '#'),
		_sym_dollar: $ => alias($._ext_sym_dollar, '$'),
		_sym_gt: $ => alias($._ext_sym_gt, '>'),
		_sym_grave: $ => alias($._ext_sym_grave, '`'),
		_sym_quote: $ => alias($._ext_sym_quote, '"'),
		_sym_asterisk: $ => alias($._ext_sym_asterisk, '*'),
		_sym_exclaim: $ => alias($._ext_sym_exclaim, '!'),
		_sym_question: $ => alias($._ext_sym_question, '?'),
		_sym_percent: $ => alias($._ext_sym_percent, '%'),
		_sym_bar: $ => alias($._ext_sym_bar, '|'),

		_keyword_import: $ => alias($._key_import, 'import'),
		_keyword_invoke: $ => alias($._key_invoke, 'invoke'),
		_keyword_private_ranges: $ => alias($._key_private_ranges, $.shortcut),
		_keyword_vars: $ => alias($._key_vars, 'vars'),
		_keyword_args: $ => alias($._key_args, 'args'),
		_keyword_env: $ => alias($._key_env, 'env'),
		_keyword_file: $ => alias($._key_file, 'file'),
		_keyword_not: $ => alias($._key_not, 'not'),

		_keyword_expression: $ => alias($._key_expression, $.identifier),

		// _keyword_path_regexp: $ => alias($._key_path_regexp, $.identifier),
		// _keyword_host_regexp: $ => alias($._key_host_regexp, $.identifier),
		// _keyword_header_regexp: $ => alias($._key_header_regexp, $.identifier),
		// _keyword_cookie_regexp: $ => alias($._key_cookie_regexp, $.identifier),
		// _keyword_vars_regexp: $ => alias($._key_vars_regexp, $.identifier),
		//
		_sym_block_start: $ => alias($._ext_sym_block_start, '{'),
		_sym_scheme: $ => alias($._ext_sym_scheme, '://'),

		_ws: $ => $._ext_ws,
		_eol: $ => choice($.comment, $._ext_eol),

		// comment: $ => seq($._sym_num, optional($._comment_content), $._ext_eol),
		_comment_content: $ => field('content', alias($._ext_str_comment, $.comment)),
		_doc_generic: $ => seq($._sym_at, field('doc', $.identifier)),
		_doc_site: $ => seq($._sym_at, field('doc', alias($._key_site, $.identifier))),

		comment: $ =>
			prec.left(
				seq(
					$._sym_num,
					optional($._doc_generic),
					optional($._comment_content),
					$._ext_eol,
				),
			),

		_doc_comment_site: $ =>
			prec.left(seq($._sym_num, $._doc_site, optional($._comment_content), $._ext_eol)),

		_doc_comment_site_aliased: $ => alias($._doc_comment_site, $.comment),
	},
});
