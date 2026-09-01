#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define KEYWORD(text, token) {text, sizeof(text) - 1, token}
#define CLASS(text) {text, sizeof(text) - 1}
#define INVERT(name, fn)                                                       \
	static inline bool name(UnicodeChar c) { return !(fn)(c); }

enum {
	U32LEN = sizeof(uint32_t),
	HDRLEN = sizeof(uint8_t) + U32LEN + sizeof(uint8_t),
	TAGLEN = 64,
};

enum TokenType {
	/*
	 * never requested
	 */
	_UNSPECIFIED,

	/*
	 * heredoc
	 */
	HEREDOC_OPERATOR,
	HEREDOC_TAG,
	HEREDOC_CONTENT,
	HEREDOC_SUFFIX,

	/*
	 * strings
	 */
	STR_WORD,
	STR_BARE,
	STR_UPPER,
	STR_NUM,
	STR_DECIMAL,
	STR_HEX_BYTE,
	STR_IPV4,
	STR_CEL,
	STR_CEL_INLINE,
	STR_COMMENT,
	STR_QTY_INTEGER,
	STR_QTY_DECIMAL,

	/*
	 * whitespace
	 */
	EOL,
	WS,

	/*
	 * keywords
	 */
	KEY_BOOLEAN,
	KEY_IMPORT,
	KEY_INVOKE,
	KEY_PRIVATE_RANGES,
	KEY_EXPRESSION,
	KEY_VARS,
	KEY_ARGS,
	KEY_ENV,
	KEY_FILE,
	KEY_NOT,
	KEY_SITE,

	/*
	 * keyword classes
	 */
	CLS_PROTOCOL,
	CLS_REGEXP,
	CLS_UNIT_DURATION,
	CLS_UNIT_SIZE,

	/*
	 * raw symbolic
	 */
	SYM_PAREN_O,
	SYM_PAREN_C,
	SYM_BRACE_O,
	SYM_BRACE_C,
	SYM_BRACKET_O,
	SYM_BRACKET_C,
	SYM_COLON,
	SYM_SOLIDUS,
	SYM_HYPHEN,
	SYM_AT,
	SYM_COMMA,
	SYM_PERIOD,
	SYM_AMPERSAND,
	SYM_PLUS,
	SYM_NUM,
	SYM_DOLLAR,
	SYM_GT,
	SYM_GRAVE,
	SYM_QUOTE,
	SYM_ASTERISK, // handled separately
	SYM_EXCLAIM,
	SYM_QUESTION,
	SYM_PERCENT,
	SYM_BAR,
	SYM_EQUAL,

	/*
	 * semantic symbolic
	 */
	SYM_BLOCK_START,
	SYM_SCHEME,
	SYM_COMMENT,

	/*
	 * indicates that tree-sitter
	 * is in error recovery mode
	 */
	ERROR_SENTINEL,
};

/**
 * Type alias for 32-bit unicode character.
 */
typedef int32_t UnicodeChar;

/**
 * Keyword entry.
 * Use `KEYWORD` or `CLASS` macro to initialise.
 */
typedef struct {
	const char *text;
	uint8_t len;
	enum TokenType token;
} Keyword;

/**
 * Character-to-token map for Unicode symbolic operators.
 */
static const UnicodeChar sym_map[128] = {
    ['('] = SYM_PAREN_O,
    [')'] = SYM_PAREN_C,
    ['{'] = SYM_BRACE_O,
    ['}'] = SYM_BRACE_C,
    ['['] = SYM_BRACKET_O,
    [']'] = SYM_BRACKET_C,
    [':'] = SYM_COLON,
    ['/'] = SYM_SOLIDUS,
    ['-'] = SYM_HYPHEN,
    ['@'] = SYM_AT,
    [','] = SYM_COMMA,
    ['.'] = SYM_PERIOD,
    ['&'] = SYM_AMPERSAND,
    ['+'] = SYM_PLUS,
    ['#'] = SYM_NUM,
    ['$'] = SYM_DOLLAR,
    ['>'] = SYM_GT,
    ['`'] = SYM_GRAVE,
    ['"'] = SYM_QUOTE,
    ['!'] = SYM_EXCLAIM,
    ['?'] = SYM_QUESTION,
    ['%'] = SYM_PERCENT,
    ['|'] = SYM_BAR,
    ['='] = SYM_EQUAL,
};

