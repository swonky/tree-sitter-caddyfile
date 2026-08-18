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

		// symbols
		$._ext_sym_paren_o,
		$._ext_sym_paren_c,
		$._ext_sym_brace_o,
		$._ext_sym_brace_c,
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

		_site_field: $ => field('site', $.identifier),

		_word: $ => $._ext_str_word,

		_nested_string: $ => alias($._ext_str_word, $.string),
		_bare_string: $ => alias($._ext_str_bare, $.string),

		_nested_identifier: $ => alias($._ext_str_word, $.identifier),
		_bare_identifier: $ => alias($._ext_str_bare, $.identifier),

		verb: $ => $._ext_str_upper,
		numeric: $ => $._ext_str_num,

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

		placeholder: $ => field('reference', repeat1(choice($.identifier, $._sym_period))),
		environment_variable: $ =>
			seq($._sym_dollar, optional(field('reference', $.identifier))),

		url: $ =>
			prec.right(
				seq(
					repeat1(
						choice($.templated_identifier, $._sym_solidus, $._sym_at, $._sym_colon),
					),
					optional($._ws),
				),
			),

		_block: $ =>
			seq(
				$._sym_brace_o,
				repeat1($._eol),
				repeat(seq($.expression, repeat1($._eol))),
				$._sym_brace_c,
			),

		_directive_block: $ =>
			seq(
				$._sym_brace_o,
				repeat1($._eol),
				repeat(seq($.subdirective, repeat1($._eol))),
				$._sym_brace_c,
			),
		_matcher_block: $ =>
			seq(
				$._sym_brace_o,
				repeat1($._eol),
				repeat(seq($.matcher_clause, repeat1($._eol))),
				$._sym_brace_c,
			),

		_snippet_name: $ =>
			seq($._sym_paren_o, optional(field('name', $._bare_identifier)), $._sym_paren_c),

		snippet_definition: $ => seq($._snippet_name, field('content', $._block)),

		expression: $ =>
			choice($.directive, $.matcher_definition, $.snippet_reference, $.invoke_statement),

		_named_route_name: $ =>
			seq(
				$._sym_ampersand,
				$._sym_paren_o,
				optional(field('name', $._bare_identifier)),
				$._sym_paren_c,
			),

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

		directive: $ => seq($._clause, optional($._directive_block)),

		subdirective: $ => $._clause,

		_matcher_name: $ => seq($._sym_at, field('name', $.identifier)),

		matcher_definition: $ =>
			seq(
				$._matcher_name,
				optional($._ws),
				choice($.matcher_clause, $._matcher_block, $.heredoc),
			),

		matcher_clause: $ => choice($.generic_matcher, $.expression_matcher),

		generic_matcher: $ =>
			prec.left(
				seq(
					field('matcher', $.identifier),
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
				seq($._keyword_field, optional($._matcher_field), repeat($._arguments_field)),
			),

		_keyword_field: $ => field('keyword', $.identifier),
		_matcher_field: $ => seq(field('matcher', $.matcher), repeat($._ws)),
		_arguments_field: $ => seq(field('argument', $.argument), repeat($._ws)),

		argument: $ =>
			choice(
				$.embedded_content,
				$.string,
				$.numeric,
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

		_ws: $ => $._ext_ws,
		_eol: $ => choice($.comment, $._ext_eol),

		comment: $ => seq($._sym_num, optional($._ext_str_comment), $._ext_eol),
	},
});
