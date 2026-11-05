#if !defined(SEXOS_SEXT_PARTITIONED_ALLOCATOR)
#define SEXOS_SEXT_PARTITIONED_ALLOCATOR 0

#endif
#include "sext.h"
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

bool sext_decode_fault(uint64_t op, uint64_t caller, uint64_t a, uint64_t b, uint64_t c,
                       SextFaultNotice *out) {
    if (op != SEXT_OP_FAULT || !caller || caller > UINT32_MAX || !a || a >= (UINT64_C(1) << 47) ||
        (b != 4 && b != 6) || !c)
        return false;
    *out = (SextFaultNotice){a, b, c, (uint32_t)caller};
    return true;
}

bool sext_decode_release(uint64_t op, uint64_t caller, uint64_t a, uint64_t b, uint64_t c,
                         SextReleaseRequest *out) {
    if (op != SEXT_OP_RELEASE || !caller || caller > UINT32_MAX || !a || a >= (UINT64_C(1) << 47) ||
        a % SEXT_PAGE_BYTES || !b || b > SEXT_MAX_REQUEST || c)
        return false;
    *out = (SextReleaseRequest){a, b, (uint32_t)caller};
    return true;
}

bool sext_decode_exit(uint64_t op, uint64_t caller, uint64_t a, uint64_t b, uint64_t c,
                      SextExitNotice *out) {
    if (op != SEXT_OP_EXIT || caller || !a || a > UINT32_MAX || !b || c)
        return false;
    *out = (SextExitNotice){(uint32_t)a, b};
    return true;
}

bool sext_decode_wait(uint64_t op, uint64_t caller, uint64_t a, uint64_t b, uint64_t c,
                      SextWaitRequest *out) {
    if (op != SEXT_OP_WAIT_CLEANUP || !caller || caller > UINT32_MAX || !a || a > UINT32_MAX ||
        a == caller || !b || b > SEXT_MAX_REQUEST || c)
        return false;
    *out = (SextWaitRequest){(uint32_t)a, b, (uint32_t)caller};
    return true;
}

uint64_t sext_release_reply(uint64_t request, uint64_t status) {
    if (!request || request > SEXT_MAX_REQUEST || status > UINT8_MAX)
        return SEXT_DENIED;
    return (request << 8) | status;
}

bool sext_release_reply_status(uint64_t value, uint64_t request, uint64_t *out) {
    if (!request || request > SEXT_MAX_REQUEST || (value >> 8) != request)
        return false;
    *out = value & UINT64_C(255);
    return true;
}

void sext_allocator_init(SextFrameAllocator *a, size_t quota) {
    *a = (SextFrameAllocator){.quota = quota};
}

bool sext_allocator_partition(SextFrameAllocator *a, uint32_t target, size_t base, size_t count) {
    if (!a || !target || !count || !SEXT_FRAME_COUNT || sext_allocator_used(a))
        return false;
    if (base > SEXT_FRAME_COUNT || count > SEXT_FRAME_COUNT - base)
        return false;
    for (size_t i = base; i < base + count; ++i)
        if (a->owners[i] && a->owners[i] != target)
            return false;
    for (size_t i = base; i < base + count; ++i)
        a->owners[i] = target;
    a->partitioned = true;
    return true;
}

size_t sext_allocator_used_by(const SextFrameAllocator *a, uint32_t target) {
    size_t count = 0, length = SEXT_FRAME_COUNT;
    for (size_t i = 0; i < length; ++i)
        if (a->frames[i].state != SEXT_FREE && a->frames[i].target == target)
            ++count;
    return count;
}

size_t sext_allocator_used(const SextFrameAllocator *a) {
    size_t count = 0, length = SEXT_FRAME_COUNT;
    for (size_t i = 0; i < length; ++i)
        if (a->frames[i].state != SEXT_FREE)
            ++count;
    return count;
}