/**
 * Safely indexes `sym_map` and returns the associated token enum.
 * Returns `_UNSPECIFIED` if no token exists for that character.
 */
static inline enum TokenType get_token(UnicodeChar c)
{
	return (c >= 128) ? _UNSPECIFIED : sym_map[c];
}

/**
 * String-to-token map for keywords.
 */
static const Keyword keywords[] = {
    KEYWORD("true", KEY_BOOLEAN),
    KEYWORD("false", KEY_BOOLEAN),
    KEYWORD("import", KEY_IMPORT),
    KEYWORD("invoke", KEY_INVOKE),
    KEYWORD("private_ranges", KEY_PRIVATE_RANGES),
    KEYWORD("expression", KEY_EXPRESSION),
    KEYWORD("vars", KEY_VARS),
    KEYWORD("args", KEY_ARGS),
    KEYWORD("not", KEY_NOT),
    KEYWORD("env", KEY_ENV),
    KEYWORD("file", KEY_FILE),
    KEYWORD("site", KEY_SITE),

};

/**
 * String array for `CLS_REGEXP`.
 */
static const Keyword regex_matchers[] = {
    CLASS("path_regexp"),
    CLASS("host_regexp"),
    CLASS("header_regexp"),
    CLASS("cookie_regexp"),
    CLASS("vars_regexp"),
};

/**
 * String array for `CLS_PROTOCOL`.
 */
static const Keyword protocols[] = {
    CLASS("unix"),
    CLASS("unixgram"),
    CLASS("unixpacket"),
    CLASS("tcp"),
    CLASS("tcp4"),
    CLASS("tcp6"),
    CLASS("udp"),
    CLASS("udp4"),
    CLASS("udp6"),
    CLASS("ip"),
    CLASS("ip4"),
    CLASS("ip6"),
    CLASS("h2c"),
    CLASS("fd"),
    CLASS("fdgram"),
};

/// === Scanner definition and convenience functions ===

typedef struct {
	/* persistent fields */
	bool in_quotation;
	uint8_t tag_len;
	UnicodeChar tag[TAGLEN];
	UnicodeChar previous;

	/* transient fields */
	TSLexer *lexer;
	const bool *vs;
	unsigned int consumed;
	uint8_t word_len;
	UnicodeChar word[TAGLEN];
} Scanner;

/**
 * Returns the current lexer column position.
 */
static inline uint32_t get_column(Scanner *s)
{
	assert(s != NULL);
	return s->lexer->get_column(s->lexer);
}

/**
 * Sets the lexer's result symbol.
 */
static inline void set_result(Scanner *s, enum TokenType token)
{
	assert(s != NULL);
	s->lexer->result_symbol = token;
}

/**
 * Sets the end boundary of the current token to the current lexer position.
 */
static inline void mark_end(Scanner *s) { s->lexer->mark_end(s->lexer); }

/**
 * Returns true if lexer has reached the end of the file.
 */
static inline bool eof(Scanner *s) { return s->lexer->eof(s->lexer); }

/**
 * Returns true if `token` is valid in the current context.
 */
static inline bool is_valid(Scanner *s, enum TokenType token)
{
	assert(s != NULL);
	return s->vs != NULL && s->vs[token];
}

/**
 * Returns current lexer lookahead character.
 */
static inline UnicodeChar peek(Scanner *s) { return s->lexer->lookahead; }

/// === Asserter predicate functions ===

/**
 * Function type for unicode character predicates.
 */
typedef bool (*Asserter)(UnicodeChar);

