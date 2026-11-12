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
/*Physical frames, the one page table of the single address space, the user virtual-address cursor, and checked access to user memory. BOOT blocks remain pinned; program frames carry exact leases and a tracked mapping before they can be reused.*/
#define FRAME_REGIONS 64u
#define PROGRAM_FRAME_RESERVE 4096u
/*The reference's first user virtual address went to its per-core message page; the cursor starts one page later so every later address is the same.*/
#define USER_VA_BASE UINT64_C(0x400000000000)
#define USER_VA_FIRST (USER_VA_BASE + KERNEL_PAGE_SIZE)
#define USER_VA_END UINT64_C(0x444444440000)
typedef struct FrameRegion {
  uint64_t base;
  uint64_t end;
}
FrameRegion;
typedef struct Frames {
  FrameRegion regions[FRAME_REGIONS];
  uint32_t count;
  uint32_t index;
  uint64_t next;
  uint64_t va_next;
  uint64_t allocated;
  uint32_t failed;
  FrameLeaseState leases;
}
Frames;
typedef struct ProgramFrameBinding {
  FrameLease lease;
  uint64_t leaf;
  uint64_t va;
  uint64_t expected;
  uint32_t run;
  bool present;
  bool mapped;
  bool returning;
}
ProgramFrameBinding;
typedef struct ProgramFrames {
  ProgramFrameBinding entries[PROGRAM_FRAME_CAPACITY];
  bool map_active;
  uint64_t base;
  uint64_t level;
}
ProgramFrames;
uint64_t hhdm_offset = 0;
static Frames frames;
static ProgramFrames program_frames;
/*The freestanding compiler may call these four for structure copies. Compile with -ffreestanding and -fno-builtin; the memory-primitives gate exercises the actual compiled definitions against libc.*/
void *memcpy(void *destination, const void *source, size_t count) {
  uint8_t *d = destination;
  const uint8_t *s = source;
  for (size_t i = 0; i < count; ++i) d[i] = s[i];
  return destination;
}
void *memmove(void *destination, const void *source, size_t count) {
  uint8_t *d = destination;
  const uint8_t *s = source;
  if ((uintptr_t) d < (uintptr_t) s) for (size_t i = 0; i < count; ++i) d[i] = s[i];
  else for (size_t i = count; i > 0; --i) d[i - 1] = s[i - 1];
  return destination;
}
void *memset(void *destination, int value, size_t count) {
  uint8_t *d = destination;
  for (size_t i = 0; i < count; ++i) d[i] = (uint8_t) value;
  return destination;
}
int memcmp(const void *first, const void *second, size_t count) {
  const uint8_t *a = first;
  const uint8_t *b = second;
  for (size_t i = 0; i < count; ++i) if (a[i] != b[i]) return (int) a[i] - (int) b[i];
  return 0;
}
void *kmemset(void *destination, int value, size_t count) {
  return memset(destination, value, count);
}
void *kmemcpy(void *destination, const void *source, size_t count) {
  return memcpy(destination, source, count);
}
void *phys_to_virt(uint64_t phys) {
  return (void *)(uintptr_t)(phys + hhdm_offset);
}
static uint64_t align_up(uint64_t value, uint64_t alignment) {
  return (value + (alignment - 1)) & ~(alignment - 1);
}
/*Usable regions only: the bootloader-reclaimable ones hold the responses the kernel keeps reading, and the module images.*/
void memory_init(const LimineMemmapResponse *map) {
  frames.count = 0;
  frames.failed = 0;
  frame_lease_init(&frames.leases, PROGRAM_FRAME_RESERVE);
  for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i) program_frames.entries[i] = (ProgramFrameBinding) {
    .present = false
  };
  program_frames.map_active = false;
  program_frames.base = 0;
  program_frames.level = 0;
  for (uint64_t i = 0; i < map->entry_count; ++i) {
    const LimineMemmapEntry *e = map->entries[i];
    if (e->type != 0 || frames.count >= FRAME_REGIONS) continue;
    uint64_t base = align_up(e->base < 0x1000 ? 0x1000 : e->base, KERNEL_PAGE_SIZE);
    uint64_t end = (e->base + e->length) & ~(KERNEL_PAGE_SIZE - 1);
    if (base >= end) continue;
    uint64_t status = frame_lease_region(&frames.leases, base, (end - base) / KERNEL_PAGE_SIZE);
    if (status != FRAME_LEASE_OK) frames.failed = 1;
    else {
      frames.regions[frames.count] = (FrameRegion) {
        .base = base,.end = end
      };
      frames.count += 1;
    }
  }
  frames.index = 0;
  frames.next = frames.count > 0 ? frames.regions[0].base : 0;
  frames.va_next = USER_VA_FIRST;
  frames.allocated = 0;
  uint64_t total = 0;
  for (uint32_t i = 0; i < frames.count; ++i) total += (frames.regions[i].end - frames.regions[i].base) / KERNEL_PAGE_SIZE;
  klog("allocator.init.done regions=%u usable_frames=%llu\n", frames.count, (unsigned long long) total);
}

