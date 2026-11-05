"""C and Cesta text whose searches ignore layout.

Mutation and source controls name code by its tokens. LayoutText makes count,
find, index, replace, split and `in` match a needle wherever the text differs
from it only in whitespace (including C line splices) at a possible token
boundary, so the controls do not depend on how the code is laid out. A space
between two identifier characters in the needle still requires a space;
identifier characters written adjacently must stay adjacent.

find and index keep exact matching for a needle that begins or ends with
whitespace: tests use those to locate structure, such as a closing brace at
the start of a line. Membership (`in`) ignores layout like count.

Wrap formatted C and Cesta sources only. Pinned Rust and other reference text
keep exact matching.
"""
import re

_GAP = r'(?:\s|\\\n)*'
_SPACE = r'(?:\s|\\\n)+'
_patterns = {}


def _word(ch):
    return ch.isalnum() or ch == '_'


def pattern(needle):
    compiled = _patterns.get(needle)
    if compiled is None:
        parts, previous, spaced = [], None, False
        for ch in needle:
            if ch.isspace():
                spaced = True
                continue
            if previous is not None:
                if _word(previous) and _word(ch):
                    parts.append(_SPACE if spaced else '')
                else:
                    parts.append(_GAP)
            parts.append(re.escape(ch))
            previous, spaced = ch, False
        compiled = _patterns[needle] = re.compile(''.join(parts))
    return compiled


class LayoutText(str):
    def _bounds(self, start, end):
        return start or 0, len(self) if end is None else end

    def count(self, sub, start=None, end=None):
        if not sub.strip():
            return str.count(self, sub, *(x for x in (start, end) if x is not None))
        begin, finish = self._bounds(start, end)
        return sum(1 for _ in pattern(sub).finditer(self, begin, finish))

    def find(self, sub, start=None, end=None):
        if not sub.strip() or sub != sub.strip():
            return str.find(self, sub, *(x for x in (start, end) if x is not None))
        begin, finish = self._bounds(start, end)
        found = pattern(sub).search(self, begin, finish)
        return found.start() if found else -1

    def index(self, sub, start=None, end=None):
        at = self.find(sub, start, end)
        if at < 0:
            raise ValueError('substring not found')
        return at

    def __contains__(self, sub):
        if not sub.strip():
            return str.__contains__(self, sub)
        return pattern(sub).search(self) is not None

    def replace(self, old, new, count=-1):
        if not old.strip():
            return LayoutText(str.replace(self, old, new, count))
        return LayoutText(pattern(old).sub(lambda _: new, self, count=0 if count < 0 else count))

    def split(self, sep=None, maxsplit=-1):
        if sep is None or not sep.strip():
            return str.split(self, sep, maxsplit)
        return pattern(sep).split(self, maxsplit=0 if maxsplit < 0 else maxsplit)

    def __add__(self, other):
        return LayoutText(str.__add__(self, other))

    def __radd__(self, other):
        return LayoutText(str.__add__(other, self))
