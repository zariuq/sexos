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
#include "sexos_kernel.h"
#include "program_memory.h"

void program_memory_init(ProgramMemoryState *state) {
    *state = (ProgramMemoryState){0};
}

uint64_t program_memory_begin(ProgramMemoryState *state, uint32_t target, uint32_t run,
                              uint8_t key, uint64_t base, uint64_t arena_pages,
                              uint64_t image_pages, uint64_t stack_pages) {
    if (!target || !run || !key || key >= 16 || base % (2 * 1024 * 1024) ||
        !arena_pages || arena_pages > UINT64_MAX / 4096 ||
        base > UINT64_MAX - arena_pages * 4096 ||
        image_pages > arena_pages || stack_pages >= arena_pages ||
        image_pages + stack_pages + 1 > arena_pages ||
        (state->phase != PROGRAM_MEMORY_FREE && state->phase != PROGRAM_MEMORY_DEAD) ||
        (state->phase == PROGRAM_MEMORY_DEAD && state->run == run))
        return PROGRAM_MEMORY_INVALID;
    *state = (ProgramMemoryState){
        .base = base, .arena_pages = arena_pages, .image_pages = image_pages,
        .stack_pages = stack_pages, .target = target, .run = run,
        .key = key, .phase = PROGRAM_MEMORY_LIVE, .main_alive = true};
    return PROGRAM_MEMORY_OK;
}

uint64_t program_memory_plan(const ProgramMemoryState *state, uint64_t want,
                             ProgramMemoryPlan *out) {
    if (want > PROGRAM_MEMORY_MAX_PAGES_PER_CALL) return PROGRAM_MEMORY_INVALID;
    if (state->phase != PROGRAM_MEMORY_LIVE || !state->main_alive) return PROGRAM_MEMORY_DENIED;
    if (state->image_pages > state->arena_pages || state->grown > state->arena_pages ||
        state->stack_pages >= state->arena_pages) return PROGRAM_MEMORY_DENIED;
    uint64_t used = state->image_pages + state->grown;
    if (used > state->arena_pages || state->stack_pages + 1 > state->arena_pages - used)
        return PROGRAM_MEMORY_FULL;
    if (want > state->arena_pages - used - state->stack_pages - 1)
        return PROGRAM_MEMORY_FULL;
    out->first = used;
    out->count = want;
    return PROGRAM_MEMORY_OK;
}

bool program_memory_map_at(ProgramMemoryState *state, uint64_t va, uint64_t flags) {
    if (state->phase != PROGRAM_MEMORY_LIVE || !state->main_alive ||
        va % 4096 || va < state->base ||
        va >= state->base + state->arena_pages * 4096)
        return false;
    FrameLeaseResult lease = program_frame_take(state->target);
    if (lease.status != FRAME_LEASE_OK) return false;
    if (program_frame_map_run(lease.value, state->run, va, flags, state->key))
        return true;
    if (!program_frame_release(lease.value)) state->phase = PROGRAM_MEMORY_QUARANTINE;
    return false;
}

ProgramMemoryReply program_memory_grow(ProgramMemoryState *state, uint64_t want) {
    ProgramMemoryReply reply = {0, 0, 0, program_frame_pressure()};
    ProgramMemoryPlan plan = {0};
    uint64_t status = program_memory_plan(state, want, &plan);
    if (status != PROGRAM_MEMORY_OK) {
        reply.status = status;
        return reply;
    }
    uint64_t heap_base = state->base + state->image_pages * 4096;
    if (!want) {
        reply.status = PROGRAM_MEMORY_OK;
        reply.base = heap_base;
        reply.held = state->grown;
        return reply;
    }
    FrameLease leases[PROGRAM_MEMORY_MAX_PAGES_PER_CALL];
    size_t count = 0;
    for (size_t i = 0; i < want; ++i) {
        FrameLeaseResult lease = program_frame_take(state->target);
        if (lease.status != FRAME_LEASE_OK) {
            status = PROGRAM_MEMORY_FULL;
            break;
        }
        leases[count++] = lease.value;
    }
    if (status == PROGRAM_MEMORY_OK) {
        for (size_t i = 0; i < want; ++i) {
            uint64_t va = state->base + (plan.first + i) * 4096;
            if (!program_frame_map_run(leases[i], state->run, va,
                                       PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE,
                                       state->key)) {
                status = PROGRAM_MEMORY_FULL;
                break;
            }
        }
    }
    if (status == PROGRAM_MEMORY_OK) {
        state->grown += want;
        reply.status = PROGRAM_MEMORY_OK;
        reply.base = heap_base;
        reply.held = state->grown;
        klog("[program.memory.grow] target=%u run=%u base=%#llx held=%llu first_frame=%#llx incarnation=%u pressure=%llu\n",
             state->target, state->run, (unsigned long long)reply.base,
             (unsigned long long)reply.held, (unsigned long long)leases[0].base,
             leases[0].incarnation, (unsigned long long)reply.pressure);
        return reply;
    }
    for (size_t i = count; i > 0; --i)
        if (!program_frame_release(leases[i - 1])) state->phase = PROGRAM_MEMORY_QUARANTINE;
    reply.status = status;
    reply.held = 0;
    return reply;
}

uint64_t program_memory_worker_spawn(ProgramMemoryState *state) {
    if (state->phase != PROGRAM_MEMORY_LIVE || !state->main_alive)
        return PROGRAM_MEMORY_DENIED;
    if (state->workers == PROGRAM_MEMORY_MAX_WORKERS)
        return PROGRAM_MEMORY_FULL;
    ++state->workers;
    return PROGRAM_MEMORY_OK;
}

uint64_t program_memory_worker_exit(ProgramMemoryState *state) {
    if (state->phase != PROGRAM_MEMORY_LIVE || !state->workers)
        return PROGRAM_MEMORY_DENIED;
    --state->workers;
    return PROGRAM_MEMORY_OK;
}

uint64_t program_memory_main_exit(ProgramMemoryState *state) {
    if (state->phase != PROGRAM_MEMORY_LIVE || !state->main_alive)
        return PROGRAM_MEMORY_DENIED;
    state->main_alive = false;
    return PROGRAM_MEMORY_OK;
}

uint64_t program_memory_end(ProgramMemoryState *state) {
    if (state->phase == PROGRAM_MEMORY_LIVE && (state->main_alive || state->workers))
        return PROGRAM_MEMORY_RETRY;
    if (state->phase == PROGRAM_MEMORY_LIVE) state->phase = PROGRAM_MEMORY_ENDING;
    if (state->phase != PROGRAM_MEMORY_ENDING && state->phase != PROGRAM_MEMORY_QUARANTINE)
        return PROGRAM_MEMORY_DENIED;
    if (!program_frame_release_run(state->target, state->run)) {
        state->phase = PROGRAM_MEMORY_QUARANTINE;
        return PROGRAM_MEMORY_RETRY;
    }
    state->phase = PROGRAM_MEMORY_DEAD;
    memset(&state->grown, 0, sizeof state->grown);
    return PROGRAM_MEMORY_OK;
}
