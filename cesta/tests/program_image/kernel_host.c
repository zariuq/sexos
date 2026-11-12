/* Actual loader, allocator, page tables and private-manifest capture.
   CPU operations are host providers. Padding allocations force image pages
   to occupy nonadjacent physical frames. No user instruction executes here. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define memcpy sexos_memcpy
#define memmove sexos_memmove
#define memset sexos_memset
#define memcmp sexos_memcmp
#define frames_alloc actual_frames_alloc
#define map_page actual_map_page
#define program_frame_take_transient actual_program_frame_take_transient
#define program_frame_map actual_program_frame_map
#define program_frame_map_run actual_program_frame_map_run
#define image_write actual_image_write
#define program_frame_commit_all actual_program_frame_commit_all
#include "kernel_memory.c"
#undef frames_alloc
#undef map_page
#undef program_frame_take_transient
#undef program_frame_map
#undef program_frame_map_run
#undef image_write
#undef program_frame_commit_all

static unsigned allocations, mappings, take_calls, map_calls, writes, captures, commit_batches;
static int fail_take = -1, fail_map = -1, fail_write = -1, fail_capture = -1, fail_commit = -1;
uint64_t frames_alloc(uint32_t order)
{
    ++allocations;
    uint64_t frame = actual_frames_alloc(order);
    if (frame != NO_ADDRESS) (void)actual_frames_alloc(0);
    return frame;
}
bool map_page(uint64_t va, uint64_t physical, uint64_t flags, uint8_t key)
{
    ++mappings;
    return actual_map_page(va, physical, flags, key);
}
FrameLeaseResult program_frame_take_transient(uint32_t holder)
{
    ++allocations;
    if (fail_take >= 0 && take_calls++ >= (unsigned)fail_take)
        return (FrameLeaseResult){.status = FRAME_LEASE_FULL};
    FrameLeaseResult result = actual_program_frame_take_transient(holder);
    if (result.status == FRAME_LEASE_OK) (void)actual_frames_alloc(0);
    return result;
}
bool program_frame_map(FrameLease lease, uint64_t va, uint64_t flags, uint8_t key)
{
    ++mappings;
    if (fail_map >= 0 && map_calls++ >= (unsigned)fail_map) return false;
    return actual_program_frame_map(lease, va, flags, key);
}
bool program_frame_map_run(FrameLease lease, uint32_t run, uint64_t va, uint64_t flags, uint8_t key)
{
    ++mappings;
    if (fail_map >= 0 && map_calls++ >= (unsigned)fail_map) return false;
    return actual_program_frame_map_run(lease, run, va, flags, key);
}
bool image_write(uint64_t address, const uint8_t *source, uint64_t count)
{
    if (fail_write >= 0 && writes++ >= (unsigned)fail_write) return false;
    return actual_image_write(address, source, count);
}
uint64_t program_frame_commit_all(FrameLease *leases, size_t count)
{
    if (fail_commit >= 0) return FRAME_LEASE_DENIED;
    ++commit_batches;
    return actual_program_frame_commit_all(leases, count);
}
#include "kernel_domains.c"
extern bool __real_kernel_app_capture_range(Domain *d, uint64_t start, uint64_t length);
bool __wrap_kernel_app_capture_range(Domain *d, uint64_t start, uint64_t length)
{
    if (fail_capture >= 0 && captures++ >= (unsigned)fail_capture) return false;
    return __real_kernel_app_capture_range(d, start, length);
}
#undef memcpy
#undef memmove
#undef memset
#undef memcmp

unsigned host_invalidations;
void klog(const char *format, ...) { (void)format; }
_Noreturn void kernel_halt(const char *reason) { fprintf(stderr, "%s\n", reason); abort(); }

enum { RAM_BYTES = 16 * 1024 * 1024, IMAGE_BYTES = 768 };
static unsigned char *ram, *before;
static unsigned refusals;

static void reset(size_t bytes)
{
    memset(ram, 0xa5, RAM_BYTES);
    memset(ram + 0x1000, 0, 4096);
    memset(&kernel, 0, sizeof kernel);
    hhdm_offset = (uint64_t)(uintptr_t)ram;
    LimineMemmapEntry entry = {.base=0x2000, .length=bytes-0x2000, .type=0};
    LimineMemmapEntry *entries[] = {&entry};
    LimineMemmapResponse memory = {.entry_count=1, .entries=entries};
    memory_init(&memory);
    allocations = mappings = take_calls = map_calls = writes = captures = commit_batches = 0;
    fail_take = fail_map = fail_write = fail_capture = fail_commit = -1;
    host_invalidations = 0;
    kernel.app_count = 1;
    kernel.apps[0].target = 9;
}

static Domain domain(void)
{
    Domain d = {.id=9, .key=9, .layout=1, .code_base=CODE_REGION_BASE,
                .stack_top=STACK_REGION_BASE+UINT64_C(0x100000), .task_id=9};
    program_memory_init(&d.program);
    d.program.target = d.id;
    d.program.run = d.task_id;
    d.program.key = d.key;
    return d;
}

static void fixture(unsigned char image[IMAGE_BYTES], uint64_t bias)
{
    memset(image, 0, IMAGE_BYTES);
    ElfHeader h = {.ident={0x7f,'E','L','F',2,1,1}, .type=3, .machine=62, .version=1,
                  .entry=bias, .phoff=64, .ehsize=64, .phentsize=56, .phnum=3};
    ProgramHeader ph[] = {
        {.type=1, .flags=5, .offset=256, .vaddr=bias, .filesz=16, .memsz=4096, .align=4096},
        {.type=1, .flags=6, .offset=512, .vaddr=bias+4096, .filesz=256, .memsz=8192, .align=4096},
        {.type=2, .flags=6, .offset=512, .vaddr=bias+4096, .filesz=80, .memsz=80, .align=8}
    };
    const uint64_t dynamic[][2] = {{7,bias+4096+80}, {8,48}, {9,24}, {0x6ffffff9,2}, {0,0}};
    const uint64_t relocations[][3] = {{bias+4096+192,8,bias+8}, {bias+8190,8,bias+16}};
    memcpy(image, &h, sizeof h); memcpy(image+64, ph, sizeof ph);
    memset(image+256, 0x90, 16);
    memcpy(image+512, dynamic, sizeof dynamic); memcpy(image+592, relocations, sizeof relocations);
}

static void set64(unsigned char *image, size_t offset, uint64_t value)
{
    memcpy(image+offset, &value, sizeof value);
}

static void refused(Domain *d, const unsigned char *image, size_t length)
{
    unsigned char old_frames[sizeof frames];
    ProgramFrames old_program;
    KernelAppPrivate old_app = kernel.apps[0];
    Domain old_domain = *d;
    memcpy(old_frames, &frames, sizeof frames);
    memcpy(&old_program, &program_frames, sizeof old_program);
    memcpy(before, ram, RAM_BYTES);
    unsigned old_allocations = allocations, old_mappings = mappings, old_invalidations = host_invalidations;
    assert(elf_load(d, image, length) == 0);
    assert(allocations == old_allocations && mappings == old_mappings && host_invalidations == old_invalidations);
    assert(!memcmp(old_frames, &frames, sizeof frames));
    assert(!memcmp(&old_program, &program_frames, sizeof old_program));
    assert(!memcmp(&old_app, &kernel.apps[0], sizeof old_app));
    assert(!memcmp(&old_domain, d, sizeof *d));
    assert(!memcmp(before, ram, RAM_BYTES));
    ++refusals;
}

static void assert_rolled_back(Domain *d)
{
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i)
        assert(!program_frames.entries[i].present);
    for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i)
        assert(frames.leases.entries[i].kind != FRAME_LEASE_TRANSIENT);
    assert(program_frame_lent() == 0 && kernel.apps[0].image.count == 0);
    for (uint64_t page = d->code_base; page < d->code_base + 3 * 4096; page += 4096) {
        uint64_t pte = 0;
        assert(!read_pte(page, &pte));
    }
    for (uint64_t page = d->stack_top - USER_STACK_SIZE; page < d->stack_top; page += 4096) {
        uint64_t pte = 0;
        assert(!read_pte(page, &pte));
    }
    assert(frame_lease_audit(&frames.leases));
}

static unsigned rollback_cases;

static void rollback_matrix(const unsigned char *image)
{
    const unsigned boundaries[] = {19, 19, 4, 2, 1};
    for (unsigned kind = 0; kind < 5; ++kind) {
        for (unsigned index = 0; index < boundaries[kind]; ++index) {
            reset(RAM_BYTES);
            Domain d = domain();
            if (kind == 0) fail_take = (int)index;
            if (kind == 1) fail_map = (int)index;
            if (kind == 2) fail_write = (int)index;
            if (kind == 3) fail_capture = (int)index;
            if (kind == 4) fail_commit = (int)index;
            assert(elf_load(&d, image, IMAGE_BYTES) == 0);
            assert_rolled_back(&d);
            ++rollback_cases;
        }
    }
}

static uint64_t mapped_word(uint64_t address)
{
    uint64_t value = 0;
    for (unsigned i=0; i<8; ++i) {
        uint8_t *byte = frame_byte(address+i); assert(byte);
        value |= (uint64_t)*byte << (8*i);
    }
    return value;
}

int main(void)
{
    ram = aligned_alloc(4096, RAM_BYTES); before = malloc(RAM_BYTES); assert(ram && before);
    unsigned char image[IMAGE_BYTES], saved[IMAGE_BYTES];
    reset(RAM_BYTES); Domain d = domain(); fixture(image, 0); memcpy(saved, image, sizeof image);
    refused(&d, NULL, 0);
    for (size_t length=0; length<sizeof(ElfHeader); ++length) refused(&d, image, length);
    /* The pure upstream parser only reads ignored header kinds. BOOT also
       requires their complete entries, as the original loader did. */
    unsigned char short_table[272]={0};
    ElfHeader sh={.ident={0x7f,'E','L','F',2,1,1},.type=3,.machine=62,.version=1,
                  .phoff=64,.ehsize=64,.phentsize=56,.phnum=4};
    ProgramHeader sp={.type=1,.flags=5,.offset=256,.filesz=16,.memsz=4096};
    memcpy(short_table,&sh,sizeof sh);memcpy(short_table+64,&sp,sizeof sp);
    memset(short_table+256,0x90,16);
    assert(!program_image_parse((cesta_read_bytes){short_table,sizeof short_table},DOMAIN_STRIDE).error);
    refused(&d,short_table,sizeof short_table);
    image[0]=0;refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    set64(image, 32, UINT64_MAX);refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    image[68]=7;refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    set64(image, 592, 32);refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    set64(image, 600, (UINT64_C(1)<<32)|8);refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    set64(image, 624, 7);refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    image[256]=0x0f;image[257]=1;image[258]=0xef;
    refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    set64(image, 120+40, 513*UINT64_C(4096));
    refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    kernel.apps[0].captured=true;refused(&d,image,sizeof image);kernel.apps[0].captured=false;
    uint16_t four=4;ProgramHeader stack={.type=0x6474e551,.memsz=USER_STACK_SIZE+1};
    memcpy(image+56,&four,sizeof four);memcpy(image+64+3*56,&stack,sizeof stack);
    refused(&d,image,sizeof image);memcpy(image,saved,sizeof image);
    d.stack_top=d.code_base+USER_STACK_SIZE;refused(&d,image,sizeof image);d=domain();
    d.code_base=UINT64_MAX-4095;refused(&d,image,sizeof image);d=domain();
    /* A relocation must not patch an unrelated mapping, even if it exists. */
    uint64_t outsider=d.code_base+2*DOMAIN_STRIDE;
    uint64_t frame=frames_alloc(0);assert(frame!=NO_ADDRESS);
    assert(map_page(outsider,frame,PTE_USER|PTE_WRITABLE|PTE_NO_EXECUTE,3));
    *frame_byte(outsider)=0x5a;set64(image,592,2*DOMAIN_STRIDE);
    refused(&d,image,sizeof image);assert(*frame_byte(outsider)==0x5a);
    /* Existing mappings in either output range are also refused before effects. */
    memcpy(image,saved,sizeof image);frame=frames_alloc(0);assert(frame!=NO_ADDRESS);
    assert(map_page(d.code_base,frame,PTE_USER|PTE_NO_EXECUTE,3));
    refused(&d,image,sizeof image);
    reset(RAM_BYTES);d=domain();frame=frames_alloc(0);assert(frame!=NO_ADDRESS);
    assert(map_page(d.stack_top-4096,frame,PTE_USER|PTE_WRITABLE|PTE_NO_EXECUTE,9));
    refused(&d,image,sizeof image);

    reset(RAM_BYTES);d=domain();fixture(image,0x200000);memcpy(saved,image,sizeof image);
    assert(elf_load(&d,image,sizeof image)==d.code_base);
    assert(mapped_word(d.code_base+4096+192)==d.code_base+8);
    assert(mapped_word(d.code_base+8190)==d.code_base+16);
    uint64_t first=0,second=0;
    assert(read_pte(d.code_base+4096,&first) && read_pte(d.code_base+8192,&second));
    assert((first&PTE_ADDRESS)+4096!=(second&PTE_ADDRESS));
    assert((first&PTE_NO_EXECUTE) && (first&PTE_WRITABLE) && ((first>>59)&15)==9);
    assert(kernel.apps[0].image.count==19);
    assert(!memcmp(saved,image,sizeof image));

    assert(commit_batches == 1 && frame_lease_audit(&frames.leases));
    assert(program_frame_lent() == 19);
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i)
        if (program_frames.entries[i].present)
            assert(program_frames.entries[i].run == d.task_id &&
                   program_frames.entries[i].lease.holder == d.id &&
                   program_frames.entries[i].lease.kind == FRAME_LEASE_PROGRAM);
    for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i)
        assert(frames.leases.entries[i].kind != FRAME_LEASE_TRANSIENT);
    uint64_t free_pages = frames.leases.free_pages;
    assert(program_memory_main_exit(&d.program) == PROGRAM_MEMORY_OK);
    assert(program_memory_end(&d.program) == PROGRAM_MEMORY_OK);
    if (d.program.phase != PROGRAM_MEMORY_DEAD || program_frame_lent() ||
        frames.leases.free_pages != free_pages + 19)
        fprintf(stderr, "loader run end: phase=%u lent=%llu before=%llu after=%llu expected=%llu\n",
                (unsigned)d.program.phase, (unsigned long long)program_frame_lent(),
                (unsigned long long)free_pages, (unsigned long long)frames.leases.free_pages,
                (unsigned long long)(free_pages + 19));
    assert(d.program.phase == PROGRAM_MEMORY_DEAD);
    assert(!program_frame_lent());
    assert(frames.leases.free_pages == free_pages + 19);
    assert(kernel.apps[0].image.count == 19);
    kernel.apps[0].image.count = 0;
    assert_rolled_back(&d);

    reset(0x3000);d=domain();assert(elf_load(&d,image,sizeof image)==0);
    assert(allocations && mappings);
    assert_rolled_back(&d);
    rollback_matrix(image);
    printf("loader: %u refusals before page effects; nonzero bias, cross-page relocation, manifests and %u rollback failures passed\n",
           refusals, rollback_cases);
    free(before);free(ram);return 0;
}
