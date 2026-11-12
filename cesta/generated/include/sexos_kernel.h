#if !defined(SEXOS_KERNEL_H)
#define SEXOS_KERNEL_H 1

#include <stdint.h>

#include <stddef.h>

#include <stdbool.h>

#include "cpu_x86_64.h"

#include "kernel_log.h"

#include "client_constants.h"

#include "kernel_genesis.h"

#include "sexos_memory_regions.h"

#include "sexos_region_writer.h"

#include "sexos_device_reset.h"

#include "sexos_nvme_msix.h"

#include "sexos_app_private.h"

#include "sexos_pager_state.h"

#include "frame_lease.h"

#include "program_memory.h"

#include "epoch_types.h"

#include "sexos_source_policy.h"

#include "sexos_source_profile.h"

#include "sexos_network_config.h"

#include "sexos_pci_policy.h"

#include "sexos_history_recovery.h"

#include "sexos_smp_park.h"

#include "kernel_current.h"


typedef struct ProgramWorker ProgramWorker;
typedef struct ProgramWorkerStorage {
    struct TrapFrame *frame;
    uint8_t stack[65536u] __attribute__((aligned(16)));
    uint8_t fpu[512u] __attribute__((aligned(16)));
} ProgramWorkerStorage;

#define MSR_FS_BASE UINT32_C(0xC0000100)

#ifndef PROGRAM_FRAME_CAPACITY
#define PROGRAM_FRAME_CAPACITY 8192u
#endif

#define COUNT_CURRENT_APP(...) +1
#define COUNT_CURRENT_DOMAIN(...) +1
#define COUNT_CURRENT_ROUTE(...) +1
#define COUNT_CURRENT_SOURCE(...) +1
#define COUNT_CURRENT_ACTIVE(p_id, p_name, p_module, p_log, p_key, p_layout, p_active)             \
    +(p_active ? 1 : 0)
#define COUNT_CURRENT_QUIL(p_target, p_ordinal, p_surface, p_shell_surface, p_lifecycle, p_close,  \
                           p_activation, p_predecessor, p_logical, p_route)                        \
    +(p_logical == CURRENT_PD_quil ? 1 : 0)
#define COUNT_CURRENT_WEB(p_target, p_ordinal, p_surface, p_shell_surface, p_lifecycle, p_close,   \
                          p_activation, p_predecessor, p_logical, p_route)                         \
    +((p_logical == CURRENT_PD_spindle || p_logical == CURRENT_PD_kaleidoscope) ? 1 : 0)

enum {
    KERNEL_CURRENT_APPS = 0 SEXOS_CURRENT_APP_ROWS(COUNT_CURRENT_APP),
    KERNEL_CURRENT_DOMAINS = 0 SEXOS_CURRENT_DOMAIN_ROWS(COUNT_CURRENT_DOMAIN),
    KERNEL_CURRENT_ACTIVE = 0 SEXOS_CURRENT_DOMAIN_ROWS(COUNT_CURRENT_ACTIVE),
    KERNEL_CURRENT_ROUTES = 0 SEXOS_CURRENT_ROUTE_ROWS(COUNT_CURRENT_ROUTE),
    KERNEL_CURRENT_SOURCE_CAPS = 0 SEXOS_CURRENT_SOURCE_ROWS(COUNT_CURRENT_SOURCE),
    KERNEL_CURRENT_QUILS = 0 SEXOS_CURRENT_APP_ROWS(COUNT_CURRENT_QUIL),
    KERNEL_CURRENT_WEB = 0 SEXOS_CURRENT_APP_ROWS(COUNT_CURRENT_WEB),
    KERNEL_CURRENT_DORMANT = KERNEL_CURRENT_DOMAINS - KERNEL_CURRENT_ACTIVE,
    /* Fixed device/display/paging/epoch/record regions, one document pair per Quil and one web pair per Spindle/Browser. */
    KERNEL_CURRENT_BACKINGS = 12 + KERNEL_CURRENT_QUILS + KERNEL_CURRENT_WEB
};

#undef COUNT_CURRENT_APP
#undef COUNT_CURRENT_DOMAIN
#undef COUNT_CURRENT_ROUTE
#undef COUNT_CURRENT_SOURCE
#undef COUNT_CURRENT_ACTIVE
#undef COUNT_CURRENT_QUIL
#undef COUNT_CURRENT_WEB