SextAllocation sext_allocator_reserve(SextFrameAllocator *a, uint64_t token, uint32_t target,
                                      uint64_t page) {
    if (!token || !target || !page || page % 4096)
        return (SextAllocation){.error = SEXT_ALLOC_INVALID};
    size_t length = SEXT_FRAME_COUNT;
    for (size_t i = 0; i < length; ++i) {
        SextFrame f = a->frames[i];
        if (f.state != SEXT_FREE && f.token == token) {
            if (f.state != SEXT_RESERVED || f.target != target || f.page != page)
                return (SextAllocation){.error = SEXT_ALLOC_CONFLICT};
            return (SextAllocation){.ok = true, .index = i, .needs_zero = !f.zeroed};
        }
    }
    for (size_t i = 0; i < length; ++i)
        if (a->frames[i].state != SEXT_FREE && a->frames[i].target == target &&
            a->frames[i].page == page)
            return (SextAllocation){.error = SEXT_ALLOC_CONFLICT};
    if (sext_allocator_used_by(a, target) >= a->quota)
        return (SextAllocation){.error = SEXT_ALLOC_QUOTA};
    bool has_partition = false;
    if (a->partitioned)
        for (size_t i = 0; i < length; ++i)
            if (a->owners[i] == target)
                has_partition = true;
    if (a->partitioned && !has_partition)
        return (SextAllocation){.error = SEXT_ALLOC_INVALID};
    for (size_t i = 0; i < length; ++i)
        if (a->frames[i].state == SEXT_FREE && (!a->partitioned || a->owners[i] == target)) {
            a->frames[i] = (SextFrame){SEXT_RESERVED, token, target, page, false};
            return (SextAllocation){.ok = true, .index = i, .needs_zero = true};
        }
    return (SextAllocation){.error = SEXT_ALLOC_EXHAUSTED};
}

bool sext_allocator_mark_zeroed(SextFrameAllocator *a, size_t index, uint64_t token) {
    size_t length = SEXT_FRAME_COUNT;
    if (index >= length || a->frames[index].state != SEXT_RESERVED ||
        a->frames[index].token != token)
        return false;
    a->frames[index].zeroed = true;
    return true;
}

bool sext_allocator_commit(SextFrameAllocator *a, size_t index, uint64_t token) {
    size_t length = SEXT_FRAME_COUNT;
    if (index >= length || a->frames[index].state != SEXT_RESERVED ||
        a->frames[index].token != token || !a->frames[index].zeroed)
        return false;
    a->frames[index].state = SEXT_COMMITTED;
    return true;
}

bool sext_allocator_rollback(SextFrameAllocator *a, size_t index, uint64_t token) {
    size_t length = SEXT_FRAME_COUNT;
    if (index >= length || a->frames[index].state != SEXT_RESERVED ||
        a->frames[index].token != token)
        return false;
    a->frames[index] = (SextFrame){0};
    return true;
}

SextMaybeBinding sext_allocator_begin_reclaim(SextFrameAllocator *a, uint32_t target,
                                              uint64_t page) {
    size_t length = SEXT_FRAME_COUNT;
    for (size_t i = 0; i < length; ++i)
        if (a->frames[i].state == SEXT_COMMITTED && a->frames[i].target == target &&
            a->frames[i].page == page) {
            a->frames[i].state = SEXT_RECLAIMING;
            return (SextMaybeBinding){true, {i, a->frames[i].token, target, page}};
        }
    return (SextMaybeBinding){0};
}

bool sext_allocator_finish_reclaim(SextFrameAllocator *a, SextBinding b) {
    size_t length = SEXT_FRAME_COUNT;
    if (b.index >= length)
        return false;
    SextFrame f = a->frames[b.index];
    if (f.state != SEXT_RECLAIMING || f.token != b.token || f.target != b.target ||
        f.page != b.page)
        return false;
    a->frames[b.index] = (SextFrame){0};
    return true;
}

