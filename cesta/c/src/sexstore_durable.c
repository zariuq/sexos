#include "sexstore.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
/*Two validated snapshots with reserved-zero cyclic sequence numbers. Publication replaces the inactive page; recovery assumes valid pages lie within one half-cycle. Checksums detect damage but do not prove media atomicity.*/
/*A failed check in the page codec (an out-of-range view or index) is fatal.*/
void sexstore_pages_failure(void) {
  sexos_fail();
}
static bool take(uint32_t *budget) {
  if (*budget == 0) return false;
  *budget -= 1;
  return true;
}
static cesta_read_bytes page_view(const DurablePage *page) {
  return (cesta_read_bytes) {
    .data = page->bytes,.len = DURABLE_PAGE_SIZE
  };
}
Durable sexstore_durable_new(void) {
  return (Durable) {
    .write_log = 16,.write_fail_log = 8
  };
}
static void page_read(const Durable *durable, size_t page_offset, DurablePage *page) {
  if (!(page_offset == DURABLE_PAGE_A_OFFSET || page_offset == DURABLE_PAGE_B_OFFSET)) sexos_fail();
  for (size_t i = 0; i < DURABLE_PAGE_SIZE; ++i) page->bytes[i] = durable->region[page_offset + i];
}
/*Write, then verify by reading back: always equal in RAM, kept for media whose readback can differ.*/
static bool page_write(Durable *durable, size_t page_offset, const DurablePage *page) {
  if (!(page_offset == DURABLE_PAGE_A_OFFSET || page_offset == DURABLE_PAGE_B_OFFSET)) sexos_fail();
  for (size_t i = 0; i < DURABLE_PAGE_SIZE; ++i) durable->region[page_offset + i] = page->bytes[i];
  DurablePage readback = {
    0
  };
  page_read(durable, page_offset, &readback);
  for (size_t i = 0; i < DURABLE_PAGE_SIZE; ++i) if (readback.bytes[i] != page->bytes[i]) return false;
  return true;
}
/*The sequence number of a valid page, or 0.*/
static uint32_t page_seq(const DurablePage *page) {
  return sexstore_pages_page_seq(page_view(page));
}
/*Zero denotes absence. The live cycle has UINT32_MAX values because zero is reserved. A tie selects the first page; an unrestricted history has no globally transitive serial order.*/
static bool sequence_current(uint32_t first, uint32_t second) {
  if (first == 0) return second == 0;
  if (second == 0 || first == second) return true;
  uint32_t distance = second <= first ? first - second : UINT32_MAX - second + first;
  return distance <= UINT32_MAX / 2u;
}
DurablePage sexstore_durable_build_page(const KvTable *table, uint32_t seq) {
  DurablePage page = {
    0
  };
  {
    cesta_write_bytes out = (cesta_write_bytes) {
      .data = page.bytes,.len = DURABLE_PAGE_SIZE
    };
    sexstore_pages_page_begin(out, seq);
    for (size_t i = 0; i < DURABLE_RECORD_COUNT; ++i) {
      const KvSlot *slot = &table->slots[i];
      sexstore_pages_page_put_record(out, i, slot->state, slot->generation, slot->key, slot->val);
    }
    sexstore_pages_page_seal(out);
  }
  return page;
}
bool sexstore_durable_write_all(Durable *durable, const KvTable *table) {
  DurablePage page_a = {
    0
  };
  DurablePage page_b = {
    0
  };
  page_read(durable, DURABLE_PAGE_A_OFFSET, &page_a);
  page_read(durable, DURABLE_PAGE_B_OFFSET, &page_b);
  uint32_t seq_a = page_seq(&page_a);
  uint32_t seq_b = page_seq(&page_b);
  bool a_current = sequence_current(seq_a, seq_b);
  size_t target = a_current ? DURABLE_PAGE_B_OFFSET : DURABLE_PAGE_A_OFFSET;
  uint32_t next_seq = (a_current ? seq_a : seq_b) + 1u;
  if (next_seq == 0) next_seq = 1;
  DurablePage snapshot = sexstore_durable_build_page(table, next_seq);
  if (page_write(durable, target, &snapshot)) {
/*The first slot's key stands for the snapshot; every slot is written.*/
    if (take(&durable->write_log)) sexos_line("[sexstore.durable.write] key=%u seq=%u page=%s", (unsigned) table->slots[0].key, (unsigned) next_seq, target == DURABLE_PAGE_A_OFFSET ? "A" : "B");
    return true;
  }
  if (take(&durable->write_fail_log)) sexos_line("[sexstore.durable.write.fail] reason=verify_fail");
  return false;
}
static DurableLoad load_page(const DurablePage *authoritative, KvTable *table) {
  DurableLoad load = {
    0
  };
  uint32_t auth_seq = page_seq(authoritative);
  if (auth_seq == 0) return load;
  cesta_read_bytes page = page_view(authoritative);
  for (size_t i = 0; i < DURABLE_RECORD_COUNT; ++i) {
    load.total += 1;
    if (!sexstore_pages_record_valid(page, i)) {
      load.corrupt += 1;
      continue;
    }
    KvSlot *slot = &table->slots[i];
    slot->state = sexstore_pages_record_state(page, i);
    slot->generation = sexstore_pages_record_generation(page, i);
    slot->key = sexstore_pages_record_key(page, i);
    slot->val = sexstore_pages_record_value(page, i);
    load.valid += 1;
  }
  sexos_line("[sexstore.durable.load] seq=%u records=%u valid=%u corrupt=%u", (unsigned) auth_seq, (unsigned) load.total, (unsigned) load.valid, (unsigned) load.corrupt);
  if (load.total > 0 && load.valid == 0) sexos_line("[sexstore.durable.all_corrupt] reason=all_records_invalid");
  return load;
}
DurableLoad sexstore_durable_load_into_ram(const Durable *durable, KvTable *table) {
  DurablePage page_a = {
    0
  };
  DurablePage page_b = {
    0
  };
  page_read(durable, DURABLE_PAGE_A_OFFSET, &page_a);
  page_read(durable, DURABLE_PAGE_B_OFFSET, &page_b);
  uint32_t seq_a = page_seq(&page_a);
  uint32_t seq_b = page_seq(&page_b);
  if (sequence_current(seq_a, seq_b)) return load_page(&page_a, table);
  return load_page(&page_b, table);
}
bool sexstore_durable_init(Durable *durable, const KvTable *table) {
  DurablePage page_a = {
    0
  };
  DurablePage page_b = {
    0
  };
  page_read(durable, DURABLE_PAGE_A_OFFSET, &page_a);
  page_read(durable, DURABLE_PAGE_B_OFFSET, &page_b);
  if (page_seq(&page_a) != 0 || page_seq(&page_b) != 0) return false;
  DurablePage snapshot = sexstore_durable_build_page(table, 1);
  bool ok = page_write(durable, DURABLE_PAGE_A_OFFSET, &snapshot);
  sexos_line("[sexstore.durable.load] seq=1 records=16 valid=16 corrupt=0 init=%s", ok ? "ok" : "fail");
  return true;
}