/*Shared interface of the SexOS kernel, translated from kernel/src. The kernel's state is one owned record (Kernel); every table in it has a declared bound, and nothing is allocated after boot except page frames.*/

/*The current user-domain namespace is 1..1023 and capability selectors are 1..1024. BOOT metadata has its own finite budget, independent of logical identity and hardware keys. Historical bounds remain selectable for the frozen reference.*/

#if !defined(SEXOS_KERNEL_CURRENT_PDX)
#define SEXOS_KERNEL_CURRENT_PDX 0

#endif


_Static_assert(SEXOS_KERNEL_CURRENT_PDX == 0 || SEXOS_KERNEL_CURRENT_PDX == 1, "PDX source cohort");


#if !defined(SEXOS_KERNEL_CURRENT_FPU)
#define SEXOS_KERNEL_CURRENT_FPU SEXOS_KERNEL_CURRENT_PDX

#endif


_Static_assert(SEXOS_KERNEL_CURRENT_FPU == 0 || SEXOS_KERNEL_CURRENT_FPU == 1, "FPU source cohort");


#if !defined(SEXOS_FPU_CONTEXT_CORRUPT_PROBE)
#define SEXOS_FPU_CONTEXT_CORRUPT_PROBE 0

#endif


_Static_assert(SEXOS_FPU_CONTEXT_CORRUPT_PROBE == 0 || SEXOS_FPU_CONTEXT_CORRUPT_PROBE == 1,
               "FPU context mutation control");


#if SEXOS_KERNEL_CURRENT_PDX
#define KERNEL_MAX_DOMAINS 1024u

#define KERNEL_CAP_SLOTS 1025u

#define KERNEL_BOOT_DOMAINS 64u

#define KERNEL_STACK_RECORDS 64u

#else
#define KERNEL_MAX_DOMAINS 16u

#define KERNEL_CAP_SLOTS 32u

#define KERNEL_STACK_RECORDS KERNEL_MAX_DOMAINS

#endif

#if !defined(SEXOS_KERNEL_BOOT_AUDIT)
#define SEXOS_KERNEL_BOOT_AUDIT 0

#endif

#if !defined(SEXOS_KERNEL_CURRENT_SMP)
#define SEXOS_KERNEL_CURRENT_SMP 0

#endif

#define KERNEL_BOOT_CAPS 1024u

#define KERNEL_BOOT_REGIONS 64u

/*The paging pool's backing key is kernel-private: it is never published as a Region, so it stays outside the published region IDs.*/

#define KERNEL_PAGING_BACKING 0x7fff0001u

#define KERNEL_BOOT_REGION_ALIASES 2u

#define KERNEL_RING_CAPACITY 256u

#define KERNEL_INPUT_CAPACITY 512u

#define KERNEL_EPOCH_EVENT_CAPACITY 8192u

#if !defined(KERNEL_RUN_QUEUE_CAPACITY)
#define KERNEL_RUN_QUEUE_CAPACITY 512u

#endif

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

/*Hardware keys are independent of logical domain IDs. The current profile gives key 14 to Sext; the frozen profile retains its shared-buffer policy. Key 0 is closed to users.*/

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


_Static_assert(sizeof(TrapFrame) == 176 && offsetof(TrapFrame, rax) == 112 &&
                   offsetof(TrapFrame, rip) == 136,
               "trap frame layout of entry_x86_64.S");


#include "program_workers.h"

#include "program_record_memory.h"

#if !defined(PROGRAM_RECORD_QUEUE_CAPACITY)
#define PROGRAM_RECORD_QUEUE_CAPACITY KERNEL_RUN_QUEUE_CAPACITY

#endif

#include "program_record_tasks.h"

_Static_assert(KERNEL_RUN_QUEUE_CAPACITY == PROGRAM_RECORD_QUEUE_CAPACITY, "shared task FIFO capacity");


/*What a capability names. MESSAGE_QUEUE is the domain's own ring (listen); DOMAIN a domain's ring (call, lend); PCI a device lease; MEM_LEND a lent frame.*/

