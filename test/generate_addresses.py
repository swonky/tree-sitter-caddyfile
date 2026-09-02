from itertools import product

SCHEME = "https://"
USERINFO = "user@"
HOST = "subdomain.domain.tld"
PORT = ":port"
PATH = "/path/subpath"
QUERY = "?key=value&foo=bar"
FRAGMENT = "#fragment"


def generate():
    seen = set()

    # filepath forms
    for path, query, fragment in product([False, True], [False, True], [False, True]):
        if not path:
            continue

        value = PATH
        if query:
            value += QUERY
        if fragment:
            value += FRAGMENT

        seen.add(value)

    # URL forms
    for scheme, userinfo, host, port, path, query, fragment in product(
        [False, True],
        [False, True],
        [False, True],
        [False, True],
        [False, True],
        [False, True],
        [False, True],
    ):
        # userinfo and port require a host
        if userinfo and not host:
            continue
        if port and not host:
            continue

        # query/fragment require something before them
        if query and not (host or path):
            continue
        if fragment and not (host or path):
            continue

        # A scheme requires a URL authority.
        if scheme and not host:
            continue

        # No URL components at all.
        if not (scheme or host or path):
            continue

        # Don't duplicate filepath forms.
        if not scheme and not host:
            continue

        value = ""
        if scheme:
            value += SCHEME
        if userinfo:
            value += USERINFO
        if host:
            value += HOST
        if port:
            value += PORT
        if path:
            value += PATH
        if query:
            value += QUERY
        if fragment:
            value += FRAGMENT

        seen.add(value)

    yield from sorted(seen)


if __name__ == "__main__":
    print("example.com")
    for value in generate():
        print("reverse_proxy " + value)