SextBindings sext_allocator_bindings(const SextFrameAllocator *a, uint32_t target) {
    SextBindings result = {0};
    size_t length = SEXT_FRAME_COUNT;
    for (size_t i = 0; i < length; ++i)
        if (a->frames[i].state != SEXT_FREE && a->frames[i].target == target)
            result.entries[i] =
                (SextMaybeBinding){true, {i, a->frames[i].token, target, a->frames[i].page}};
    return result;
}

uint64_t sext_allocator_target_mask(const SextFrameAllocator *a, uint32_t target) {
    uint64_t mask = 0;
    size_t length = SEXT_FRAME_COUNT;
    for (size_t i = 0; i < length; ++i)
        if (i < 64 && a->frames[i].state != SEXT_FREE && a->frames[i].target == target)
            mask |= UINT64_C(1) << i;
    return mask;
}

size_t sext_allocator_finish_cleanup(SextFrameAllocator *a, uint32_t target) {
    size_t released = 0, length = SEXT_FRAME_COUNT;
    for (size_t i = 0; i < length; ++i)
        if (a->frames[i].state != SEXT_FREE && a->frames[i].target == target) {
            a->frames[i] = (SextFrame){0};
            ++released;
        }
    return released;
}

static bool sext_valid_range(uint64_t base, uint64_t bytes) {
    return base && base % SEXT_PAGE_BYTES == 0 && bytes && bytes % SEXT_PAGE_BYTES == 0 &&
           base <= UINT64_MAX - bytes && base + bytes <= (UINT64_C(1) << 47);
}

uint64_t sext_pager_init(SextPager *p, SextPagingInfo pool, SextMappings mappings, size_t quota) {
    if (!sext_valid_range(pool.base, pool.length) || pool.target_pd ||
        pool.length / SEXT_PAGE_BYTES != SEXT_FRAME_COUNT || (pool.rights & 6) != 6 || !quota ||
        quota > SEXT_FRAME_COUNT || !mappings.entries[0].present)
        return SEXT_INVALID;
    for (size_t i = 0; i < 2; ++i)
        if (mappings.entries[i].present) {
            SextMapping m = mappings.entries[i];
            if (!m.slot || !m.info.target_pd || !(m.info.rights & SEXT_READ) ||
                !sext_valid_range(m.info.base, m.info.length))
                return SEXT_INVALID;
        }
    SextMapping a = mappings.entries[0], b = mappings.entries[1];
    if (a.present && b.present &&
        (a.slot == b.slot || a.info.target_pd == b.info.target_pd ||
         (a.info.base < b.info.base + b.info.length && b.info.base < a.info.base + a.info.length)))
        return SEXT_INVALID;
    *p = (SextPager){.pool = pool, .mappings = mappings};
    sext_allocator_init(&p->allocator, quota);
    if (SEXOS_SEXT_PARTITIONED_ALLOCATOR) {
        size_t clients = 0;
        for (size_t i = 0; i < 2; ++i)
            if (mappings.entries[i].present)
                ++clients;
        if (!clients || quota > SEXT_FRAME_COUNT / clients)
            return SEXT_INVALID;
        for (size_t i = 0; i < 2; ++i)
            if (mappings.entries[i].present &&
                !sext_allocator_partition(&p->allocator, mappings.entries[i].info.target_pd,
                                          i * quota, quota))
                return SEXT_INVALID;
    }
    return SEXT_OK;
}

size_t sext_pager_client_index(const SextPager *p, uint32_t target) {
    for (size_t i = 0; i < 2; ++i)
        if (p->mappings.entries[i].present && p->mappings.entries[i].info.target_pd == target)
            return i;
    return 2;
}

SextMapping sext_pager_mapping(const SextPager *p, uint32_t target) {
    size_t index = sext_pager_client_index(p, target);
    return index < 2 ? p->mappings.entries[index] : (SextMapping){0};
}