uint64_t frames_alloc(uint32_t order) {
    if (order > FRAME_LEASE_MAX_ORDER || frames.failed) return NO_ADDRESS;
    FrameLeaseResult lease = frame_lease_take(&frames.leases, (uint8_t)order,
                                               FRAME_LEASE_PINNED, 0);
    if (lease.status != FRAME_LEASE_OK) return NO_ADDRESS;
    uint64_t size = KERNEL_PAGE_SIZE << order;
    frames.allocated += (uint64_t)1 << order;
    memset(phys_to_virt(lease.value.base), 0, size);
    return lease.value.base;
}

static bool program_frame_same(FrameLease a, FrameLease b) {
    return a.base == b.base && a.holder == b.holder &&
           a.incarnation == b.incarnation && a.order == b.order && a.kind == b.kind;
}

static size_t program_frame_find(FrameLease lease) {
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i)
        if (program_frames.entries[i].present &&
            program_frame_same(program_frames.entries[i].lease, lease)) return i;
    return PROGRAM_FRAME_CAPACITY;
}

static bool program_frame_blocked(uint64_t physical) {
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i)
        if (program_frames.entries[i].present &&
            program_frames.entries[i].lease.base == physical) return true;
    return false;
}

static FrameLeaseResult program_frame_take_kind(uint32_t holder, uint8_t kind) {
    size_t slot = PROGRAM_FRAME_CAPACITY;
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i)
        if (!program_frames.entries[i].present) { slot = i; break; }
    if (slot == PROGRAM_FRAME_CAPACITY) {
        klog("[program.frame.binding.full] holder=%u kind=%u capacity=%zu\n",
             holder, kind, (size_t)PROGRAM_FRAME_CAPACITY);
        return (FrameLeaseResult){.status = FRAME_LEASE_FULL};
    }
    if (frames.leases.count == FRAME_LEASE_CAPACITY) {
        size_t empty = 0, free = 0, live = 0, detached = 0, pinned = 0, program = 0, transient = 0;
        for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i) {
            const FrameLeaseEntry *entry = &frames.leases.entries[i];
            if (entry->phase == FRAME_LEASE_EMPTY) { ++empty; continue; }
            if (entry->phase == FRAME_LEASE_FREE) ++free;
            if (entry->phase == FRAME_LEASE_LIVE) ++live;
            if (entry->phase == FRAME_LEASE_DETACHED) ++detached;
            if (entry->kind == FRAME_LEASE_PINNED) ++pinned;
            if (entry->kind == FRAME_LEASE_PROGRAM) ++program;
            if (entry->kind == FRAME_LEASE_TRANSIENT) ++transient;
        }
        klog("[program.frame.core.census] empty=%zu free=%zu live=%zu detached=%zu pinned=%zu program=%zu transient=%zu\n",
             empty, free, live, detached, pinned, program, transient);
    }
    FrameLeaseResult result = frame_lease_take(&frames.leases, 0, kind, holder);
    if (result.status != FRAME_LEASE_OK) {
        klog("[program.frame.core.refused] holder=%u kind=%u status=%u count=%zu free=%llu\n",
             holder, kind, (unsigned)result.status, frames.leases.count,
             (unsigned long long)frames.leases.free_pages);
        return result;
    }
    memset(phys_to_virt(result.value.base), 0, KERNEL_PAGE_SIZE);
    program_frames.entries[slot] = (ProgramFrameBinding){
        .lease = result.value, .present = true};
    return result;
}

