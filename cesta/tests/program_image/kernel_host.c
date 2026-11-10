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
#include "kernel_memory.c"
#undef frames_alloc
#undef map_page

static unsigned allocations, mappings;
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
#include "kernel_domains.c"
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
    allocations = mappings = host_invalidations = 0;
    kernel.app_count = 1;
    kernel.apps[0].target = 9;
}

static Domain domain(void)
{
    return (Domain){.id=9, .key=9, .layout=1, .code_base=CODE_REGION_BASE,
                    .stack_top=STACK_REGION_BASE+UINT64_C(0x100000)};
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
    KernelAppPrivate old_app = kernel.apps[0];
    Domain old_domain = *d;
    memcpy(old_frames, &frames, sizeof frames); memcpy(before, ram, RAM_BYTES);
    unsigned old_allocations = allocations, old_mappings = mappings, old_invalidations = host_invalidations;
    assert(elf_load(d, image, length) == 0);
    assert(allocations == old_allocations && mappings == old_mappings && host_invalidations == old_invalidations);
    assert(!memcmp(old_frames, &frames, sizeof frames));
    assert(!memcmp(&old_app, &kernel.apps[0], sizeof old_app));
    assert(!memcmp(&old_domain, d, sizeof *d));
    assert(!memcmp(before, ram, RAM_BYTES));
    ++refusals;
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

    /* Allocation failure is a separate boundary: admission can succeed but
       fail to materialize the plan. This control makes no rollback claim. */
    reset(0x3000);d=domain();assert(elf_load(&d,image,sizeof image)==0);
    assert(allocations && mappings);
    printf("loader: %u refusals before page effects; nonzero bias, cross-page relocation, manifests and resource failure passed\n",refusals);
    free(before);free(ram);return 0;
}
