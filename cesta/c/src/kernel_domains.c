#include "sexos_kernel.h"
/*Protection domains: their capability tables and rings, loading their images, the boot genesis, device leases, the framebuffer hand-off, and the scheduler. Translated from kernel/src/init.rs, pd/create.rs, loader/elf.rs, elf.rs, capability.rs, devmgr.rs, graphics/gpu_lease.rs and scheduler.rs.*/
#define CODE_REGION_BASE UINT64_C(0x40000000)
#define STACK_REGION_BASE UINT64_C(0x700000000000)
#define DOMAIN_STRIDE UINT64_C(0x01000000)
#define USER_STACK_SIZE UINT64_C(0x10000)
/*The reference's ProtectionDomain::new grants three capabilities by insertion: a mock VFS node, the domain's own ring, a mock signal interrupt. Only the ring at slot 2 can be used; the other two answer every request as an empty slot does, so they are not kept.*/
#define SELF_RING_SLOT 2u
/*The first slot insertion gives after those three, where devmgr.rs inserts the display controller's lease.*/
#define FIRST_INSERTED_SLOT 4u
#define SLOT_USB_HOST 8u
#define SLOT_NVME_HOST 16u
#define SLOT_NIC 19u
#define NVME_VECTOR 0x22u
#define DISPLAY_VECTOR 0x23u
#define MAX_PCI_FUNCTIONS 64u
#define PF_X 1u
#define PF_W 2u
#define PT_LOAD 1u
#define PT_DYNAMIC 2u
enum KernelDomainKey {
#define KERNEL_DOMAIN_KEY(id, name, module, log_name) KERNEL_DOMAIN_KEY_##name = id,
SEXOS_KERNEL_DOMAIN_ROWS(KERNEL_DOMAIN_KEY)
#undef KERNEL_DOMAIN_KEY
};

typedef struct GenesisDomain { uint8_t key; const char *module; const char *log_name; } GenesisDomain;

static const GenesisDomain genesis_domains[] = {
#define KERNEL_DOMAIN_ROW(id, name, module, log_name) { (uint8_t)(id), module, log_name },
SEXOS_KERNEL_DOMAIN_ROWS(KERNEL_DOMAIN_ROW)
#undef KERNEL_DOMAIN_ROW
};

Kernel kernel;
uint64_t kernel_syscall_stack_top = 0;
uint64_t kernel_syscall_user_rsp = 0;
uint8_t kernel_smap_enabled = 0;
/*One kernel stack per domain; a domain's saved user context is the trap frame at the top of its stack.*/
static uint8_t kernel_stacks[KERNEL_MAX_DOMAINS][KERNEL_STACK_SIZE] __attribute__((aligned(16)));

