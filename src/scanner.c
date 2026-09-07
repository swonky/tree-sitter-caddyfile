#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define KEYWORD(text, token) {{text, sizeof(text) - 1}, token}
#define CLASS(text) {text, sizeof(text) - 1}
#define INVERT(name, fn)                                                       \
	static inline bool name(CodePoint c) { return !(fn)(c); }

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
	KEY_ENV,
	KEY_FILE,
	KEY_SYSTEM,
	KEY_TIME,
	KEY_NOW,
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
	SYM_CHEVRON_O,
	SYM_CHEVRON_C,
	SYM_COLON,
	SYM_SLASH,
	SYM_BSLASH,
	SYM_HYPHEN,
	SYM_AT,
	SYM_COMMA,
	SYM_PERIOD,
	SYM_AMPERSAND,
	SYM_PLUS,
	SYM_NUM,
	SYM_DOLLAR,
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
	SYM_DOT_PATH,
	SYM_DOT_DOT_PATH,
	SYM_COLON_DRIVE,

	/*
	 * indicates that tree-sitter
	 * is in error recovery mode
	 */
	ERROR_SENTINEL,
};

enum {
	U32_SIZE = sizeof(uint32_t),
	HEADER_SIZE = sizeof(uint8_t) + U32_SIZE + sizeof(uint8_t),
	STRING_BUFFER_SIZE = 64,
};

/**
 * Type alias for 32-bit unicode character.
 */
typedef int32_t CodePoint;

typedef struct {
	CodePoint s[STRING_BUFFER_SIZE];
	size_t len;
} StrBuffer;

static void append(StrBuffer *buf, CodePoint c)
{
	if (buf->len >= STRING_BUFFER_SIZE)
		return;
	buf->s[buf->len++] = c;
}

static void reset(StrBuffer *buf) { buf->len = 0; }

/**
 * Word entry. Use `CLASS` macro to initialise.
 */
typedef struct {
	const char *s;
	size_t len;
} StrView;

/**
 * Keyword entry. Use `KEYWORD` macro to initialise.
 */
typedef struct {
	StrView word;
	enum TokenType token;
} Keyword;

/**
 * Character-to-token map for Unicode symbolic operators.
 * Don't access directly. Use `get_token`.
 */