SextClosed sext_pager_closed(const SextPager *p, uint32_t target) {
    size_t index = sext_pager_client_index(p, target);
    return index < 2 ? p->closed[index] : (SextClosed){0};
}

SextCleanupOutcome sext_pager_cleanup(SextPager *p, SextExitNotice notice) {
    size_t client = sext_pager_client_index(p, notice.target_pd);
    if (client == 2 || !notice.task_id)
        return (SextCleanupOutcome){.tag = SEXT_CLEANUP_IGNORED};
    if (p->closed[client].present) {
        if (p->closed[client].value.task_id == notice.task_id)
            return (SextCleanupOutcome){
                .tag = SEXT_CLEANUP_CLOSED, .result = p->closed[client].value, .replay = true};
        return (SextCleanupOutcome){.tag = SEXT_CLEANUP_IGNORED};
    }
    SextCleanupResult result = {0};
    uint64_t status = sext_backend_cleanup(p->mappings.entries[client].slot, &result);
    if (status == SEXT_RETRY)
        return (SextCleanupOutcome){.tag = SEXT_CLEANUP_RETRY};
    if (status == SEXT_LIVE)
        return (SextCleanupOutcome){.tag = SEXT_CLEANUP_LIVE};
    if (status != SEXT_OK)
        return (SextCleanupOutcome){.tag = SEXT_CLEANUP_FATAL, .status = status};
    if (result.task_id != notice.task_id ||
        (result.frame_bitmap & ~sext_allocator_target_mask(&p->allocator, notice.target_pd)))
        return (SextCleanupOutcome){.tag = SEXT_CLEANUP_FATAL, .status = SEXT_DENIED};
    size_t released = sext_allocator_finish_cleanup(&p->allocator, notice.target_pd);
    p->active[client] = (SextActive){0};
    p->releases[client] = (SextRelease){0};
    p->closed[client] = (SextClosed){true, result};
    return (SextCleanupOutcome){.tag = SEXT_CLEANUP_CLOSED, .result = result, .released = released};
}

SextWaitOutcome sext_pager_wait(const SextPager *p, SextWaitRequest request) {
    SextWaitRequest checked = {0};
    if (!sext_decode_wait(SEXT_OP_WAIT_CLEANUP, request.caller_pd, request.target_pd,
                          request.request_id, 0, &checked) ||
        sext_pager_client_index(p, request.caller_pd) == 2)
        return (SextWaitOutcome){true, SEXT_DENIED};
    size_t target = sext_pager_client_index(p, request.target_pd);
    if (target == 2)
        return (SextWaitOutcome){true, SEXT_DENIED};
    return p->closed[target].present ? (SextWaitOutcome){true, SEXT_OK} : (SextWaitOutcome){0};
}

static bool sext_release_equal(SextReleaseRequest a, SextReleaseRequest b) {
    return a.page == b.page && a.request_id == b.request_id && a.caller_pd == b.caller_pd;
}

static bool sext_notice_equal(SextFaultNotice a, SextFaultNotice b) {
    return a.fault_addr == b.fault_addr && a.error_code == b.error_code && a.token == b.token &&
           a.caller_pd == b.caller_pd;
}

