#include "shell_linen_snapshot.h"
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

void sexos_fail(void);

static void linen_invalid(void) {
    sexos_fail();
    for (;;) {
    }
}

static void linen_shape(const ShellLinenSnapshot *s) {
    if (s->phase > SHELL_LINEN_OPEN || s->slot > SHELL_LINEN_CAPACITY ||
        s->count > SHELL_LINEN_CAPACITY || s->open_count > SHELL_LINEN_OPEN_CAPACITY ||
        s->current.len > 24 || s->offset > s->current.len)
        linen_invalid();
}

ShellLinenSnapshot shell_linen_snapshot_new(void) {
    return (ShellLinenSnapshot){0};
}

void shell_linen_snapshot_refresh(ShellLinenSnapshot *s, bool rescan) {
    linen_shape(s);
    s->refresh = true;
    s->rescan = s->rescan || rescan;
}

bool shell_linen_snapshot_queue_open(ShellLinenSnapshot *s, uint64_t id, uint64_t choice) {
    linen_shape(s);
    if (!id || s->open_count == SHELL_LINEN_OPEN_CAPACITY)
        return false;
    s->opens[s->open_count] = (ShellLinenOpen){true, id, choice};
    ++s->open_count;
    return true;
}

bool shell_linen_snapshot_busy(const ShellLinenSnapshot *s) {
    linen_shape(s);
    return s->phase != SHELL_LINEN_IDLE || s->refresh;
}

bool shell_linen_snapshot_can_send(const ShellLinenSnapshot *s) {
    linen_shape(s);
    return !s->awaiting && (s->phase != SHELL_LINEN_IDLE || s->refresh || s->open_count != 0);
}

bool shell_linen_snapshot_request(ShellLinenSnapshot *s, ShellLinenRequest *out) {
    linen_shape(s);
    if (s->awaiting)
        return false;
    if (s->phase == SHELL_LINEN_IDLE) {
        if (s->open_count) {
            ShellLinenOpen first = s->opens[0];
            if (!first.present)
                linen_invalid();
            for (size_t i = 1; i < s->open_count; ++i)
                s->opens[i - 1] = s->opens[i];
            --s->open_count;
            s->opens[s->open_count] = (ShellLinenOpen){0};
            s->open_id = first.id;
            s->open_choice = first.choice;
            s->phase = SHELL_LINEN_OPEN;
        } else if (s->refresh) {
            s->refresh = false;
            s->slot = 0;
            s->offset = 0;
            s->count = 0;
            for (size_t i = 0; i < SHELL_LINEN_CAPACITY; ++i) {
                s->present[i] = false;
                s->objects[i] = (ShellLinenObject){0};
            }
            s->phase = s->rescan ? SHELL_LINEN_RESCAN : SHELL_LINEN_METADATA;
            s->rescan = false;
        }
    }
    switch (s->phase) {
    case SHELL_LINEN_IDLE:
        return false;
    case SHELL_LINEN_RESCAN:
        *out = (ShellLinenRequest){0x49, {0, 0, 0}};
        return true;
    case SHELL_LINEN_METADATA:
        *out = (ShellLinenRequest){0x44, {s->slot, 0, 0}};
        return true;
    case SHELL_LINEN_NAME:
        *out = (ShellLinenRequest){0x45, {s->current.id, s->offset, 8}};
        return true;
    case SHELL_LINEN_OPEN:
        *out = (ShellLinenRequest){0x46, {s->open_id, s->open_choice, 0}};
        return true;
    default:
        linen_invalid();
        return false;
    }
}

void shell_linen_snapshot_sent(ShellLinenSnapshot *s) {
    linen_shape(s);
    if (s->phase == SHELL_LINEN_IDLE || s->awaiting)
        linen_invalid();
    s->awaiting = true;
}

static ShellLinenReply linen_failed(ShellLinenSnapshot *s) {
    s->phase = SHELL_LINEN_IDLE;
    return (ShellLinenReply){SHELL_LINEN_REJECTED, 0};
}

