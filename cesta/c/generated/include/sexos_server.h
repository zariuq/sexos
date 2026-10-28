#if !defined(SEXOS_CESTA_SERVER_H)
#define SEXOS_CESTA_SERVER_H 1

#include "sexos_client.h"

/*Server-side calls of crates/sex-pdx beyond the client interface.*/

/*Capability error sentinels; must match kernel/src/ipc.rs.*/

#define SEXOS_ERR_SERVICE_NOT_READY UINT64_C(0xFFFFFFFFFFFFFFFE)

#define SEXOS_ERR_CAP_INVALID UINT64_C(0xFFFFFFFFFFFFFFFC)

/*get_ticks: the kernel tick counter (0 under QEMU TCG).*/

uint64_t sexos_get_ticks(void);

/*pdx_try_listen_raw: one listen; when nothing waits, yield once and report none.*/

bool sexos_try_listen(uint64_t slot, SexosMessage *out);

/*sys_set_state: announce the service state (SEXOS_SERVICE_STATE_*).*/

uint64_t sexos_set_state(uint64_t state);

/*Memory syscalls of sex-rt; each returns all ones on failure.*/

uint64_t sexos_allocate_memory(uint64_t size);

uint64_t sexos_map_memory(uint64_t pfn, uint64_t size);

/*MemLend syscalls of sex-pdx (sys_grant_mem_lend / sys_map_mem_lend).*/

uint64_t sexos_grant_mem_lend(uint64_t domain_slot, uint64_t length, uint64_t lend_slot);

uint64_t sexos_map_mem_lend(uint64_t cap_slot);

/*heap_init of sex-rt: reserve the first 64 KiB of heap.*/

void sexos_heap_init(void);

/*pdx_reply: answer the caller of the message being served.*/

uint64_t sexos_reply(uint32_t target_pd, uint64_t value);

void silkbar_main(void);

void sexinput_main(void);

void sexstore_main(void);

void sexdisplay_main(void);

void sexbell_main(void);

void linen_main(void);

void quil_main(void);

void spindle_main(void);

void purple_scanout_main(void);

void sexfiles_main(void);

/*core::hint::spin_loop: one pause, a hint to the processor inside a busy wait.*/

void sexos_spin_loop_hint(void);

/*A Rust panic handler that spins: never returns.*/

_Noreturn void sexos_spin_forever(void);

#endif