static const enum TokenType sym_map[128] = {
    ['('] = SYM_PAREN_O,
    [')'] = SYM_PAREN_C,
    ['{'] = SYM_BRACE_O,
    ['}'] = SYM_BRACE_C,
    ['['] = SYM_BRACKET_O,
    [']'] = SYM_BRACKET_C,
    ['<'] = SYM_CHEVRON_O,
    ['>'] = SYM_CHEVRON_C,
    [':'] = SYM_COLON,
    ['/'] = SYM_SLASH,
    ['-'] = SYM_HYPHEN,
    ['@'] = SYM_AT,
    [','] = SYM_COMMA,
    ['.'] = SYM_PERIOD,
    ['&'] = SYM_AMPERSAND,
    ['+'] = SYM_PLUS,
    ['#'] = SYM_NUM,
    ['$'] = SYM_DOLLAR,
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
static inline enum TokenType get_token(CodePoint c)
{
	unsigned int uc = (unsigned int)c;
	return (uc >= 128) ? _UNSPECIFIED : sym_map[uc];
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
    KEYWORD("env", KEY_ENV),
    KEYWORD("file", KEY_FILE),
    KEYWORD("system", KEY_SYSTEM),
    KEYWORD("time", KEY_TIME),
    KEYWORD("site", KEY_SITE),
    KEYWORD("now", KEY_NOW),
    KEYWORD("not", KEY_NOT),
};

/**
 * String array for `CLS_REGEXP`.
 */
static const StrView regex_matchers[] = {
    CLASS("path_regexp"),
    CLASS("host_regexp"),
    CLASS("header_regexp"),
    CLASS("cookie_regexp"),
    CLASS("vars_regexp"),
};

/**
 * String array for `CLS_PROTOCOL`.
 */
static const StrView protocols[] = {
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

typedef struct {
	/* persistent fields */
	bool in_quotation;
	// Current heredoc tag.
	StrBuffer hdoc_tag;
	// Previous character
	CodePoint previous;

	// Tree-sitter lexer pointer.
	TSLexer *lexer;
	// Valid symbols array.
	const bool *vs;
	// Consumed character counter.
	uint32_t consumed;
	// Consumed character buffer.
	StrBuffer buffer;
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
	s->lexer->result_symbol = (TSSymbol)token;
}

/**
 * Sets the end boundary of the current token to the current lexer position.
 */
static inline void mark_end(Scanner *s) { s->lexer->mark_end(s->lexer); }

/**
 * Returns true if lexer has reached the end of the file.
 */
static inline bool eof(const Scanner *s) { return s->lexer->eof(s->lexer); }

/**
 * Returns true if `token` is valid in the current context.
 */
static inline bool is_valid(const Scanner *s, enum TokenType token)
{
	assert(s != NULL);
	return s->vs != NULL && s->vs[token];
}

/**
 * Returns current lexer lookahead character.
 */
static inline CodePoint peek(const Scanner *s) { return s->lexer->lookahead; }

/// === Asserter predicate functions ===

/**
 * Function type for unicode character predicates.
 */
typedef bool (*Asserter)(CodePoint);

/**
 * Matches ASCII decimal digits.
 * Implements `Asserter`.
 */
static inline bool is_digit(CodePoint c) { return (c >= '0' && c <= '9'); }

/**
 * Matches ASCII hexadecimal digits.
 * Implements `Asserter`.
 */
static inline bool is_hex(CodePoint c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
	       (c >= 'a' && c <= 'f');
}

/**
 * Matches ASCII uppercase letters.
 * Implements `Asserter`.
 */
static inline bool is_upper(CodePoint c) { return (c >= 'A' && c <= 'Z'); }

/**
 * Matches ASCII lowercase letters.
 * Implements `Asserter`.
 */
static inline bool is_lower(CodePoint c) { return (c >= 'a' && c <= 'z'); }

/**
 * Matches all ASCII characters.
 * Implements `Asserter`.
 */
static inline bool is_ascii(CodePoint c) { return (c >= 0 && c <= 0x7e); }

/**
 * Matches ASCII alphabetic characters..
 * Implements `Asserter`.
 */
static inline bool is_alpha(CodePoint c) { return is_upper(c) || is_lower(c); }

/**
 * Matches ASCII alphanumeric characters.
 * Implements `Asserter`.
 */
static inline bool is_alnum(CodePoint c) { return is_digit(c) || is_alpha(c); }

/**
 * Matches Unicode whitespace characters.
 * Implements `Asserter`.
 */
static inline bool is_ws(CodePoint c)
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
static inline bool is_eol(CodePoint c)
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

/**
 * Matches characters that can be escaped with a preceding backslash.
 * Implements `Asserter`.
 */
static inline bool is_escapable(CodePoint c)
{
	switch (c) {
	case '\\':
	case '"':
	case '{':
	case '}':
	case '<':
		return true;
	default:
		return false;
	}
}

/*
 * Matches a subset of address delimiter characters.
 * Implements `Asserter`.
 */
static inline bool is_delim(CodePoint c)
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
 * Matches a subset of unary operator characters.
 * [spec](https://caddyserver.com/docs/caddyfile/directives/header)
 * Implements `Asserter`.
 */
static inline bool is_unary_operator(CodePoint c)
{
	switch (c) {
	case '+':
	case '-':
	case '?':
	case '!':
	case '>':
	case '<':
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
static inline bool is_size_prefix(CodePoint c)
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
static inline bool is_size_suffix(CodePoint c) { return c == 'b' || c == 'B'; }

/*
 *	=== Sized string matcher functions ===
 */

/*
 * Matches valid size unit unicode strings (eg. b, MB, GiB, k).
 *
 * [spec](https://caddyserver.com/docs/caddyfile/directives/request_body#syntax)
 * [go-humanize](https://pkg.go.dev/github.com/dustin/go-humanize#pkg-constants)
 */
static inline bool is_size_unit(StrBuffer buf)
{
	switch (buf.len) {
	case 1:
		return is_size_suffix(buf.s[0]) || is_size_prefix(buf.s[0]);
	case 2:
		return is_size_prefix(buf.s[0]) && is_size_suffix(buf.s[1]);
	case 3:
		return is_size_prefix(buf.s[0]) && buf.s[1] == 'i' &&
		       is_size_suffix(buf.s[2]);
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
static bool is_duration_unit(StrBuffer buf)
{
	switch (buf.len) {
	case 1:
		switch (buf.s[0]) {
		case 's':
		case 'm':
		case 'h':
		case 'd':
			return true;
		default:
			return false;
		}
	case 2:
		switch (buf.s[0]) {
		case 'n':
		case 'u':
		case 0x00B5:
		case 'm':
			return buf.s[1] == 's';
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
static inline bool is_unit(StrBuffer buf)
{
	return is_duration_unit(buf) || is_size_unit(buf);
}

/*
 * Sized unicode string comparator for `s->word` and `kw`.
 */
static bool word_equals(StrBuffer buf, const StrView *view)
{
	assert(view != NULL);
	if (buf.len != view->len)
		return false;
	for (size_t i = 0; i < buf.len && i < STRING_BUFFER_SIZE; i++)
		if (buf.s[i] != (CodePoint)view->s[i])
			return false;
	return true;
}

/**
 * Performs search on `s->token` for CLS_PROTOCOL token matches.
 * Do not call directly without performing token length and null checks.
 */
static enum TokenType check_protocol(const Scanner *s)
{
	CodePoint c = peek(s);
	if (!is_valid(s, CLS_PROTOCOL) || (c != '+' && c != '/'))
		return _UNSPECIFIED;
	for (size_t i = 0; i < ARRAY_LEN(protocols); i++) {
		const StrView *ref = &protocols[i];
		if (word_equals(s->buffer, ref))
			return CLS_PROTOCOL;
	}
	return _UNSPECIFIED;
}

/**
 * Performs search on `s->token` for CLS_REGEXP token matches.
 * Do not call directly without performing token length and null checks.
 */
static enum TokenType check_regex_matchers(const Scanner *s)
{
	if (!is_valid(s, CLS_REGEXP))
		return _UNSPECIFIED;
	for (size_t i = 0; i < ARRAY_LEN(regex_matchers); i++) {
		const StrView *ref = &regex_matchers[i];
		if (word_equals(s->buffer, ref))
			return CLS_REGEXP;
	}
	return _UNSPECIFIED;
}

/**
 * Performs search on `s->token` for keyword matches
 * Do not call directly without performing token length and null checks.
 */
static enum TokenType check_keyword(const Scanner *s)
{
	for (size_t i = 0; i < ARRAY_LEN(keywords); i++) {
		const Keyword *kw = &keywords[i];
		if (is_valid(s, kw->token) && word_equals(s->buffer, &kw->word))
			return kw->token;
	}
	return _UNSPECIFIED;
}

/**
 * Matches `s->word` against the scanner's token classes in precedence order.
 * Returns the first matching token type, or `_UNSPECIFIED` if no match exists.
 */
static enum TokenType match(const Scanner *s)
{
	assert(s != NULL);

	if (s->consumed != s->buffer.len)
		return _UNSPECIFIED;

	// [spec](https://caddyserver.com/docs/conventions#durations)
	if (is_valid(s, CLS_UNIT_DURATION) && is_duration_unit(s->buffer))
		return CLS_UNIT_DURATION;

	// [spec](https://caddyserver.com/docs/conventions#durations)
	if (is_valid(s, CLS_UNIT_SIZE) && is_size_unit(s->buffer))
		return CLS_UNIT_SIZE;

	enum TokenType token;

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

static inline bool is_word_char(CodePoint c) { return is_alnum(c) || c == '_'; }

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

	if (is_word_char(s->previous) && (s->buffer.len == s->consumed)) {
		append(&s->buffer, s->previous);
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

static bool scan_tag(Scanner *s)
{
	for (size_t i = 0; i < STRING_BUFFER_SIZE && i < s->hdoc_tag.len; i++) {
		if (eof(s) || peek(s) != s->hdoc_tag.s[i])
			return false;
		advance(s);
	}
	return true;
}

static bool inline is_heredoc_char(CodePoint c)
{
	return is_alnum(c) || c == '_' || c == '-';
}

/*
 * Handles lexing the heredoc operator, tag, and content.
 */
static bool scan_heredoc(Scanner *s)
{
	if (is_valid(s, ERROR_SENTINEL))
		return false;

	if (is_valid(s, HEREDOC_CONTENT) && s->hdoc_tag.len != 0) {
		while (!eof(s)) {
			advance_while(s, is_ws);
			mark_end(s);
			if (scan_tag(s)) {
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
		for (unsigned int i = 0; i < s->hdoc_tag.len; i++)
			advance(s);
		if (s->hdoc_tag.len != s->consumed) {
			s->hdoc_tag.len = 0;
			return false;
		}
		reset(&s->hdoc_tag);
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
			CodePoint c = peek(s);
			if (!is_heredoc_char(c))
				break;
			append(&s->hdoc_tag, c);
			advance(s);
		}
		if (s->hdoc_tag.len == 0)
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
	CodePoint prefix = s->previous;

	if (s->consumed == 1 && prefix == '<' && is_valid(s, SYM_CHEVRON_C)) {
		mark_end(s);
		set_result(s, SYM_CHEVRON_C);
		return;
	}

	if (is_valid(s, WS) && is_ws(peek(s))) {
		advance_while(s, is_ws);
		set_result(s, WS);
		mark_end(s);
	}

	skip_while(s, is_ws);

	CodePoint c = peek(s);

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

	bool escape = false;
	bool digits = true;
	bool hex = true;
	bool upper = true;
	bool kw = true;
	int nperiod = 0;

	if (c == '.') {
		advance(s);
		mark_end(s);

		nperiod += 1;
		upper = false;

		CodePoint x = peek(s);
		if (is_valid(s, SYM_DOT_PATH) && (x == '/' || x == '\\')) {
			set_result(s, SYM_DOT_PATH);
			return;
		}
		if (is_valid(s, SYM_DOT_DOT_PATH) && x == '.') {
			advance(s);
			x = peek(s);
			if ((x == '/' || x == '\\')) {
				mark_end(s);
				set_result(s, SYM_DOT_DOT_PATH);
				return;
			}
		}
		if (is_valid(s, SYM_PERIOD)) {
			set_result(s, SYM_PERIOD);
			return;
		}
	}

	enum TokenType token = get_token(c);

	if (token != _UNSPECIFIED && is_valid(s, token)) {
		advance(s);
		mark_end(s);
		set_result(s, token);

		if (token == SYM_QUOTE) {
			s->in_quotation = !s->in_quotation;
			return;
		}

		if (token == SYM_COLON) {
			if (is_valid(s, SYM_COLON_DRIVE) && peek(s) == '\\') {
				set_result(s, SYM_COLON_DRIVE);
				return;
			}
			if (is_valid(s, SYM_SCHEME) && peek(s) == '/') {
				advance(s);
				if (peek(s) == '/') {
					advance(s);
					mark_end(s);
					set_result(s, SYM_SCHEME);
				}
			}
		}
		return;
	}

	switch (c) {
	case '\\':
		if (!is_valid(s, SYM_BSLASH))
			break;
		advance(s);
		mark_end(s);
		c = peek(s);
		escape = is_escapable(c);
		if (escape)
			break;
		set_result(s, SYM_BSLASH);
		return;
	case '*':
		if (!is_valid(s, SYM_ASTERISK))
			break;
		advance(s);
		c = peek(s);
		if (!(is_ws(c) || is_eol(c) || eof(s)))
			break;
		mark_end(s);
		set_result(s, SYM_ASTERISK);
		return;
	}

	while (!eof(s)) {
		/* skip logic upon ESCAPE char */
		if (escape) {
			advance(s);
			mark_end(s);
			escape = false;
			continue;
		}

		c = peek(s);

		if (!is_valid(s, ERROR_SENTINEL) && is_valid(s, STR_COMMENT) &&
		    c != '@' && prefix != '@') {
			advance_while(s, is_not_eol);
			mark_end(s);
			set_result(s, STR_COMMENT);
			return;
		}

		if (!is_valid(s, ERROR_SENTINEL) && c == '\\') {
			if (s->consumed > 1) {
				mark_end(s);
			}
			advance(s);
			if (is_escapable(peek(s))) {
				escape = true;
				continue;
			}
			if (is_valid(s, SYM_BSLASH))
				break;
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

		bool breakpoint = is_eol(c) || (is_ws(c) && !s->in_quotation);
		bool checkpoint = !digits && is_digit(c) && s->consumed <= 2;

		if (is_valid(s, CLS_UNIT_DURATION) && kw && checkpoint &&
		    is_duration_unit(s->buffer)) {
			mark_end(s);
			set_result(s, CLS_UNIT_DURATION);
			return;
		}
		if (kw && breakpoint) {
			enum TokenType keyword = match(s);
			if (keyword != _UNSPECIFIED) {
				mark_end(s);
				set_result(s, keyword);
				return;
			}
		}
		if (breakpoint) {
			s->in_quotation = false;
			break;
		}

		nperiod += (c == '.');

		token = get_token(c);
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
			if (token == SYM_HYPHEN) {
				advance(s);
				continue;
			}
			if (is_unary_operator(c) && is_unary_operator(prefix)) {
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
				StrBuffer buf = {0};
				append(&buf, c);
				advance(s);
				c = peek(s);
				while (!eof(s) && buf.len < 3) {
					if (!is_alpha(c))
						break;
					append(&buf, c);
					advance(s);
					c = peek(s);
				}

				if (!(is_ws(c) || is_eol(c) || is_digit(c))) {
					continue;
				}

				if (!is_unit(buf))
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

	upper = upper && (is_ws(c) || is_eol(c) || eof(s));

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

	// mark_end(s);
	return;
}

/// # Scanner initialisation logic

/*
 * Sets transient field values.
 * These fields do not persist across scanner instances.
 */
static inline void reset_transient_fields(Scanner *s)
{
	s->consumed = 0;
	s->buffer = (StrBuffer){0};
}

/*
 * Serialises a uint32 value to raw bytes.
 */
static inline void ser_u32_le(char *buffer, uint32_t value)
{
	buffer[0] = (char)(value >> 0);
	buffer[1] = (char)(value >> 8);
	buffer[2] = (char)(value >> 16);
	buffer[3] = (char)(value >> 24);
}

/*
 * Deserialises a uint32 value from raw bytes.
 */
static inline uint32_t deser_u32_le(const char *buffer)
{
	return ((uint32_t)(uint8_t)buffer[0] << 0) |
	       ((uint32_t)(uint8_t)buffer[1] << 8) |
	       ((uint32_t)(uint8_t)buffer[2] << 16) |
	       ((uint32_t)(uint8_t)buffer[3] << 24);
}

void *tree_sitter_caddyfile_external_scanner_create(void)
{
	Scanner *s = ts_calloc(1, sizeof(Scanner));
	s->in_quotation = false;
	s->hdoc_tag = (StrBuffer){0};
	s->previous = '\0';
	reset_transient_fields(s);
	return s;
}

void tree_sitter_caddyfile_external_scanner_destroy(void *payload)
{
	ts_free(payload);
}

unsigned tree_sitter_caddyfile_external_scanner_serialize(
    void *payload, char *buffer)
{
	Scanner *s = payload;

	buffer[0] = (char)s->hdoc_tag.len;
	ser_u32_le(buffer + 1, (uint32_t)s->previous);
	buffer[1 + U32_SIZE] = (char)s->in_quotation;

	for (unsigned i = 0; i < s->hdoc_tag.len; i++)
		ser_u32_le(buffer + HEADER_SIZE + i * U32_SIZE,
		    (uint32_t)s->hdoc_tag.s[i]);

	return HEADER_SIZE + s->hdoc_tag.len * U32_SIZE;
}

void tree_sitter_caddyfile_external_scanner_deserialize(
    void *payload, const char *buffer, unsigned length)
{
	Scanner *s = payload;

	reset_transient_fields(s);

	if (length < HEADER_SIZE)
		return;

	s->hdoc_tag.len = (uint8_t)buffer[0];
	s->previous = (CodePoint)deser_u32_le(buffer + 1);
	s->in_quotation = buffer[1 + U32_SIZE] != 0;

	if (s->hdoc_tag.len > STRING_BUFFER_SIZE)
		s->hdoc_tag.len = STRING_BUFFER_SIZE;

	unsigned available = (length - HEADER_SIZE) / U32_SIZE;

	if (s->hdoc_tag.len > available)
		s->hdoc_tag.len = (uint8_t)available;

	for (size_t i = 0; i < s->hdoc_tag.len; i++) {
		s->hdoc_tag.s[i] = (CodePoint)deser_u32_le(
		    buffer + HEADER_SIZE + i * U32_SIZE);
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
