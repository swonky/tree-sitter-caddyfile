package main

import (
	"errors"
	"os"
	"strings"
	"testing"

	"github.com/caddyserver/caddy/v2/caddyconfig/caddyfile"
	ts_caddyfile "github.com/swonky/tree-sitter-caddyfile/bindings/go"
	sitter "github.com/tree-sitter/go-tree-sitter"
)

func applyQuery(t *testing.T, queryPath string, document []byte) []string {
	t.Helper()

	querySource, err := os.ReadFile(queryPath)
	if err != nil {
		t.Fatalf("read query: %v", err)
	}

	parser := sitter.NewParser()
	defer parser.Close()

	language := sitter.NewLanguage(ts_caddyfile.Language())
	if err := parser.SetLanguage(language); err != nil {
		t.Fatalf("set language: %v", err)
	}

	tree := parser.Parse(document, nil)
	defer tree.Close()

	query, qerr := sitter.NewQuery(language, string(querySource))
	if qerr != nil {
		t.Fatalf("compile query: %v", qerr)
	}
	defer query.Close()

	cursor := sitter.NewQueryCursor()
	defer cursor.Close()

	captures := cursor.Captures(
		query,
		tree.RootNode(),
		document,
	)

	result := make([]string, 0)

	for match, index := captures.Next(); match != nil; match, index = captures.Next() {
		node := match.Captures[index].Node
		text := node.Utf8Text(document)

		if strings.Contains(text, "\n") {
			continue
		}

		result = append(result, text)
	}

	return result
}

func tokenize(document []byte) ([]string, error) {
	tokens, err := caddyfile.Tokenize(document, "parser_test/Caddyfile")
	if err != nil {
		return nil, err
	}

	result := make([]string, 0, len(tokens))

	for _, token := range tokens {
		if len(token.Text) == 1 && (token.Text[0] == '{' || token.Text[0] == '}') {
			continue
		}
		if strings.Contains(token.Text, "\n") {
			continue
		}

		result = append(result, token.Text)
	}

	return result, nil
}

func compare(s1, s2 string) (bool, error) {
	if s1 == "" {
		return false, errors.New("first string is empty")
	}
	if s2 == "" {
		return false, errors.New("second string is empty")
	}

	return s1 == s2 ||
		strings.TrimSuffix(s1, ",") == strings.TrimSuffix(s2, ","), nil
}

func TestTokens(t *testing.T) {
	input, err := os.ReadFile("parser_test/Caddyfile")
	if err != nil {
		t.Fatalf("read Caddyfile: %v", err)
	}

	cfTokens, err := tokenize(input)
	if err != nil {
		t.Fatalf("tokenize Caddyfile: %v", err)
	}

	tsTokens := applyQuery(t, "parser_test/tokens.scm", input)

	length := min(len(cfTokens), len(tsTokens))
	if length == 0 {
		t.Fatal("no tokens to compare")
	}

	for i := range length {
		t1 := cfTokens[i]
		t2 := tsTokens[i]

		same, err := compare(t1, t2)
		if err != nil {
			t.Errorf("token %d: %v", i, err)
			continue
		}

		if !same {
			t.Errorf("token %d: expected %q, got %q", i, t1, t2)
		}
	}

	if len(cfTokens) != len(tsTokens) {
		t.Errorf(
			"token count mismatch: Caddyfile=%d, tree-sitter=%d",
			len(cfTokens),
			len(tsTokens),
		)
	}
}
