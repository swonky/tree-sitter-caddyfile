#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define KEYWORD(text, token) {text, sizeof(text) - 1, token}
#define UNIT(text) KEYWORD(text, STR_DUR_UNIT)

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
	STR_IPV4,
	STR_CEL,
	STR_CEL_INLINE,
	STR_COMMENT,
	STR_DUR_INTEGER,
	STR_DUR_DECIMAL,
	STR_DUR_UNIT,

	/*
	 * whitespace
	 */
	EOL,
	WS,

	/*
	 * keywords
	 */
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
	KEY_PATH_REGEXP,
	KEY_HOST_REGEXP,
	KEY_HEADER_REGEXP,
	KEY_COOKIE_REGEXP,
	KEY_VARS_REGEXP,

	/*
	 * symbolic operators
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
	SYM_ASTERISK,
	SYM_EXCLAIM,
	SYM_QUESTION,

	SYM_BLOCK_START,
	SYM_SCHEME,

	/*
	 * indicates that tree-sitter
	 * is in error recovery mode
	 */
	ERROR_SENTINEL,
};

/**
 *	unicode character
 */
typedef int32_t UnicodeChar;

/**
 *	keyword entry.
 *	use @ref KEYWORD() macro to initialise.
 */
typedef struct {
	const char *text;
	uint8_t len;
	enum TokenType token;
} Keyword;

/**
 *	character-to-token map for ASCII symbolic operators.
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
    ['*'] = SYM_ASTERISK,
    ['!'] = SYM_EXCLAIM,
    ['?'] = SYM_QUESTION,
};

/**
 *	string-to-token map for keywords.
 */
static const Keyword keywords[] = {
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
    KEYWORD("path_regexp", KEY_PATH_REGEXP),
    KEYWORD("host_regexp", KEY_HOST_REGEXP),
    KEYWORD("header_regexp", KEY_HEADER_REGEXP),
    KEYWORD("cookie_regexp", KEY_COOKIE_REGEXP),
    KEYWORD("vars_regexp", KEY_VARS_REGEXP),
};

static inline enum TokenType get_token(UnicodeChar c)
{
	if (c >= 128)
		return _UNSPECIFIED;

	return sym_map[c];
}

typedef bool (*Asserter)(UnicodeChar);

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

static inline bool is_valid(Scanner *s, enum TokenType token)
{
	assert(s != NULL);

	return s->vs != NULL && s->vs[token];
}

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

static bool is_unit(const UnicodeChar *kw, uint8_t len)
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

static enum TokenType check_keyword(Scanner *s)
{
	assert(s != NULL);

	if (is_valid(s, STR_DUR_UNIT) && is_unit(s->word, s->word_len))
		return STR_DUR_UNIT;

	for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
		const Keyword *kw = &keywords[i];

		if (is_valid(s, kw->token) && word_equals(s, kw))
			return kw->token;
	}

	return _UNSPECIFIED;
}

static inline bool eof(Scanner *s) { return s->lexer->eof(s->lexer); }
static inline UnicodeChar peek(Scanner *s) { return s->lexer->lookahead; }
static inline UnicodeChar previous(Scanner *s) { return s->previous; }
static inline void mark_end(Scanner *s) { s->lexer->mark_end(s->lexer); }

static inline void set_result(Scanner *s, enum TokenType token)
{
	assert(s != NULL);
	s->lexer->result_symbol = token;
}

static inline uint32_t get_column(Scanner *s)
{
	assert(s != NULL);
	return s->lexer->get_column(s->lexer);
}

static inline bool between(UnicodeChar x, UnicodeChar lo, UnicodeChar hi)
{
	return (x >= lo && x <= hi);
}
static inline bool is_num(UnicodeChar c) { return between(c, '0', '9'); }
static inline bool is_posnum(UnicodeChar c) { return between(c, '1', '9'); }
static inline bool is_upper(UnicodeChar c) { return between(c, 'A', 'Z'); }
static inline bool is_lower(UnicodeChar c) { return between(c, 'a', 'z'); }

static inline bool is_alpha(UnicodeChar c)
{
	return is_upper(c) || is_lower(c);
}
static inline bool is_alnum(UnicodeChar c) { return is_num(c) || is_alpha(c); }

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