FrameLeaseResult program_frame_take(uint32_t holder) {
    return program_frame_take_kind(holder, FRAME_LEASE_PROGRAM);
}

FrameLeaseResult program_frame_take_transient(uint32_t holder) {
    return program_frame_take_kind(holder, FRAME_LEASE_TRANSIENT);
}

static size_t program_frame_core(FrameLease lease) {
    for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i) {
        FrameLeaseEntry *entry = &frames.leases.entries[i];
        if (entry->phase == FRAME_LEASE_LIVE && entry->kind == FRAME_LEASE_TRANSIENT &&
            entry->base == lease.base && entry->order == lease.order &&
            entry->holder == lease.holder && entry->incarnation == lease.incarnation)
            return i;
    }
    return FRAME_LEASE_CAPACITY;
}

uint64_t program_frame_commit_all(FrameLease *leases, size_t count) {
    if (!leases || !count || count > APP_PRIVATE_MAX_PAGES) return FRAME_LEASE_INVALID;
    size_t bindings[APP_PRIVATE_MAX_PAGES], cores[APP_PRIVATE_MAX_PAGES];
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = 0; j < i; ++j)
            if (program_frame_same(leases[i], leases[j])) return FRAME_LEASE_DENIED;
        bindings[i] = program_frame_find(leases[i]);
        cores[i] = program_frame_core(leases[i]);
        if (bindings[i] == PROGRAM_FRAME_CAPACITY || cores[i] == FRAME_LEASE_CAPACITY ||
            program_frames.entries[bindings[i]].returning) return FRAME_LEASE_DENIED;
    }
    for (size_t i = 0; i < count; ++i) {
        ProgramFrameBinding *binding = &program_frames.entries[bindings[i]];
        if (!binding->run) {
            frames.leases.entries[cores[i]].kind = FRAME_LEASE_PINNED;
            frames.leases.entries[cores[i]].holder = 0;
            *binding = (ProgramFrameBinding){0};
            continue;
        }
        frames.leases.entries[cores[i]].kind = FRAME_LEASE_PROGRAM;
        frames.leases.lent += frames.leases.entries[cores[i]].pages;
        leases[i].kind = FRAME_LEASE_PROGRAM;
        binding->lease.kind = FRAME_LEASE_PROGRAM;
    }
    return FRAME_LEASE_OK;
}

uint64_t program_frame_commit(FrameLease lease) {
    size_t binding = program_frame_find(lease);
    if (binding == PROGRAM_FRAME_CAPACITY || program_frames.entries[binding].run)
        return FRAME_LEASE_DENIED;
    return program_frame_commit_all(&lease, 1);
}

uint64_t program_frame_available(void) {
    return frame_lease_available(&frames.leases);
}

uint64_t program_frame_lent(void) {
    return frames.leases.lent;
}

static uint64_t program_pressure_level(uint64_t free, uint64_t base) {
    uint64_t critical = base / 32 > 2048 ? base / 32 : 2048;
    if (free < critical) return 2;
    uint64_t low = base / 8 > 8192 ? base / 8 : 8192;
    return free < low ? 1 : 0;
}