typedef enum CapKind {
  CAP_NONE, CAP_DOMAIN, CAP_INPUT_RING, CAP_MESSAGE_QUEUE, CAP_PCI, CAP_MEM_LEND, CAP_REGION, CAP_APP_LIFECYCLE, CAP_APP_ACTIVATION, CAP_FRAME_POOL, CAP_PAGER_MAPPING, CAP_DEMAND_REGION, CAP_ENTROPY_SOURCE, CAP_CLOCK_SOURCE, CAP_TRUST_POLICY, CAP_HISTORY_RECOVERY, CAP_RECORD_POOL
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
  MemoryRegionCapability region;
  AppLifecycleGrant lifecycle;
  AppActivationGrant activation;
  PciCommandGrant command;
  PgFramePool pool;
  PgMapping mapping;
  PgDemand demand;
  SourceGrant source_grant;
  HistoryGrant history;
}
Cap;


#if SEXOS_KERNEL_CURRENT_PDX
typedef const Cap *DomainCaps[KERNEL_CAP_SLOTS];

typedef struct {
    Cap entries[KERNEL_BOOT_CAPS];
    uint32_t count;
} CapabilityStorage;
#else
typedef Cap DomainCaps[KERNEL_CAP_SLOTS];

typedef struct {
    uint8_t unused;
} CapabilityStorage;
#endif


typedef struct KernelEpochCheckpoint {
  uint64_t through;
  KernelEpochEvent boundary;
}
KernelEpochCheckpoint;

/*A cursor certificate includes a non-reused revision. Exhaustion refuses mutation rather than wrapping into an old certificate.*/

typedef struct Ring {
  RingEntry entries[KERNEL_RING_CAPACITY];
  uint32_t head;
  uint32_t count;
  uint64_t revision;
}
Ring;

#if !defined(KERNEL_RPC_CAPACITY)
#define KERNEL_RPC_CAPACITY 8u

#endif


_Static_assert(KERNEL_RPC_CAPACITY > 0 && KERNEL_RPC_CAPACITY <= KERNEL_RING_CAPACITY,
               "RPC window capacity");


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
  TASK_NONE, TASK_READY, TASK_RUNNING, TASK_EXITED, TASK_DORMANT, TASK_WAIT_IO, TASK_WAIT_FAULT
}
TaskState;

/*A protection domain and its one task.*/

typedef struct Domain {
  bool live;
  uint32_t id;
/*Hardware key and user-mode enforcement mask. Current masks open only the holder's key; frozen-profile sharing remains explicit.*/
  uint8_t key;
  uint8_t layout;
  uint32_t mask;
  const char *name;
  DomainCaps caps;
  uint16_t storage_index;
  Ring ring;
/*The one-deep reply slot, as in the reference.*/
  bool reply_ready;
  uint64_t reply_value;
  uint32_t reply_caller;
  uint32_t reply_task;
/*Reply obligations by caller domain. The historical cohort grants at hearing; current transport reserves before accepted publication.*/
  uint64_t reply_rights[KERNEL_MAX_DOMAINS];
/*Opt-in calls retain a distinct slot until consumption or cancellation. Identities never wrap or reuse a cancelled call.*/
  bool rpc_enabled;
  uint64_t rpc_next;
  RpcSlot rpc[KERNEL_RPC_CAPACITY];
  uint32_t task_id;
  uint8_t task_state;
  bool queued;
  bool admission_pending;
  bool wake_pending;
  uint64_t wait_deadline;
  uint64_t wait_count;
  uint64_t expired_count;
  bool exit_notice;
  PgFaultState fault;
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
  uint64_t wait_address;
  uint64_t fs_base;
  uint8_t fault_main_state;
  ProgramMemoryState program;
}
Domain;


#if SEXOS_KERNEL_CURRENT_PDX
typedef struct {
    Domain *slots[KERNEL_MAX_DOMAINS];
    Domain records[KERNEL_BOOT_DOMAINS];
    uint32_t count;
} DomainRegistry;
#else
typedef Domain DomainRegistry[KERNEL_MAX_DOMAINS];
#endif


typedef struct BootRegionAlias {
  uint32_t holder;
  uint64_t virtual_base;
  uint8_t key;
  bool writer_eligible;
}
BootRegionAlias;