/*Domain ids by protection key (the genesis table's id column), filled by spawning.*/
static uint32_t pd_of_key[KERNEL_MAX_DOMAINS];
static uint32_t next_pd_id = 1;
Domain *domain_get(uint32_t id) {
  if (id == 0 || id >= KERNEL_MAX_DOMAINS) return NULL;
  Domain *d = &kernel.domains[id];
  return d->live ? d : NULL;
}
static Domain *domain_of_key(uint64_t key) {
  if (key >= KERNEL_MAX_DOMAINS) return NULL;
  return domain_get(pd_of_key[key]);
}
static bool ring_push(Ring *r, RingEntry entry) {
  if (r->count >= KERNEL_RING_CAPACITY) return false;
  r->entries[(r->head + r->count) % KERNEL_RING_CAPACITY] = entry;
  r->count += 1;
  return true;
}
static bool ring_pop(Ring *r, RingEntry *out) {
  if (r->count == 0) return false;
  *out = r->entries[r->head];
  r->head = (r->head + 1) % KERNEL_RING_CAPACITY;
  r->count -= 1;
  return true;
}
bool domain_enqueue(Domain *d, RingEntry entry) {
  return ring_push(&d->ring, entry);
}
bool domain_dequeue(Domain *d, RingEntry *out) {
  return ring_pop(&d->ring, out);
}
bool input_push(uint8_t scancode) {
  if (kernel.input_count >= KERNEL_INPUT_CAPACITY) return false;
  kernel.input[(kernel.input_head + kernel.input_count) % KERNEL_INPUT_CAPACITY] = scancode;
  kernel.input_count += 1;
  return true;
}
bool input_pop(uint8_t *out) {
  if (kernel.input_count == 0) return false;
  *out = kernel.input[kernel.input_head];
  kernel.input_head = (kernel.input_head + 1) % KERNEL_INPUT_CAPACITY;
  kernel.input_count -= 1;
  return true;
}
const Cap *cap_get(Domain *d, uint64_t slot) {
  if (slot == 0 || slot >= KERNEL_CAP_SLOTS) return NULL;
  const Cap *c = &d->caps[slot];
  return c->kind == CAP_NONE ? NULL : c;
}
bool cap_set(Domain *d, uint64_t slot, Cap c) {
  if (slot == 0 || slot >= KERNEL_CAP_SLOTS) return false;
  d->caps[slot] = c;
  return true;
}
static Cap domain_cap(uint32_t target) {
  Cap c = (Cap) {
    .kind = CAP_DOMAIN
  };
  c.target = target;
  return c;
}
/*The key register value of a domain, PkruValue::for_domain of the reference with one change: key 0 stays closed. Two bits per key, access-disable then write-disable; the domain's own key is open, and the shared key 14 is writable for the producer (key 2), read-only for the consumer (key 1) and closed for the rest. The reference also opens key 0, which every domain's heap and device registers carried; here no user page carries key 0.*/
uint32_t mask_for_key(uint8_t key) {
  uint32_t mask = 0xFFFFFFFFu;
  if (key < 16) mask &= ~(3u << 2 * key);
  uint32_t shift = 2 * KEY_SHARED;
  mask &= ~(3u << shift);
  if (key == 2) return mask;
  if (key == 1) return mask | 2u << shift;
  return mask | 3u << shift;
}
static bool contains(const char *haystack, const char *needle) {
  for (size_t i = 0; haystack[i] != 0; ++i) {
    size_t k = 0;
    while (needle[k] != 0 && haystack[i + k] == needle[k]) k += 1;
    if (needle[k] == 0) return true;
  }
  return needle[0] == 0;
}
typedef struct ElfHeader {
  uint8_t ident[16];
  uint16_t type;
  uint16_t machine;
  uint32_t version;
  uint64_t entry;
  uint64_t phoff;
  uint64_t shoff;
  uint32_t flags;
  uint16_t ehsize;
  uint16_t phentsize;
  uint16_t phnum;
  uint16_t shentsize;
  uint16_t shnum;
  uint16_t shstrndx;
}
ElfHeader;
typedef struct ProgramHeader {
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  uint64_t vaddr;
  uint64_t paddr;
  uint64_t filesz;
  uint64_t memsz;
  uint64_t align;
}
ProgramHeader;
typedef struct RelaEntry {
  uint64_t offset;
  uint64_t info;
  int64_t addend;
}
RelaEntry;
static bool program_header(const uint8_t *image, uint64_t size, const ElfHeader *h, uint64_t i, ProgramHeader *out) {
  uint64_t at = h->phoff + i * h->phentsize;
  if (h->phentsize < sizeof(ProgramHeader) || at + sizeof(ProgramHeader) > size) return false;
  memcpy(out, image + at, sizeof(ProgramHeader));
  return true;
}
/*WRPKRU is 0F 01 EF. Found at any byte offset of executable code, it would let the domain open every protection key; the reference loads such code.*/
static bool has_key_write(const uint8_t *bytes, uint64_t length) {
  for (uint64_t i = 0; i + 2 < length; ++i) if (bytes[i] == 0x0F && (bytes[i + 1] == 0x01 && bytes[i + 2] == 0xEF)) return true;
  return false;
}
/*The frame mapped at a user address, for writing the image through the higher-half map.*/
static uint8_t *frame_byte(uint64_t va) {
  uint64_t pte = 0;
  if (!read_pte(va, &pte)) return NULL;
  return (uint8_t *) phys_to_virt(pte & PTE_ADDRESS) + (va & 0xFFF);
}
/*Load a static PIE at code_base, as elf.rs::load_elf_for_pd: every PT_LOAD at code_base plus its offset from the lowest one, then the R_X86_64_RELATIVE relocations of PT_DYNAMIC. Pages get their final permissions when mapped, and the image is written through the kernel's own map, so no user page is ever writable and executable at once. Refused: segments both writable and executable, executable bytes containing WRPKRU. Returns the entry point, or 0.*/
static uint64_t elf_load(Domain *d, const uint8_t *image, uint64_t size) {
  ElfHeader h;
  if (size < sizeof(ElfHeader)) return 0;
  memcpy(&h, image, sizeof(ElfHeader));
  if (h.ident[0] != 0x7F || (h.ident[1] != 0x45 || (h.ident[2] != 0x4C || h.ident[3] != 0x46))) {
    klog("ELF: Invalid magic number\n");
    return 0;
  }
  uint64_t min_vaddr = UINT64_MAX;
  for (uint64_t i = 0; i < h.phnum; ++i) {
    ProgramHeader ph;
    if (!program_header(image, size, &h, i, &ph)) return 0;
    if (ph.type != PT_LOAD) continue;
    if ((ph.flags & PF_W) != 0 && (ph.flags & PF_X) != 0) {
      klog("[kernel.elf.refused] reason=writable_executable_segment idx=%llu\n", (unsigned long long) i);
      return 0;
    }
    if (ph.offset > size || ph.filesz > size - ph.offset) return 0;
    if ((ph.flags & PF_X) != 0 && has_key_write(image + ph.offset, ph.filesz)) {
      klog("[kernel.elf.refused] reason=key_register_write idx=%llu\n", (unsigned long long) i);
      return 0;
    }
    if (ph.memsz > 0 && ph.vaddr < min_vaddr) min_vaddr = ph.vaddr;
  }
  if (min_vaddr == UINT64_MAX) return 0;
  uint64_t base = d->code_base;
  for (uint64_t i = 0; i < h.phnum; ++i) {
    ProgramHeader ph;
    program_header(image, size, &h, i, &ph);
    if (ph.type != PT_LOAD) continue;
    uint64_t va = base + (ph.vaddr - min_vaddr);
    klog("ELF: Loading segment: vaddr=%#llx, memsz=%#llx (Key: %u)\n", (unsigned long long) va, (unsigned long long) ph.memsz, d->key);
    uint64_t flags = PTE_USER | ((ph.flags & PF_W) != 0 ? PTE_WRITABLE : 0);
    if ((ph.flags & PF_X) == 0) flags |= PTE_NO_EXECUTE;
    uint64_t end = va + ph.memsz;
    for (uint64_t page = va & ~(uint64_t) 0xFFF; page < end; page += KERNEL_PAGE_SIZE) {
      uint64_t pte = 0;
      if (read_pte(page, &pte)) {
/*A page shared with the previous segment keeps its frame; both permissions must agree with W^X.*/
        bool exec = (pte & PTE_NO_EXECUTE) == 0 || (flags & PTE_NO_EXECUTE) == 0;
        bool write = (pte & PTE_WRITABLE) != 0 || (flags & PTE_WRITABLE) != 0;
        if (exec && write) {
          klog("[kernel.elf.refused] reason=shared_page_writable_executable\n");
          return 0;
        }
        continue;
      }
      uint64_t frame = frames_alloc(0);
      if (frame == NO_ADDRESS || !map_page(page, frame, flags, d->key)) return 0;
    }
    uint64_t done = 0;
    while (done < ph.filesz) {
      uint64_t at = va + done;
      uint64_t chunk = KERNEL_PAGE_SIZE - (at & 0xFFF);
      if (chunk > ph.filesz - done) chunk = ph.filesz - done;
      memcpy(frame_byte(at), image + (ph.offset + done), chunk);
      done += chunk;
    }
  }
/*Relocations: the RELA table is found through the PT_LOAD segment whose file bytes cover DT_RELA.*/
  for (uint64_t i = 0; i < h.phnum; ++i) {
    ProgramHeader ph;
    program_header(image, size, &h, i, &ph);
    if (ph.type != PT_DYNAMIC) continue;
    uint64_t rela = 0;
    uint64_t rela_size = 0;
    uint64_t rela_entry = 24;
    uint64_t rela_count = 0;
    for (uint64_t j = 0; j < ph.memsz / 16 && ph.offset + (j + 1) * 16 <= size; ++j) {
      uint64_t dyn[2];
      memcpy(dyn, image + (ph.offset + j * 16), 16);
      if (dyn[0] == 0) break;
      if (dyn[0] == 7) rela = dyn[1];
      if (dyn[0] == 8) rela_size = dyn[1];
      if (dyn[0] == 9) rela_entry = dyn[1];
      if (dyn[0] == 0x6ffffff9) rela_count = dyn[1];
    }
    if (rela_count == 0 || (rela_size == 0 || rela_entry < sizeof(RelaEntry))) break;
    uint64_t rela_file = 0;
    for (uint64_t k = 0; k < h.phnum; ++k) {
      ProgramHeader seg;
      program_header(image, size, &h, k, &seg);
      if (seg.type == PT_LOAD && (rela >= seg.vaddr && rela < seg.vaddr + seg.filesz)) {
        rela_file = seg.offset + (rela - seg.vaddr);
        break;
      }
    }
    if (rela_file == 0 || rela_file + rela_count * rela_entry > size) {
      klog("ELF: R_X86_64_RELATIVE table not in LOAD segment, skipping\n");
      break;
    }
    uint64_t applied = 0;
    for (uint64_t k = 0; k < rela_count; ++k) {
      RelaEntry r;
      memcpy(&r, image + (rela_file + k * rela_entry), sizeof(RelaEntry));
      if ((r.info & 0xFFFFFFFFu) != 8) continue;
      uint64_t target = base + (r.offset - min_vaddr);
      uint8_t *slot = frame_byte(target);
      if (slot == NULL || (target & 0xFFF) > KERNEL_PAGE_SIZE - 8) return 0;
      uint64_t value = base + (uint64_t) r.addend;
      memcpy(slot, &value, 8);
      applied += 1;
    }
    klog("ELF: applied %llu R_X86_64_RELATIVE relocations\n", (unsigned long long) applied);
    break;
  }
/*The stack: 64 KiB below the stack top, writable and not executable.*/
  for (uint64_t page = d->stack_top - USER_STACK_SIZE; page < d->stack_top; page += KERNEL_PAGE_SIZE) {
    uint64_t frame = frames_alloc(0);
    if (frame == NO_ADDRESS || !map_page(page, frame, PTE_USER | (PTE_WRITABLE | PTE_NO_EXECUTE), d->key)) return 0;
  }
  uint64_t entry = base + (h.entry - min_vaddr);
  uint64_t pte = 0;
  if (!read_pte(entry, &pte) || ((pte & PTE_USER) == 0 || (pte & PTE_NO_EXECUTE) != 0)) {
    klog("loader: entrypoint mapping not user-executable\n");
    return 0;
  }
  return entry;
}
/*Create a domain from a module: the next domain id, the image at the key's code region, the stack at the key's stack region, the domain's own ring at slot 2, and a first trap frame that enters the image in user mode.*/
static uint32_t domain_spawn(uint8_t key, const char *name, const LimineFile *module) {
  uint32_t id = next_pd_id;
  next_pd_id += 1;
  if (id >= KERNEL_MAX_DOMAINS) return 0;
  Domain *d = &kernel.domains[id];
  memset(d, 0, sizeof(Domain));
  d->id = id;
  d->key = key;
  d->mask = mask_for_key(key);
  d->name = name;
  d->code_base = CODE_REGION_BASE + (key - 1) * DOMAIN_STRIDE;
  d->stack_top = STACK_REGION_BASE + (key - 1) * DOMAIN_STRIDE + 0x100000;
  uint64_t entry = elf_load(d, (const uint8_t *) module->address, module->size);
  if (entry == 0) return 0;
  d->entry = entry;
  klog("pd: checkpoint=elf.after_load entry=%#llx\n", (unsigned long long) entry);
  cap_set(d, SELF_RING_SLOT, domain_cap(id));
  d->kstack_top = (uint64_t)(uintptr_t) kernel_stacks[id] + KERNEL_STACK_SIZE;
  d->frame = (TrapFrame *)(uintptr_t)(d->kstack_top - sizeof(TrapFrame));
  memset(d->frame, 0, sizeof(TrapFrame));
  d->frame->rip = entry;
  d->frame->cs = USER_CS;
  d->frame->rflags = USER_RFLAGS;
  d->frame->rsp = d->stack_top;
  d->frame->ss = USER_SS;
  d->task_state = TASK_READY;
  d->live = true;
  return id;
}
/*The primary display controller, as gpu_lease.rs::select_primary_gpu chooses it: buses 0 to 7, function 0, class 3; Intel first, then AMD, then virtio, then the first other. The lease is packed as pdx_get_init_arg returns it.*/
static void select_display_lease(void) {
  int best_rank = 99;
  uint64_t lease = 0;
  for (uint32_t bus = 0; bus < 8; ++bus) for (uint32_t dev = 0; dev < 32; ++dev) {
    uint32_t id = pci_read((uint8_t) bus, (uint8_t) dev, 0, 0);
    if ((id & 0xFFFF) == 0xFFFF) continue;
    if (pci_read((uint8_t) bus, (uint8_t) dev, 0, 8) >> 24 != 3) continue;
    uint32_t vendor = id & 0xFFFF;
    int rank = vendor == 0x8086 ? 0 : vendor == 0x1002 ? 1 : vendor == 0x1af4 ? 2 : 3;
    if (rank < best_rank) {
      best_rank = rank;
      lease = (uint64_t) bus | ((uint64_t) dev << 8 | ((uint64_t) vendor << 24 | (uint64_t)(id >> 16) << 40));
    }
  }
  if (best_rank < 99) {
    kernel.display_lease = lease;
    kernel.display_lease_ready = true;
    klog("init: PRIMARY_GPU_LEASE populated\n");
  }
}
static Cap pci_cap(const PciFunction *f) {
  Cap c = (Cap) {
    .kind = CAP_PCI
  };
  c.bus = f->bus;
  c.dev = f->dev;
  c.func = f->func;
  c.vendor_id = f->vendor_id;
  c.device_id = f->device_id;
  return c;
}
/*Device leases, as devmgr.rs grants them: an NVMe controller to sexdrive at slot 16 with vector 0x22, a display controller to sexdisplay at its first inserted slot, an xHCI controller to sexusb (else sexdrive) at slot 8, a network controller to sexnet at slot 19 with memory decoding, I/O decoding and bus mastering enabled as hal/pci.rs enables them.*/
static void lease_devices(void) {
  PciFunction functions[MAX_PCI_FUNCTIONS];
  uint32_t count = pci_scan(functions, MAX_PCI_FUNCTIONS);
  klog("DevMgr: Enumerated %u PCI devices.\n", count);
  bool nvme_found = false;
  Domain *sexdrive = domain_of_key(KERNEL_DOMAIN_KEY_sexdrive);
  Domain *sexdisplay = domain_of_key(KERNEL_DOMAIN_KEY_sexdisplay);
  Domain *sexusb = domain_of_key(KERNEL_DOMAIN_KEY_sexusb);
  Domain *sexnet = domain_of_key(KERNEL_DOMAIN_KEY_sexnet);
  for (uint32_t i = 0; i < count; ++i) {
    const PciFunction *f = &functions[i];
    if (f->class_id == 0x01 && f->subclass_id == 0x08) {
      klog("[kernel.pci.nvme.found] %02x:%02x.%u vendor=%04x device=%04x\n", f->bus, f->dev, f->func, f->vendor_id, f->device_id);
      if (sexdrive != NULL) {
        cap_set(sexdrive, SLOT_NVME_HOST, pci_cap(f));
        kernel.vector_owner[NVME_VECTOR] = sexdrive->id;
        klog("[kernel.cap.nvme_bar.grant] pd=%u slot=%u\n", sexdrive->id, SLOT_NVME_HOST);
      }
      nvme_found = true;
    }
    if (f->class_id == 0x03 && sexdisplay != NULL) {
      cap_set(sexdisplay, FIRST_INSERTED_SLOT, pci_cap(f));
      kernel.vector_owner[DISPLAY_VECTOR] = sexdisplay->id;
    } else if (f->class_id == 0x0C && (f->subclass_id == 0x03 && f->prog_if == 0x30)) {
      Domain *owner = sexusb != NULL ? sexusb : sexdrive;
      if (owner != NULL) {
        cap_set(owner, SLOT_USB_HOST, pci_cap(f));
        klog("DevMgr: Leased XHCI to pd=%u slot=%u\n", owner->id, SLOT_USB_HOST);
      }
    } else if (f->class_id == 0x02 && f->subclass_id == 0x00) {
      uint32_t command = pci_read(f->bus, f->dev, f->func, 0x04);
      pci_write(f->bus, f->dev, f->func, 0x04, command | 0x7);
      klog("[kernel.pci.nic.found] %02x:%02x.%u vendor=%04x device=%04x\n", f->bus, f->dev, f->func, f->vendor_id, f->device_id);
      if (sexnet != NULL) {
        cap_set(sexnet, SLOT_NIC, pci_cap(f));
        klog("[kernel.cap.nic.grant] pd=%u slot=%u\n", sexnet->id, SLOT_NIC);
      }
    }
  }
  if (!nvme_found) klog("[kernel.pci.nvme.absent]\n");
}
/*One genesis row: the bootgraph line the reference prints in its bracketed form, which both the boot oracle and the upstream checker read.*/
static void grant_ring(uint64_t holder_key, uint64_t slot, const char *label, uint64_t target_key, bool optional) {
  Domain *holder = domain_of_key(holder_key);
  Domain *target = domain_of_key(target_key);
  if (holder == NULL || target == NULL) {
    klog("[bootgraph.cap.grant from=kernel to=%u slot=%s target=missing ok=0 optional=1 reason=pd_missing]\n", holder != NULL ? holder->id : 0u, label);
    return;
  }
  cap_set(holder, slot, domain_cap(target->id));
  klog("[bootgraph.cap.grant from=kernel to=%u slot=%s target=%u ok=1%s]\n", holder->id, label, target->id, optional ? " optional=1" : "");
}
static void grant_input(uint64_t holder_key, uint64_t slot, const char *label) {
  Domain *holder = domain_of_key(holder_key);
  if (holder == NULL) return;
  cap_set(holder, slot, (Cap) {
    .kind = CAP_INPUT_RING
  });
  klog("[bootgraph.cap.grant from=kernel to=%u slot=%s target=input_ring ok=1]\n", holder->id, label);
}
/*The grant rows of the genesis table, in their order.*/
static void apply_genesis_grants(void)
{
#define KERNEL_RING_GRANT(holder, slot, label, target, optional) grant_ring(KERNEL_DOMAIN_KEY_##holder, slot, label, KERNEL_DOMAIN_KEY_##target, optional);
    SEXOS_KERNEL_RING_GRANT_ROWS(KERNEL_RING_GRANT)
#ifdef SEXOS_NETWORK_PROFILE
    SEXOS_KERNEL_NETWORK_GRANT_ROWS(KERNEL_RING_GRANT)
#endif
#undef KERNEL_RING_GRANT
#define KERNEL_INPUT_GRANT(holder, slot, label) grant_input(KERNEL_DOMAIN_KEY_##holder, slot, label);
    SEXOS_KERNEL_INPUT_GRANT_ROWS(KERNEL_INPUT_GRANT)
#undef KERNEL_INPUT_GRANT
}

