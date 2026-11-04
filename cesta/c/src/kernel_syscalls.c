#include "sexos_kernel.h"
/*System calls, translated from kernel/src/syscalls/mod.rs, ipc.rs and ipc/router.rs. The register interface is the reference's: the number in rax; arguments in rdi, rsi, rdx, r10 and r8; the result in rax; call and listen return their extra words in rsi, rdx, r10 and r8. Three requests are checked where the reference accepts anything: a reply needs a call the replier heard and has not answered; a mapping needs frames the caller owns; and every page the kernel maps for a domain carries that domain's key and is not executable.*/
#define RAW_PRINT_MAX 4096u
#define MAX_ORDER 18u
#define MAX_ALLOCATION_BYTES (KERNEL_PAGE_SIZE << MAX_ORDER)
#define SHARED_BUFFER_KEY 14u
#define REPLY_TYPE UINT64_C(0x1)
#define INPUT_TYPE UINT64_C(0x201)
#define DISPLAY_INFO_SIZE 24u
/*PDX_GET_DISPLAY_INFO's record (repr(C) DisplayInfo of syscalls/mod.rs).*/
typedef struct DisplayInfo {
  uint64_t virt_addr;
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
}
DisplayInfo;
_Static_assert(sizeof(DisplayInfo) == 24, "DisplayInfo layout");

