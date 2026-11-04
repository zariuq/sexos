
#ifndef SEXOS_SEXT_H
#define SEXOS_SEXT_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#ifndef SEXT_FRAME_COUNT
#define SEXT_FRAME_COUNT 8u
#endif
#define SEXT_FRAME_STORAGE (SEXT_FRAME_COUNT ? SEXT_FRAME_COUNT : 1u)
#define SEXT_PAGE_BYTES UINT64_C(4096)
#define SEXT_MAX_REQUEST (UINT64_MAX >> 8)
#define SEXT_READ 2u
#define SEXT_WRITE 4u
enum { SEXT_OK, SEXT_INVALID, SEXT_RETRY, SEXT_DENIED, SEXT_EXHAUSTED, SEXT_LIVE };
enum { SEXT_OP_FAULT=0x103, SEXT_OP_RELEASE, SEXT_OP_EXIT, SEXT_OP_WAIT_CLEANUP };
enum { SEXT_SLOT_POOL=34, SEXT_SLOT_DEMAND, SEXT_SLOT_MAPPING, SEXT_SLOT_MAPPING_SECOND };
typedef struct { uint64_t base, length; uint32_t target_pd; uint8_t rights; } SextPagingInfo;
typedef struct { uint64_t fault_addr, error_code, token; uint32_t caller_pd; } SextFaultNotice;
typedef struct { uint64_t page_va, error_code; uint32_t target_pd; uint8_t allowed_rights; } SextFaultQuery;
typedef struct { uint64_t page, request_id; uint32_t caller_pd; } SextReleaseRequest;
typedef struct { uint32_t target_pd; uint64_t task_id; } SextExitNotice;
typedef struct { uint64_t frame_bitmap, task_id; } SextCleanupResult;
typedef struct { uint32_t target_pd; uint64_t request_id; uint32_t caller_pd; } SextWaitRequest;
typedef enum { SEXT_FREE, SEXT_RESERVED, SEXT_COMMITTED, SEXT_RECLAIMING } SextFrameState;
typedef struct { SextFrameState state; uint64_t token; uint32_t target; uint64_t page; bool zeroed; } SextFrame;
typedef enum { SEXT_ALLOC_INVALID, SEXT_ALLOC_CONFLICT, SEXT_ALLOC_QUOTA, SEXT_ALLOC_EXHAUSTED } SextAllocationError;
typedef struct { size_t index; uint64_t token; uint32_t target; uint64_t page; } SextBinding;
typedef struct { bool present; SextBinding value; } SextMaybeBinding;
typedef struct { SextMaybeBinding entries[SEXT_FRAME_STORAGE]; } SextBindings;
typedef struct { bool ok; SextAllocationError error; size_t index; bool needs_zero; } SextAllocation;
typedef struct {
    SextFrame frames[SEXT_FRAME_STORAGE]; uint32_t owners[SEXT_FRAME_STORAGE];
    size_t quota; bool partitioned;
} SextFrameAllocator;
typedef enum { SEXT_REASON_QUOTA, SEXT_REASON_EXHAUSTED, SEXT_REASON_POLICY, SEXT_REASON_COMMIT_DENIED } SextRejectReason;
typedef struct { bool present; uint64_t slot; SextPagingInfo info; } SextMapping;
typedef struct { SextMapping entries[2]; } SextMappings;
typedef struct {
    bool present; SextFaultNotice notice; bool commit; size_t index; uint8_t rights;
    SextRejectReason reason; bool may_have_published;
} SextActive;
typedef struct { bool present; SextReleaseRequest request; SextMaybeBinding binding; bool done; uint64_t status; } SextRelease;
typedef struct { bool present; SextCleanupResult value; } SextClosed;
typedef struct {
    SextFrameAllocator allocator; SextPagingInfo pool; SextMappings mappings;
    SextActive active[2]; SextRelease releases[2]; SextClosed closed[2];
} SextPager;
typedef enum { SEXT_OUT_RETRY, SEXT_OUT_IGNORED, SEXT_OUT_FATAL, SEXT_OUT_COMMITTED, SEXT_OUT_REJECTED } SextOutcomeTag;
typedef struct { SextOutcomeTag tag; uint64_t status; size_t index; uint64_t page; uint32_t target; uint64_t token; SextRejectReason reason; } SextOutcome;
typedef enum { SEXT_RELEASE_RETRY, SEXT_RELEASE_FATAL, SEXT_RELEASE_DONE } SextReleaseTag;
typedef struct { SextReleaseTag tag; uint64_t status; SextMaybeBinding binding; bool replay; } SextReleaseOutcome;
typedef enum { SEXT_CLEANUP_RETRY, SEXT_CLEANUP_LIVE, SEXT_CLEANUP_IGNORED, SEXT_CLEANUP_FATAL, SEXT_CLEANUP_CLOSED } SextCleanupTag;
typedef struct { SextCleanupTag tag; uint64_t status; SextCleanupResult result; size_t released; bool replay; } SextCleanupOutcome;
typedef struct { bool ready; uint64_t status; } SextWaitOutcome;
typedef struct { uint64_t base; size_t bytes, next; uint64_t records; } SextWorkspace;
bool sext_decode_fault(uint64_t opcode, uint64_t caller, uint64_t a, uint64_t b, uint64_t c, SextFaultNotice *out);
bool sext_decode_release(uint64_t opcode, uint64_t caller, uint64_t a, uint64_t b, uint64_t c, SextReleaseRequest *out);
bool sext_decode_exit(uint64_t opcode, uint64_t caller, uint64_t a, uint64_t b, uint64_t c, SextExitNotice *out);
bool sext_decode_wait(uint64_t opcode, uint64_t caller, uint64_t a, uint64_t b, uint64_t c, SextWaitRequest *out);
uint64_t sext_release_reply(uint64_t request, uint64_t status);
bool sext_release_reply_status(uint64_t value, uint64_t request, uint64_t *out);
void sext_allocator_init(SextFrameAllocator *a, size_t quota);
bool sext_allocator_partition(SextFrameAllocator *a,uint32_t target,size_t base,size_t count);
SextAllocation sext_allocator_reserve(SextFrameAllocator *a, uint64_t token, uint32_t target, uint64_t page);
bool sext_allocator_mark_zeroed(SextFrameAllocator *a, size_t index, uint64_t token);
bool sext_allocator_commit(SextFrameAllocator *a, size_t index, uint64_t token);
bool sext_allocator_rollback(SextFrameAllocator *a, size_t index, uint64_t token);
SextMaybeBinding sext_allocator_begin_reclaim(SextFrameAllocator *a, uint32_t target, uint64_t page);
bool sext_allocator_finish_reclaim(SextFrameAllocator *a, SextBinding binding);
SextBindings sext_allocator_bindings(const SextFrameAllocator *a, uint32_t target);
uint64_t sext_allocator_target_mask(const SextFrameAllocator *a, uint32_t target);
size_t sext_allocator_finish_cleanup(SextFrameAllocator *a, uint32_t target);
size_t sext_allocator_used_by(const SextFrameAllocator *a, uint32_t target);
size_t sext_allocator_used(const SextFrameAllocator *a);
uint64_t sext_pager_init(SextPager *p, SextPagingInfo pool, SextMappings mappings, size_t quota);
size_t sext_pager_client_index(const SextPager *p, uint32_t target);
SextMapping sext_pager_mapping(const SextPager *p, uint32_t target);
SextClosed sext_pager_closed(const SextPager *p, uint32_t target);
SextOutcome sext_pager_step(SextPager *p, SextFaultNotice notice);
SextReleaseOutcome sext_pager_release(SextPager *p, SextReleaseRequest request);
SextCleanupOutcome sext_pager_cleanup(SextPager *p, SextExitNotice notice);
SextWaitOutcome sext_pager_wait(const SextPager *p, SextWaitRequest request);
uint64_t sext_backend_query(uint64_t mapping, uint64_t token, SextFaultQuery *out);
void sext_backend_zero(uint64_t address, uint64_t bytes);
uint64_t sext_backend_commit(uint64_t mapping, size_t index, uint64_t token, uint8_t rights);
uint64_t sext_backend_reject(uint64_t mapping, uint64_t token);
uint64_t sext_backend_reclaim(uint64_t mapping, size_t index, uint64_t token);
uint64_t sext_backend_cleanup(uint64_t mapping, SextCleanupResult *out);
#endif
