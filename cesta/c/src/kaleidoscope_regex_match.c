#include "kaleidoscope_regex.h"
#ifndef CESTA_OWNERSHIP_ABI_PROFILE
#define CESTA_OWNERSHIP_ABI_PROFILE 1
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#endif

#define RE_STEPS UINT32_C(200000)
#define RE_STACK_FRAMES 512u
#define RE_LOOPS 32u

typedef struct ReFrame {
    uint8_t kind;
    uint16_t x;
    size_t y;
    bool present;
} ReFrame;

typedef struct ReStack {
    ReFrame frames[RE_STACK_FRAMES];
    size_t count;
} ReStack;

enum {
    FRAME_TRY = 1,
    FRAME_RESTORE = 2,
    FRAME_UNMARK = 3
};

static bool re_match_word(uint8_t b) {
    return (b >= 'A' && b <= 'Z') || (b >= 'a' && b <= 'z') || (b >= '0' && b <= '9') || b == '_';
}

static uint8_t re_match_lower(uint8_t b) {
    return b >= 'A' && b <= 'Z' ? (uint8_t)(b + ('a' - 'A')) : b;
}

static bool re_push(ReStack *stack, ReFrame frame) {
    if (stack->count >= RE_STACK_FRAMES)
        return false;
    stack->frames[stack->count++] = frame;
    return true;
}

static bool re_class_has(const KaleidoscopeRegex *regex, uint16_t id, uint8_t byte) {
    return id < regex->class_count && ((regex->classes[id][byte / 64] >> (byte % 64)) & 1u) != 0;
}

static uint8_t re_run(const KaleidoscopeRegex *regex, const KaleidoscopeRegexText *text,
                      size_t start, uint32_t *steps, KaleidoscopeRegexFind *result) {
    ReStack stack = {0};
    bool slot_present[KALEIDOSCOPE_REGEX_CAPTURES * 2] = {0};
    size_t slots[KALEIDOSCOPE_REGEX_CAPTURES * 2] = {0};
    size_t marks[RE_LOOPS];
    for (size_t i = 0; i < RE_LOOPS; i++)
        marks[i] = SIZE_MAX;
    if (!re_push(&stack, (ReFrame){FRAME_TRY, 0, start, false}))
        return KALEIDOSCOPE_REGEX_LIMIT;
    while (stack.count) {
        ReFrame frame = stack.frames[--stack.count];
        if (frame.kind == FRAME_RESTORE) {
            slot_present[frame.x] = frame.present;
            slots[frame.x] = frame.y;
            continue;
        }
        if (frame.kind == FRAME_UNMARK) {
            marks[frame.x] = frame.y;
            continue;
        }
        size_t pc = frame.x, pos = frame.y;
        for (;;) {
            if (++*steps > RE_STEPS)
                return KALEIDOSCOPE_REGEX_LIMIT;
            if (pc >= regex->program_len)
                return KALEIDOSCOPE_REGEX_SYNTAX;
            KaleidoscopeRegexInst in = regex->program[pc];
            if (in.kind == KALEIDOSCOPE_RE_BYTE) {
                if (pos < text->len &&
                    (regex->icase ? re_match_lower(text->bytes[pos]) == re_match_lower(in.byte)
                                  : text->bytes[pos] == in.byte)) {
                    pc++;
                    pos++;
                    continue;
                }
                break;
            }
            if (in.kind == KALEIDOSCOPE_RE_ANY) {
                if (pos < text->len && text->bytes[pos] != '\n') {
                    pc++;
                    pos++;
                    continue;
                }
                break;
            }
            if (in.kind == KALEIDOSCOPE_RE_CLASS) {
                if (pos < text->len && re_class_has(regex, in.a, text->bytes[pos])) {
                    pc++;
                    pos++;
                    continue;
                }
                break;
            }
            if (in.kind == KALEIDOSCOPE_RE_START) {
                if (pos == 0) {
                    pc++;
                    continue;
                }
                break;
            }
            if (in.kind == KALEIDOSCOPE_RE_END) {
                if (pos == text->len) {
                    pc++;
                    continue;
                }
                break;
            }
            if (in.kind == KALEIDOSCOPE_RE_BOUNDARY) {
                bool before = pos > 0 && re_match_word(text->bytes[pos - 1]),
                     after = pos < text->len && re_match_word(text->bytes[pos]);
                if ((before != after) == (in.byte != 0)) {
                    pc++;
                    continue;
                }
                break;
            }
            if (in.kind == KALEIDOSCOPE_RE_SPLIT) {
                if (!re_push(&stack, (ReFrame){FRAME_TRY, in.b, pos, false}))
                    return KALEIDOSCOPE_REGEX_LIMIT;
                pc = in.a;
                continue;
            }
            if (in.kind == KALEIDOSCOPE_RE_JMP) {
                pc = in.a;
                continue;
            }
            if (in.kind == KALEIDOSCOPE_RE_SAVE) {
                if (in.a >= KALEIDOSCOPE_REGEX_CAPTURES * 2 ||
                    !re_push(&stack,
                             (ReFrame){FRAME_RESTORE, in.a, slots[in.a], slot_present[in.a]}))
                    return KALEIDOSCOPE_REGEX_LIMIT;
                slot_present[in.a] = true;
                slots[in.a] = pos;
                pc++;
                continue;
            }
            if (in.kind == KALEIDOSCOPE_RE_MARK) {
                if (in.a >= RE_LOOPS ||
                    !re_push(&stack, (ReFrame){FRAME_UNMARK, in.a, marks[in.a], false}))
                    return KALEIDOSCOPE_REGEX_LIMIT;
                marks[in.a] = pos;
                pc++;
                continue;
            }
            if (in.kind == KALEIDOSCOPE_RE_PROGRESS) {
                if (in.a < RE_LOOPS && pos != marks[in.a]) {
                    pc++;
                    continue;
                }
                break;
            }
            if (in.kind == KALEIDOSCOPE_RE_MATCH) {
                for (size_t n = 0; n <= regex->groups && n < KALEIDOSCOPE_REGEX_CAPTURES; n++)
                    if (slot_present[n * 2] && slot_present[n * 2 + 1])
                        result->captures[n] =
                            (KaleidoscopeRegexCapture){true, slots[n * 2], slots[n * 2 + 1]};
                result->matched = true;
                return KALEIDOSCOPE_REGEX_OK;
            }
            return KALEIDOSCOPE_REGEX_SYNTAX;
        }
    }
    return KALEIDOSCOPE_REGEX_OK;
}

KaleidoscopeRegexFind kaleidoscope_regex_find(const KaleidoscopeRegex *regex,
                                              const KaleidoscopeRegexText *text, size_t from) {
    KaleidoscopeRegexFind result = {0};
    if (!regex || !text || !regex->valid || text->len > KALEIDOSCOPE_REGEX_MAX_TEXT) {
        result.error = KALEIDOSCOPE_REGEX_SYNTAX;
        return result;
    }
    result.groups = regex->groups;
    uint32_t steps = 0;
    if (from > text->len)
        return result;
    for (size_t start = from; start <= text->len; start++) {
        uint8_t error = re_run(regex, text, start, &steps, &result);
        if (error) {
            result.error = error;
            return result;
        }
        if (result.matched)
            return result;
    }
    return result;
}