static inline void advance(Scanner *s)
{
	if (eof(s))
		return;

	s->previous = peek(s);
	s->lexer->advance(s->lexer, false);

	if (s->word_len == 0 && s->consumed < TAGLEN) {
		s->word[s->consumed] = s->previous;

		UnicodeChar c = peek(s);
		if (!is_alpha(c) && c != '_')
			s->word_len = s->consumed + 1;
	}

	s->consumed++;
}

static inline void skip(Scanner *s)
{
	if (eof(s))
		return;

	s->previous = peek(s);
	s->lexer->advance(s->lexer, true);
}

static inline void skip_while(Scanner *s, Asserter fn)
{
	while (!eof(s) && fn(peek(s))) {
		skip(s);
	}
}

static inline void advance_while(Scanner *s, Asserter fn)
{
	while (!eof(s) && fn(peek(s))) {
		advance(s);
	}
}

/*
 * Consume the rest of the current line, including terminating EOL characters.
 */
static inline void advance_rol(Scanner *s)
{
	while (!eof(s) && !is_eol(peek(s)))
		advance(s);

	if (eof(s))
		return;

	advance(s);

	if (previous(s) == '\r' && !eof(s) && peek(s) == '\n')
		advance(s);
}
static inline bool is_error(Scanner *s) { return is_valid(s, ERROR_SENTINEL); }

static bool scan_heredoc(Scanner *s)
{
	if (is_error(s))
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

static void scan_text(Scanner *s)
{
	UnicodeChar prefix = previous(s);

	if (is_valid(s, WS) && is_ws(peek(s))) {
		advance_while(s, is_ws);
		set_result(s, WS);
		mark_end(s);
		return;
	}

	skip_while(s, is_ws);

	if (is_eol(peek(s))) {
		advance(s);
		set_result(s, EOL);
		mark_end(s);
		return;
	}

	UnicodeChar c = peek(s);

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

	bool escape = false;
	bool digits = true;
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
			mark_end(s);
			if (!is_eol(c)) {
				advance(s);
				continue;
			}
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
				enum TokenType token = check_keyword(s);
				if (token != _UNSPECIFIED) {
					mark_end(s);
					set_result(s, token);
					return;
				}
				kw = false;
			}
			s->in_quotation = false;
			break;
		}

		nperiod += (c == '.');

		enum TokenType token = get_token(c);
		if (token != _UNSPECIFIED && is_valid(s, token)) {
			if (kw) {
				enum TokenType token = check_keyword(s);
				if (token != _UNSPECIFIED) {
					if (token == SYM_PERIOD && digits &&
					    (is_valid(s, STR_DECIMAL) ||
						is_valid(s, STR_IPV4)))
						break;
					mark_end(s);
					set_result(s, token);
					return;
				}
				kw = false;
			}
			mark_end(s);
			break;
		}

		if (upper && !is_upper(c))
			upper = false;

		if (digits && !is_num(c) && c != '.') {
			mark_end(s);
			digits = false;
			if (s->consumed > 0 && nperiod <= 1 &&
			    is_valid(s, STR_DUR_INTEGER) &&
			    is_valid(s, STR_DUR_DECIMAL)) {
				advance(s);

				UnicodeChar next = peek(s);
				if (next == c) {
					continue;
				}

				UnicodeChar suffix[2] = {c};
				uint8_t len = 1;
				if (!(is_ws(next) || is_eol(next))) {
					suffix[1] = next;
					len = 2;
				}

				if (!is_unit(suffix, len))
					continue;

				if (len == 2) {
					advance(s);
					next = peek(s);
					if (!(is_ws(next) || is_eol(next)))
						continue;
				}

				// advance(s);
				// mark_end(s);

				switch (nperiod) {
				case 0:
					set_result(s, STR_DUR_INTEGER);
					return;
				case 1:
					set_result(s, STR_DUR_DECIMAL);
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

	if (upper && is_valid(s, STR_UPPER))
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

/*
 * Initialises persistent field values.
 * Modifications to these values persist across scanner instances.
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