SextReleaseOutcome sext_pager_release(SextPager *p, SextReleaseRequest request) {
    SextReleaseRequest checked = {0};
    if (!sext_decode_release(SEXT_OP_RELEASE, request.caller_pd, request.page, request.request_id,
                             0, &checked))
        return (SextReleaseOutcome){.tag = SEXT_RELEASE_DONE, .status = SEXT_INVALID};
    size_t client = sext_pager_client_index(p, request.caller_pd);
    if (client == 2 || p->closed[client].present)
        return (SextReleaseOutcome){.tag = SEXT_RELEASE_DONE, .status = SEXT_DENIED};
    SextRelease previous = p->releases[client];
    if (previous.present) {
        if (request.request_id < previous.request.request_id ||
            (request.request_id == previous.request.request_id &&
             !sext_release_equal(request, previous.request)))
            return (SextReleaseOutcome){.tag = SEXT_RELEASE_DONE, .status = SEXT_DENIED};
        if (sext_release_equal(request, previous.request)) {
            if (previous.done)
                return (SextReleaseOutcome){.tag = SEXT_RELEASE_DONE,
                                            .status = previous.status,
                                            .binding = previous.binding,
                                            .replay = true};
        } else if (!previous.done)
            return (SextReleaseOutcome){.tag = SEXT_RELEASE_RETRY};
    }
    if (p->active[client].present)
        return (SextReleaseOutcome){.tag = SEXT_RELEASE_RETRY};
    if (!previous.present || !sext_release_equal(previous.request, request)) {
        SextMaybeBinding binding =
            sext_allocator_begin_reclaim(&p->allocator, request.caller_pd, request.page);
        p->releases[client] = (SextRelease){.present = true,
                                            .request = request,
                                            .binding = binding,
                                            .done = !binding.present,
                                            .status = binding.present ? 0 : SEXT_DENIED};
    }
    SextRelease release = p->releases[client];
    if (!release.binding.present)
        return (SextReleaseOutcome){.tag = SEXT_RELEASE_DONE, .status = SEXT_DENIED};
    SextBinding binding = release.binding.value;
    uint64_t status =
        sext_backend_reclaim(p->mappings.entries[client].slot, binding.index, binding.token);
    if (status == SEXT_OK) {
        if (!sext_allocator_finish_reclaim(&p->allocator, binding))
            return (SextReleaseOutcome){.tag = SEXT_RELEASE_FATAL, .status = SEXT_INVALID};
        p->releases[client].done = true;
        p->releases[client].status = SEXT_OK;
        return (SextReleaseOutcome){
            .tag = SEXT_RELEASE_DONE, .status = SEXT_OK, .binding = release.binding};
    }
    if (status == SEXT_RETRY)
        return (SextReleaseOutcome){.tag = SEXT_RELEASE_RETRY};
    return (SextReleaseOutcome){.tag = SEXT_RELEASE_FATAL, .status = status};
}