static uint32_t size_to_order(uint64_t size) {
  uint64_t pages = (size + 4095) / 4096;
  uint32_t order = 0;
  while (order < MAX_ORDER && (uint64_t) 1 << order < pages) order += 1;
  return order;
}
/*Map length bytes of frames from phys at a fresh address, for the caller, under its key and never executable.*/
static uint64_t map_for(Domain *d, uint64_t phys, uint64_t length, uint64_t flags) {
  uint64_t va = va_alloc(length);
  if (va == NO_ADDRESS) return NO_ADDRESS;
  for (uint64_t offset = 0; offset < length; offset += KERNEL_PAGE_SIZE) if (!map_page(va + offset, (phys & ~(uint64_t) 0xFFF) + offset, flags | (PTE_USER | PTE_NO_EXECUTE), d->key)) return NO_ADDRESS;
  return va;
}
static bool owns(Domain *d, uint64_t base, uint64_t length) {
  for (uint32_t i = 0; i < kernel.block_count; ++i) {
    const OwnedBlock *b = &kernel.blocks[i];
    if (b->owner == d->id && base >= b->base) {
      uint64_t offset = base - b->base;
      if (offset <= b->length && length <= b->length - offset) return true;
    }
  }
  return false;
}
static void record_block(uint64_t base, uint64_t length, uint32_t owner) {
  if (kernel.block_count >= KERNEL_BLOCKS) return;
  kernel.blocks[kernel.block_count] = (OwnedBlock) {
    .base = base,.length = length,.owner = owner
  };
  kernel.block_count += 1;
}
/*raw_print: bytes collect in the domain's line until a newline, then the line goes out whole; a full line goes out as it is. The reference writes each request's bytes at once, so a domain preempted between the requests of one line leaves a cut record that another domain's output continues.*/
static void line_write(Domain *d, const uint8_t *bytes, uint64_t length) {
  for (uint64_t i = 0; i < length; ++i) {
    d->line[d->line_length] = bytes[i];
    d->line_length += 1;
    if (bytes[i] == 0x0A || d->line_length >= KERNEL_LINE_CAPACITY) {
      kernel_serial_write(d->line, d->line_length);
      d->line_length = 0;
    }
  }
}
/*Slot 0 of PDX_CALL: the kernel's own operations.*/
static void kernel_operation(Domain *d, TrapFrame *f, uint64_t operation, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
  uint64_t status = ERR_CAP_INVALID;
  uint64_t value = 0;
  if (!SEXOS_KERNEL_CURRENT_PDX && operation == 0xE3) {
    if (arg0 != 0 && (user_range_ok(arg0, DISPLAY_INFO_SIZE, true, d->mask) && kernel.fb_address != 0)) {
      DisplayInfo info = (DisplayInfo) {
        .virt_addr = kernel.fb_address - hhdm_offset
      };
      info.width = (uint32_t) kernel.fb_width;
      info.height = (uint32_t) kernel.fb_height;
      info.pitch = (uint32_t)(kernel.fb_pitch / 4);
/*The reference writes the four fields and leaves the padding alone.*/
      if (copy_to_user(arg0, &info, DISPLAY_INFO_SIZE - 4, d->mask)) status = 0;
    }
  }
  if (operation == 69) {
    uint64_t length = arg1;
    if (length == 0) status = 0;
    else if (arg0 != 0 && length <= RAW_PRINT_MAX) {
      uint8_t text[RAW_PRINT_MAX];
      if (copy_from_user(text, arg0, length, d->mask)) {
        line_write(d, text, length);
        status = 0;
      }
    }
  }
  if (operation == 0x12) if (d->init_arg_ready) {
    d->init_arg_ready = false;
    status = 0;
    value = d->init_arg;
  }
  if (!SEXOS_KERNEL_CURRENT_PDX && operation == 0xBB) {
    klog("PD1: FRAME_PRESENT event\n");
    status = 0;
  }
  if (SEXOS_KERNEL_CURRENT_PDX && (operation == 0xE6 && (arg1 == 0 && (arg2 == 0 && kernel.fb_address != 0)))) if (d->id == kernel.display_pd || (d->id == kernel.shell_pd || d->id == kernel.input_pd)) {
    if (arg0 == 0) {
      value = (uint64_t)(uint32_t) kernel.fb_width | (uint64_t)(uint32_t) kernel.fb_height << 32;
      status = 0;
    }
    if (arg0 == 1 && d->id == kernel.display_pd) {
      value = kernel.fb_address;
      status = 0;
    }
    if (arg0 == 2) {
      value = (uint32_t) kernel.fb_pitch;
      status = 0;
    }
  }
  f->rsi = value;
  f->rax = status;
}
/*PDX_CALL to a slot holding a domain capability enqueues a call on the target's ring; the call is the message. A full ring answers ERR_SERVICE_NOT_READY.*/
static void pdx_call(Domain *d, TrapFrame *f) {
  uint64_t slot = f->rdi;
  if (slot == 0) {
    kernel_operation(d, f, f->rsi, f->rdx, f->r10, f->r8);
    return;
  }
  uint64_t status = ERR_CAP_INVALID;
  const Cap *cap = cap_get(d, slot);
  if (cap != NULL && cap->kind == CAP_DOMAIN) {
    Domain *target = domain_get(cap->target);
    if (target != NULL) {
      if (SEXOS_KERNEL_CURRENT_PDX && (target->task_state == TASK_EXITED || target->task_state == TASK_DORMANT)) {
        f->rax = ERR_SERVICE_NOT_READY;
        f->rsi = 0;
        return;
      }
      uint32_t task = SEXOS_KERNEL_CURRENT_PDX ? kernel_current_task(d) : 0;
      if (SEXOS_KERNEL_CURRENT_PDX && task == 0) {
        f->rax = ERR_CAP_INVALID;
        f->rsi = 0;
        return;
      }
      RingEntry call_entry = (RingEntry) {
        .type_id = f->rsi,.caller = d->id
      };
      call_entry.caller_task = SEXOS_KERNEL_CURRENT_PDX ? task : 0;
      call_entry.arg0 = f->rdx;
      call_entry.arg1 = f->r10;
      call_entry.arg2 = f->r8;
      call_entry.sender = d->id;
      call_entry.from_call = true;
/*Reserve one consumable reply right before publishing each accepted invocation. Exhaustion refuses before enqueue; a failed enqueue withdraws exactly its reservation.*/
      if (SEXOS_KERNEL_CURRENT_PDX && target->reply_rights[d->id] == UINT64_MAX) {
        f->rax = ERR_SERVICE_NOT_READY;
        f->rsi = 0;
        return;
      }
      bool reserved = SEXOS_KERNEL_CURRENT_PDX;
      if (reserved) target->reply_rights[d->id] += 1;
      status = domain_enqueue(target, call_entry) ? 0 : ERR_SERVICE_NOT_READY;
      if (reserved && status != 0) target->reply_rights[d->id] -= 1;
    }
  }
  f->rsi = 0;
  f->rax = status;
}
/*Take the next message off a ring. The historical cohort grants at hearing; the current cohort reserved its obligation before enqueue.*/
static bool take_message(Domain *d, RingEntry *out) {
  while (d->ring.count > 0) {
    const RingEntry *head = &d->ring.entries[d->ring.head];
/*A full reply-right counter defers a call until an outstanding reply is consumed; it never wraps or drops the queued call.*/
    if (!SEXOS_KERNEL_CURRENT_PDX && (head->from_call && (head->sender < KERNEL_MAX_DOMAINS && d->reply_rights[head->sender] == UINT64_MAX))) return false;
    if (!domain_dequeue(d, out)) return false;
    if (out->type_id == 0xFF && (out->sender == 0 && !out->from_call)) {
      uint64_t vector = out->arg0;
      uint64_t revision = out->arg1;
      bool invalid = vector >= 256;
      if (!invalid) invalid = kernel.vector_owner[vector] != d->id || (kernel.vector_heard_revision[vector] == UINT64_MAX || (revision != kernel.vector_heard_revision[vector] + 1 || revision > kernel.vector_revision[vector]));
      if (invalid) kernel_halt("device interrupt receipt invariant");
      kernel.vector_heard_revision[vector] = revision;
    }
    if (out->request_id != 0) {
      if (kernel_rpc_claim(d, out)) return true;
      continue;
    }
    if (!SEXOS_KERNEL_CURRENT_PDX && (out->from_call && out->sender < KERNEL_MAX_DOMAINS)) d->reply_rights[out->sender] += 1;
    return true;
  }
  *out = (RingEntry) {
    .type_id = 0
  };
  return false;
}
/*PDX_LISTEN: on slot 0 a waiting reply first (type 1), then the domain's ring; on a slot holding the input-ring capability the next scancode (type 0x201); anything else is empty (type 0).*/
RingEntry kernel_listen_message(Domain *d, uint64_t slot) {
  RingEntry m = (RingEntry) {
    .type_id = 0
  };
  if (slot == 0) if (d->reply_ready) {
    d->reply_ready = false;
    m = (RingEntry) {
      .type_id = REPLY_TYPE,.caller = SEXOS_KERNEL_CURRENT_PDX ? d->reply_caller : 1,.caller_task = SEXOS_KERNEL_CURRENT_PDX ? d->reply_task : 0,.arg0 = d->reply_value
    };
    if (SEXOS_KERNEL_CURRENT_PDX) {
      d->reply_value = 0;
      d->reply_caller = 0;
      d->reply_task = 0;
    }
  } else take_message(d, &m);
  else {
    const Cap *cap = cap_get(d, slot);
    if (cap != NULL) {
      if (cap->kind == CAP_INPUT_RING) {
        uint8_t scancode = 0;
        if (input_pop(&scancode)) m = (RingEntry) {
          .type_id = INPUT_TYPE,.caller = 1,.arg0 = scancode
        };
      }
      if (cap->kind == CAP_MESSAGE_QUEUE) take_message(d, &m);
    }
  }
  return m;
}
static void pdx_listen(Domain *d, TrapFrame *f) {
  RingEntry m = kernel_listen_message(d, f->rdi);
  if (SEXOS_KERNEL_CURRENT_PDX) f->rdi = m.caller_task;
  f->rsi = m.caller;
  f->rdx = m.arg0;
  f->r10 = m.arg1;
  f->r8 = m.arg2;
  f->rax = m.type_id;
}
/*PDX_REPLY consumes a held obligation only after publication. The current cohort preserves a full destination slot and stamps the scheduled sender; the historical cohort retains its earlier slot behavior.*/
static void pdx_reply(Domain *d, TrapFrame *f) {
  if (f->rdi >= KERNEL_MAX_DOMAINS) {
    f->rax = 1;
    return;
  }
  uint32_t target_id = (uint32_t) f->rdi;
  Domain *target = domain_get(target_id);
  if (target == NULL) {
    f->rax = 1;
    return;
  }
  if (d->reply_rights[target_id] == 0) {
    if (kernel.reply_refused_log < 16) {
      kernel.reply_refused_log += 1;
      klog("[kernel.reply.refused] from=%u to=%u reason=no_call_heard\n", d->id, target_id);
    }
    f->rax = 1;
    return;
  }
  uint32_t task = SEXOS_KERNEL_CURRENT_PDX ? kernel_current_task(d) : 0;
  if (SEXOS_KERNEL_CURRENT_PDX && (task == 0 || (target->reply_ready || (target->task_state == TASK_EXITED || target->task_state == TASK_DORMANT)))) {
    f->rax = 1;
    return;
  }
  target->reply_caller = SEXOS_KERNEL_CURRENT_PDX ? d->id : 0;
  target->reply_task = SEXOS_KERNEL_CURRENT_PDX ? task : 0;
  target->reply_ready = true;
  target->reply_value = f->rsi;
  if (SEXOS_KERNEL_CURRENT_PDX) kernel_task_wake(target);
  d->reply_rights[target_id] -= 1;
  f->rax = 0;
}
/*MAP_MEMORY: the reference maps any physical range, writable, executable and under key 0, which every domain can reach. Here only frames ALLOCATE_MEMORY gave the caller, under the caller's key.*/
static uint64_t map_memory(Domain *d, uint64_t phys, uint64_t size) {
  if (size == 0) return va_alloc(0);
  if (size > UINT64_MAX - (phys & 0xFFF)) return NO_ADDRESS;
  if (!owns(d, phys & ~(uint64_t) 0xFFF, size + (phys & 0xFFF))) {
    if (kernel.map_refused_log < 16) {
      kernel.map_refused_log += 1;
      klog("[kernel.map.refused] pd=%u phys=%#llx size=%#llx reason=not_owned\n", d->id, (unsigned long long) phys, (unsigned long long) size);
    }
    return NO_ADDRESS;
  }
  return map_for(d, phys, size, PTE_WRITABLE);
}
static uint64_t allocate_memory(Domain *d, uint64_t size) {
  if (size > MAX_ALLOCATION_BYTES || kernel.block_count >= KERNEL_BLOCKS) return NO_ADDRESS;
  uint32_t order = size_to_order(size);
  uint64_t phys = frames_alloc(order);
  if (phys == NO_ADDRESS) return NO_ADDRESS;
  record_block(phys, KERNEL_PAGE_SIZE << order, d->id);
  return phys;
}
/*SYSCALL_ALLOC_SHARED_BUFFER: fresh frames under the shared key 14, writable for key 2 and readable for key 1 (mask_for_key).*/
static uint64_t alloc_shared_buffer(Domain *d, uint64_t size, uint8_t consumer) {
  if ((uint8_t) d->id == consumer) return 0;
  Domain *receiver = domain_get(consumer);
/*The shared key's fixed policy must agree with the requested endpoints: the producer can write it and the consumer can read it.*/
  if (receiver == NULL || ((d->mask >> 2 * SHARED_BUFFER_KEY & 3) != 0 || (receiver->mask >> 2 * SHARED_BUFFER_KEY & 1) != 0)) return 0;
  if (size == 0 || (size > MAX_ALLOCATION_BYTES || kernel.shared_count >= KERNEL_SHARED_BUFFERS)) return 0;
  uint64_t va = va_alloc(size);
  if (va == NO_ADDRESS) return 0;
  for (uint64_t offset = 0; offset < size; offset += KERNEL_PAGE_SIZE) {
    uint64_t frame = frames_alloc(0);
    if (frame == NO_ADDRESS || !map_page(va + offset, frame, PTE_USER | (PTE_WRITABLE | PTE_NO_EXECUTE), SHARED_BUFFER_KEY)) return 0;
  }
  if (kernel.shared_count < KERNEL_SHARED_BUFFERS) {
    kernel.shared[kernel.shared_count] = (SharedBuffer) {
      .address = va,.size = size,.producer = (uint8_t) d->id,.consumer = consumer
    };
    kernel.shared_count += 1;
  }
  return va;
}
/*MAP_PCI_BAR: a leased xHCI, NVMe or network function's memory BAR, uncached, under the caller's key (the reference uses key 0, reachable by every domain).*/
static uint64_t map_pci_bar(Domain *d, uint64_t slot, uint64_t bar_index, uint64_t requested) {
  uint64_t size = requested == 0 ? 0x1000 : requested < 0x10000 ? requested : 0x10000;
  const Cap *cap = cap_get(d, slot);
  if (cap == NULL || cap->kind != CAP_PCI) return NO_ADDRESS;
  uint32_t class_rev = pci_read(cap->bus, cap->dev, cap->func, 0x08);
  uint8_t class_id = (uint8_t)(class_rev >> 24);
  uint8_t subclass_id = (uint8_t)(class_rev >> 16);
  uint8_t prog_if = (uint8_t)(class_rev >> 8);
  bool xhci = class_id == 0x0c && (subclass_id == 0x03 && prog_if == 0x30);
  bool nvme = class_id == 0x01 && subclass_id == 0x08;
  bool nic = class_id == 0x02 && subclass_id == 0x00;
  if (!(xhci || (nvme || nic))) return NO_ADDRESS;
  uint8_t index = (uint8_t)(bar_index & 0xFF);
  if ((pci_read(cap->bus, cap->dev, cap->func, (uint8_t)(0x10 + index * 4)) & 1) != 0) return NO_ADDRESS;
  uint64_t bar = pci_bar(cap->bus, cap->dev, cap->func, index) & ~(uint64_t) 0xFFF;
  if (bar == 0) return NO_ADDRESS;
  return map_for(d, bar, size, PTE_WRITABLE | (PTE_NO_CACHE | PTE_WRITE_THROUGH));
}
/*SYS_GRANT_MEM_LEND: the kernel allocates a page, maps it for the caller, and lends it to the domain behind the caller's domain capability, at an empty slot. The lender owns the frame.*/
static uint64_t grant_mem_lend(Domain *d, uint64_t domain_slot, uint64_t length, uint64_t lend_slot) {
  if (!kernel.genesis_sealed) return NO_ADDRESS;
  if (length != 4096) {
    klog("[kernel.memlend.grant.err] reason=bad_len len=%llu\n", (unsigned long long) length);
    return NO_ADDRESS;
  }
  const Cap *cap = cap_get(d, domain_slot);
  if (cap == NULL || cap->kind != CAP_DOMAIN) {
    klog("[kernel.memlend.grant.err] reason=no_domain_cap slot=%llu\n", (unsigned long long) domain_slot);
    return NO_ADDRESS;
  }
  Domain *target = domain_get(cap->target);
  if (target == NULL) {
    klog("[kernel.memlend.grant.err] reason=no_target_pd\n");
    return NO_ADDRESS;
  }
  if (lend_slot == 0 || lend_slot >= KERNEL_CAP_SLOTS) {
    klog("[kernel.memlend.grant.err] reason=bad_slot slot=%llu\n", (unsigned long long) lend_slot);
    return NO_ADDRESS;
  }
  if (cap_get(target, lend_slot) != NULL) {
    klog("[kernel.memlend.grant.err] reason=slot_occupied slot=%llu\n", (unsigned long long) lend_slot);
    return NO_ADDRESS;
  }
  if (kernel.block_count >= KERNEL_BLOCKS) return NO_ADDRESS;
  klog("[kernel.memlend.grant.begin] caller_pd=%u target_pd=%u slot=%llu\n", d->id, target->id, (unsigned long long) lend_slot);
  uint64_t phys = frames_alloc(0);
  if (phys == NO_ADDRESS) {
    klog("[kernel.memlend.grant.err] reason=no_phys\n");
    return NO_ADDRESS;
  }
  uint64_t va = map_for(d, phys, 4096, PTE_WRITABLE);
  if (va == NO_ADDRESS) {
    klog("[kernel.memlend.grant.err] reason=map_fail\n");
    return NO_ADDRESS;
  }
  record_block(phys, 4096, d->id);
  Cap lent = (Cap) {
    .kind = CAP_MEM_LEND
  };
  lent.base = phys;
  lent.length = 4096;
  lent.lend_key = d->key;
  lent.permissions = 3;
  lent.source = d->id;
/*This subordinate page grant preserves the sealed communication graph. The lender's domain capability, target vacancy and frame ownership were checked above.*/
  if (!cap_install_lend(target, lend_slot, lent)) return NO_ADDRESS;
  klog("[kernel.memlend.grant.ok] va=%#llx phys=%#llx len=4096\n", (unsigned long long) va, (unsigned long long) phys);
  return va;
}
/*SYS_MAP_MEM_LEND: map a lent frame for its borrower, under the borrower's key.*/
static uint64_t map_mem_lend(Domain *d, uint64_t slot) {
  const Cap *cap = cap_get(d, slot);
  if (cap == NULL) {
    klog("[kernel.memlend.map.err] reason=no_cap slot=%llu\n", (unsigned long long) slot);
    return NO_ADDRESS;
  }
  if (cap->kind != CAP_MEM_LEND) {
    klog("[kernel.memlend.map.err] reason=wrong_kind slot=%llu\n", (unsigned long long) slot);
    return NO_ADDRESS;
  }
  if ((cap->permissions & 2) == 0) {
    klog("[kernel.memlend.map.err] reason=no_write_perm\n");
    return NO_ADDRESS;
  }
  uint64_t va = map_for(d, cap->base, cap->length, PTE_WRITABLE);
  if (va != NO_ADDRESS) klog("[kernel.memlend.map.ok] va=%#llx len=%llu\n", (unsigned long long) va, (unsigned long long) cap->length);
  return va;
}
/*SYS_NET_DIAG: the reference's in-kernel e1000 HTTP proof reports through it; this kernel leaves networking to sexnet and reports what the reference reports when no network controller is present.*/
static uint64_t net_diag(uint64_t index) {
  if (index == 0) {
    klog("[net.diag.syscall.reply] status=0 bytes=0 source=0\n");
    return 0;
  }
  if (index == 1) {
    klog("[net.diag.body.len.reply] len=0\n");
    return 0;
  }
  if (index >= 2 && index <= 9) {
    klog("[net.diag.body.chunk.reply] idx=%llu ok=1\n", (unsigned long long)(index - 2));
    return NO_ADDRESS;
  }
  return NO_ADDRESS;
}
static void kernel_region_query(Domain *d, TrapFrame *f, uint64_t number)
{
    MemoryRegionCapability absent={0};
    const Cap *held=cap_get(d,f->rdi);
    const MemoryRegionCapability *region=&absent;
    if (held && (held->kind == CAP_REGION || held->kind == CAP_PCI)) region=&held->region;
    MemoryRegionReply r=sexos_region_query(region,number,f->rsi,f->rdx,f->r10);
    f->rax=r.status; f->rsi=r.value0; f->rdx=r.value1; f->r10=r.value2; f->r8=r.value3;
}
static uint64_t current_bar_view(Domain *d, uint64_t slot, uint64_t bar, uint64_t length)
{
    const Cap *held=cap_get(d,slot);
    if (!held || held->kind != CAP_PCI || bar) return NO_ADDRESS;
    MemoryRegionViewResult result=sexos_region_register_view(&held->region,0,length ? length : 4096,
        REGION_QUERY|REGION_READ|REGION_WRITE);
    return result.error ? NO_ADDRESS : result.value.virtual_address;
}

