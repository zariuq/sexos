#include "sexos_app_private.h"
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

uint64_t app_lifecycle_authorize(AppLifecycleGrant g, uint32_t holder, uint32_t target,
                                 uint32_t task, uint8_t right) {
    if (g.holder_pd != holder || g.target_pd != target || g.task_id != task ||
        (g.rights & right) != right)
        return APP_PRIVATE_DENIED;
    if (right == APP_PRIVATE_ACK &&
        (!g.ack_bit || (g.ack_bit & ~APP_PRIVATE_REQUIRED) || (g.ack_bit & (g.ack_bit - 1))))
        return APP_PRIVATE_DENIED;
    return APP_PRIVATE_OK;
}

uint64_t app_private_final_flags(uint64_t flags, uint8_t key) {
    /* Invalid hardware keys are not accepted by the caller; retain fail-closed
     * behavior at this boundary as well. Valid inputs match final_elf_flags. */
    if (key >= 16)
        return 0;
    return (flags & ~(UINT64_C(15) << 59)) | ((uint64_t)key << 59);
}

AppPrivateResult app_private_retire(const AppPrivatePages *p) {
    if (!p->count || p->count > APP_PRIVATE_MAX_PAGES)
        return (AppPrivateResult){.status = APP_PRIVATE_INVALID};
    for (size_t i = 0; i < p->count; ++i) {
        AppPrivatePage page = p->pages[i];
        if (!page.va || page.va % 4096 || !page.leaf || (page.expected & 5) != 5 ||
            (app_private_leaf_read(page.leaf) & ~UINT64_C(0x60)) !=
                (page.expected & ~UINT64_C(0x60)))
            return (AppPrivateResult){.status = APP_PRIVATE_DENIED};
        for (size_t j = 0; j < i; ++j) {
            AppPrivatePage old = p->pages[j];
            if (old.va == page.va || old.leaf == page.leaf ||
                (old.expected & APP_PRIVATE_PHYSICAL) == (page.expected & APP_PRIVATE_PHYSICAL))
                return (AppPrivateResult){.status = APP_PRIVATE_DENIED};
        }
    }
    for (size_t i = 0; i < p->count; ++i) {
        app_private_leaf_write(p->pages[i].leaf, 0);
        app_private_leaf_flush(p->pages[i].va);
    }
    return (AppPrivateResult){.count = p->count};
}

AppPrivateResult app_private_restore_check(const AppPrivatePages *p) {
    if (!p->count || p->count > APP_PRIVATE_MAX_PAGES)
        return (AppPrivateResult){.status = APP_PRIVATE_INVALID};
    for (size_t i = 0; i < p->count; ++i) {
        AppPrivatePage page = p->pages[i];
        if (!page.va || page.va % 4096 || !page.leaf || (page.expected & 5) != 5 ||
            app_private_leaf_read(page.leaf))
            return (AppPrivateResult){.status = APP_PRIVATE_DENIED};
        for (size_t j = 0; j < i; ++j) {
            AppPrivatePage old = p->pages[j];
            if (old.va == page.va || old.leaf == page.leaf ||
                (old.expected & APP_PRIVATE_PHYSICAL) == (page.expected & APP_PRIVATE_PHYSICAL))
                return (AppPrivateResult){.status = APP_PRIVATE_DENIED};
        }
    }
    return (AppPrivateResult){.count = p->count};
}

AppPrivateResult app_private_restore(const AppPrivatePages *p) {
    AppPrivateResult checked = app_private_restore_check(p);
    if (checked.status)
        return checked;
    for (size_t i = 0; i < p->count; ++i) {
        app_private_leaf_write(p->pages[i].leaf, p->pages[i].expected);
        app_private_leaf_flush(p->pages[i].va);
    }
    return (AppPrivateResult){.count = p->count};
}