uint64_t program_frame_pressure(void) {
    uint64_t free = program_frame_available();
    if (free > program_frames.base) program_frames.base = free;
    uint64_t base = program_frames.base;
    uint64_t now = program_frames.level;
    uint64_t current = program_pressure_level(free, base);
    uint64_t falling = program_pressure_level(free - free / 5, base);
    uint64_t level = current > (now < falling ? now : falling) ? current :
                     (now < falling ? now : falling);
    program_frames.level = level;
    return level;
}

/*allocate_va of the reference: page-aligned addresses from one ascending cursor.*/
uint64_t va_alloc(uint64_t size) {
/*A reservation names at least one page and never wraps; a misaligned cursor reserves nothing.*/
  if (size == 0 || size > UINT64_MAX - (KERNEL_PAGE_SIZE - 1)) return NO_ADDRESS;
  if (frames.va_next < USER_VA_BASE || frames.va_next >= USER_VA_END || frames.va_next % KERNEL_PAGE_SIZE != 0) return NO_ADDRESS;
  uint64_t rounded = align_up(size, KERNEL_PAGE_SIZE);
/*The shared reservation range ends before the kernel heap and program records. Reserving an address does not map a frame or grant authority.*/
  if (rounded > USER_VA_END - frames.va_next) return NO_ADDRESS;
  uint64_t va = frames.va_next;
  frames.va_next += rounded;
  return va;
}
static uint64_t *table_at(uint64_t phys) {
  return (uint64_t *) phys_to_virt(phys & PTE_ADDRESS);
}
/*The table an entry points to, created when absent. Entries above a user page carry the user bit, which permits nothing by itself: the leaf decides. A large page has no table below it.*/
static uint64_t *next_table(uint64_t *table, uint64_t index, bool user, bool create) {
  uint64_t entry = table[index];
  if ((entry & PTE_PRESENT) != 0) {
    if ((entry & PTE_HUGE) != 0) return NULL;
    if (user && (entry & PTE_USER) == 0) table[index] |= PTE_USER;
    return table_at(entry);
  }
  if (!create) return NULL;
  uint64_t phys = frames_alloc(0);
  if (phys == NO_ADDRESS) return NULL;
  table[index] = phys | (PTE_PRESENT | (PTE_WRITABLE | (user ? PTE_USER : 0)));
  return table_at(phys);
}
static uint64_t *root_table(void) {
  return table_at(cpu_read_cr3());
}
static uint64_t table_index(uint64_t va, unsigned level) {
  return va >> (12 + 9 * level) & 511;
}
/*Map one 4 KiB page with the given flags and protection key. A page that is already mapped is refused, as the reference's mapper refuses it. Program frames use their single-tracking wrapper.*/
bool map_page(uint64_t va, uint64_t phys, uint64_t flags, uint8_t key) {
  if (program_frame_blocked(phys) && !program_frames.map_active) return false;
  bool user = (flags & PTE_USER) != 0;
  uint64_t *table = root_table();
  for (unsigned level = 3; level > 0; --level) {
    table = next_table(table, table_index(va, level), user, true);
    if (table == NULL) return false;
  }
  uint64_t *pte = &table[table_index(va, 0)];
  if ((*pte & PTE_PRESENT) != 0) return false;
  *pte = (phys & PTE_ADDRESS) | (flags | (PTE_PRESENT | (uint64_t)(key & 15) << PTE_KEY_SHIFT));
  cpu_invlpg(va);
  return true;
}

uint64_t page_leaf_address(uint64_t va) {
    uint64_t *table = root_table();
    for (unsigned level = 3; level > 0; --level) {
        uint64_t entry = table[table_index(va, level)];
        if (!(entry & PTE_PRESENT) || (entry & PTE_HUGE))
            return 0;
        table = table_at(entry);
    }
    return (uint64_t)(uintptr_t)&table[table_index(va, 0)];
}

