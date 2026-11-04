/* Host boundary for the actual generated domain and syscall translation units.
 * Physical allocation and page mapping are deterministic providers. CPU entry,
 * ELF loading, device access and user-pointer copies are not executed here. */
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel_domains.c"
#include "kernel_syscalls.c"
#include "kernel_rpc.c"
#if SEXOS_KERNEL_CURRENT_PDX
#include "kernel_task_wait.c"
#include "kernel_sources.c"
#include "kernel_pci.c"
#include "kernel_history.c"
void kernel_app_activation(Domain *d,TrapFrame *f){(void)d;(void)f;assert(!"activation is outside this transport/region harness");}
void kernel_app_query(Domain *d,TrapFrame *f,uint64_t n){(void)d;(void)f;(void)n;assert(!"lifecycle is outside this transport/region harness");}
void kernel_app_retire(Domain *d,TrapFrame *f){(void)d;(void)f;assert(!"retirement is outside this transport/region harness");}
void kernel_app_poll(void){assert(kernel.app_count==0);}
void kernel_paging_syscall(Domain *d,TrapFrame *f,uint64_t n){(void)d;(void)f;(void)n;assert(!"paging is outside this transport/region harness");}
_Noreturn void kernel_idle(void){abort();}
#endif

enum { HOST_FRAMES = KERNEL_BLOCKS, HOST_MAPPINGS = 8192, CALL_TYPE = 0x600 };
static uint64_t fresh, virtual_next = UINT64_C(0x400000000000);
static struct { uint64_t frame; unsigned key; } mappings[HOST_MAPPINGS];
static size_t mapping_count;
uint64_t hhdm_offset;

static uint64_t physical(uint64_t frame) { return (frame + 1) * KERNEL_PAGE_SIZE; }
static uint64_t abstract_frame(uint64_t pa)
{
    assert(pa >= KERNEL_PAGE_SIZE && pa % KERNEL_PAGE_SIZE == 0);
    return pa / KERNEL_PAGE_SIZE - 1;
}
static unsigned domain_key(uint32_t id)
{
    Domain *d = domain_get(id);
    assert(d);
    return d->key;
}

uint64_t frames_alloc(uint32_t order)
{
    assert(order == 0 && fresh < HOST_FRAMES);
    return physical(fresh++);
}
uint64_t va_alloc(uint64_t length)
{
    uint64_t va = virtual_next;
    virtual_next += (length + 4095) & ~UINT64_C(4095);
    return va;
}
bool map_page(uint64_t va, uint64_t pa, uint64_t flags, uint8_t key)
{
    (void)va;
    assert(mapping_count < HOST_MAPPINGS && abstract_frame(pa) < fresh);
    assert((flags & (PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE)) ==
           (PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE));
    mappings[mapping_count].frame = abstract_frame(pa);
    mappings[mapping_count++].key = key;
    return true;
}
void tss_set_rsp0(uint64_t top) { (void)top; }
void klog(const char *format, ...) { (void)format; }
void kernel_serial_write(const uint8_t *bytes, size_t count) { (void)bytes; (void)count; }
_Noreturn void kernel_halt(const char *reason) { fprintf(stderr, "%s\n", reason); abort(); }
bool copy_from_user(void *dst, uint64_t src, size_t length, uint32_t mask)
{ (void)dst; (void)src; (void)length; (void)mask; abort(); }
bool copy_to_user(uint64_t dst, const void *src, size_t length, uint32_t mask)
{ (void)dst; (void)src; (void)length; (void)mask; abort(); }
bool user_range_ok(uint64_t start, uint64_t length, bool write, uint32_t mask)
{ (void)start; (void)length; (void)write; (void)mask; abort(); }
uint32_t pci_read(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset)
{ (void)bus; (void)dev; (void)func; (void)offset; abort(); }
uint64_t pci_bar(uint8_t bus, uint8_t dev, uint8_t func, uint8_t index)
{ (void)bus; (void)dev; (void)func; (void)index; abort(); }

