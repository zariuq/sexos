#if !defined(SEXOS_SEXSTORE_H)
#define SEXOS_SEXSTORE_H 1

#include "sexos_client.h"

#include "sexstore_pages.h"

/*sexstore keeps 16 key-value slots in RAM and persists them as two 512-byte pages, written alternately (servers/sexstore). The page and record formats are checked code in sexstore_pages.*/

#define KV_SLOT_COUNT ((size_t)16)

#define DURABLE_PAGE_SIZE ((size_t)512)

#define DURABLE_RECORD_COUNT ((size_t)16)

#define DURABLE_PAGE_A_OFFSET ((size_t)0)

#define DURABLE_PAGE_B_OFFSET ((size_t)512)

/*Slot states. A generation of 0 means never written; writes count 1..255, wrapping to 1.*/

#define SLOT_EMPTY ((uint8_t)0)

#define SLOT_ACTIVE ((uint8_t)1)

#define SLOT_TOMBSTONED ((uint8_t)2)

typedef struct KvSlot {
  uint8_t state;
  uint8_t generation;
  uint32_t key;
  uint64_t val;
}
KvSlot;

typedef struct KvTable {
  KvSlot slots[KV_SLOT_COUNT];
}
KvTable;

typedef struct DurablePage {
  uint8_t bytes[DURABLE_PAGE_SIZE];
}
DurablePage;

/*The durable backend: a RAM-backed region for now. Only page_read and page_write touch it, so persistent media replaces those two.*/

typedef struct Durable {
  uint8_t region[2 * DURABLE_PAGE_SIZE];
  uint32_t write_log;
  uint32_t write_fail_log;
}
Durable;

typedef struct DurableLoad {
  uint32_t total;
  uint32_t valid;
  uint32_t corrupt;
}
DurableLoad;

Durable sexstore_durable_new(void);

/*A snapshot of the table under sequence number SEQ.*/

DurablePage sexstore_durable_build_page(const KvTable *table, uint32_t seq);

/*First boot (both pages invalid): write the table as page A, sequence 1.*/

bool sexstore_durable_init(Durable *durable, const KvTable *table);

/*Load the page with the higher sequence (page A on a tie) into the table; invalid records leave their slots alone.*/

DurableLoad sexstore_durable_load_into_ram(Durable *durable, KvTable *table);

/*Write a snapshot to the inactive page and verify it by reading back.*/

bool sexstore_durable_write_all(Durable *durable, const KvTable *table);

#endif
