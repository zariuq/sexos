#include "sexstore.h"
/*The durable backend of sexstore (E13 in servers/sexstore/src/main.rs): a dual-page atomic swap. Every write goes to the inactive page and carries the next sequence number, so a torn write leaves the previous page authoritative.*/
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
  for (size_t i = 0; i < DURABLE_PAGE_SIZE; ++i) page->bytes[i] = durable->region[page_offset + i];
}
/*Write, then verify by reading back: always equal in RAM, kept for media whose readback can differ.*/
static bool page_write(Durable *durable, size_t page_offset, const DurablePage *page) {
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
DurablePage sexstore_durable_build_page(const KvTable *table, uint32_t seq) {
  DurablePage page = {
    0
  };
  cesta_write_bytes out = (cesta_write_bytes) {
    .data = page.bytes,.len = DURABLE_PAGE_SIZE
  };
  sexstore_pages_page_begin(out, seq);
  for (size_t i = 0; i < DURABLE_RECORD_COUNT; ++i) {
    const KvSlot *slot = &table->slots[i];
    sexstore_pages_page_put_record(out, i, slot->state, slot->generation, slot->key, slot->val);
  }
  sexstore_pages_page_seal(out);
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
/*The target is the inactive page, the one with the lower sequence; page B on a tie. Sequence numbers wrap from UINT32_MAX to 1, as the Rust release build does, since 0 marks an uninitialized page.*/
  size_t target = seq_a >= seq_b ? DURABLE_PAGE_B_OFFSET : DURABLE_PAGE_A_OFFSET;
  uint32_t next_seq = (seq_a > seq_b ? seq_a : seq_b) + 1u;
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
DurableLoad sexstore_durable_load_into_ram(Durable *durable, KvTable *table) {
  DurableLoad load = {
    0
  };
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
  const DurablePage *authoritative = seq_a >= seq_b ? &page_a : &page_b;
  uint32_t auth_seq = seq_a >= seq_b ? seq_a : seq_b;
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
