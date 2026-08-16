/**
 * @file A grammar for Caddyfile
 * @author Tom Spencer
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
	name: 'caddyfile',

	supertypes: $ => [$.reference, $.definition],

	externals: $ => [
		$._ext_unspecified,

		// heredoc
		$._ext_heredoc_operator,
		$._ext_heredoc_tag,
		$._ext_heredoc_content,
		$._ext_heredoc_suffix,

		// plain text content
		$._ext_str_word,
		$._ext_str_bare,
		$._ext_str_upper,
		$._ext_str_num,
		$._ext_str_cel,

		// whitespace
		$._ext_eol,
		$._ext_ws,
		$._ext_comment,

		// keywords
		$._ext_key_import,

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

		// error recovery indicator
		$._error_sentinel,
	],

	extras: $ => [],

	rules: {
		caddyfile: $ => repeat1(choice($.site_block, $.snippet, $.snippet_reference, $._eol)),

		site_block: $ => seq(repeat(seq($._site_field, $._sd)), field('content', $._block)),

		_sd: $ => seq(repeat1($._d), optional(seq($._eol, repeat($._d)))),
		_d: $ => choice($._ws, $._sym_comma),

		_site_field: $ => field('site', $.identifier),

		_word: $ => $._ext_str_word,

		// identifier: $ => $._ext_str_bare,
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
				repeat(seq($.definition, repeat1($._eol))),
				$._sym_brace_c,
			),

		_snippet_name: $ =>
			seq($._sym_paren_o, optional(field('name', $._bare_identifier)), $._sym_paren_c),
		snippet: $ => seq($._snippet_name, field('content', $._block)),

		definition: $ => choice($.statement, $.matcher_definition, $.snippet_reference),
		_matcher_name: $ => seq($._sym_at, field('name', $.identifier)),
		matcher_definition: $ => seq($._matcher_name, $._block),
		named_matcher: $ => $._matcher_name,
		path_matcher: $ => prec.left(seq(field('path', $.path), $._ws)),

		path: $ =>
			seq(
				$._sym_solidus,
				repeat(field('segment', choice($.wildcard, $.identifier, $._sym_solidus))),
			),

		matcher: $ => choice($.wildcard, $.named_matcher, $.path_matcher),
		wildcard: $ => $._sym_asterisk,

		statement: $ =>
			seq(
				field('directive', $.identifier),
				optional(seq(field('matcher', $.matcher), repeat($._ws))),
				repeat(seq(field('argument', $._arg), repeat($._ws))),
			),

		snippet_reference: $ =>
			prec.right(
				seq(
					$._key_import,
					repeat($._ws),
					repeat(seq(field('argument', $._arg), repeat($._ws))),
				),
			),

		_key_import: $ => alias($._ext_key_import, 'import'),

		_quoted_string: $ =>
			seq($._sym_quote, repeat(choice($._substring, $._ws)), $._sym_quote),

		cel_expression: $ =>
			seq($._sym_grave, optional(field('content', $.cel_content)), $._sym_grave),
		cel_content: $ => $._ext_str_cel,

		_arg: $ => choice($.cel_expression, $.string, $.numeric, $.verb),
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
		_eol: $ => $._ext_eol,
	},
});