SextOutcome sext_pager_step(SextPager *p, SextFaultNotice notice) {
    SextFaultNotice checked = {0};
    if (!sext_decode_fault(SEXT_OP_FAULT, notice.caller_pd, notice.fault_addr, notice.error_code,
                           notice.token, &checked))
        return (SextOutcome){.tag = SEXT_OUT_IGNORED};
    size_t client = sext_pager_client_index(p, notice.caller_pd);
    if (client == 2 || p->closed[client].present)
        return (SextOutcome){.tag = SEXT_OUT_IGNORED};
    SextMapping mapping = p->mappings.entries[client];
    if (p->releases[client].present && !p->releases[client].done)
        return (SextOutcome){.tag = SEXT_OUT_RETRY};
    SextActive active = p->active[client];
    if (active.present) {
        if (!sext_notice_equal(active.notice, notice))
            return (SextOutcome){.tag = SEXT_OUT_IGNORED};
    } else {
        SextFaultQuery query = {0};
        uint64_t status = sext_backend_query(mapping.slot, notice.token, &query);
        if (status == SEXT_RETRY)
            return (SextOutcome){.tag = SEXT_OUT_RETRY};
        if (status == SEXT_INVALID || status == SEXT_DENIED)
            return (SextOutcome){.tag = SEXT_OUT_IGNORED};
        if (status != SEXT_OK)
            return (SextOutcome){.tag = SEXT_OUT_FATAL, .status = status};
        if (query.page_va != (notice.fault_addr & ~(SEXT_PAGE_BYTES - 1)) ||
            query.target_pd != notice.caller_pd || query.target_pd != mapping.info.target_pd ||
            query.error_code != notice.error_code)
            return (SextOutcome){.tag = SEXT_OUT_IGNORED};
        bool fits = query.page_va >= mapping.info.base &&
                    query.page_va <= UINT64_MAX - SEXT_PAGE_BYTES &&
                    query.page_va + SEXT_PAGE_BYTES <= mapping.info.base + mapping.info.length;
        uint8_t rights = query.allowed_rights & mapping.info.rights & (SEXT_READ | SEXT_WRITE);
        active = (SextActive){.present = true, .notice = notice, .reason = SEXT_REASON_POLICY};
        if (fits && (rights & SEXT_READ) && (notice.error_code != 6 || (rights & SEXT_WRITE))) {
            SextAllocation allocation =
                sext_allocator_reserve(&p->allocator, notice.token, query.target_pd, query.page_va);
            if (allocation.ok) {
                if (allocation.needs_zero) {
                    sext_backend_zero(p->pool.base + (uint64_t)allocation.index * SEXT_PAGE_BYTES,
                                      SEXT_PAGE_BYTES);
                    if (!sext_allocator_mark_zeroed(&p->allocator, allocation.index, notice.token))
                        return (SextOutcome){.tag = SEXT_OUT_FATAL, .status = SEXT_INVALID};
                }
                active.commit = true;
                active.index = allocation.index;
                active.rights = rights;
            } else if (allocation.error == SEXT_ALLOC_QUOTA)
                active.reason = SEXT_REASON_QUOTA;
            else if (allocation.error == SEXT_ALLOC_EXHAUSTED)
                active.reason = SEXT_REASON_EXHAUSTED;
        }
        p->active[client] = active;
    }
    if (active.commit) {
        uint64_t status =
            sext_backend_commit(mapping.slot, active.index, notice.token, active.rights);
        if (status == SEXT_OK) {
            if (!sext_allocator_commit(&p->allocator, active.index, notice.token))
                return (SextOutcome){.tag = SEXT_OUT_FATAL, .status = SEXT_INVALID};
            p->active[client] = (SextActive){0};
            return (SextOutcome){.tag = SEXT_OUT_COMMITTED,
                                 .index = active.index,
                                 .page = notice.fault_addr & ~(SEXT_PAGE_BYTES - 1),
                                 .target = notice.caller_pd,
                                 .token = notice.token};
        }
        if (status == SEXT_RETRY) {
            p->active[client].may_have_published = true;
            return (SextOutcome){.tag = SEXT_OUT_RETRY};
        }
        if (status == SEXT_INVALID || status == SEXT_DENIED || status == SEXT_EXHAUSTED) {
            if (active.may_have_published)
                return (SextOutcome){.tag = SEXT_OUT_FATAL, .status = SEXT_DENIED};
            if (!sext_allocator_rollback(&p->allocator, active.index, notice.token))
                return (SextOutcome){.tag = SEXT_OUT_FATAL, .status = SEXT_INVALID};
            p->active[client] = (SextActive){
                .present = true, .notice = notice, .reason = SEXT_REASON_COMMIT_DENIED};
            return (SextOutcome){.tag = SEXT_OUT_RETRY};
        }
        return (SextOutcome){.tag = SEXT_OUT_FATAL, .status = status};
    }
    uint64_t status = sext_backend_reject(mapping.slot, notice.token);
    if (status == SEXT_OK) {
        p->active[client] = (SextActive){0};
        return (SextOutcome){.tag = SEXT_OUT_REJECTED,
                             .reason = active.reason,
                             .page = notice.fault_addr & ~(SEXT_PAGE_BYTES - 1),
                             .target = notice.caller_pd,
                             .token = notice.token};
    }
    if (status == SEXT_RETRY)
        return (SextOutcome){.tag = SEXT_OUT_RETRY};
    if (status == SEXT_INVALID || status == SEXT_DENIED) {
        p->active[client] = (SextActive){0};
        return (SextOutcome){.tag = SEXT_OUT_IGNORED};
    }
    return (SextOutcome){.tag = SEXT_OUT_FATAL, .status = status};
}