static ShellLinenReply linen_next_slot(ShellLinenSnapshot *s) {
    if (s->slot >= SHELL_LINEN_CAPACITY) {
        linen_invalid();
    }
    ++s->slot;
    if (s->slot == SHELL_LINEN_CAPACITY) {
        s->phase = SHELL_LINEN_IDLE;
        return (ShellLinenReply){SHELL_LINEN_PUBLISHED, 0};
    }
    s->phase = SHELL_LINEN_METADATA;
    return (ShellLinenReply){SHELL_LINEN_PROGRESS, 0};
}

static ShellLinenReply linen_push_current(ShellLinenSnapshot *s) {
    if (s->count >= SHELL_LINEN_CAPACITY)
        linen_invalid();
    s->objects[s->count] = s->current;
    s->present[s->count] = true;
    ++s->count;
    return linen_next_slot(s);
}

ShellLinenReply shell_linen_snapshot_reply(ShellLinenSnapshot *s, uint32_t caller, uint64_t opcode,
                                           uint64_t value) {
    linen_shape(s);
    if (caller != 7 || opcode != 1 || !s->awaiting)
        return (ShellLinenReply){SHELL_LINEN_IGNORED, 0};
    s->awaiting = false;
    switch (s->phase) {
    case SHELL_LINEN_IDLE:
        return (ShellLinenReply){SHELL_LINEN_IGNORED, 0};
    case SHELL_LINEN_OPEN:
        s->phase = SHELL_LINEN_IDLE;
        return (ShellLinenReply){SHELL_LINEN_OPEN_COMPLETE, value};
    case SHELL_LINEN_RESCAN:
        if (value >> 63)
            return linen_failed(s);
        s->phase = SHELL_LINEN_METADATA;
        return (ShellLinenReply){SHELL_LINEN_PROGRESS, 0};
    case SHELL_LINEN_METADATA: {
        if (!value)
            return linen_next_slot(s);
        uint64_t id = value & UINT64_C(0xffffffff);
        uint8_t kind = (uint8_t)(value >> 32), len = (uint8_t)(value >> 40);
        bool duplicate = false;
        for (size_t i = 0; i < SHELL_LINEN_CAPACITY; ++i)
            if (s->present[i] && s->objects[i].id == id)
                duplicate = true;
        if (!id || kind > 2 || len > 24 || duplicate)
            return linen_failed(s);
        s->current =
            (ShellLinenObject){.id = id, .kind = kind, .len = len, .disk = (uint8_t)(value >> 48)};
        s->offset = 0;
        if (!len)
            return linen_push_current(s);
        s->phase = SHELL_LINEN_NAME;
        return (ShellLinenReply){SHELL_LINEN_PROGRESS, 0};
    }
    case SHELL_LINEN_NAME: {
        if (!value || value >> 63)
            return linen_failed(s);
        size_t take = s->current.len - s->offset;
        if (take > 8)
            take = 8;
        for (size_t i = 0; i < take; ++i) {
            uint8_t b = (uint8_t)(value >> (i * 8));
            s->current.name[s->offset + i] = (b >= 0x20 && b <= 0x7e) ? b : '?';
        }
        s->offset = (uint8_t)(s->offset + take);
        if (s->offset == s->current.len)
            return linen_push_current(s);
        return (ShellLinenReply){SHELL_LINEN_PROGRESS, 0};
    }
    default:
        linen_invalid();
        return (ShellLinenReply){SHELL_LINEN_IGNORED, 0};
    }
}

bool shell_workspace_allows(uint64_t subject, uint8_t operation, uint64_t requested,
                            uint64_t published_id, uint8_t disk_path, bool published, bool active,
                            bool explicit_denial) {
    return active && !explicit_denial && published && subject == 200 &&
           (operation == 0 || operation == 6) && requested != 0 && requested != UINT64_MAX &&
           requested == published_id && disk_path >= 3 && disk_path <= 15;
}


