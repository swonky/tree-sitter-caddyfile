/**
 * @file A grammar for Caddyfile
 * @author Tom Spencer
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
	name: 'caddyfile',

	supertypes: $ => [$.reference, $.expression, $.argument],

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
		$._ext_str_num_dot,
		$._ext_str_cel,
		$._ext_str_cel_inline,
		$._ext_str_comment,

		// whitespace
		$._ext_eol,
		$._ext_ws,

		// keywords
		$.keyword_import,
		$.keyword_invoke,
		$.keyword_private_ranges,
		$.keyword_client_ip,
		$.keyword_expression,
		$.keyword_vars,
		$.keyword_args,
		$.keyword_not,

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

		$._ext_sym_block_start,
		// error recovery indicator
		$._error_sentinel,
	],

	extras: $ => [],

	rules: {
		caddyfile: $ =>
			repeat1(
				choice(
					$.global_block,
					$.site_block,
					$.snippet_definition,
					$.named_route_definition,
					$.snippet_reference,
					$._eol,
				),
			),

		global_block: $ => $._block,
		site_block: $ => seq(repeat1(seq($._site_field, $._sd)), field('content', $._block)),

		_sd: $ => seq(repeat1($._d), optional(seq($._eol, repeat($._d)))),
		_d: $ => choice($._ws, $._sym_comma),

		_site_field: $ => field('site', $.address),

		_word: $ => $._ext_str_word,

		_nested_string: $ => alias($._ext_str_word, $.string),
		_bare_string: $ => alias($._ext_str_bare, $.string),

		_nested_identifier: $ => alias($._ext_str_word, $.identifier),
		_bare_identifier: $ => alias($._ext_str_bare, $.identifier),

		verb: $ => $._ext_str_upper,
		numeric: $ => $._ext_str_num,

		// dot_delimited: $ =>
		// 	prec.right(
		// 		seq(
		// 			optional($._sym_period),
		// 			repeat1(seq($._primitive, repeat1($._sym_period))),
		// 			$._primitive,
		// 		),
		// 	),

		substitution: $ => seq($._sym_brace_o, optional($.reference), $._sym_brace_c),

		identifier: $ => choice($.templated_identifier, $._ext_str_bare),
		string: $ => choice($._quoted_string, $._substring),
		_substring: $ => choice($.templated_string, $._ext_str_bare),

		templated_identifier: $ => prec.right(seq(repeat1($._tmpl_identifier_fragment))),
		templated_string: $ => prec.right(seq(repeat1($._tmpl_string_fragment))),

		_tmpl_string_fragment: $ => field('fragment', choice($._nested_string, $.substitution)),
		_tmpl_identifier_fragment: $ =>
			field('fragment', choice($._nested_identifier, $.substitution)),

		reference: $ => choice($.environment_variable, $.placeholder),

		// parameter: $ => repeat1(choice($.string, )),
		_index: $ =>
			field(
				'index',
				seq($._sym_bracket_o, optional(choice($.numeric, $.range)), $._sym_bracket_c),
			),

		range: $ =>
			prec.left(
				seq(
					optional(field('left', $.numeric)),
					$._sym_colon,
					optional(field('right', $.numeric)),
				),
			),

		placeholder: $ =>
			field('reference', repeat1(choice($.identifier, $._sym_period, $._index))),
		environment_variable: $ =>
			seq($._sym_dollar, optional(field('reference', $.identifier))),

		// url: $ =>
		// 	prec.right(
		// 		seq(
		// 			optional($._protocol_field),
		// 			repeat1(
		// 				choice(
		// 					$._primitive,
		// 					$._sym_period,
		// 					$._sym_solidus,
		// 					$._sym_at,
		// 					$._sym_colon,
		// 				),
		// 			),
		// 			optional($._ws),
		// 		),
		// 	),

		dotted_address: $ =>
			repeat1(field('segment', choice($._primitive, $._sym_period, $.wildcard))),

		_scheme: $ => prec(1, seq($.string, $._sym_colon, $._sym_solidus, $._sym_solidus)),
		_scheme_field: $ => field('scheme', $._scheme),

		// _scheme_field: $ =>
		// 	field('scheme', seq($.string, $._sym_colon, $._sym_solidus, $._sym_solidus)),

		_port_field: $ => field('port', seq($._sym_colon, $._primitive)),
		_path_field: $ => field('path', seq($._sym_solidus, $.string)),
		_prefix_length_field: $ => field('prefix_length', seq($._sym_solidus, $.numeric)),
		_addr_suffix: $ => choice($._path_field, $._prefix_length_field),
		_ipv4_octet: $ => field('octet', $.numeric),
		_domain_segment: $ => field('segment', choice($.string, $.wildcard)),

		ipv6: $ =>
			seq(
				$._sym_bracket_o,
				repeat(choice(field('hextet', $._primitive), $._sym_colon)),
				$._sym_bracket_c,
			),

		_host_field: $ => field('host', choice($.ipv6, $.dotted_address)),

		_address: $ =>
			prec.left(
				choice(
					seq($._host_field, optional($._port_field), optional($._addr_suffix)),
					seq($._port_field, optional($._addr_suffix)),
					$._addr_suffix,
				),
			),

		address: $ =>
			choice(
				$._address,
				seq(
					$._scheme_field,
					optional($._host_field),
					optional($._port_field),
					optional($._addr_suffix),
				),
			),

		_block: $ =>
			seq(
				$._sym_block_start,
				repeat1($._eol),
				repeat(seq($.expression, repeat1($._eol))),
				$._sym_brace_c,
			),

		_matcher_block: $ =>
			seq(
				$._sym_block_start,
				repeat1($._eol),
				repeat(seq($.matcher_clause, repeat1($._eol))),
				$._sym_brace_c,
			),

		_vars_block: $ =>
			seq(
				$._sym_block_start,
				repeat1($._eol),
				repeat(seq($.assignment, repeat1($._eol))),
				$._sym_brace_c,
			),

		_snippet_name: $ =>
			seq($._sym_paren_o, optional(field('name', $._bare_identifier)), $._sym_paren_c),

		snippet_definition: $ => seq($._snippet_name, field('content', $._block)),

		expression: $ =>
			choice(
				$.construction,
				$.matcher_definition,
				$.snippet_reference,
				$.invoke_statement,
				$.variable_declaration,
			),

		_named_route_name: $ =>
			seq(
				$._sym_ampersand,
				$._sym_paren_o,
				optional(field('name', $._bare_identifier)),
				$._sym_paren_c,
			),

		variable_declaration: $ =>
			seq(
				$.keyword_vars,
				optional($._matcher_field),
				choice($.assignment, $._vars_block),
			),

		assignment: $ => seq(field('key', $._bare_identifier), repeat1($._value_field)),

		named_route_definition: $ => seq($._named_route_name, field('content', $._block)),

		named_matcher: $ => $._matcher_name,
		path_matcher: $ => prec.left(seq(field('path', $.path), $._ws)),

		path: $ =>
			seq(
				$._sym_solidus,
				repeat(field('segment', choice($.wildcard, $.identifier, $._sym_solidus))),
			),

		matcher: $ => choice($.wildcard, $.named_matcher, $.path_matcher),
		wildcard: $ => $._sym_asterisk,

		construction: $ => seq($._clause, optional($._block)),

		_matcher_name: $ => seq($._sym_at, field('name', $.identifier)),

		matcher_definition: $ =>
			seq(
				$._matcher_name,
				optional($._ws),
				choice($.matcher_clause, $._matcher_block, $.heredoc),
			),

		matcher_clause: $ =>
			choice($.negative_matcher, $.generic_matcher, $.expression_matcher),

		negative_matcher: $ => seq($.keyword_not, choice($.generic_matcher, $._matcher_block)),

		generic_matcher: $ =>
			prec.left(
				seq(
					field('matcher', $.identifier),
					optional($._ws),
					repeat($._arguments_field),
					optional($._matcher_block),
				),
			),

		expression_matcher: $ =>
			seq(
				field('matcher', $.keyword_expression),
				repeat($._ws),
				choice($._implied_cel_expression, $.embedded_content),
			),

		_implied_cel_expression: $ => alias($._ext_str_cel_inline, $.cel_expression),

		_clause: $ =>
			prec.left(
				seq(
					$._keyword_field,
					repeat($._ws),
					optional(seq($._matcher_field, repeat($._ws))),
					repeat($._arguments_field),
				),
			),

		_keyword_field: $ => field('keyword', $.identifier),
		_matcher_field: $ => prec.left(seq(field('matcher', $.matcher), repeat($._ws))),
		_arguments_field: $ => seq(field('argument', $.argument), repeat($._ws)),
		_value_field: $ => seq(field('value', $.argument), repeat($._ws)),

		_primitive: $ => prec.right(choice($.string, $.numeric)),

		argument: $ =>
			choice(
				$._primitive,
				$.embedded_content,
				$.verb,
				$.heredoc,
				$.keyword_private_ranges,
			),

		heredoc: $ => seq($._sym_heredoc, $.heredoc_tag, $.heredoc_content, $.heredoc_suffix),
		_sym_heredoc: $ => alias($._ext_heredoc_operator, '<<'),

		invoke_statement: $ =>
			prec.right(
				seq(
					$.keyword_invoke,
					repeat($._ws),
					optional($._matcher_field),
					field('route', $._bare_identifier),
					// repeat($._ws),
				),
			),

		snippet_reference: $ =>
			prec.right(
				seq(
					$.keyword_import,
					repeat($._ws),
					field('snippet', $._bare_identifier),
					repeat($._ws),
					repeat($._arguments_field),
					optional($._block),
				),
			),

		_quoted_string: $ =>
			seq($._sym_quote, repeat(choice($._substring, $._ws)), $._sym_quote),

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

		_sym_block_start: $ => alias($._ext_sym_block_start, '{'),

		_ws: $ => $._ext_ws,
		_eol: $ => choice($.comment, $._ext_eol),

		comment: $ => seq($._sym_num, optional($._ext_str_comment), $._ext_eol),
	},
});