/**
 * Matches ASCII decimal digits.
 * Implements `Asserter`.
 */
static inline bool is_digit(UnicodeChar c) { return (c >= '0' && c <= '9'); }

/**
 * Matches ASCII hexadecimal digits.
 * Implements `Asserter`.
 */
static inline bool is_hex(UnicodeChar c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
	       (c >= 'a' && c <= 'f');
}

/**
 * Matches ASCII uppercase letters.
 * Implements `Asserter`.
 */
static inline bool is_upper(UnicodeChar c) { return (c >= 'A' && c <= 'Z'); }

/**
 * Matches ASCII lowercase letters.
 * Implements `Asserter`.
 */
static inline bool is_lower(UnicodeChar c) { return (c >= 'a' && c <= 'z'); }

/**
 * Matches all ASCII characters.
 * Implements `Asserter`.
 */
static inline bool is_ascii(UnicodeChar c) { return (c >= 0 && c <= 0x7e); }

/**
 * Matches ASCII alphabetic characters..
 * Implements `Asserter`.
 */
static inline bool is_alpha(UnicodeChar c)
{
	return is_upper(c) || is_lower(c);
}

/**
 * Matches ASCII alphanumeric characters.
 * Implements `Asserter`.
 */
static inline bool is_alnum(UnicodeChar c)
{
	return is_digit(c) || is_alpha(c);
}

/**
 * Matches Unicode whitespace characters.
 * Implements `Asserter`.
 */
static inline bool is_ws(UnicodeChar c)
{
	switch (c) {
	case ' ':
	case '\t':
	case 0x00A0:
	case 0x1680:
	case 0x2000:
	case 0x2001:
	case 0x2002:
	case 0x2003:
	case 0x2004:
	case 0x2005:
	case 0x2006:
	case 0x2007:
	case 0x2008:
	case 0x2009:
	case 0x200A:
	case 0x202F:
	case 0x205F:
	case 0x3000:
		return true;
	default:
		return false;
	}
}

/**
 * Matches unicode end-of-line characters.
 * Implements `Asserter`.
 */
static inline bool is_eol(UnicodeChar c)
{
	switch (c) {
	case 0x000A: // LF \n
	case 0x000B: // VT \v
	case 0x000C: // FF \f
	case 0x000D: // CR \r
	case 0x0085: // NEL
	case 0x2028: // LS
	case 0x2029: // PS
		return true;
	default:
		return false;
	}
}

/**
 * Matches all unicode chars except end-of-line characters.
 * Inverse of `is_eol`.
 * Implements `Asserter`.
 */
INVERT(is_not_eol, is_eol)

/*
 * Matches a subset of address delimiter characters.
 * Implements `Asserter`.
 */
static inline bool is_delim(UnicodeChar c)
{
	switch (c) {
	case '.':
	case ':':
	case '#':
	case '/':
	case '?':
	case '+':
		return true;
	default:
		return false;
	}
}

/*
 * Matches parentheses, braces, and square brackets. But never chevrons.
 * Implements `Asserter`.
 */
static inline bool is_bracket(UnicodeChar c)
{
	switch (c) {
	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
		return true;
	default:
		return false;
	}
}

/**
 * Returns true if `c` is an size-unit prefix.
 * Valid prefixes are k, m, g, t, p, and e, case-insensitive.
 * Implements `Asserter`.
 */
static inline bool is_size_prefix(UnicodeChar c)
{
	if (!is_ascii(c))
		return false;
	c |= 0x20;
	return c == 'k' || c == 'm' || c == 'g' || c == 't' || c == 'p' ||
	       c == 'e';
}

/**
 * Returns true if `c` is byte-size unit.
 * The valid unit is b, case-insensitive.
 * Implements `Asserter`.
 */
static inline bool is_size_suffix(UnicodeChar c)
{
	return c == 'b' || c == 'B';
}

/*
 *	=== Sized string matcher functions ===
 */