bool prepare_page_table(uint64_t va, uint8_t key) {
    if (va % KERNEL_PAGE_SIZE || !key || key >= 16) return false;
    uint64_t *table = root_table();
    for (unsigned level = 3; level > 0; --level) {
        table = next_table(table, table_index(va, level), true, true);
        if (!table) return false;
    }
    uint64_t *leaf = &table[table_index(va, 0)];
    return !*leaf;
}

bool image_write(uint64_t address, const uint8_t *source, uint64_t count) {
    while (count) {
        uint64_t chunk = KERNEL_PAGE_SIZE - (address & UINT64_C(4095));
        if (chunk > count) chunk = count;
        uint8_t *target = frame_byte(address);
        if (!target) return false;
        memcpy(target, source, (size_t)chunk);
        address += chunk;
        source += chunk;
        count -= chunk;
    }
    return true;
}

bool program_frame_map_run(FrameLease lease, uint32_t run, uint64_t va,
                           uint64_t flags, uint8_t key) {
    size_t slot = program_frame_find(lease);
    if (slot == PROGRAM_FRAME_CAPACITY || va % KERNEL_PAGE_SIZE) return false;
    ProgramFrameBinding *binding = &program_frames.entries[slot];
    if (binding->mapped || binding->returning) return false;
    program_frames.map_active = true;
    bool mapped = map_page(va, lease.base, flags, key);
    program_frames.map_active = false;
    if (!mapped) return false;
    binding->run = run;
    binding->va = va;
    binding->leaf = page_leaf_address(va);
    if (!binding->leaf) {
        binding->mapped = true;
        binding->returning = true;
        return false;
    }
    binding->expected = *(volatile uint64_t *)(uintptr_t)binding->leaf;
    binding->mapped = true;
    return true;
}

bool program_frame_map(FrameLease lease, uint64_t va, uint64_t flags, uint8_t key) {
    return program_frame_map_run(lease, 0, va, flags, key);
}

static size_t program_frame_program_core(FrameLease lease) {
    for (size_t i = 0; i < FRAME_LEASE_CAPACITY; ++i) {
        FrameLeaseEntry *entry = &frames.leases.entries[i];
        if (entry->phase == FRAME_LEASE_LIVE && entry->kind == FRAME_LEASE_PROGRAM &&
            entry->base == lease.base && entry->order == lease.order &&
            entry->holder == lease.holder && entry->incarnation == lease.incarnation)
            return i;
    }
    return FRAME_LEASE_CAPACITY;
}

static size_t program_frame_handover_slot(const AppPrivatePage *page, uint32_t target, uint32_t run) {
    if (!page || !target || !run || !page->va || page->va % KERNEL_PAGE_SIZE || !page->leaf ||
        (page->expected & 5) != 5 || page->leaf != page_leaf_address(page->va))
        return PROGRAM_FRAME_CAPACITY;
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i) {
        ProgramFrameBinding *binding = &program_frames.entries[i];
        if (!binding->present || !binding->mapped || binding->returning ||
            binding->lease.kind != FRAME_LEASE_PROGRAM || binding->lease.holder != target ||
            binding->run != run || binding->va != page->va || binding->leaf != page->leaf ||
            binding->lease.base != (page->expected & APP_PRIVATE_PHYSICAL) ||
            (binding->expected & ~UINT64_C(0x60)) != (page->expected & ~UINT64_C(0x60)))
            continue;
        return program_frame_program_core(binding->lease) == FRAME_LEASE_CAPACITY ?
               PROGRAM_FRAME_CAPACITY : i;
    }
    return PROGRAM_FRAME_CAPACITY;
}

