#if !defined(SEXOS_KERNEL_H)
#define SEXOS_KERNEL_H 1

#include <stdint.h>

#include <stddef.h>

#include <stdbool.h>

#include "cpu_x86_64.h"

#include "kernel_log.h"

#include "client_constants.h"

#include "kernel_genesis.h"

/*Shared interface of the SexOS kernel, translated from kernel/src. The kernel's state is one owned record (Kernel); every table in it has a declared bound, and nothing is allocated after boot except page frames.*/

/*Bounds. Domain ids are 1 to 15 (14 is kept for the shared key); slots 0 to 31 cover the ABI's slots 0 to 19.*/

#define KERNEL_MAX_DOMAINS 16u

#define KERNEL_CAP_SLOTS 32u

#define KERNEL_RING_CAPACITY 256u

#define KERNEL_INPUT_CAPACITY 512u

#define KERNEL_RUN_QUEUE_CAPACITY 32u

#define KERNEL_STACK_SIZE 65536u

#define KERNEL_BLOCKS 4096u

#define KERNEL_SHARED_BUFFERS 64u

#define KERNEL_LINE_CAPACITY 512u

#define KERNEL_PAGE_SIZE UINT64_C(4096)

/*Segment selectors, the reference's GDT layout: kernel code 0x08, kernel data 0x10, a 32-bit user code pad 0x18, user data 0x20, user code 0x28, the TSS 0x30.*/

#define KERNEL_CS 0x08

#define KERNEL_DS 0x10

#define KERNEL_TSS_SELECTOR 0x30

#define USER_CS UINT64_C(0x2b)

#define USER_SS UINT64_C(0x23)

#define USER_RFLAGS UINT64_C(0x202)

/*Protection keys: each domain's key is its domain id; key 14 is the shared producer/consumer key, key 0 the default key no user page carries.*/

#define KEY_SHARED 14u

/*Page-table entry bits.*/

#define PTE_PRESENT UINT64_C(0x1)

#define PTE_WRITABLE UINT64_C(0x2)

#define PTE_USER UINT64_C(0x4)

#define PTE_WRITE_THROUGH UINT64_C(0x8)

#define PTE_NO_CACHE UINT64_C(0x10)

#define PTE_HUGE UINT64_C(0x80)

#define PTE_NO_EXECUTE (UINT64_C(1) << 63)

#define PTE_ADDRESS UINT64_C(0x000ffffffffff000)

#define PTE_KEY_SHIFT 59u

/*Status words of the PDX ABI (crates/sex-pdx).*/

#define ERR_SERVICE_NOT_READY UINT64_C(0xFFFFFFFFFFFFFFFE)

#define ERR_CAP_INVALID UINT64_C(0xFFFFFFFFFFFFFFFC)

#define NO_ADDRESS UINT64_MAX

/*A trap frame, built by entry_x86_64.S on every entry from user mode and resumed by every exit.*/

typedef struct TrapFrame {
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rbp;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rbx;
  uint64_t rax;
  uint64_t vector;
  uint64_t error;
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
}
TrapFrame;

_Static_assert(sizeof(TrapFrame) == 176 && offsetof(TrapFrame, rax) == 112 && offsetof(TrapFrame, rip) == 136, "trap frame layout of entry_x86_64.S");


/*What a capability names. MESSAGE_QUEUE is the domain's own ring (listen); DOMAIN a domain's ring (call, lend); PCI a device lease; MEM_LEND a lent frame.*/

typedef enum CapKind {
  CAP_NONE, CAP_DOMAIN, CAP_INPUT_RING, CAP_MESSAGE_QUEUE, CAP_PCI, CAP_MEM_LEND
}
CapKind;

typedef struct Cap {
  uint8_t kind;
/*CAP_DOMAIN: the target domain.*/
  uint32_t target;
/*CAP_PCI: the leased function.*/
  uint8_t bus;
  uint8_t dev;
  uint8_t func;
  uint16_t vendor_id;
  uint16_t device_id;
/*CAP_MEM_LEND: the frame, its length, the lender's key, permission bits (2 = write) and the lender.*/
  uint64_t base;
  uint64_t length;
  uint8_t lend_key;
  uint8_t permissions;
  uint32_t source;
}
Cap;

/*A ring entry holds exactly what listen returns: the type word, the reported caller and three arguments. from_call marks an IpcCall, whose receiver may then reply once to its sender.*/

typedef struct RingEntry {
  uint64_t type_id;
  uint64_t caller;
  uint64_t arg0;
  uint64_t arg1;
  uint64_t arg2;
  uint32_t sender;
  bool from_call;
  uint64_t request_id;
}
RingEntry;

typedef struct Ring {
  RingEntry entries[KERNEL_RING_CAPACITY];
  uint32_t head;
  uint32_t count;
}
Ring;

#if !defined(KERNEL_RPC_CAPACITY)
#define KERNEL_RPC_CAPACITY 8u

#endif

_Static_assert(KERNEL_RPC_CAPACITY > 0 && KERNEL_RPC_CAPACITY <= KERNEL_RING_CAPACITY, "RPC window capacity");