/*Boot: choose the display lease, spawn every domain of the genesis table in its order from the module whose path contains its name, grant the table's capabilities, lease the devices, hand the framebuffer to sexdisplay, and queue every domain. init.rs does the same with a list of names and 777 lines of grants, each printed twice.*/
void domains_boot(void) {
  select_display_lease();
  const LimineModuleResponse *modules = limine_module_request.response;
  if (modules == NULL) kernel_halt("FATAL: Limine returned no modules. Check limine.cfg and ISO layout.");
  klog("init: Found %llu Limine modules\n", (unsigned long long) modules->module_count);
  for (size_t r = 0; r < sizeof genesis_domains / sizeof genesis_domains[0]; ++r) {
    const GenesisDomain *g = &genesis_domains[r];
    for (uint64_t m = 0; m < modules->module_count; ++m) {
      const LimineFile *module = modules->modules[m];
      if (!contains(module->path, g->module)) continue;
      klog("[bootgraph.pd.spawn.begin] pd=%s\n", g->log_name);
      uint32_t id = domain_spawn(g->key, g->log_name, module);
      if (id == 0) klog("[bootgraph.pd.spawn.err] pd=%s reason=spawn_failed\n", g->log_name);
      else {
        pd_of_key[g->key] = id;
        klog("[bootgraph.pd.spawn.ok] pd=%s id=%u pkey=%u\n", g->log_name, id, g->key);
        if (g->key == KERNEL_DOMAIN_KEY_sexdisplay && kernel.display_lease_ready) {
          kernel.domains[id].init_arg = kernel.display_lease;
          kernel.domains[id].init_arg_ready = true;
          kernel.display_lease_ready = false;
        }
      }
      break;
    }
  }
  klog("[bootgraph.phase25.begin]\n");
  apply_genesis_grants();
  lease_devices();
  klog("[bootgraph.phase25.complete]\n");
/*The framebuffer: exactly its pages become reachable from sexdisplay, under sexdisplay's key, and its geometry is the first message on sexdisplay's ring.*/
  Domain *display = domain_of_key(KERNEL_DOMAIN_KEY_sexdisplay);
  const LimineFramebufferResponse *fbs = limine_framebuffer_request.response;
  if (display != NULL && (fbs != NULL && fbs->framebuffer_count > 0)) {
    const LimineFramebuffer *fb = fbs->framebuffers[0];
    uint64_t address = (uint64_t)(uintptr_t) fb->address;
    kernel.fb_address = address;
    kernel.fb_width = fb->width;
    kernel.fb_height = fb->height;
    kernel.fb_pitch = fb->pitch;
    if (expose_to_user(address, fb->pitch * fb->height, display->key)) klog("init: FB remapped USER_ACCESSIBLE (%#llx, %llu bytes) key=%u\n", (unsigned long long) address, (unsigned long long)(fb->pitch * fb->height), display->key);
    else klog("init: FB remap failed\n");
    RingEntry message = (RingEntry) {
      .type_id = 0x11,.caller = 1
    };
    message.arg0 = address;
    message.arg1 = fb->width | fb->height << 32;
    message.arg2 = fb->pitch / 4;
    domain_enqueue(display, message);
    klog("init: FB handed to sexdisplay (%llux%llu @ %#llx)\n", (unsigned long long) fb->width, (unsigned long long) fb->height, (unsigned long long) address);
  }
  for (uint32_t id = 1; id < KERNEL_MAX_DOMAINS; ++id) {
    Domain *d = domain_get(id);
    if (d == NULL) continue;
    klog("[bootgraph.boot.handoff] target=%s id=%u entry=%#llx\n", d->name, id, (unsigned long long) d->entry);
    run_push(id);
  }
  klog("init: Ready for Scheduler.\n");
}
/*The scheduler: one queue, first in first out; a preempted or yielding task goes to the back.*/
void run_push(uint32_t id) {
  if (kernel.run_count >= KERNEL_RUN_QUEUE_CAPACITY) return;
  kernel.run_queue[(kernel.run_head + kernel.run_count) % KERNEL_RUN_QUEUE_CAPACITY] = id;
  kernel.run_count += 1;
  kernel.domains[id].task_state = TASK_READY;
}
uint32_t run_pop(void) {
  while (kernel.run_count > 0) {
    uint32_t id = kernel.run_queue[kernel.run_head];
    kernel.run_head = (kernel.run_head + 1) % KERNEL_RUN_QUEUE_CAPACITY;
    kernel.run_count -= 1;
    if (domain_get(id) != NULL && kernel.domains[id].task_state == TASK_READY) return id;
  }
  return 0;
}
/*Make a domain current and return its frame: its kernel stack becomes the stack of the next entry from user mode, and its key register value is loaded on the way out.*/
TrapFrame *switch_to(uint32_t id) {
  Domain *d = &kernel.domains[id];
  kernel.current = id;
  d->task_state = TASK_RUNNING;
  tss_set_rsp0(d->kstack_top);
  kernel_syscall_stack_top = d->kstack_top;
  if (!kernel.first_scheduled_logged) {
    kernel.first_scheduled_logged = true;
    klog("first scheduled pd_id=%u\n", id);
  }
  if (kernel.pku) cpu_wrpkru(d->mask);
  return d->frame;
}
/*Resume the interrupted context, restoring the current domain's key register when it is user mode.*/
TrapFrame *resume_frame(TrapFrame *frame) {
  if ((frame->cs & 3) == 3 && kernel.pku) {
    Domain *d = domain_get(kernel.current);
    if (d != NULL) cpu_wrpkru(d->mask);
  }
  return frame;
}
/*Timer preemption from user mode: the next queued domain runs, the current one goes to the back. With nothing queued the current one continues.*/
TrapFrame *schedule_tick(TrapFrame *frame) {
  if (kernel.run_count == 0) return resume_frame(frame);
  uint32_t next = run_pop();
  if (next == 0) return resume_frame(frame);
  run_push(kernel.current);
  return switch_to(next);
}
/*SYSCALL_YIELD: the current domain goes to the back and the first queued domain runs, possibly itself.*/
TrapFrame *schedule_yield(void) {
  run_push(kernel.current);
  return switch_to(run_pop());
}
/*With no domain to run, wait for an interrupt with interrupts enabled; the timer handler switches to a domain once one is queued.*/
_Noreturn void kernel_idle(void) {
  kernel.current = 0;
  while (1) cpu_sti_hlt();
}