/*
 * Matches valid size unit unicode strings (eg. b, MB, GiB, k).
 *
 * [spec](https://caddyserver.com/docs/caddyfile/directives/request_body#syntax)
 * [go-humanize](https://pkg.go.dev/github.com/dustin/go-humanize#pkg-constants)
 */
static inline bool is_size_unit(const UnicodeChar *kw, uint8_t len)
{
	assert(kw != NULL);
	switch (len) {
	case 1:
		return is_size_suffix(kw[0]) || is_size_prefix(kw[0]);
	case 2:
		return is_size_prefix(kw[0]) && is_size_suffix(kw[1]);
	case 3:
		return is_size_prefix(kw[0]) && kw[1] == 'i' &&
		       is_size_suffix(kw[2]);
	default:
		return false;
	}
}

/*
 * Matches valid duration unit unicode strings (eg. ms, s, h, µs).
 *
 * [spec](https://caddyserver.com/docs/conventions#durations)
 * [time.ParseDuration](https://golang.org/pkg/time/#ParseDuration)
 */
static bool is_duration_unit(const UnicodeChar *kw, uint8_t len)
{
	assert(kw != NULL);
	switch (len) {
	case 1:
		switch (kw[0]) {
		case 's':
		case 'm':
		case 'h':
		case 'd':
			return true;
		default:
			return false;
		}
	case 2:
		switch (kw[0]) {
		case 'n':
		case 'u':
		case 0x00B5:
		case 'm':
			return kw[1] == 's';
		default:
			return false;
		}
	default:
		return false;
	}
}

/**
 * Returns true if `kw` matches either a duration or unit keyword class.
 */
static inline bool is_unit(const UnicodeChar *kw, uint8_t len)
{
	return is_duration_unit(kw, len) || is_size_unit(kw, len);
}

/*
 * Sized unicode string comparator for `s->word` and `kw`.
 */
static bool word_equals(Scanner *s, const Keyword *kw)
{
	assert(s != NULL);
	if (s->word_len != kw->len)
		return false;
	for (size_t i = 0; i < s->word_len && i < TAGLEN; i++)
		if (s->word[i] != (UnicodeChar)kw->text[i])
			return false;
	return true;
}

/**
 * Performs search on `s->token` for CLS_UNIT_DURATION token matches.
 * Do not call directly without performing token length and null checks.
 *
 * [spec](https://caddyserver.com/docs/conventions#durations)
 */
static enum TokenType check_unit_duration(Scanner *s)
{
	return is_valid(s, CLS_UNIT_DURATION) &&
		       is_duration_unit(s->word, s->word_len)
		   ? CLS_UNIT_DURATION
		   : _UNSPECIFIED;
}

/**
 * Performs search on `s->token` for CLS_UNIT_SIZE token matches.
 * Do not call directly without performing token length and null checks.
 *
 * Based on [Go's
 * time.ParseDuration](https://golang.org/pkg/time/#ParseDuration) syntax
 * [spec](https://caddyserver.com/docs/conventions#durations)
 */
static enum TokenType check_unit_size(Scanner *s)
{
	return is_valid(s, CLS_UNIT_SIZE) && is_size_unit(s->word, s->word_len)
		   ? CLS_UNIT_SIZE
		   : _UNSPECIFIED;
}

/**
 * Performs search on `s->token` for CLS_PROTOCOL token matches.
 * Do not call directly without performing token length and null checks.
 */
static enum TokenType check_protocol(Scanner *s)
{
	UnicodeChar c = peek(s);
	if (!is_valid(s, CLS_PROTOCOL) || (c != '+' && c != '/'))
		return _UNSPECIFIED;
	for (size_t i = 0; i < ARRAY_LEN(protocols); i++) {
		const Keyword *kw = &protocols[i];
		if (word_equals(s, kw))
			return CLS_PROTOCOL;
	}
	return _UNSPECIFIED;
}