bool program_frame_workspace_map(const AppPrivatePage *image, uint32_t target, uint32_t run,
                                 uint64_t address, uint8_t key) {
    size_t slot = program_frame_handover_slot(image, target, run);
    if (slot == PROGRAM_FRAME_CAPACITY || !key || key >= 16 || address % KERNEL_PAGE_SIZE ||
        program_frames.map_active)
        return false;
    ProgramFrameBinding *binding = &program_frames.entries[slot];
    uint64_t actual = *(volatile uint64_t *)(uintptr_t)binding->leaf;
    if (actual && (actual & ~UINT64_C(0x60)) != (binding->expected & ~UINT64_C(0x60)))
        return false;
    program_frames.map_active = true;
    bool mapped = map_page(address, binding->lease.base,
                           PTE_USER | PTE_WRITABLE | PTE_NO_EXECUTE, key);
    program_frames.map_active = false;
    return mapped;
}

bool program_frame_handover_check(const AppPrivatePages *image, const AppPrivatePages *workspace,
                                  uint32_t target, uint32_t run, uint32_t holder) {
    if (!image || !workspace || !target || !run || !holder || holder == target ||
        !image->count || image->count > APP_PRIVATE_MAX_PAGES ||
        workspace->count != image->count)
        return false;
    for (size_t i = 0; i < image->count; ++i) {
        AppPrivatePage old = image->pages[i], next = workspace->pages[i];
        size_t slot = program_frame_handover_slot(&old, target, run);
        if (slot == PROGRAM_FRAME_CAPACITY || !next.va || next.va % KERNEL_PAGE_SIZE || !next.leaf ||
            (next.expected & 7) != 7 || !(next.expected & PTE_NO_EXECUTE) ||
            next.leaf != page_leaf_address(next.va) ||
            (next.expected & APP_PRIVATE_PHYSICAL) != program_frames.entries[slot].lease.base ||
            (*(volatile uint64_t *)(uintptr_t)old.leaf & ~UINT64_C(0x60)) !=
                (old.expected & ~UINT64_C(0x60)) ||
            *(volatile uint64_t *)(uintptr_t)next.leaf)
            return false;
        for (size_t j = 0; j < i; ++j)
            if (workspace->pages[j].va == next.va) return false;
    }
    return true;
}

bool program_frame_handover_commit(const AppPrivatePages *image, const AppPrivatePages *workspace,
                                   uint32_t target, uint32_t run, uint32_t holder) {
    if (!image || !workspace || !target || !run || !holder || holder == target ||
        !image->count || image->count > APP_PRIVATE_MAX_PAGES ||
        workspace->count != image->count)
        return false;
    for (size_t i = 0; i < image->count; ++i) {
        AppPrivatePage old = image->pages[i], next = workspace->pages[i];
        size_t slot = program_frame_handover_slot(&old, target, run);
        if (slot == PROGRAM_FRAME_CAPACITY || !next.leaf ||
            (next.expected & APP_PRIVATE_PHYSICAL) != program_frames.entries[slot].lease.base ||
            *(volatile uint64_t *)(uintptr_t)old.leaf ||
            (*(volatile uint64_t *)(uintptr_t)next.leaf & ~UINT64_C(0x60)) !=
                (next.expected & ~UINT64_C(0x60)))
            return false;
        for (size_t j = 0; j < i; ++j)
            if (workspace->pages[j].va == next.va) return false;
        ProgramFrameBinding *binding = &program_frames.entries[slot];
        size_t core = program_frame_program_core(binding->lease);
        frames.leases.entries[core].holder = holder;
        binding->lease.holder = holder;
        binding->run = 0;
        binding->va = next.va;
        binding->leaf = next.leaf;
        binding->expected = next.expected;
    }
    return true;
}

uint64_t program_frame_detach(FrameLease lease) {
    size_t slot = program_frame_find(lease);
    if (slot == PROGRAM_FRAME_CAPACITY) return FRAME_LEASE_DENIED;
    ProgramFrameBinding *binding = &program_frames.entries[slot];
    if (binding->mapped) {
        if (!binding->leaf) return FRAME_LEASE_DENIED;
        volatile uint64_t *leaf = (volatile uint64_t *)(uintptr_t)binding->leaf;
        uint64_t actual = *leaf;
        if ((actual & ~UINT64_C(0x60)) != (binding->expected & ~UINT64_C(0x60)))
            return FRAME_LEASE_DENIED;
        *leaf = 0;
        cpu_invlpg(binding->va);
        if (*leaf) {
            binding->returning = true;
            return FRAME_LEASE_RETRY;
        }
        binding->mapped = false;
    }
    binding->returning = true;
    return frame_lease_detach(&frames.leases, lease);
}