typedef enum RpcState {
  RPC_FREE, RPC_QUEUED, RPC_HEARD, RPC_READY
}
RpcState;

typedef struct RpcSlot {
  uint64_t id;
  uint32_t peer;
  RpcState state;
  uint64_t value;
}
RpcSlot;

typedef enum TaskState {
  TASK_NONE, TASK_READY, TASK_RUNNING, TASK_EXITED
}
TaskState;

/*A protection domain and its one task.*/

typedef struct Domain {
  bool live;
  uint32_t id;
/*Protection key and the key register value for user mode: its own key open, key 0 closed, key 14 per the shared policy.*/
  uint8_t key;
  uint32_t mask;
  const char *name;
  Cap caps[KERNEL_CAP_SLOTS];
  Ring ring;
/*The one-deep reply slot, as in the reference.*/
  bool reply_ready;
  uint64_t reply_value;
/*Reply rights: calls heard from each domain and not yet answered.*/
  uint64_t reply_rights[KERNEL_MAX_DOMAINS];
/*Opt-in calls retain a distinct slot until consumption or cancellation. Identities never wrap or reuse a cancelled call.*/
  bool rpc_enabled;
  uint64_t rpc_next;
  RpcSlot rpc[KERNEL_RPC_CAPACITY];
  uint8_t task_state;
  TrapFrame *frame;
  uint64_t kstack_top;
/*The domain's serial output up to its next newline: each line reaches the serial port whole, so lines of different domains never interleave.*/
  uint8_t line[KERNEL_LINE_CAPACITY];
  uint32_t line_length;
/*The display lease handed to sexdisplay once (pdx_get_init_arg).*/
  bool init_arg_ready;
  uint64_t init_arg;
  uint64_t code_base;
  uint64_t stack_top;
  uint64_t entry;
}
Domain;

/*A block of frames given to a domain by ALLOCATE_MEMORY; the domain may map exactly these frames.*/

typedef struct OwnedBlock {
  uint64_t base;
  uint64_t length;
  uint32_t owner;
}
OwnedBlock;

typedef struct PciFunction {
  uint8_t bus;
  uint8_t dev;
  uint8_t func;
  uint16_t vendor_id;
  uint16_t device_id;
  uint8_t class_id;
  uint8_t subclass_id;
  uint8_t prog_if;
}
PciFunction;

typedef struct SharedBuffer {
  uint64_t address;
  uint64_t size;
  uint8_t producer;
  uint8_t consumer;
}
SharedBuffer;

/*Limine 7 response layouts, as the reference's limine crate reads them.*/

typedef struct LimineMemmapEntry {
  uint64_t base;
  uint64_t length;
  uint64_t type;
}
LimineMemmapEntry;

typedef struct LimineMemmapResponse {
  uint64_t revision;
  uint64_t entry_count;
  LimineMemmapEntry **entries;
}
LimineMemmapResponse;

typedef struct LimineHhdmResponse {
  uint64_t revision;
  uint64_t offset;
}
LimineHhdmResponse;

typedef struct LimineRsdpResponse {
  uint64_t revision;
  uint64_t address;
}
LimineRsdpResponse;

typedef struct LimineUuid {
  uint32_t a;
  uint16_t b;
  uint16_t c;
  uint8_t d[8];
}
LimineUuid;

typedef struct LimineFile {
  uint64_t revision;
  void *address;
  uint64_t size;
  char *path;
  char *cmdline;
  uint32_t media_type;
  uint32_t unused;
  uint32_t tftp_ip;
  uint32_t tftp_port;
  uint32_t partition_index;
  uint32_t mbr_disk_id;
  LimineUuid gpt_disk_uuid;
  LimineUuid gpt_part_uuid;
  LimineUuid part_uuid;
}
LimineFile;

typedef struct LimineModuleResponse {
  uint64_t revision;
  uint64_t module_count;
  LimineFile **modules;
}
LimineModuleResponse;

typedef struct LimineFramebuffer {
  void *address;
  uint64_t width;
  uint64_t height;
  uint64_t pitch;
  uint16_t bpp;
  uint8_t memory_model;
  uint8_t red_mask_size;
  uint8_t red_mask_shift;
  uint8_t green_mask_size;
  uint8_t green_mask_shift;
  uint8_t blue_mask_size;
  uint8_t blue_mask_shift;
  uint8_t unused[7];
  uint64_t edid_size;
  void *edid;
}
LimineFramebuffer;

typedef struct LimineFramebufferResponse {
  uint64_t revision;
  uint64_t framebuffer_count;
  LimineFramebuffer **framebuffers;
}
LimineFramebufferResponse;

/*A request: its identifier words, revision and the response pointer the bootloader fills in.*/

typedef struct LimineRequest {
  uint64_t id[4];
  uint64_t revision;
  void *response;
}
LimineRequest;

_Static_assert(offsetof(LimineFile, path) == 24 && offsetof(LimineFramebuffer, edid_size) == 48 && offsetof(LimineRequest, response) == 40, "Limine 7 layouts");


