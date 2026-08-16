#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef int32_t UnicodeChar;

static const uint8_t MAX_LEN = 64;

enum {
	U32LEN = sizeof(uint32_t),
	HDRLEN = sizeof(uint8_t) + U32LEN,
};

enum TokenType {
	_UNSPECIFIED,
	// heredoc
	HEREDOC_OPERATOR,
	HEREDOC_TAG,
	HEREDOC_CONTENT,
	HEREDOC_SUFFIX,
	// plain text content
	STR_WORD,
	STR_BARE,
	STR_UPPER,
	STR_NUM,
	STR_CEL,
	// whitespace
	EOL,
	WS,
	COMMENT,
	// keywords
	KEY_IMPORT,
	// symbolic operators
	SYM_PAREN_O,
	SYM_PAREN_C,
	SYM_BRACE_O,
	SYM_BRACE_C,
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
	// error recovery indicator
	ERROR_SENTINEL,
};

static const UnicodeChar sym_map[128] = {
    ['('] = SYM_PAREN_O,
    [')'] = SYM_PAREN_C,
    ['{'] = SYM_BRACE_O,
    ['}'] = SYM_BRACE_C,
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
};

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
	UnicodeChar tag[64];
	UnicodeChar previous;
	/* transient fields */
	TSLexer *lexer;
	unsigned int consumed;
} Scanner;

static inline bool eof(Scanner *s) { return s->lexer->eof(s->lexer); }

static inline UnicodeChar peek(Scanner *s) { return s->lexer->lookahead; }

static inline void advance(Scanner *s)
{
	if (eof(s))
		return;

	s->previous = peek(s);
	s->lexer->advance(s->lexer, false);
	s->consumed++;
}

static inline void skip(Scanner *s)
{
	if (eof(s))
		return;

	s->previous = peek(s);
	s->lexer->advance(s->lexer, true);
}

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

// consume the rest of the line (incl. newline characters)
static inline void advance_rol(Scanner *s)
{
	while (!eof(s) && get_column(s) != 0) {
		advance(s);
	}
}

static inline void submit_heredoc_content(Scanner *s)
{
	advance_rol(s);
	mark_end(s);
	set_result(s, HEREDOC_CONTENT);
}

static bool scan_heredoc(TSLexer *lex, Scanner *s, const bool *vs)
{
	if (s->tag_len != 0) {
		advance_while(s, is_ws);
		uint8_t nchar = 0;
		for (; nchar < MAX_LEN && nchar < s->tag_len; nchar++) {
			if (peek(s) != s->tag[nchar])
				break;
			if (is_eol(peek(s))) {
				submit_heredoc_content(s);
				return true;
			}
			advance(s);
		}
		if (nchar == s->tag_len) {
			advance(s);
			mark_end(s);
			lex->result_symbol = HEREDOC_SUFFIX;
			s->tag_len = 0;
			return true;
		}
		submit_heredoc_content(s);
		return true;
	}

	if (vs[HEREDOC_OPERATOR]) {
		if (peek(s) != '>')
			return false;
		advance(s);
		if (peek(s) != '>')
			return false;
		advance(s);
		if (!(is_alpha(peek(s)) || is_num(peek(s))))
			return false;
		mark_end(s);
		lex->result_symbol = HEREDOC_OPERATOR;
		return true;
	}

	if (vs[HEREDOC_TAG]) {
		while (!eof(s)) {
			UnicodeChar c = peek(s);
			if (is_ws(c) || is_eol(c) || c == '#')
				break;
			s->tag[s->tag_len] = c;
			s->tag_len++;
			advance(s);
		}
		if (s->tag_len == 0)
			return false;
		mark_end(s);
		lex->result_symbol = HEREDOC_TAG;
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

	enum TokenType token = get_token(peek(s));

	if (token != _UNSPECIFIED && vs[token]) {
		set_result(s, token);
		advance(s);
		mark_end(s);
		return;
	}

	bool escape = false;
	bool digits = true;
	bool upper = true;
	bool kw_import = true;

	while (!eof(s)) {

		/* skip logic upon ESCAPE char */
		if (escape) {
			advance(s);
			mark_end(s);
			escape = false;
			continue;
		}

		UnicodeChar c = peek(s);
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

		if (vs[KEY_IMPORT] && kw_import) {
			if (s->consumed <= 5 && c != import_string[s->consumed])
				kw_import = false;
			if (s->consumed == 5 && kw_import) {
				advance(s);
				mark_end(s);
				c = peek(s);
				if (is_ws(c)) {
					set_result(s, KEY_IMPORT);
					return;
				}
				kw_import = false;
			}
		}

		if (is_eol(c) || is_ws(c)) {
			break;
		}

		if (!is_num(c))
			digits = false;

		if (!is_upper(c))
			upper = false;

		enum TokenType token = get_token(c);
		if (token != _UNSPECIFIED && vs[token]) {
			break;
		}

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
static inline void reset_transient_fields(Scanner *s) { s->consumed = 0; }

void *tree_sitter_caddyfile_external_scanner_create(void)
{
	Scanner *s = ts_calloc(1, sizeof(Scanner));
	init_persistent_fields(s);
	reset_transient_fields(s);
	return s;
}

void tree_sitter_caddyfile_external_scanner_destroy(void *payload)
{
	free(payload);
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

	if (s->tag_len > MAX_LEN)
		s->tag_len = MAX_LEN;

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
	// if (scan_heredoc(lex, s, vs))
	// 	return true;
	scan_text(scanner, valid_symbols);

	return scanner->consumed != 0;
}