/**
 * Performs search on `s->token` for CLS_REGEXP token matches.
 * Do not call directly without performing token length and null checks.
 */
static enum TokenType check_regex_matchers(Scanner *s)
{
	if (!is_valid(s, CLS_REGEXP))
		return _UNSPECIFIED;
	for (size_t i = 0; i < ARRAY_LEN(regex_matchers); i++) {
		const Keyword *kw = &regex_matchers[i];
		if (word_equals(s, kw))
			return CLS_REGEXP;
	}
	return _UNSPECIFIED;
}

/**
 * Performs search on `s->token` for keyword matches
 * Do not call directly without performing token length and null checks.
 */
static enum TokenType check_keyword(Scanner *s)
{
	for (size_t i = 0; i < ARRAY_LEN(keywords); i++) {
		const Keyword *kw = &keywords[i];
		if (is_valid(s, kw->token) && word_equals(s, kw))
			return kw->token;
	}
	return _UNSPECIFIED;
}

/**
 * Matches `s->word` against the scanner's token classes in precedence order.
 * Returns the first matching token type, or `_UNSPECIFIED` if no match exists.
 */
static enum TokenType match(Scanner *s)
{
	assert(s != NULL);

	if (s->consumed != s->word_len)
		return _UNSPECIFIED;

	enum TokenType token;

	token = check_unit_duration(s);
	if (token != _UNSPECIFIED)
		return token;

	token = check_unit_size(s);
	if (token != _UNSPECIFIED)
		return token;

	token = check_protocol(s);
	if (token != _UNSPECIFIED)
		return token;

	token = check_regex_matchers(s);
	if (token != _UNSPECIFIED)
		return token;

	token = check_keyword(s);
	if (token != _UNSPECIFIED)
		return token;

	return _UNSPECIFIED;
}

/// === Navigation convenience functions ===

/**
 * Consumes the current character and advances the lexer.
 */
static inline void advance(Scanner *s)
{
	if (eof(s))
		return;
	s->previous = peek(s);
	s->lexer->advance(s->lexer, false);
	if (s->word_len == 0 && s->consumed < TAGLEN) {
		s->word[s->consumed] = s->previous;
		UnicodeChar c = peek(s);
		if (!is_alnum(c) && c != '_')
			s->word_len = s->consumed + 1;
	}
	s->consumed++;
}

/**
 * Skips the current character and advances the lexer.
 */
static inline void skip(Scanner *s)
{
	if (eof(s))
		return;
	s->previous = peek(s);
	s->lexer->advance(s->lexer, true);
}

/**
 * Skips characters while `fn` accepts the next character.
 * Stops at EOF or when `fn(peek(s))` returns false.
 */
static inline void skip_while(Scanner *s, Asserter fn)
{
	while (!eof(s) && fn(peek(s))) {
		skip(s);
	}
}

/**
 * Advances through characters while `fn` accepts the next character.
 * Stops at EOF or when `fn(peek(s))` returns false.
 */
static inline void advance_while(Scanner *s, Asserter fn)
{
	while (!eof(s) && fn(peek(s))) {
		advance(s);
	}
}

/*
 * Consume the rest of the current line, including terminating
 * EOL characters.
 */
static inline void advance_rol(Scanner *s)
{
	while (!eof(s) && !is_eol(peek(s)))
		advance(s);
	if (eof(s))
		return;
	advance(s);
	if (s->previous == '\r' && !eof(s) && peek(s) == '\n')
		advance(s);
}

/// === Scanner control flow ===

/*
 * Handles lexing the heredoc operator, tag, and content.
 */