uint64_t program_frame_give(FrameLease lease) {
    size_t slot = program_frame_find(lease);
    if (slot == PROGRAM_FRAME_CAPACITY) return FRAME_LEASE_DENIED;
    ProgramFrameBinding *binding = &program_frames.entries[slot];
    if (binding->mapped) return FRAME_LEASE_RETRY;
    binding->returning = true;
    uint64_t status = frame_lease_give(&frames.leases, lease);
    if (status == FRAME_LEASE_RETRY) {
        status = frame_lease_detach(&frames.leases, lease);
        if (status == FRAME_LEASE_OK)
            status = frame_lease_give(&frames.leases, lease);
    }
    if (status == FRAME_LEASE_OK) *binding = (ProgramFrameBinding){0};
    return status;
}

bool program_frame_release(FrameLease lease) {
    uint64_t status = program_frame_give(lease);
    if (status == FRAME_LEASE_RETRY) {
        status = program_frame_detach(lease);
        if (status == FRAME_LEASE_OK)
            status = program_frame_give(lease);
    }
    return status == FRAME_LEASE_OK;
}

bool program_frame_release_run(uint32_t target, uint32_t run) {
    if (!target || !run) return false;
    uint64_t first = 0, pages = 0;
    for (size_t i = 0; i < PROGRAM_FRAME_CAPACITY; ++i) {
        ProgramFrameBinding *binding = &program_frames.entries[i];
        if (!binding->present || binding->lease.holder != target || binding->run != run)
            continue;
        uint64_t base = binding->lease.base, count = UINT64_C(1) << binding->lease.order;
        if (!program_frame_release(binding->lease)) return false;
        if (!first || base < first) first = base;
        pages += count;
    }
    if (pages)
        klog("[program.frame.released] target=%u run=%u first=%#llx pages=%llu\n",
             target, run, (unsigned long long)first, (unsigned long long)pages);
    return true;
}