extern volatile LimineRequest limine_framebuffer_request, limine_memmap_request, limine_hhdm_request, limine_rsdp_request, limine_module_request;


/*The kernel's state.*/

typedef struct Kernel {
  Domain domains[KERNEL_MAX_DOMAINS];
/*Run queue of domain ids, first in first out, as the reference's single-core work-stealing queue behaves.*/
  uint32_t run_queue[KERNEL_RUN_QUEUE_CAPACITY];
  uint32_t run_head;
  uint32_t run_count;
/*The running domain; 0 before the first switch and while idle.*/
  uint32_t current;
  bool scheduler_running;
  bool first_scheduled_logged;
/*Scancodes from the PS/2 keyboard, for the holder of the input-ring capability.*/
  uint8_t input[KERNEL_INPUT_CAPACITY];
  uint32_t input_head;
  uint32_t input_count;
  bool keyboard_ready;
  uint64_t ticks;
/*Hardware-interrupt routes: the domain told of each vector.*/
  uint32_t vector_owner[256];
  OwnedBlock blocks[KERNEL_BLOCKS];
  uint32_t block_count;
  SharedBuffer shared[KERNEL_SHARED_BUFFERS];
  uint32_t shared_count;
/*Processor features found at boot.*/
  bool pku;
  bool smap;
  bool smep;
/*Framebuffer, as Limine reports it.*/
  uint64_t fb_address;
  uint64_t fb_width;
  uint64_t fb_height;
  uint64_t fb_pitch;
/*The primary display lease (pdx_get_init_arg), packed as the reference packs it.*/
  bool display_lease_ready;
  uint64_t display_lease;
/*Log budgets for refusals.*/
  uint32_t reply_refused_log;
  uint32_t map_refused_log;
}
Kernel;

extern Kernel kernel;

extern uint64_t kernel_syscall_stack_top;

extern uint64_t kernel_syscall_user_rsp;

extern uint8_t kernel_smap_enabled;

extern uint64_t hhdm_offset;

/*kernel_memory: frames, page tables, the user virtual-address cursor, and checked user access.*/

void memory_init(const LimineMemmapResponse *map);

uint64_t frames_alloc(uint32_t order);

void *phys_to_virt(uint64_t phys);

uint64_t va_alloc(uint64_t size);

bool map_page(uint64_t va, uint64_t phys, uint64_t flags, uint8_t key);

bool read_pte(uint64_t va, uint64_t *pte);

bool expose_to_user(uint64_t va, uint64_t length, uint8_t key);

bool user_range_ok(uint64_t start, uint64_t length, bool write, uint32_t mask);

bool copy_from_user(void *destination, uint64_t source, size_t length, uint32_t mask);

bool copy_to_user(uint64_t destination, const void *source, size_t length, uint32_t mask);

void *memcpy(void *destination, const void *source, size_t count);

void *memmove(void *destination, const void *source, size_t count);

void *memset(void *destination, int value, size_t count);

int memcmp(const void *first, const void *second, size_t count);

void *kmemset(void *destination, int value, size_t count);

void *kmemcpy(void *destination, const void *source, size_t count);

/*kernel_platform: ACPI, the local and I/O APICs, the timer, the keyboard and PCI.*/

bool platform_init(uint64_t rsdp_phys);

void lapic_eoi(void);

void keyboard_init(void);

bool keyboard_read(uint8_t *scancode);

uint32_t pci_read(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);

void pci_write(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value);

uint64_t pci_bar(uint8_t bus, uint8_t dev, uint8_t func, uint8_t index);

uint32_t pci_scan(PciFunction *out, uint32_t max);

/*kernel_domains: spawning, the genesis grants, the scheduler, traps and system calls.*/

void kernel_main(void);

void domains_boot(void);

Domain *domain_get(uint32_t id);

uint32_t mask_for_key(uint8_t key);

const Cap *cap_get(Domain *d, uint64_t slot);

bool cap_set(Domain *d, uint64_t slot, Cap c);

bool domain_enqueue(Domain *d, RingEntry entry);

bool domain_dequeue(Domain *d, RingEntry *out);

RingEntry kernel_listen_message(Domain *d, uint64_t slot);

bool kernel_rpc_claim(Domain *receiver, const RingEntry *message);

void kernel_rpc_syscall(Domain *d, TrapFrame *frame, uint64_t number);

bool input_push(uint8_t scancode);

bool input_pop(uint8_t *out);

void run_push(uint32_t id);

uint32_t run_pop(void);

TrapFrame *switch_to(uint32_t id);

TrapFrame *resume_frame(TrapFrame *frame);

TrapFrame *schedule_tick(TrapFrame *frame);

TrapFrame *schedule_yield(void);

_Noreturn void kernel_idle(void);

void tss_set_rsp0(uint64_t top);

TrapFrame *kernel_trap(TrapFrame *frame);

TrapFrame *kernel_syscall(TrapFrame *frame);

_Noreturn void kernel_resume(TrapFrame *frame);

_Noreturn void kernel_halt(const char *reason);

void kernel_syscall_entry(void);

extern const uint64_t kernel_isr_table[36];

#endif