static bool scan_heredoc(Scanner *s)
{
	if (is_valid(s, ERROR_SENTINEL))
		return false;
	uint8_t n;
	if (is_valid(s, HEREDOC_CONTENT) && s->tag_len != 0) {
		while (!eof(s)) {
			advance_while(s, is_ws);
			mark_end(s);
			for (n = 0; n < TAGLEN && n < s->tag_len; n++) {
				if (eof(s))
					break;
				if (peek(s) != s->tag[n])
					break;
				advance(s);
			}

			if (n == s->tag_len) {
				set_result(s, HEREDOC_CONTENT);
				return true;
			}
			advance_rol(s);
		}
		mark_end(s);
		set_result(s, HEREDOC_CONTENT);
		return true;
	}

	if (is_valid(s, HEREDOC_SUFFIX)) {
		for (int i = 0; i < s->tag_len; i++)
			advance(s);
		if (s->tag_len != s->consumed) {
			s->tag_len = 0;
			return false;
		}
		s->tag_len = 0;
		mark_end(s);
		set_result(s, HEREDOC_SUFFIX);
		return true;
	}

	if (is_valid(s, HEREDOC_OPERATOR)) {
		if (peek(s) != '<')
			return false;
		advance(s);
		if (peek(s) != '<')
			return false;
		advance(s);
		if (!(is_alnum(peek(s))) && peek(s) != '_')
			return false;
		mark_end(s);
		set_result(s, HEREDOC_OPERATOR);
		return true;
	}

	if (is_valid(s, HEREDOC_TAG)) {
		while (!eof(s)) {
			UnicodeChar c = peek(s);
			if (is_ws(c) || is_eol(c) || c == '#')
				break;
			if (s->tag_len < TAGLEN) {
				s->tag[s->tag_len] = c;
				s->tag_len++;
			}
			advance(s);
		}
		if (s->tag_len == 0)
			return false;
		mark_end(s);
		set_result(s, HEREDOC_TAG);
		return true;
	}
	return false;
}

/*
 * Main scanner control flow.
 */