/*The entry that maps va, a large page's entry included.*/
bool read_pte(uint64_t va, uint64_t *pte) {
  uint64_t *table = root_table();
  for (unsigned level = 3; level > 0; --level) {
    uint64_t entry = table[table_index(va, level)];
    if ((entry & PTE_PRESENT) == 0) return false;
    if (level < 3 && (entry & PTE_HUGE) != 0) {
      *pte = entry;
      return true;
    }
    table = table_at(entry);
  }
  uint64_t leaf = table[table_index(va, 0)];
  if ((leaf & PTE_PRESENT) == 0) return false;
  *pte = leaf;
  return true;
}
/*Replace a large-page entry by a table of the next smaller pages with the same attributes. A 4 KiB entry keeps the PAT bit at bit 7, where a large one keeps it at bit 12.*/
static bool split_entry(uint64_t *entry, unsigned level) {
  uint64_t old = *entry;
  uint64_t phys = frames_alloc(0);
  if (phys == NO_ADDRESS) return false;
  uint64_t *table = table_at(phys);
  uint64_t step = KERNEL_PAGE_SIZE << 9 * (level - 1);
  uint64_t frame_mask = PTE_ADDRESS & ~((KERNEL_PAGE_SIZE << 9 * level) - 1);
  uint64_t base = old & frame_mask;
  uint64_t attributes = old & ~frame_mask;
  if (level == 1) {
    uint64_t pat = old >> 12 & 1;
    attributes = (attributes & ~(PTE_HUGE | 1 << 12)) | pat << 7;
  }
  for (uint64_t i = 0; i < 512; ++i) table[i] = (base + i * step) | attributes;
  *entry = phys | (PTE_PRESENT | (PTE_WRITABLE | (old & PTE_USER)));
  klog("[kernel.map.split] large_kib=%llu\n", (unsigned long long)((KERNEL_PAGE_SIZE << 9 * level) / 1024));
  return true;
}
/*Make exactly the 4 KiB pages of [va, va + length) user-accessible and writable under key. The reference sets the user bit on whatever entry maps each framebuffer page, so where the higher-half map uses a 1 GiB or 2 MiB page the whole large page, with the local and I/O APIC and the PCI windows beside the framebuffer, became reachable from user mode. Large pages are split first.*/
bool expose_to_user(uint64_t va, uint64_t length, uint8_t key) {
  uint64_t start = va & ~(KERNEL_PAGE_SIZE - 1);
  uint64_t end = align_up(va + length, KERNEL_PAGE_SIZE);
  for (uint64_t page = start; page < end; page += KERNEL_PAGE_SIZE) {
    uint64_t *table = root_table();
    for (unsigned level = 3; level > 0; --level) {
      uint64_t *entry = &table[table_index(page, level)];
      if ((*entry & PTE_PRESENT) == 0) return false;
      if (level < 3 && (*entry & PTE_HUGE) != 0) if (!split_entry(entry, level)) return false;
      *entry |= PTE_USER;
      table = table_at(*entry);
    }
    uint64_t *pte = &table[table_index(page, 0)];
    if ((*pte & PTE_PRESENT) == 0) return false;
    *pte = (*pte & ~((uint64_t) 15 << PTE_KEY_SHIFT)) | (PTE_USER | (PTE_WRITABLE | (PTE_NO_EXECUTE | (uint64_t)(key & 15) << PTE_KEY_SHIFT)));
    cpu_invlpg(page);
  }
  return true;
}
static bool canonical(uint64_t address) {
  uint64_t high = address >> 47;
  return high == 0 || high == 0x1ffff;
}
/*validate_user_bytes of the reference: every page of the range is present, user-accessible, writable when asked, and open under the domain's key register.*/
bool user_range_ok(uint64_t start, uint64_t length, bool write, uint32_t mask) {
  if (length == 0) return true;
  if (!canonical(start)) return false;
  uint64_t last = start + (length - 1);
  if (last < start || !canonical(last)) return false;
  for (uint64_t page = start & ~(KERNEL_PAGE_SIZE - 1); true; page += KERNEL_PAGE_SIZE) {
    uint64_t pte = 0;
    if (!read_pte(page, &pte)) return false;
    if ((pte & PTE_USER) == 0) return false;
    if (write && (pte & PTE_WRITABLE) == 0) return false;
    unsigned shift = 2 * (unsigned)(pte >> PTE_KEY_SHIFT & 15);
    if ((mask >> shift & 1) != 0) return false;
    if (write && (mask >> (shift + 1) & 1) != 0) return false;
    if (page == (last & ~(KERNEL_PAGE_SIZE - 1))) break;
  }
  return true;
}
/*The only places the kernel touches user memory on a domain's behalf. Each checks the range against the domain's own rights first; with SMAP present, the processor refuses every other kernel access to a user page.*/
bool copy_from_user(void *destination, uint64_t source, size_t length, uint32_t mask) {
  if (!user_range_ok(source, length, false, mask)) return false;
  if (kernel.smap) cpu_stac();
  memcpy(destination, (const void *)(uintptr_t) source, length);
  if (kernel.smap) cpu_clac();
  return true;
}
bool copy_to_user(uint64_t destination, const void *source, size_t length, uint32_t mask) {
  if (!user_range_ok(destination, length, true, mask)) return false;
  if (kernel.smap) cpu_stac();
  memcpy((void *)(uintptr_t) destination, source, length);
  if (kernel.smap) cpu_clac();
  return true;
}