AppPrivateResult app_private_handover_check(const AppPrivatePages *p,
                                            const AppPrivatePages *workspace) {
    if (!p->count || p->count > APP_PRIVATE_MAX_PAGES || p->count != workspace->count)
        return (AppPrivateResult){.status = APP_PRIVATE_INVALID};
    for (size_t i = 0; i < p->count; ++i) {
        AppPrivatePage old = p->pages[i], next = workspace->pages[i];
        if (!old.va || old.va % 4096 || !old.leaf || (old.expected & 5) != 5 || !next.va ||
            next.va % 4096 || !next.leaf || (next.expected & 7) != 7 ||
            !(next.expected & (UINT64_C(1) << 63)) ||
            (old.expected & APP_PRIVATE_PHYSICAL) != (next.expected & APP_PRIVATE_PHYSICAL) ||
            (app_private_leaf_read(old.leaf) & ~UINT64_C(0x60)) !=
                (old.expected & ~UINT64_C(0x60)) ||
            app_private_leaf_read(next.leaf))
            return (AppPrivateResult){.status = APP_PRIVATE_DENIED};
        for (size_t j = 0; j < p->count; ++j)
            if (p->pages[j].va == next.va || p->pages[j].leaf == next.leaf)
                return (AppPrivateResult){.status = APP_PRIVATE_DENIED};
        for (size_t j = 0; j < i; ++j) {
            AppPrivatePage a = p->pages[j], b = workspace->pages[j];
            if (a.va == old.va || a.leaf == old.leaf || b.va == next.va || b.leaf == next.leaf ||
                (a.expected & APP_PRIVATE_PHYSICAL) == (old.expected & APP_PRIVATE_PHYSICAL))
                return (AppPrivateResult){.status = APP_PRIVATE_DENIED};
        }
    }
    return (AppPrivateResult){.count = p->count};
}

AppPrivateResult app_private_handover(const AppPrivatePages *p, const AppPrivatePages *workspace) {
    AppPrivateResult checked = app_private_handover_check(p, workspace);
    if (checked.status)
        return checked;
    for (size_t i = 0; i < p->count; ++i) {
        app_private_leaf_write(p->pages[i].leaf, 0);
        app_private_leaf_flush(p->pages[i].va);
    }
    for (size_t i = 0; i < workspace->count; ++i) {
        app_private_leaf_write(workspace->pages[i].leaf, workspace->pages[i].expected);
        app_private_leaf_flush(workspace->pages[i].va);
    }
    return (AppPrivateResult){.count = p->count};
}

bool app_notice_due(uint8_t acked, uint8_t hinted, uint64_t last, uint64_t now, uint8_t recipient) {
    if (recipient >= 6 || (recipient < 5 && (acked & (1u << recipient))))
        return false;
    return !(hinted & (1u << recipient)) || now - last >= 16;
}

AppIncarnation app_incarnation_original(uint32_t target) {
    return (AppIncarnation){target, 1};
}

AppNextIncarnation app_incarnation_next(AppIncarnation old) {
    if (!old.logical_app || !old.number || old.number == UINT32_MAX)
        return (AppNextIncarnation){0};
    return (AppNextIncarnation){true, {old.logical_app, old.number + 1}};
}

uint64_t app_activation_authorize(AppActivationGrant g, uint32_t holder, uint32_t predecessor,
                                  uint32_t predecessor_task, uint32_t successor,
                                  uint32_t successor_task, AppIncarnation old,
                                  AppIncarnation next) {
    AppNextIncarnation expected = app_incarnation_next(old);
    if (g.holder != holder || !holder || holder == predecessor || holder == successor ||
        !g.logical_app || old.logical_app != g.logical_app || next.logical_app != g.logical_app ||
        !expected.present || expected.value.logical_app != next.logical_app ||
        expected.value.number != next.number || g.incarnation != next.number ||
        ((old.number == 1) != (predecessor == g.logical_app)) || successor == g.logical_app ||
        !predecessor || !successor || predecessor == successor || !predecessor_task ||
        !successor_task || predecessor_task == successor_task || g.predecessor != predecessor ||
        g.predecessor_task != predecessor_task || g.successor != successor ||
        g.successor_task != successor_task)
        return APP_PRIVATE_DENIED;
    return APP_PRIVATE_OK;
}