static void scan_text(Scanner *s)
{
	UnicodeChar prefix = s->previous;

	if (is_valid(s, WS) && is_ws(peek(s))) {
		advance_while(s, is_ws);
		set_result(s, WS);
		mark_end(s);
	}

	skip_while(s, is_ws);

	UnicodeChar c = peek(s);

	if (c == '#' && (get_column(s) == 0 || is_ws(s->previous))) {
		advance(s);
		mark_end(s);
		set_result(s, SYM_COMMENT);
		return;
	}

	if (s->consumed > 0)
		return;

	if (is_eol(peek(s))) {
		advance(s);
		set_result(s, EOL);
		mark_end(s);
		return;
	}

	if (is_valid(s, SYM_BLOCK_START) && c == '{') {
		advance(s);
		mark_end(s);
		advance_while(s, is_ws);
		c = peek(s);
		if (eof(s) || is_eol(c) || c == '#') {
			set_result(s, SYM_BLOCK_START);
			return;
		}
		if (is_valid(s, SYM_BRACE_O)) {
			set_result(s, SYM_BRACE_O);
			return;
		}
	}

	enum TokenType token = get_token(c);
	if (token != _UNSPECIFIED && is_valid(s, token)) {
		advance(s);
		mark_end(s);
		set_result(s, token);
		if (token == SYM_QUOTE)
			s->in_quotation = !s->in_quotation;
		if (token == SYM_COLON && is_valid(s, SYM_SCHEME) &&
		    peek(s) == '/') {
			advance(s);
			if (peek(s) == '/') {
				advance(s);
				mark_end(s);
				set_result(s, SYM_SCHEME);
			}
		}
		return;
	}

	if (c == '*' && is_valid(s, SYM_ASTERISK)) {
		advance(s);
		c = peek(s);
		if (is_ws(c) || is_eol(c) || eof(s)) {
			mark_end(s);
			set_result(s, SYM_ASTERISK);
			return;
		}
	}

	bool escape = false;
	bool digits = true;
	bool hex = true;
	bool upper = true;
	bool kw = true;
	int nperiod = 0;

	while (!eof(s)) {
		/* skip logic upon ESCAPE char */
		if (escape) {
			advance(s);
			mark_end(s);
			escape = false;
			continue;
		}

		UnicodeChar c = peek(s);

		if (is_valid(s, STR_COMMENT) && c != '@' && prefix != '@') {
			advance_while(s, is_not_eol);
			mark_end(s);
			set_result(s, STR_COMMENT);
			return;
		}

		if (c == '\\') {
			advance(s);
			escape = true;
			continue;
		}

		if (is_valid(s, STR_CEL) && prefix == '`') {
			mark_end(s);
			if (c != '`') {
				advance(s);
				continue;
			}
			set_result(s, STR_CEL);
			return;
		}

		if (!is_valid(s, ERROR_SENTINEL) &&
		    is_valid(s, STR_CEL_INLINE) && c != '`') {
			while (!eof(s) && !is_eol(peek(s))) {
				if (is_ws(peek(s))) {
					advance(s);
					if (peek(s) == '#')
						break;
					continue;
				}
				advance(s);
			}
			mark_end(s);
			set_result(s, STR_CEL_INLINE);
			return;
		}

		if (is_eol(c) || (is_ws(c) && !s->in_quotation)) {
			if (kw) {
				enum TokenType keyword = match(s);
				if (keyword != _UNSPECIFIED) {
					mark_end(s);
					set_result(s, keyword);
					return;
				}
				kw = false;
			}
			s->in_quotation = false;
			break;
		}

		nperiod += (c == '.');

		enum TokenType token = get_token(c);
		if ((s->consumed > 0 && (is_delim(c) && !s->in_quotation)) ||
		    (token != _UNSPECIFIED && is_valid(s, token))) {
			if (kw) {
				enum TokenType keyword = match(s);
				if (keyword != _UNSPECIFIED) {
					mark_end(s);
					set_result(s, keyword);
					return;
				}
				kw = false;
			}
			if (token == SYM_PERIOD &&
			    (digits && (is_valid(s, STR_DECIMAL) ||
					   is_valid(s, STR_QTY_DECIMAL) ||
					   is_valid(s, STR_IPV4)))) {
				advance(s);
				continue;
			}
			mark_end(s);
			break;
		}

		upper = upper && is_upper(c);
		hex = hex && is_hex(c);

		if (digits && !is_digit(c) && c != '.') {
			mark_end(s);
			digits = false;
			if (is_alpha(c) && s->consumed > 0 && nperiod <= 1 &&
			    is_valid(s, STR_QTY_INTEGER) &&
			    is_valid(s, STR_QTY_DECIMAL)) {
				UnicodeChar suffix[3] = {c};
				advance(s);
				c = peek(s);
				uint8_t len = 1;
				while (!eof(s) && len < 3) {
					if (!is_alpha(c))
						break;
					suffix[len] = c;
					len++;
					advance(s);
					c = peek(s);
				}

				if (!(is_ws(c) || is_eol(c))) {
					continue;
				}

				if (!is_unit(suffix, len))
					continue;

				switch (nperiod) {
				case 0:
					set_result(s, STR_QTY_INTEGER);
					return;
				case 1:
					set_result(s, STR_QTY_DECIMAL);
					return;
				default:
					continue;
				}

				return;
			}
			continue;
		}

		advance(s);
		mark_end(s);
	}

	if (escape)
		mark_end(s);

	if (s->consumed == 0)
		return;

	c = peek(s);

	if (is_valid(s, STR_WORD) && !is_valid(s, STR_CEL) &&
	    (eof(s) || peek(s) == '{')) {
		set_result(s, STR_WORD);
		mark_end(s);
		return;
	}

	if (hex && s->consumed == 2 && is_valid(s, STR_HEX_BYTE) &&
	    (c == ':' || prefix == ':'))
		set_result(s, STR_HEX_BYTE);
	else if (upper && is_valid(s, STR_UPPER))
		set_result(s, STR_UPPER);
	else if (digits && is_valid(s, STR_NUM) && nperiod == 0)
		set_result(s, STR_NUM);
	else if (digits && is_valid(s, STR_DECIMAL) && nperiod == 1)
		set_result(s, STR_DECIMAL);
	else if (digits && is_valid(s, STR_IPV4) && nperiod == 3)
		set_result(s, STR_IPV4);
	else if (!is_valid(s, ERROR_SENTINEL) && is_valid(s, STR_CEL))
		set_result(s, STR_CEL);
	else if (is_valid(s, STR_BARE) && (get_column(s) == 0 || prefix != '}'))
		set_result(s, STR_BARE);
	else
		set_result(s, STR_WORD);

	mark_end(s);
	return;
}

