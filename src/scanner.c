#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define KEYWORD(text, token) {text, sizeof(text) - 1, token}

enum {
	U32LEN = sizeof(uint32_t),
	HDRLEN = sizeof(uint8_t) + U32LEN,
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
	STR_NUM_DOT,
	STR_CEL,
	STR_CEL_INLINE,
	STR_COMMENT,

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
	KEY_CLIENT_IP,
	KEY_EXPRESSION,
	KEY_VARS,
	KEY_ARGS,
	KEY_NOT,

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

	SYM_BLOCK_START,

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
};

/**
 *	string-to-token map for keywords.
 */
static const Keyword keywords[] = {
    KEYWORD("import", KEY_IMPORT),
    KEYWORD("invoke", KEY_INVOKE),
    KEYWORD("private_ranges", KEY_PRIVATE_RANGES),
    KEYWORD("client_ip", KEY_CLIENT_IP),
    KEYWORD("expression", KEY_EXPRESSION),
    KEYWORD("vars", KEY_VARS),
    KEYWORD("args", KEY_ARGS),
    KEYWORD("not", KEY_NOT),
};

/**
 * Checks @ref keywords entries for a given string.
 *
 * @param *text Unicode character array.
 * @param len Character length.
 * @return Matching keyword token, or NULL
 */
static const Keyword *find_keyword(const UnicodeChar *text, size_t len)
{
	for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
		const Keyword *kw = &keywords[i];

		if (kw->len != len)
			continue;

		bool match = true;

		for (size_t j = 0; j < len; j++)
			if (text[j] != (UnicodeChar)kw->text[j]) {
				match = false;
				break;
			}

		if (match)
			return kw;
	}

	return NULL;
}

static const UnicodeChar import_string[] = {'i', 'm', 'p', 'o', 'r', 't'};

static inline enum TokenType get_token(UnicodeChar c)
{
	if (c >= 128)
		return _UNSPECIFIED;
	return sym_map[c];
}

typedef bool (*Asserter)(UnicodeChar);
typedef struct {
	/* persistent fields */
	uint8_t tag_len;
	UnicodeChar tag[TAGLEN];
	UnicodeChar previous;
	/* transient fields */
	TSLexer *lexer;
	unsigned int consumed;
	uint8_t word_len;
	UnicodeChar word[TAGLEN];
} Scanner;

static enum TokenType check_keyword(Scanner *scanner)
{
	const Keyword *kw = find_keyword(scanner->word, scanner->word_len);
	if (kw == NULL)
		return _UNSPECIFIED;
	return kw->token;
}

static inline bool eof(Scanner *s) { return s->lexer->eof(s->lexer); }

static inline UnicodeChar peek(Scanner *s) { return s->lexer->lookahead; }

static inline UnicodeChar previous(Scanner *s) { return s->previous; }

static inline void mark_end(Scanner *s) { s->lexer->mark_end(s->lexer); }

static inline void set_result(Scanner *s, enum TokenType token)
{
	s->lexer->result_symbol = token;
}

static inline uint32_t get_column(Scanner *s)
{
	return s->lexer->get_column(s->lexer);
}

static inline bool is_num(UnicodeChar c) { return (c >= '0' && c <= '9'); }
static inline bool is_posnum(UnicodeChar c) { return (c >= '1' && c <= '9'); }
static inline bool is_upper(UnicodeChar c) { return (c >= 'A' && c <= 'Z'); }
static inline bool is_lower(UnicodeChar c) { return (c >= 'a' && c <= 'z'); }

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

	if (s->word_len == 0) {
		if (s->consumed < TAGLEN)
			s->word[s->consumed] = s->previous;

		UnicodeChar c = peek(s);
		if (is_eol(c) || is_ws(c))
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

static bool scan_heredoc(Scanner *s, const bool *vs)
{
	if (vs[ERROR_SENTINEL])
		return false;

	uint8_t n;

	if (vs[HEREDOC_CONTENT] && s->tag_len != 0) {
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

	if (vs[HEREDOC_SUFFIX]) {
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

	if (vs[HEREDOC_OPERATOR]) {
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

	if (vs[HEREDOC_TAG]) {
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

static void scan_text(Scanner *s, const bool *vs)
{
	UnicodeChar prefix = previous(s);

	if (vs[WS] && is_ws(peek(s))) {
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

	if (vs[SYM_BLOCK_START] && c == '{') {
		advance(s);
		mark_end(s);
		advance_while(s, is_ws);
		c = peek(s);
		if (eof(s) || is_eol(c) || c == '#') {
			set_result(s, SYM_BLOCK_START);
			return;
		}
		if (vs[SYM_BRACE_O]) {
			set_result(s, SYM_BRACE_O);
			return;
		}
	}

	enum TokenType token = get_token(c);

	if (token != _UNSPECIFIED && vs[token]) {
		set_result(s, token);
		advance(s);
		mark_end(s);
		return;
	}

	bool escape = false;
	bool digits = true;
	bool upper = true;
	bool kw = true;

	while (!eof(s)) {

		/* skip logic upon ESCAPE char */
		if (escape) {
			advance(s);
			mark_end(s);
			escape = false;
			continue;
		}

		UnicodeChar c = peek(s);

		if (vs[STR_COMMENT]) {
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

		if (vs[STR_CEL] && prefix == '`') {
			mark_end(s);
			if (c != '`') {
				advance(s);
				continue;
			}
			set_result(s, STR_CEL);
			return;
		}

		if (!vs[ERROR_SENTINEL] && vs[STR_CEL_INLINE] && c != '`') {
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

		if (is_eol(c) || is_ws(c)) {
			if (kw) {
				enum TokenType token = check_keyword(s);
				if (token != _UNSPECIFIED && vs[token]) {
					mark_end(s);
					set_result(s, token);
					return;
				}
				kw = false;
			}
			break;
		}

		enum TokenType token = get_token(c);
		if (token != _UNSPECIFIED && vs[token]) {
			break;
		}

		if (digits && !is_num(c)) {
			digits = false;
		}

		if (upper && !is_upper(c))
			upper = false;

		advance(s);
		mark_end(s);
	}

	if (escape)
		mark_end(s);

	if (s->consumed == 0)
		return;

	if (vs[STR_WORD] && !vs[STR_CEL] && (eof(s) || peek(s) == '{')) {
		set_result(s, STR_WORD);
		mark_end(s);
		return;
	}

	if (upper && vs[STR_UPPER])
		set_result(s, STR_UPPER);
	else if (digits && vs[STR_NUM_DOT] && peek(s) == '.')
		set_result(s, STR_NUM_DOT);
	else if (digits && vs[STR_NUM])
		set_result(s, STR_NUM);
	else if (!vs[ERROR_SENTINEL] && vs[STR_CEL])
		set_result(s, STR_CEL);
	else if (vs[STR_BARE] && (get_column(s) == 0 || prefix != '}'))
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
	scanner->lexer = lexer;

	if (scan_heredoc(scanner, valid_symbols))
		return scanner->consumed != 0;

	scan_text(scanner, valid_symbols);
	return scanner->consumed != 0;
}