typedef struct BootBacking {
  uint32_t region_id;
  uint64_t physical;
  uint64_t length;
  uint64_t reserved;
  BootRegionAlias aliases[KERNEL_BOOT_REGION_ALIASES];
  uint8_t alias_count;
  RegionWriterState writer;
}
BootBacking;

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


typedef struct LimineMpInfo LimineMpInfo;
typedef void (*LimineMpEntry)(LimineMpInfo *);

struct LimineMpInfo {
    uint32_t processor_id, lapic_id;
    uint64_t reserved;
    LimineMpEntry goto_address;
    uint64_t extra_argument;
};

typedef struct LimineMpResponse {
    uint64_t revision;
    uint32_t flags, bsp_lapic_id;
    uint64_t cpu_count;
    LimineMpInfo **cpus;
} LimineMpResponse;



_Static_assert(offsetof(LimineFile, path) == 24 && offsetof(LimineFramebuffer, edid_size) == 48 &&
                   offsetof(LimineRequest, response) == 40,
               "Limine 7 layouts");



_Static_assert(sizeof(LimineMpInfo) == 32 && offsetof(LimineMpInfo, goto_address) == 16 &&
                   sizeof(LimineMpResponse) == 32,
               "Limine 7 MP layouts");



extern volatile LimineRequest limine_framebuffer_request, limine_memmap_request,
    limine_hhdm_request, limine_rsdp_request, limine_module_request;
#if SEXOS_KERNEL_CURRENT_SMP
extern volatile LimineRequest limine_mp_request;
bool kernel_smp_park_boot(void);
bool kernel_smp_epoch_current(uint64_t expected);
bool kernel_smp_epoch_advance(uint64_t expected, uint64_t next);
#endif



typedef struct KernelAppPrivate {
    uint32_t target, task, predecessor, pager, coordinator;
    uint8_t key, acked;
    bool captured, ready, dormant, retired, activated, kicked;
    uint8_t hinted;
    uint64_t last_hint[6];
    uint64_t surface, pool;
    AppIncarnation identity;
    AppPrivatePages image, workspace;
} KernelAppPrivate;



typedef struct {
    bool initialized;
    uint32_t pager;
    PgFramePool pool;
    PgMapping mappings[PG_CLIENTS];
    uint8_t keys[PG_CLIENTS];
    uint64_t physical, leaves[PG_CLIENTS][PG_PAGES], last_token;
    PgLedger ledger;
} KernelPaging;


/*The kernel's state.*/