/// Scanner initialisation logic

/*
 * Initialises persistent field values.
 * Modifications to these values persist across scanner
 * instances.
 */
static inline void init_persistent_fields(Scanner *s)
{
	s->in_quotation = false;
	s->tag_len = 0;
	s->previous = '\0';
}

/*
 * Sets transient field values.
 * These fields do not persist across scanner instances.
 */
static inline void reset_transient_fields(Scanner *s)
{
	s->consumed = 0;
	s->word_len = 0;
}

void *tree_sitter_caddyfile_external_scanner_create(void)
{
	Scanner *s = ts_calloc(1, sizeof(Scanner));
	init_persistent_fields(s);
	reset_transient_fields(s);
	return s;
}

void tree_sitter_caddyfile_external_scanner_destroy(void *payload)
{
	ts_free(payload);
}

static inline void ser_u32_le(char *buffer, uint32_t value)
{
	buffer[0] = (char)(value >> 0);
	buffer[1] = (char)(value >> 8);
	buffer[2] = (char)(value >> 16);
	buffer[3] = (char)(value >> 24);
}

static inline uint32_t deser_u32_le(const char *buffer)
{
	return ((uint32_t)(uint8_t)buffer[0] << 0) |
	       ((uint32_t)(uint8_t)buffer[1] << 8) |
	       ((uint32_t)(uint8_t)buffer[2] << 16) |
	       ((uint32_t)(uint8_t)buffer[3] << 24);
}

static inline unsigned serialized_size(const Scanner *s)
{
	return HDRLEN + s->tag_len * U32LEN;
}

unsigned tree_sitter_caddyfile_external_scanner_serialize(
    void *payload, char *buffer)
{
	Scanner *s = payload;

	buffer[0] = (char)s->tag_len;
	ser_u32_le(buffer + 1, (uint32_t)s->previous);
	buffer[1 + U32LEN] = (char)s->in_quotation;

	for (unsigned i = 0; i < s->tag_len; i++)
		ser_u32_le(buffer + HDRLEN + i * U32LEN, (uint32_t)s->tag[i]);

	return serialized_size(s);
}

void tree_sitter_caddyfile_external_scanner_deserialize(
    void *payload, const char *buffer, unsigned length)
{
	Scanner *s = payload;

	reset_transient_fields(s);

	if (length < HDRLEN)
		return;

	s->tag_len = (uint8_t)buffer[0];
	s->previous = (UnicodeChar)deser_u32_le(buffer + 1);
	s->in_quotation = buffer[1 + U32LEN] != 0;

	if (s->tag_len > TAGLEN)
		s->tag_len = TAGLEN;

	unsigned available = (length - HDRLEN) / U32LEN;

	if (s->tag_len > available)
		s->tag_len = available;

	for (unsigned i = 0; i < s->tag_len; i++) {
		s->tag[i] =
		    (UnicodeChar)deser_u32_le(buffer + HDRLEN + i * U32LEN);
	}
}

bool tree_sitter_caddyfile_external_scanner_scan(
    void *payload, TSLexer *lexer, const bool *valid_symbols)
{
	Scanner *scanner = payload;
	reset_transient_fields(scanner);

	scanner->lexer = lexer;
	scanner->vs = valid_symbols;

	if (scan_heredoc(scanner))
		return scanner->consumed != 0;

	scan_text(scanner);
	return scanner->consumed != 0;
}