TrapFrame *kernel_syscall(TrapFrame *f) {
  if (kernel.pku) cpu_wrpkru(0);
  Domain *d = domain_get(kernel.current);
  if (d == NULL) kernel_halt("system call without a current domain");
  if (SEXOS_KERNEL_CURRENT_PDX) {
    d->frame = f;
#if SEXOS_KERNEL_CURRENT_FPU
kernel_fpu_save_current();

#endif
    kernel_task_poll();
  }
  uint64_t number = f->rax;
  switch (number) {
    case SEXOS_SYSCALL_call: pdx_call(d, f);
    break;
    case SEXOS_SYSCALL_listen: pdx_listen(d, f);
    break;
    case SEXOS_SYSCALL_reply: pdx_reply(d, f);
    break;
    case SEXOS_SYSCALL_map_memory: f->rax = SEXOS_KERNEL_CURRENT_PDX ? NO_ADDRESS : map_memory(d, f->rdi, f->rsi);
    break;
    case SEXOS_SYSCALL_allocate_memory: f->rax = SEXOS_KERNEL_CURRENT_PDX ? NO_ADDRESS : allocate_memory(d, f->rdi);
    break;
/*Yield leaves rax holding 32, as the reference's yield path does; Rust callers rely on it.*/
    case SEXOS_SYSCALL_yield: return schedule_yield();
    case SEXOS_SYSCALL_ticks: f->rax = kernel.ticks;
    break;
    case SEXOS_SYSCALL_alloc_shared_buffer: f->rax = SEXOS_KERNEL_CURRENT_PDX ? 0 : alloc_shared_buffer(d, f->rdi, (uint8_t) f->rsi);
    break;
    case SEXOS_SYSCALL_set_state: f->rax = SEXOS_KERNEL_CURRENT_PDX ? NO_ADDRESS : 0;
    break;
    case SEXOS_SYSCALL_map_pci_bar: f->rax = SEXOS_KERNEL_CURRENT_PDX ? current_bar_view(d, f->rdi, f->rsi, f->rdx) : map_pci_bar(d, f->rdi, f->rsi, f->rdx);
    break;
    case SEXOS_SYSCALL_grant_mem_lend: f->rax = SEXOS_KERNEL_CURRENT_PDX ? NO_ADDRESS : grant_mem_lend(d, f->rdi, f->rsi, f->rdx);
    break;
    case SEXOS_SYSCALL_map_mem_lend: f->rax = SEXOS_KERNEL_CURRENT_PDX ? NO_ADDRESS : map_mem_lend(d, f->rdi);
    break;
    case SEXOS_SYSCALL_net_diag: f->rax = SEXOS_KERNEL_CURRENT_PDX ? NO_ADDRESS : net_diag(f->rdi);
    break;
    case 53: if (SEXOS_KERNEL_CURRENT_PDX) if (f->rdi == 0) return kernel_task_block(d, f, 0);
    else f->rax = ERR_CAP_INVALID;
    else f->rax = NO_ADDRESS;
    break;
    case 70: if (SEXOS_KERNEL_CURRENT_PDX) if (f->rdi != 0 && (f->rdi <= 64 && (f->rsi | f->rdx | (f->r10 | f->r8)) == 0)) return kernel_task_block(d, f, f->rdi);
    else f->rax = ERR_CAP_INVALID;
    else f->rax = NO_ADDRESS;
    break;
    case 56: if (SEXOS_KERNEL_CURRENT_PDX) kernel_paging_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 57: if (SEXOS_KERNEL_CURRENT_PDX) kernel_paging_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 58: if (SEXOS_KERNEL_CURRENT_PDX) kernel_paging_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 59: if (SEXOS_KERNEL_CURRENT_PDX) kernel_paging_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 60: if (SEXOS_KERNEL_CURRENT_PDX) kernel_paging_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 61: if (SEXOS_KERNEL_CURRENT_PDX) kernel_paging_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 63: if (SEXOS_KERNEL_CURRENT_PDX) kernel_app_query(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 72: if (SEXOS_KERNEL_CURRENT_PDX) kernel_source_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 67: if (SEXOS_KERNEL_CURRENT_PDX) kernel_history_query(d, f);
    else f->rax = NO_ADDRESS;
    break;
    case 68: if (SEXOS_KERNEL_CURRENT_PDX) kernel_pci_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 69: if (SEXOS_KERNEL_CURRENT_PDX) kernel_pci_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 71: if (SEXOS_KERNEL_CURRENT_PDX) kernel_pci_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 73: if (SEXOS_KERNEL_CURRENT_PDX) kernel_source_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 74: if (SEXOS_KERNEL_CURRENT_PDX) kernel_source_syscall(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case SEXOS_SYSCALL_region_writer: if (SEXOS_KERNEL_CURRENT_PDX) kernel_region_writer_syscall(d, f);
    else f->rax = NO_ADDRESS;
    break;
    case SEXOS_SYSCALL_device_reset: if (SEXOS_KERNEL_CURRENT_PDX) kernel_device_reset_syscall(d, f);
    else f->rax = NO_ADDRESS;
    break;
    case 65: if (SEXOS_KERNEL_CURRENT_PDX) kernel_app_retire(d, f);
    else f->rax = NO_ADDRESS;
    break;
    case 66: if (SEXOS_KERNEL_CURRENT_PDX) kernel_app_activation(d, f);
    else f->rax = NO_ADDRESS;
    break;
    case 64: if (SEXOS_KERNEL_CURRENT_PDX) kernel_app_query(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 62: if (SEXOS_KERNEL_CURRENT_PDX) return kernel_task_finish(d, f);
    else f->rax = NO_ADDRESS;
    break;
    case 54: if (SEXOS_KERNEL_CURRENT_PDX) kernel_region_query(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case 55: if (SEXOS_KERNEL_CURRENT_PDX) kernel_region_query(d, f, number);
    else f->rax = NO_ADDRESS;
    break;
    case SEXOS_RPC_register: kernel_rpc_syscall(d, f, number);
    break;
    case SEXOS_RPC_call: kernel_rpc_syscall(d, f, number);
    break;
    case SEXOS_RPC_poll: kernel_rpc_syscall(d, f, number);
    break;
    case SEXOS_RPC_listen: kernel_rpc_syscall(d, f, number);
    break;
    case SEXOS_RPC_reply: kernel_rpc_syscall(d, f, number);
    break;
    case SEXOS_RPC_cancel: kernel_rpc_syscall(d, f, number);
    break;
    default: {
      f->rax = NO_ADDRESS;
/*A failed current five-word ABI call cannot return its input arguments as result data.*/
      if (SEXOS_KERNEL_CURRENT_PDX) {
        f->rsi = 0;
        f->rdx = 0;
        f->r10 = 0;
        f->r8 = 0;
      }
    }
  }
  return resume_frame(f);
}
