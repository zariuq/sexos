#include "sexos_kernel.h"
/*Physical frames, the one page table of the single address space, the user virtual-address cursor, and checked access to user memory. Translated from the files of kernel/src/memory and the page walks of pku.rs and syscalls/mod.rs. The reference maps a 256 MiB kernel heap page by page and runs a lock-free buddy allocator beside it; this kernel needs no heap, so frames come from one cursor over the usable regions, aligned to their block size as the buddy allocator aligns them, and are zeroed before use.*/
#define FRAME_REGIONS 64u
/*The reference's first user virtual address went to its per-core message page; the cursor starts one page later so every later address is the same.*/
#define USER_VA_BASE UINT64_C(0x400000000000)
#define USER_VA_FIRST (USER_VA_BASE + KERNEL_PAGE_SIZE)
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
}
Frames;
uint64_t hhdm_offset = 0;
static Frames frames;
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
  for (uint64_t i = 0; i < map->entry_count; ++i) {
    const LimineMemmapEntry *e = map->entries[i];
    if (e->type != 0 || frames.count >= FRAME_REGIONS) continue;
    uint64_t base = align_up(e->base < 0x1000 ? 0x1000 : e->base, KERNEL_PAGE_SIZE);
    uint64_t end = (e->base + e->length) & ~(KERNEL_PAGE_SIZE - 1);
    if (base >= end) continue;
    frames.regions[frames.count] = (FrameRegion) {
      .base = base,.end = end
    };
    frames.count += 1;
  }
  frames.index = 0;
  frames.next = frames.count > 0 ? frames.regions[0].base : 0;
  frames.va_next = USER_VA_FIRST;
  frames.allocated = 0;
  uint64_t total = 0;
  for (uint32_t i = 0; i < frames.count; ++i) total += (frames.regions[i].end - frames.regions[i].base) / KERNEL_PAGE_SIZE;
  klog("allocator.init.done regions=%u usable_frames=%llu\n", frames.count, (unsigned long long) total);
}
/*A zeroed block of 2^order frames aligned to its size, or NO_ADDRESS. Nothing is freed: the reference never frees a user block either.*/
uint64_t frames_alloc(uint32_t order) {
  if (order >= 52) return NO_ADDRESS;
  uint64_t size = KERNEL_PAGE_SIZE << order;
  uint32_t index = frames.index;
  uint64_t next = frames.next;
  while (index < frames.count) {
    uint64_t candidate = next <= UINT64_MAX - (size - 1) ? align_up(next, size) : UINT64_MAX;
    uint64_t end = frames.regions[index].end;
    if (candidate <= end && size <= end - candidate) {
      frames.index = index;
      frames.next = candidate + size;
      frames.allocated += (uint64_t) 1 << order;
      memset(phys_to_virt(candidate), 0, size);
      return candidate;
    }
    index += 1;
    if (index < frames.count) next = frames.regions[index].base;
  }
  return NO_ADDRESS;
}
/*allocate_va of the reference: page-aligned addresses from one ascending cursor.*/
uint64_t va_alloc(uint64_t size) {
  if (size > UINT64_MAX - (KERNEL_PAGE_SIZE - 1)) return NO_ADDRESS;
  uint64_t rounded = align_up(size, KERNEL_PAGE_SIZE);
/*The heap cursor stays below the domain stacks and never wraps.*/
  uint64_t limit = 0x700000000000ULL;
  if (frames.va_next > limit || rounded > limit - frames.va_next) return NO_ADDRESS;
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
/*Map one 4 KiB page with the given flags and protection key. A page that is already mapped is refused, as the reference's mapper refuses it.*/
bool map_page(uint64_t va, uint64_t phys, uint64_t flags, uint8_t key) {
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