static void initialize(void)
{
    for (size_t i = 0; i < sizeof genesis_domains / sizeof genesis_domains[0]; ++i) {
        const GenesisDomain *g = &genesis_domains[i];
        uint32_t id = (uint32_t)i + 1;
        Domain *d = domain_boot_record(id);
        assert(d);
        d->id = id;
        d->live = true;
        d->key = g->key;
        d->mask = mask_for_key(g->key);
        d->name = g->log_name;
        pd_of_key[g->key] = id;
        assert(cap_set(d, SELF_RING_SLOT, domain_cap(id)));
    }
    apply_genesis_grants();
    kernel_seal_genesis();
}

/* All declared domains and slots, every ring entry, every reply right, every
 * allocated owner and every page mapping are observed after each request. */
static void snapshot(void)
{
    printf("|F%" PRIu64, fresh);
    for (uint32_t id = 1; domain_get(id); ++id) {
        Domain *d = domain_get(id);
        unsigned key = d->key;
        printf("|D%u", key);
        for (unsigned slot = 1; slot < KERNEL_CAP_SLOTS; ++slot) {
            const Cap *cap = cap_get(d, slot);
            if (!cap) continue;
            if (cap->kind == CAP_DOMAIN) printf("c%u:r%u,", slot, domain_key(cap->target));
            if (cap->kind == CAP_MEM_LEND) {
                assert(cap->length == KERNEL_PAGE_SIZE && cap->permissions == 3);
                printf("c%u:f%" PRIu64 ",", slot, abstract_frame(cap->base));
            }
        }
        printf("q");
        for (unsigned i = 0; i < d->ring.count; ++i) {
            const RingEntry *e = &d->ring.entries[(d->ring.head + i) % KERNEL_RING_CAPACITY];
            assert(e->from_call && e->type_id == CALL_TYPE && e->caller == e->sender);
            assert(e->arg1 == e->arg0 + 1000 && e->arg2 == e->arg0 + 2000);
            printf("%u:%" PRIu64 ",", domain_key(e->sender), e->arg0);
        }
        printf("p");
        for (uint32_t sender = 1; domain_get(sender); ++sender)
            if (d->reply_rights[sender]) printf("%u:%" PRIu64 ",", domain_key(sender), d->reply_rights[sender]);
        if (d->reply_ready) printf("s%" PRIu64, d->reply_value);
        else printf("s-");
        printf("x");
        for (uint32_t other = 1; domain_get(other); ++other) {
            unsigned shift = 2 * domain_key(other);
            printf("%u", ((d->mask >> shift) & 3) == 0);
        }
    }
    printf("|O");
    for (uint64_t f = 0; f < fresh; ++f) {
        unsigned owner = 0;
        for (uint32_t i = 0; i < kernel.block_count; ++i)
            if (kernel.blocks[i].base == physical(f)) owner = domain_key(kernel.blocks[i].owner);
        assert(owner);
        printf("%" PRIu64 ":%u,", f, owner);
    }
    printf("|M");
    for (size_t i = 0; i < mapping_count; ++i)
        printf("%" PRIu64 ":%u,", mappings[i].frame, mappings[i].key);
    putchar('\n');
}