typedef struct Kernel {
  DomainRegistry domains;
  KernelAppPrivate apps[KERNEL_CURRENT_APPS];
  uint32_t app_count;
  KernelPaging paging;
  CapabilityStorage cap_storage;
  BootBacking boot_backings[KERNEL_BOOT_REGIONS];
  uint32_t boot_backing_count;
/*Shared FIFO of domain, worker and record tokens, with bounded direct handoff and exact dispatch types.*/
  ProgramRecordQueue run_queue;
/*The running domain; 0 before the first switch and while idle.*/
  uint32_t current;
  uint32_t display_pd;
  uint32_t shell_pd;
  uint32_t input_pd;
  bool scheduler_running;
  bool first_scheduled_logged;
  bool idle_timed_logged;
/*The boot authority graph is fixed before the first user domain runs. Later page lending cannot add or replace a communication capability.*/
  bool genesis_sealed;
/*Bounded committed event window and elided-prefix checkpoint. Total epoch and event count never wrap; elided events cannot be replayed.*/
  KernelEpochEvent epoch_events[KERNEL_EPOCH_EVENT_CAPACITY];
  uint64_t epoch_event_count;
  uint64_t epoch;
  uint64_t epoch_compacted_through;
  KernelEpochCheckpoint epoch_checkpoint;
  DeviceResetState device_reset;
  uint64_t vector_revision[256];
  uint64_t vector_pending[256];
  uint64_t vector_heard_revision[256];
  uint64_t vector_ack_revision[256];
/*Scancodes from the PS/2 keyboard, for the holder of the input-ring capability.*/
  uint8_t input[KERNEL_INPUT_CAPACITY];
  uint32_t input_head;
  uint32_t input_count;
  uint64_t input_revision;
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
  uint64_t entropy_samples;
  uint64_t clock_samples;
  bool network_config_present;
  uint32_t network_config_owner;
  NetConfig network_config;
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

FrameLeaseResult program_frame_take(uint32_t holder);

FrameLeaseResult program_frame_take_transient(uint32_t holder);

uint64_t program_frame_commit(FrameLease lease);

uint64_t program_frame_commit_all(FrameLease *leases, size_t count);

bool program_frame_map(FrameLease lease, uint64_t va, uint64_t flags, uint8_t key);

bool program_frame_map_run(FrameLease lease, uint32_t run, uint64_t va, uint64_t flags, uint8_t key);

bool program_frame_workspace_map(const AppPrivatePage *image, uint32_t target, uint32_t run, uint64_t address, uint8_t key);

bool program_frame_handover_check(const AppPrivatePages *image, const AppPrivatePages *workspace, uint32_t target, uint32_t run, uint32_t holder);

bool program_frame_handover_commit(const AppPrivatePages *image, const AppPrivatePages *workspace, uint32_t target, uint32_t run, uint32_t holder);

uint64_t program_frame_detach(FrameLease lease);

uint64_t program_frame_give(FrameLease lease);

bool program_frame_release(FrameLease lease);

bool program_frame_release_run(uint32_t target, uint32_t run);

uint64_t program_frame_available(void);

uint64_t program_frame_lent(void);

uint64_t program_frame_pressure(void);

void *phys_to_virt(uint64_t phys);

uint64_t va_alloc(uint64_t size);

bool map_page(uint64_t va, uint64_t phys, uint64_t flags, uint8_t key);

bool prepare_page_table(uint64_t va, uint8_t key);

bool image_write(uint64_t address, const uint8_t *source, uint64_t count);

uint8_t *frame_byte(uint64_t address);

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

bool kernel_epoch_log_boot(void);

bool kernel_epoch_log_ready(uint64_t expected);

bool kernel_epoch_log_append(uint64_t next, const KernelEpochEvent *event);

bool kernel_epoch_log_internal(void);

bool kernel_epoch_log_eligible(uint32_t id);

bool kernel_epoch_log_flush(Domain *d, bool enter);

void kernel_epoch_log_poll(void);

bool kernel_epoch_current(uint64_t epoch);

bool kernel_epoch_event_read(uint64_t event_epoch, KernelEpochEvent *out);

void kernel_epoch_record_page(uint64_t expected, uint8_t kind, uint32_t subject, uint64_t address, uint64_t token, uint64_t frame, uint8_t rights);

void kernel_epoch_record_page_key(uint64_t expected, uint8_t kind, uint32_t subject, uint64_t address, uint64_t token, uint64_t frame, uint8_t rights, uint8_t key);

void kernel_epoch_record_cleanup(uint64_t expected, uint32_t subject, uint64_t address, uint64_t task, uint64_t bitmap, uint8_t key);

void kernel_epoch_record_handover(uint64_t expected, uint32_t subject, uint64_t task, uint64_t pool, uint64_t pages, uint8_t source_key, uint8_t key);

void kernel_epoch_record_activation(uint64_t expected, uint32_t predecessor, uint32_t successor, uint64_t task, uint64_t surface, uint64_t pages, uint8_t key);

void kernel_epoch_record_pci(uint64_t expected, uint32_t subject, uint64_t slot, uint64_t set, uint64_t clear, PciCommandGrant grant, PciApplyTrace trace);

void kernel_epoch_record_writer(uint64_t expected, uint32_t subject, uint32_t peer, uint32_t region, uint64_t revision, uint64_t job, uint64_t pages, uint8_t source_key, uint8_t target_key, uint8_t action);

void kernel_epoch_record_device_reset(uint64_t expected, uint32_t subject, uint64_t slot, uint32_t device, uint32_t identity, uint8_t vector, uint8_t action, uint64_t generation, uint64_t interrupt_revision, uint32_t cc, uint32_t csts);

void kernel_main(void);

void domains_boot(void);

void domains_boot_current(void);

extern ProgramRecords kernel_program_records;

extern ProgramRecordMemory kernel_program_record_memory;

bool kernel_records_boot(void);

bool kernel_records_boot_audit(void);

uint64_t kernel_record_image_physical(void);

extern ProgramRecordTask kernel_record_tasks[PROGRAM_RECORD_CAPACITY];

extern ProgramRecordTaskStorage kernel_record_task_storage[PROGRAM_RECORD_CAPACITY];

extern ProgramRecordTask *kernel_current_record;

extern ProgramRecordTask *kernel_outgoing_record;

bool kernel_record_run_push(uint32_t id);

ProgramRecordTask *kernel_record_dispatch(uintptr_t id);

ProgramRecordTaskStorage *kernel_record_storage(ProgramRecordTask *task);

TrapFrame *kernel_record_switch(uintptr_t id);

bool kernel_record_outgoing(TrapFrame *frame);

void kernel_record_stack_departed(void);

TrapFrame *kernel_record_pdx_call(TrapFrame *frame);

TrapFrame *kernel_record_syscall(TrapFrame *frame);

TrapFrame *kernel_record_sleep(TrapFrame *frame);

TrapFrame *kernel_record_finish(TrapFrame *frame, bool fault);

void kernel_records_poll(void);

void kernel_record_fpu_save(void);

void kernel_record_fpu_restore(void);

void kernel_paging_boot(void);

void kernel_sources_boot(void);

void kernel_source_syscall(Domain *d, TrapFrame *frame, uint64_t number);

void kernel_history_boot(void);

void kernel_history_query(Domain *d, TrapFrame *frame);

bool kernel_history_boot_valid(void);

void kernel_pci_boot(void);

bool kernel_pci_boot_valid(Domain *d, uint64_t slot);

void kernel_pci_syscall(Domain *d, TrapFrame *frame, uint64_t number);

void kernel_region_writer_syscall(Domain *d, TrapFrame *frame);

void kernel_device_interrupt_publish(uint32_t vector);

bool kernel_device_interrupt_retry(uint32_t vector);

void kernel_device_interrupt_drain(Domain *d);

void kernel_device_reset_syscall(Domain *d, TrapFrame *frame);

bool kernel_device_reset_owner_dead(Domain *dead);

bool kernel_region_writer_expected_write(uint32_t region, uint32_t holder, bool granted);

bool kernel_region_writer_recover_dead(Domain *dead);

void kernel_paging_syscall(Domain *d, TrapFrame *frame, uint64_t number);

bool kernel_pager_forward(Domain *d, TrapFrame *frame, uint64_t address);

void kernel_task_wake(Domain *d);

void kernel_task_poll(void);

void kernel_idle_observe(void);

TrapFrame *kernel_task_block(Domain *d, TrapFrame *frame, uint64_t ticks);

TrapFrame *kernel_task_finish(Domain *d, TrapFrame *frame);

TrapFrame *kernel_dispatch_blocked(void);

bool kernel_paging_cleanup_complete(uint32_t target);

void kernel_current_boot_audit(void);

void kernel_current_catalog_publish(void);

bool kernel_current_catalog_valid(void);

void kernel_boot_display_lease(void);

void kernel_boot_devices(void);

void kernel_boot_framebuffer(void);

void kernel_current_routes(void);

void kernel_current_route_markers(void);

void kernel_current_lifecycle_grants(void);

bool kernel_boot_pci_region(Domain *d, uint64_t slot, const PciFunction *device, MemoryRegionCapability *out);

bool kernel_nvme_msix_set_mask(const Cap *lease, bool masked);

Domain *domain_get(uint32_t id);

uint32_t domain_spawn_at(uint32_t id, uint8_t layout, uint8_t key, const char *name, const LimineFile *module);

uint64_t page_leaf_address(uint64_t address);

void kernel_apps_plan(void);

void kernel_app_query(Domain *d, TrapFrame *frame, uint64_t number);

void kernel_app_activation(Domain *d, TrapFrame *frame);

KernelAppPrivate *kernel_app_get(uint32_t target);

void kernel_apps_workspaces_boot(void);

void kernel_app_retire(Domain *d, TrapFrame *frame);

void kernel_app_poll(void);

bool kernel_app_capture_range(Domain *d, uint64_t start, uint64_t length);

bool kernel_app_finish_capture(Domain *d);

bool kernel_app_reserve_successor(uint32_t predecessor, uint32_t successor);

uint32_t kernel_current_task(Domain *d);

ProgramMemoryReply kernel_program_memory(Domain *d, uint64_t want);

void kernel_workers_boot(void);

struct ProgramWorkerStorage *kernel_worker_storage(ProgramWorker *worker);

bool kernel_worker_entry(TrapFrame *frame);

void kernel_worker_stack_departed(void);

bool kernel_worker_token(uintptr_t token);

ProgramWorker *kernel_worker_dispatch(uintptr_t token);

TrapFrame *kernel_worker_schedule(TrapFrame *frame, bool resume_current);

TrapFrame *kernel_worker_yield(void);

void kernel_worker_poll(void);

void kernel_worker_end(Domain *d);

void kernel_worker_memory_end(Domain *d);

void kernel_worker_fault(Domain *d);

bool kernel_worker_fault_begin(Domain *d);

bool kernel_worker_fault_matches(Domain *d, const PgFaultRecord *record);

bool kernel_worker_fault_complete(Domain *d, const PgFaultRecord *record);

bool kernel_worker_fault_reject(Domain *d, const PgFaultRecord *record);

bool kernel_worker_all_dead(uint32_t target);

TrapFrame *kernel_worker_syscall(Domain *d, TrapFrame *frame);

void kernel_fpu_restore(Domain *d);

extern ProgramWorker *kernel_current_worker;
extern ProgramWorker *kernel_outgoing_worker;

uint32_t mask_for_key(uint8_t key);

const Cap *cap_get(Domain *d, uint64_t slot);

bool cap_set(Domain *d, uint64_t slot, Cap c);

bool cap_install_lend(Domain *d, uint64_t slot, Cap c);

bool cap_boot_available(uint32_t count);

bool boot_backing_reserve(uint32_t id, uint64_t length, BootBacking *out);

bool boot_region_map(uint32_t holder, const BootBacking *backing, bool writable, uint64_t *out);

bool boot_region_dma(uint32_t holder, uint64_t slot, uint32_t id, uint64_t length);

bool boot_region_pair(uint32_t first, uint32_t second, uint64_t first_slot, uint64_t second_slot, uint32_t id, uint64_t length, bool second_writable);

bool boot_region_shared_pair(uint32_t first, uint32_t second, uint64_t first_slot, uint64_t second_slot, uint32_t id, uint64_t length);

bool boot_epoch_log_region(uint32_t holder, uint64_t *physical_out);

bool boot_record_image_region(uint32_t holder, uint64_t *physical_out);

/*Display's two BOOT frame buffers: checked packed-XRGB geometry and reservation sizes, as crates/sex-pdx/src/display_frames.rs.*/

#define DISPLAY_FRAME_SLOT 98u

#define DISPLAY_FRAME_REGION_ID 11u

#define DISPLAY_FRAME_MAX_WIDTH 8192u

#define DISPLAY_FRAME_MAX_HEIGHT 4320u

typedef struct DisplayFrameLayout {
  uint64_t width;
  uint64_t height;
  uint32_t pitch;
  uint64_t frame_bytes;
  uint64_t span_bytes;
  uint64_t total_bytes;
  uint64_t buddy_bytes;
}
DisplayFrameLayout;

bool display_frame_layout(uint64_t width, uint64_t height, uint64_t pitch, DisplayFrameLayout *out);

void kernel_boot_display_frames(uint32_t display, const DisplayFrameLayout *layout);

MemoryRegionReply kernel_region_reply(Domain *d, uint64_t number, uint64_t slot, uint64_t a, uint64_t b, uint64_t c);

bool domain_enqueue(Domain *d, RingEntry entry);

bool domain_dequeue(Domain *d, RingEntry *out);

RingEntry kernel_listen_message(Domain *d, uint64_t slot);

bool kernel_rpc_claim(Domain *receiver, const RingEntry *message);

void kernel_rpc_syscall(Domain *d, TrapFrame *frame, uint64_t number);

bool input_push(uint8_t scancode);

bool input_pop(uint8_t *out);

void run_push(uint32_t id);

uintptr_t run_pop(void);

TrapFrame *switch_to(uintptr_t id);

void kernel_fpu_save_current(void);

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