static void boundary_controls(void)
{
    Domain *producer = domain_of_key(2), *consumer = domain_of_key(1);
    Domain *other = domain_of_key(3);
    uint64_t initial_fresh = fresh;
    assert(allocate_memory(producer, MAX_ALLOCATION_BYTES + 1) == NO_ADDRESS);
    assert(allocate_memory(producer, UINT64_MAX) == NO_ADDRESS && fresh == initial_fresh);
    assert(map_memory(producer, UINT64_MAX, UINT64_MAX) == NO_ADDRESS);
    kernel.block_count = KERNEL_BLOCKS;
    assert(allocate_memory(producer, 4096) == NO_ADDRESS && fresh == initial_fresh);
    assert(grant_mem_lend(domain_of_key(3), 5, 4096, 20) == NO_ADDRESS && fresh == initial_fresh);
    kernel.block_count = 0;
    assert(alloc_shared_buffer(other, 4096, (uint8_t)consumer->id) == 0);
    assert(alloc_shared_buffer(producer, 4096, (uint8_t)other->id) == 0);
    assert(alloc_shared_buffer(producer, 0, (uint8_t)consumer->id) == 0);
    kernel.shared_count = KERNEL_SHARED_BUFFERS;
    assert(alloc_shared_buffer(producer, 4096, (uint8_t)consumer->id) == 0 && fresh == initial_fresh);
    kernel.shared_count = 0;
    assert(alloc_shared_buffer(producer, 4096, (uint8_t)consumer->id) != 0);
    assert(kernel.shared_count == 1 && kernel.shared[0].producer == producer->id
           && kernel.shared[0].consumer == consumer->id && fresh == initial_fresh + 1);
    other->reply_rights[producer->id] = UINT16_MAX;
    RingEntry call = {.type_id = CALL_TYPE, .caller = producer->id, .sender = producer->id, .from_call = true};
    assert(domain_enqueue(other, call));
    RingEntry out = {0};
    assert(take_message(other, &out) && other->ring.count == 0);
    assert(other->reply_rights[producer->id] == (uint64_t)UINT16_MAX + 1);
    other->reply_rights[producer->id] = UINT64_MAX;
    assert(domain_enqueue(other, call));
    assert(!take_message(other, &out) && other->ring.count == 1);
    assert(other->reply_rights[producer->id] == UINT64_MAX);
    other->reply_rights[producer->id] -= 1;
    assert(take_message(other, &out) && other->ring.count == 0);
    assert(other->reply_rights[producer->id] == UINT64_MAX);
    puts("kernel boundary controls passed");
}

int main(int argc, char **argv)
{
    initialize();
    if (argc == 2 && strcmp(argv[1], "--bounds") == 0) { boundary_controls(); return 0; }
    assert(argc == 1);
    unsigned key, op;
    uint64_t a, b;
    while (scanf("%u %u %" SCNu64 " %" SCNu64, &key, &op, &a, &b) == 4) {
        Domain *d = domain_of_key(key);
        assert(d && !kernel.pku);
        kernel.current = d->id;
        TrapFrame f = {0};
        switch (op) {
        case 0:
            f.rax = SEXOS_SYSCALL_call; f.rdi = a; f.rsi = CALL_TYPE;
            f.rdx = b; f.r10 = b + 1000; f.r8 = b + 2000; break;
        case 1: f.rax = SEXOS_SYSCALL_listen; break;
        case 2:
            f.rax = SEXOS_SYSCALL_reply; f.rdi = domain_of_key(a)->id; f.rsi = b; break;
        case 3: f.rax = SEXOS_SYSCALL_allocate_memory; f.rdi = KERNEL_PAGE_SIZE; break;
        case 4: f.rax = SEXOS_SYSCALL_map_memory; f.rdi = physical(a); f.rsi = KERNEL_PAGE_SIZE; break;
        case 5:
            f.rax = SEXOS_SYSCALL_grant_mem_lend; f.rdi = a; f.rsi = KERNEL_PAGE_SIZE; f.rdx = b; break;
        case 6: f.rax = SEXOS_SYSCALL_map_mem_lend; f.rdi = a; break;
        default: return 2;
        }
        assert(kernel_syscall(&f) == &f);
        if (op == 1) {
            if (f.rax == CALL_TYPE) {
                assert(f.r10 == f.rdx + 1000 && f.r8 == f.rdx + 2000);
                printf("H%u:%" PRIu64, domain_key((uint32_t)f.rsi), f.rdx);
            } else if (f.rax == REPLY_TYPE) printf("A%" PRIu64, f.rdx);
            else { assert(f.rax == 0); printf("E"); }
        } else if (op == 3) printf("D%" PRIu64, abstract_frame(f.rax));
        else if (op == 5 && f.rax != NO_ADDRESS) printf("D%" PRIu64, fresh - 1);
        else if ((op == 4 || op == 6) && f.rax != NO_ADDRESS) printf("D0");
        else if ((op == 0 || op == 2) && f.rax == 0) printf("D0");
        else printf("R");
        snapshot();
    }
    return ferror(stdin) ? 2 : 0;
}
