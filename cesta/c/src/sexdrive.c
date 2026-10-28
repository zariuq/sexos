#include "sexos_mmio.h"
/*NVMe storage driver and block service (apps/sexdrive/src/main.rs).*/
#if !defined(SEXOS_STORAGE_100_PERSIST_WRITE)
#define SEXOS_STORAGE_100_PERSIST_WRITE 0

#endif
#if !defined(SEXOS_STORAGE_100_PERSIST_READ)
#define SEXOS_STORAGE_100_PERSIST_READ 0

#endif
#if !defined(SEXOS_STORAGE_100_FLUSH_AUDIT)
#define SEXOS_STORAGE_100_FLUSH_AUDIT 0

#endif
#if !defined(SEXOS_STORAGE_100_NEGATIVE)
#define SEXOS_STORAGE_100_NEGATIVE 0

#endif
#if !defined(SEXOS_STORAGE_100_NEG_MISMATCH)
#define SEXOS_STORAGE_100_NEG_MISMATCH 0

#endif
#if !defined(SEXOS_STORAGE_100_IO_READ_PROBE)
#define SEXOS_STORAGE_100_IO_READ_PROBE 0

#endif
#define STORAGE_100_PERSIST_WRITE_ENABLED SEXOS_STORAGE_100_PERSIST_WRITE
#define STORAGE_100_PERSIST_READ_ENABLED SEXOS_STORAGE_100_PERSIST_READ
#define STORAGE_100_FLUSH_AUDIT_ENABLED SEXOS_STORAGE_100_FLUSH_AUDIT
#define STORAGE_100_NEGATIVE_ENABLED SEXOS_STORAGE_100_NEGATIVE
#define STORAGE_100_NEG_MISMATCH_ENABLED SEXOS_STORAGE_100_NEG_MISMATCH
#define STORAGE_100_IO_READ_PROBE_ENABLED SEXOS_STORAGE_100_IO_READ_PROBE
#define OP_SHELL_BIND_BUFFER UINT64_C(0x14)
#define SLOT_SHELL ((uint64_t)SEXOS_SLOT_shell)
#define SLOT_USB_HOST UINT64_C(8)
#define SLOT_NVME_HOST UINT64_C(16)
#define SLOT_BUF_LEND ((uint64_t)SEXOS_SLOT_buf_lend)
#define BLOCK_READ UINT64_C(1)
#define BLOCK_WRITE UINT64_C(2)
#define BLOCK_SYNC UINT64_C(3)
#define BLOCK_ERR_BAD_CMD UINT64_C(1)
#define BLOCK_ERR_BAD_LEN UINT64_C(2)
#define BLOCK_ERR_NO_DEVICE UINT64_C(4)
#define BLOCK_SECTOR_SIZE UINT64_C(512)
#define BLOCK_MAX_XFER UINT64_C(4096)
#define PAGE_SIZE UINT64_C(4096)
#define NVME_SQ0TDBL UINT64_C(0x1000)
#define NVME_CQ0HDBL UINT64_C(0x1004)
#define NVME_LBA_BYTES UINT64_C(512)
#define WRITE_PROOF_LBA UINT64_C(2047)
#define WRITE_PROOF_LEN UINT64_C(512)
#define WRITE_PROOF_MAGIC UINT64_C(0x3156455449525753)
#define AP4_MULTI_BASE_LBA UINT64_C(128)
#define AP4_MULTI_BLOCKS UINT64_C(4)
#define AP4_MULTI_BLOCK_BYTES NVME_LBA_BYTES
#define AP5A_PERSIST_BASE_LBA UINT64_C(256)
#define AP5A_PERSIST_BLOCKS UINT64_C(4)
#define AP5A_PERSIST_BLOCK_BYTES NVME_LBA_BYTES
#define AP6_NEG_MISMATCH_LBA UINT64_C(384)
#define AP6_NEG_MISMATCH_BYTES NVME_LBA_BYTES
#define MANIFEST_LBA UINT64_C(2046)
#define PROOF_OBJECT_START_LBA UINT64_C(2038)
#define PROOF_OBJECT_END_LBA UINT64_C(2045)
#define LINEN_OBJECT_START_LBA UINT64_C(2030)
#define LINEN_OBJECT_END_LBA UINT64_C(2037)
#define QUIL_OBJECT_START_LBA UINT64_C(2022)
#define QUIL_OBJECT_END_LBA UINT64_C(2029)
#define SEXFS_V0_META_START_LBA UINT64_C(0)
#define SEXFS_V0_META_END_LBA UINT64_C(47)
#define SEXFS_V0_OBJECT_START_LBA UINT64_C(128)
#define SEXFS_V0_OBJECT_END_LBA UINT64_C(2019)
typedef struct {
  bool ready;
  uint64_t map_va;
  uint64_t io_sq_va;
  uint64_t io_cq_va;
  uint64_t sq1tdbl;
  uint64_t cq1hdbl;
  uint32_t sq_tail;
  uint32_t cq_head;
  uint32_t cq_phase;
  uint16_t next_cid;
}
NvmeIoState;
typedef struct {
  uint32_t hot_log_budget;
  NvmeIoState nvme_io_state;
}
SexdriveState;
static SexdriveState sexdrive_state;
static void sexdrive_state_init(SexdriveState *s) {
  memset(s, 0, sizeof *s);
  s->hot_log_budget = 240;
  s->nvme_io_state.cq_phase = 1;
  s->nvme_io_state.next_cid = 0x0500;
}
static bool hot_log(void) {
  if (sexdrive_state.hot_log_budget > 0) {
    sexdrive_state.hot_log_budget -= 1;
    return true;
  }
  return false;
}
static inline uint64_t sys_alloc_phys(uint64_t size) {
  return sexos_allocate_memory(size);
}
static inline uint64_t sys_map_phys(uint64_t phys, uint64_t size) {
  return sexos_map_memory(phys, size);
}
static uint64_t nvme_read_into_bounce(uint64_t offset, uint64_t size) {
  if (size == 0 || (size > BLOCK_MAX_XFER || (size > PAGE_SIZE || (offset % NVME_LBA_BYTES != 0 || size % NVME_LBA_BYTES != 0)))) {
    sexos_line("[sexdrive.block.read.api.err] reason=bad_len offset=%#llx size=%llu lba=%llu", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) NVME_LBA_BYTES);
    return BLOCK_ERR_BAD_LEN;
  }
  if (!sexdrive_state.nvme_io_state.ready) {
    sexos_line("[sexdrive.block.read.api.err] reason=no_ioq_ready");
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t map_va = sexdrive_state.nvme_io_state.map_va;
  uint64_t io_sq_va = sexdrive_state.nvme_io_state.io_sq_va;
  uint64_t io_cq_va = sexdrive_state.nvme_io_state.io_cq_va;
  uint64_t sq1tdbl = sexdrive_state.nvme_io_state.sq1tdbl;
  uint64_t cq1hdbl = sexdrive_state.nvme_io_state.cq1hdbl;
  uint32_t sq_tail = sexdrive_state.nvme_io_state.sq_tail;
  uint32_t cq_head = sexdrive_state.nvme_io_state.cq_head;
  uint32_t cq_phase = sexdrive_state.nvme_io_state.cq_phase;
  uint16_t cid = sexdrive_state.nvme_io_state.next_cid;
  sexdrive_state.nvme_io_state.next_cid = (uint16_t)(cid + 1u);
  uint64_t data_phys = sys_alloc_phys(PAGE_SIZE);
  if (data_phys == 0 || (data_phys == UINT64_MAX || data_phys % PAGE_SIZE != 0)) {
    sexos_line("[sexdrive.block.read.api.err] reason=data_alloc_invalid phys=%#llx", (unsigned long long) data_phys);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t data_va = sys_map_phys(data_phys, PAGE_SIZE);
  if (data_va == 0 || data_va == UINT64_MAX) {
    sexos_line("[sexdrive.block.read.api.err] reason=data_map_failed va=%#llx", (unsigned long long) data_va);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t *zero_ptr = (uint64_t *)(uintptr_t) data_va;
  for (size_t i = 0; i < (size_t)(PAGE_SIZE / 8ULL); ++i) sexos_volatile_write64(zero_ptr + i, 0ULL);
  uint64_t slba = offset / NVME_LBA_BYTES;
  uint64_t nlb = size / NVME_LBA_BYTES - 1ULL;
  uint32_t *sqe_ptr = (uint32_t *)(uintptr_t)(io_sq_va + (uint64_t) sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(sqe_ptr + i, 0u);
  sexos_volatile_write32(sqe_ptr + 0, 0x02u | (uint32_t) cid << 16);
  sexos_volatile_write32(sqe_ptr + 1, 1u);
  sexos_volatile_write32(sqe_ptr + 6, (uint32_t)(data_phys & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 7, (uint32_t)(data_phys >> 32));
  sexos_volatile_write32(sqe_ptr + 10, (uint32_t)(slba & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 11, (uint32_t)(slba >> 32));
  sexos_volatile_write32(sqe_ptr + 12, (uint32_t)(nlb & 0xFFFFULL));
  sexos_line("[sexdrive.block.read.api.nvme.submit] cid=%llu nsid=%llu slba=%llu nlb=%llu prp1=%#llx sq_tail=%llu", (unsigned long long) cid, 1ULL, (unsigned long long) slba, (unsigned long long) nlb, (unsigned long long) data_phys, (unsigned long long) sq_tail);
  if (hot_log()) sexos_line("[sexdrive.nvme.submit.detail] path=typed op=READ cid=%llu sq_tail_before=%llu sq_tail_after=%llu cq_head=%llu cq_phase=%llu lba=%llu bytes=%llu", (unsigned long long) cid, (unsigned long long) sq_tail, (unsigned long long)((sq_tail + 1u) % 16u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) slba, (unsigned long long) size);
  sexos_compiler_fence_seq_cst();
  sq_tail = (sq_tail + 1u) % 16u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + sq1tdbl), sq_tail);
  if (hot_log()) sexos_line("[sexdrive.nvme.doorbell] path=typed qid=1 tail=%llu", (unsigned long long) sq_tail);
  bool done = false;
  uint32_t dw2 = 0;
  uint32_t dw3 = 0;
  uint32_t max_polls = 1000000u;
  uint32_t polls = 0;
  uint32_t last_dw3 = 0;
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.begin] path=typed cid=%llu cq_head=%llu phase=%llu max_polls=%llu", (unsigned long long) cid, (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) max_polls);
  for (uint32_t p = 0; p < max_polls; ++p) {
    polls += 1u;
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(io_cq_va + (uint64_t) cq_head * 16ULL);
    uint32_t rd_dw2 = sexos_volatile_read32(cqe_ptr + 2);
    uint32_t rd_dw3 = sexos_volatile_read32(cqe_ptr + 3);
    last_dw3 = rd_dw3;
    uint32_t cid_now = rd_dw3 & 0xFFFFu;
    uint32_t phase_now = rd_dw3 >> 16 & 0x1u;
    if (cid_now == (uint32_t) cid && phase_now == cq_phase) {
      done = true;
      dw2 = rd_dw2;
      dw3 = rd_dw3;
      sexos_line("[sexdrive.block.read.api.cqe] cid=%llu phase=%llu dw2=%#llx dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) rd_dw2, (unsigned long long) rd_dw3);
      break;
    }
  }
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.end] path=typed cid=%llu found=%llu status=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) cid, (unsigned long long)(done ? 1ULL : 0ULL), (unsigned long long)(dw3 >> 17 & 0x7FFFu), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
  if (!done) {
    sexos_line("[sexdrive.block.read.api.err] reason=cqe_timeout cid=%llu head=%llu phase=%llu", (unsigned long long) cid, (unsigned long long) cq_head, (unsigned long long) cq_phase);
    sexos_line("[sexdrive.nvme.poll.timeout.detail] path=typed cid=%llu expected_cid=%llu seen_cid=%llu seen_status=%llu seen_phase=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) cid, (unsigned long long) cid, (unsigned long long)(last_dw3 & 0xFFFFu), (unsigned long long)(last_dw3 >> 17 & 0x7FFFu), (unsigned long long)(last_dw3 >> 16 & 0x1u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint32_t sf = dw3 >> 17 & 0x7FFFu;
  uint32_t sc = sf & 0xFFu;
  uint32_t sct = sf >> 8 & 0x7u;
  if (sc != 0 || sct != 0) {
    uint32_t sqhd = dw2 & 0xFFFFu;
    uint32_t sqid = dw2 >> 16 & 0xFFFFu;
    sexos_line("[sexdrive.block.read.api.err] reason=status_fail cid=%llu sc=%llu sct=%llu sqhd=%llu sqid=%llu dw3=%#llx", (unsigned long long) cid, (unsigned long long) sc, (unsigned long long) sct, (unsigned long long) sqhd, (unsigned long long) sqid, (unsigned long long) dw3);
    return BLOCK_ERR_NO_DEVICE;
  }
  cq_head = (cq_head + 1u) % 16u;
  if (cq_head == 0u) cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + cq1hdbl), cq_head);
  sexdrive_state.nvme_io_state.sq_tail = sq_tail;
  sexdrive_state.nvme_io_state.cq_head = cq_head;
  sexdrive_state.nvme_io_state.cq_phase = cq_phase;
  uint64_t d0 = sexos_volatile_read64((const uint64_t *)(uintptr_t) data_va);
  uint64_t d1 = sexos_volatile_read64((const uint64_t *)(uintptr_t)(data_va + 8ULL));
  sexos_line("[sexdrive.block.read.api.ok] cid=%llu slba=%llu nlb=%llu d0=%#llx d1=%#llx", (unsigned long long) cid, (unsigned long long) slba, (unsigned long long) nlb, (unsigned long long) d0, (unsigned long long) d1);
  return 0ULL;
}
static uint64_t nvme_read_into_mapped_va(uint64_t offset, uint64_t size, uint64_t dst_va) {
  if (size == 0 || (size > BLOCK_MAX_XFER || (size > PAGE_SIZE || (offset % NVME_LBA_BYTES != 0 || (size % NVME_LBA_BYTES != 0 || (dst_va == 0 || dst_va == UINT64_MAX)))))) {
    sexos_line("[sexdrive.block.read.handoff.err] reason=bad_args offset=%#llx size=%llu dst_va=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) dst_va);
    return BLOCK_ERR_BAD_LEN;
  }
  if (hot_log()) sexos_line("[sexdrive.block.read.handoff.nvme.begin] offset=%#llx size=%llu dst_va=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) dst_va);
  if (!sexdrive_state.nvme_io_state.ready) {
    sexos_line("[sexdrive.block.read.handoff.err] reason=no_ioq_ready");
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t map_va = sexdrive_state.nvme_io_state.map_va;
  uint64_t io_sq_va = sexdrive_state.nvme_io_state.io_sq_va;
  uint64_t io_cq_va = sexdrive_state.nvme_io_state.io_cq_va;
  uint64_t sq1tdbl = sexdrive_state.nvme_io_state.sq1tdbl;
  uint64_t cq1hdbl = sexdrive_state.nvme_io_state.cq1hdbl;
  uint32_t sq_tail = sexdrive_state.nvme_io_state.sq_tail;
  uint32_t cq_head = sexdrive_state.nvme_io_state.cq_head;
  uint32_t cq_phase = sexdrive_state.nvme_io_state.cq_phase;
  uint16_t cid = sexdrive_state.nvme_io_state.next_cid;
  sexdrive_state.nvme_io_state.next_cid = (uint16_t)(cid + 1u);
  uint64_t data_phys = sys_alloc_phys(PAGE_SIZE);
  if (data_phys == 0 || (data_phys == UINT64_MAX || data_phys % PAGE_SIZE != 0)) {
    sexos_line("[sexdrive.block.read.handoff.err] reason=data_alloc_invalid phys=%#llx", (unsigned long long) data_phys);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t data_va = sys_map_phys(data_phys, PAGE_SIZE);
  if (data_va == 0 || data_va == UINT64_MAX) {
    sexos_line("[sexdrive.block.read.handoff.err] reason=data_map_failed va=%#llx", (unsigned long long) data_va);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t *zero_ptr = (uint64_t *)(uintptr_t) data_va;
  for (size_t i = 0; i < (size_t)(PAGE_SIZE / 8ULL); ++i) sexos_volatile_write64(zero_ptr + i, 0ULL);
  uint64_t slba = offset / NVME_LBA_BYTES;
  uint64_t nlb = size / NVME_LBA_BYTES - 1ULL;
  uint64_t max_lba = WRITE_PROOF_LBA;
  uint64_t last_lba = slba + nlb;
  uint64_t range_ok = slba > max_lba ? 0ULL : nlb > max_lba - slba ? 0ULL : 1ULL;
  if (hot_log()) sexos_line("[sexdrive.nvme.cmd.range] path=typed slba=%llu nlb=%llu max_lba=%llu ok=%llu", (unsigned long long) slba, (unsigned long long) nlb, (unsigned long long) max_lba, (unsigned long long) range_ok);
  uint32_t *sqe_ptr = (uint32_t *)(uintptr_t)(io_sq_va + (uint64_t) sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(sqe_ptr + i, 0u);
  sexos_volatile_write32(sqe_ptr + 0, 0x02u | (uint32_t) cid << 16);
  sexos_volatile_write32(sqe_ptr + 1, 1u);
  sexos_volatile_write32(sqe_ptr + 6, (uint32_t)(data_phys & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 7, (uint32_t)(data_phys >> 32));
  sexos_volatile_write32(sqe_ptr + 10, (uint32_t)(slba & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 11, (uint32_t)(slba >> 32));
  sexos_volatile_write32(sqe_ptr + 12, (uint32_t)(nlb & 0xFFFFULL));
  if (hot_log()) sexos_line("[sexdrive.nvme.cmd.shape] path=typed op=READ cid=%llu nsid=%llu slba=%llu nlb=%llu bytes=%llu prp1=%#llx prp2=%#llx buf_va=%#llx buf_phys=%#llx buffer_cap=%#llx buffer_kind=memlend last_lba=%llu", (unsigned long long) cid, 1ULL, (unsigned long long) slba, (unsigned long long) nlb, (unsigned long long) size, (unsigned long long) data_phys, 0ULL, (unsigned long long) dst_va, (unsigned long long) data_phys, (unsigned long long) SLOT_BUF_LEND, (unsigned long long) last_lba);
  sexos_compiler_fence_seq_cst();
  if (hot_log()) sexos_line("[sexdrive.nvme.submit.detail] path=typed op=READ cid=%llu sq_tail_before=%llu sq_tail_after=%llu cq_head=%llu cq_phase=%llu lba=%llu bytes=%llu", (unsigned long long) cid, (unsigned long long) sq_tail, (unsigned long long)((sq_tail + 1u) % 16u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) slba, (unsigned long long) size);
  sq_tail = (sq_tail + 1u) % 16u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + sq1tdbl), sq_tail);
  if (hot_log()) sexos_line("[sexdrive.nvme.doorbell] path=typed qid=1 tail=%llu", (unsigned long long) sq_tail);
  bool done = false;
  uint32_t dw2 = 0;
  uint32_t dw3 = 0;
  uint32_t max_polls = 1000000u;
  uint32_t polls = 0;
  uint32_t last_dw3 = 0;
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.begin] path=typed cid=%llu cq_head=%llu phase=%llu max_polls=%llu", (unsigned long long) cid, (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) max_polls);
  for (uint32_t p = 0; p < max_polls; ++p) {
    polls += 1u;
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(io_cq_va + (uint64_t) cq_head * 16ULL);
    uint32_t rd_dw2 = sexos_volatile_read32(cqe_ptr + 2);
    uint32_t rd_dw3 = sexos_volatile_read32(cqe_ptr + 3);
    last_dw3 = rd_dw3;
    uint32_t cid_now = rd_dw3 & 0xFFFFu;
    uint32_t phase_now = rd_dw3 >> 16 & 0x1u;
    if (cid_now == (uint32_t) cid && phase_now == cq_phase) {
      done = true;
      dw2 = rd_dw2;
      dw3 = rd_dw3;
      if (hot_log()) sexos_line("[sexdrive.block.read.handoff.nvme.cqe] cid=%llu phase=%llu dw2=%#llx dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) rd_dw2, (unsigned long long) rd_dw3);
      break;
    }
  }
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.end] path=typed cid=%llu found=%llu status=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) cid, (unsigned long long)(done ? 1ULL : 0ULL), (unsigned long long)(dw3 >> 17 & 0x7FFFu), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
  if (!done) {
    sexos_line("[sexdrive.block.read.handoff.err] reason=cqe_timeout cid=%llu head=%llu phase=%llu", (unsigned long long) cid, (unsigned long long) cq_head, (unsigned long long) cq_phase);
    sexos_line("[sexdrive.nvme.poll.timeout.detail] path=typed cid=%llu expected_cid=%llu seen_cid=%llu seen_status=%llu seen_phase=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) cid, (unsigned long long) cid, (unsigned long long)(last_dw3 & 0xFFFFu), (unsigned long long)(last_dw3 >> 17 & 0x7FFFu), (unsigned long long)(last_dw3 >> 16 & 0x1u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint32_t sf = dw3 >> 17 & 0x7FFFu;
  uint32_t sc = sf & 0xFFu;
  uint32_t sct = sf >> 8 & 0x7u;
  if (sc != 0 || sct != 0) {
    uint32_t sqhd = dw2 & 0xFFFFu;
    uint32_t sqid = dw2 >> 16 & 0xFFFFu;
    sexos_line("[sexdrive.block.read.handoff.err] reason=status_fail cid=%llu sc=%llu sct=%llu sqhd=%llu sqid=%llu dw3=%#llx", (unsigned long long) cid, (unsigned long long) sc, (unsigned long long) sct, (unsigned long long) sqhd, (unsigned long long) sqid, (unsigned long long) dw3);
    return BLOCK_ERR_NO_DEVICE;
  }
  cq_head = (cq_head + 1u) % 16u;
  if (cq_head == 0u) cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + cq1hdbl), cq_head);
  sexdrive_state.nvme_io_state.sq_tail = sq_tail;
  sexdrive_state.nvme_io_state.cq_head = cq_head;
  sexdrive_state.nvme_io_state.cq_phase = cq_phase;
  const uint8_t *src = (const uint8_t *)(uintptr_t) data_va;
  uint8_t *dst = (uint8_t *)(uintptr_t) dst_va;
  for (size_t i = 0; i < (size_t) size; ++i) sexos_volatile_write8(dst + i, sexos_volatile_read8(src + i));
  if (hot_log()) sexos_line("[sexdrive.block.read.handoff.copy.ok] phase=B len=%llu", (unsigned long long) size);
  return 0ULL;
}
static bool write_guard_allows(uint64_t offset, uint64_t size, uint64_t buf_cap) {
  uint64_t expected_offset = WRITE_PROOF_LBA * NVME_LBA_BYTES;
  uint64_t manifest_offset = MANIFEST_LBA * NVME_LBA_BYTES;
  uint64_t object_start_offset = PROOF_OBJECT_START_LBA * NVME_LBA_BYTES;
  uint64_t object_end_offset = PROOF_OBJECT_END_LBA * NVME_LBA_BYTES;
  bool proof_mode = buf_cap == SLOT_BUF_LEND;
  bool allow_manifest = proof_mode && (size == WRITE_PROOF_LEN && offset == manifest_offset);
  uint64_t linen_start = LINEN_OBJECT_START_LBA * NVME_LBA_BYTES;
  uint64_t linen_end = LINEN_OBJECT_END_LBA * NVME_LBA_BYTES;
  uint64_t quil_start = QUIL_OBJECT_START_LBA * NVME_LBA_BYTES;
  uint64_t quil_end = QUIL_OBJECT_END_LBA * NVME_LBA_BYTES;
  bool allow_object = proof_mode && (size == WRITE_PROOF_LEN && (offset % NVME_LBA_BYTES == 0 && (offset >= object_start_offset && offset <= object_end_offset)));
  bool allow_linen = proof_mode && (size == WRITE_PROOF_LEN && (offset % NVME_LBA_BYTES == 0 && (offset >= linen_start && offset <= linen_end)));
  bool allow_quil = proof_mode && (size == WRITE_PROOF_LEN && (offset % NVME_LBA_BYTES == 0 && (offset >= quil_start && offset <= quil_end)));
  bool allow_proof = proof_mode && (offset == expected_offset && size == WRITE_PROOF_LEN);
  uint64_t sexfs_meta_start = SEXFS_V0_META_START_LBA * NVME_LBA_BYTES;
  uint64_t sexfs_meta_end = SEXFS_V0_META_END_LBA * NVME_LBA_BYTES;
  uint64_t sexfs_obj_start = SEXFS_V0_OBJECT_START_LBA * NVME_LBA_BYTES;
  uint64_t sexfs_obj_end = SEXFS_V0_OBJECT_END_LBA * NVME_LBA_BYTES;
  bool allow_sexfs_meta = proof_mode && (size == WRITE_PROOF_LEN && (offset % NVME_LBA_BYTES == 0 && (offset >= sexfs_meta_start && offset <= sexfs_meta_end)));
  bool allow_sexfs_obj = proof_mode && (size == WRITE_PROOF_LEN && (offset % NVME_LBA_BYTES == 0 && (offset >= sexfs_obj_start && offset <= sexfs_obj_end)));
  bool allow = allow_proof || (allow_manifest || (allow_object || (allow_linen || (allow_quil || (allow_sexfs_meta || allow_sexfs_obj)))));
  sexos_line("[sexdrive.write.guard.config] proof_lba=%llu proof_offset=%#llx manifest_lba=%llu object_lba_start=%llu object_lba_end=%llu proof_len=%llu magic=%#llx sexfs_meta=%llu-%llu sexfs_obj=%llu-%llu", (unsigned long long) WRITE_PROOF_LBA, (unsigned long long) expected_offset, (unsigned long long) MANIFEST_LBA, (unsigned long long) PROOF_OBJECT_START_LBA, (unsigned long long) PROOF_OBJECT_END_LBA, (unsigned long long) WRITE_PROOF_LEN, (unsigned long long) WRITE_PROOF_MAGIC, (unsigned long long) SEXFS_V0_META_START_LBA, (unsigned long long) SEXFS_V0_META_END_LBA, (unsigned long long) SEXFS_V0_OBJECT_START_LBA, (unsigned long long) SEXFS_V0_OBJECT_END_LBA);
  sexos_line("[sexdrive.write.guard.begin] offset=%#llx size=%llu buf_cap=%#llx proof_mode=%llu", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) buf_cap, (unsigned long long)(proof_mode ? 1ULL : 0ULL));
  uint64_t lba = offset / NVME_LBA_BYTES;
  if (allow) {
    if (allow_sexfs_meta || allow_sexfs_obj) sexos_line("[sexdrive.write_guard.sexfs.allow] lba=%llu size=%llu ok=1", (unsigned long long) lba, (unsigned long long) size);
    else sexos_line("[sexdrive.write.guard.allow] offset=%#llx size=%llu buf_cap=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) buf_cap);
  } else sexos_line("[sexdrive.write_guard.reject] lba=%llu size=%llu ok=1", (unsigned long long) lba, (unsigned long long) size);
  return allow;
}
static uint64_t nvme_write_one_block(uint64_t offset, uint64_t size, uint64_t src_va) {
  if (size != WRITE_PROOF_LEN || (src_va == 0 || src_va == UINT64_MAX)) {
    sexos_line("[sexdrive.nvme.write.err] reason=bad_args offset=%#llx size=%llu src_va=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) src_va);
    return BLOCK_ERR_BAD_LEN;
  }
  if (!sexdrive_state.nvme_io_state.ready) {
    sexos_line("[sexdrive.nvme.write.err] reason=no_ioq_ready");
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t map_va = sexdrive_state.nvme_io_state.map_va;
  uint64_t io_sq_va = sexdrive_state.nvme_io_state.io_sq_va;
  uint64_t io_cq_va = sexdrive_state.nvme_io_state.io_cq_va;
  uint64_t sq1tdbl = sexdrive_state.nvme_io_state.sq1tdbl;
  uint64_t cq1hdbl = sexdrive_state.nvme_io_state.cq1hdbl;
  uint32_t sq_tail = sexdrive_state.nvme_io_state.sq_tail;
  uint32_t cq_head = sexdrive_state.nvme_io_state.cq_head;
  uint32_t cq_phase = sexdrive_state.nvme_io_state.cq_phase;
  uint16_t write_cid = sexdrive_state.nvme_io_state.next_cid;
  sexdrive_state.nvme_io_state.next_cid = (uint16_t)(write_cid + 1u);
  uint64_t write_phys = sys_alloc_phys(PAGE_SIZE);
  uint64_t write_va = sys_map_phys(write_phys, PAGE_SIZE);
  if (write_phys == 0 || (write_phys == UINT64_MAX || (write_phys % PAGE_SIZE != 0 || (write_va == 0 || write_va == UINT64_MAX)))) {
    sexos_line("[sexdrive.nvme.write.err] reason=write_buf_invalid phys=%#llx va=%#llx", (unsigned long long) write_phys, (unsigned long long) write_va);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint8_t *write_ptr = (uint8_t *)(uintptr_t) write_va;
  const uint8_t *src_ptr = (const uint8_t *)(uintptr_t) src_va;
  for (size_t i = 0; i < (size_t) PAGE_SIZE; ++i) sexos_volatile_write8(write_ptr + i, 0u);
  for (size_t i = 0; i < (size_t) WRITE_PROOF_LEN; ++i) sexos_volatile_write8(write_ptr + i, sexos_volatile_read8(src_ptr + i));
  uint64_t slba = offset / NVME_LBA_BYTES;
  uint32_t nlb = 0u;
  uint64_t max_lba = WRITE_PROOF_LBA;
  uint64_t last_lba = slba;
  uint64_t range_ok = slba <= max_lba ? 1ULL : 0ULL;
  if (hot_log()) sexos_line("[sexdrive.nvme.cmd.range] path=typed slba=%llu nlb=%llu max_lba=%llu ok=%llu", (unsigned long long) slba, (unsigned long long) nlb, (unsigned long long) max_lba, (unsigned long long) range_ok);
  uint32_t *sqe_ptr = (uint32_t *)(uintptr_t)(io_sq_va + (uint64_t) sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(sqe_ptr + i, 0u);
  sexos_volatile_write32(sqe_ptr + 0, 0x01u | (uint32_t) write_cid << 16);
  sexos_volatile_write32(sqe_ptr + 1, 1u);
  sexos_volatile_write32(sqe_ptr + 6, (uint32_t)(write_phys & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 7, (uint32_t)(write_phys >> 32));
  sexos_volatile_write32(sqe_ptr + 10, (uint32_t)(slba & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 11, (uint32_t)(slba >> 32));
  sexos_volatile_write32(sqe_ptr + 12, nlb);
  sexos_line("[sexdrive.nvme.write.submit] cid=%llu nsid=1 slba=%llu nlb=0 prp1=%#llx sq_tail=%llu", (unsigned long long) write_cid, (unsigned long long) slba, (unsigned long long) write_phys, (unsigned long long) sq_tail);
  sexos_line("[sexdrive.block.write.api.nvme.submit] cid=%llu nsid=1 slba=%llu nlb=0 prp1=%#llx", (unsigned long long) write_cid, (unsigned long long) slba, (unsigned long long) write_phys);
  if (hot_log()) sexos_line("[sexdrive.nvme.cmd.shape] path=typed op=WRITE cid=%llu nsid=%llu slba=%llu nlb=%llu bytes=%llu prp1=%#llx prp2=%#llx buf_va=%#llx buf_phys=%#llx buffer_cap=%#llx buffer_kind=memlend last_lba=%llu", (unsigned long long) write_cid, 1ULL, (unsigned long long) slba, (unsigned long long) nlb, (unsigned long long) size, (unsigned long long) write_phys, 0ULL, (unsigned long long) src_va, (unsigned long long) write_phys, (unsigned long long) SLOT_BUF_LEND, (unsigned long long) last_lba);
  if (hot_log()) sexos_line("[sexdrive.nvme.submit.detail] path=typed op=WRITE cid=%llu sq_tail_before=%llu sq_tail_after=%llu cq_head=%llu cq_phase=%llu lba=%llu bytes=%llu", (unsigned long long) write_cid, (unsigned long long) sq_tail, (unsigned long long)((sq_tail + 1u) % 16u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) slba, (unsigned long long) size);
  sexos_compiler_fence_seq_cst();
  sq_tail = (sq_tail + 1u) % 16u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + sq1tdbl), sq_tail);
  if (hot_log()) sexos_line("[sexdrive.nvme.doorbell] path=typed qid=1 tail=%llu", (unsigned long long) sq_tail);
  bool done = false;
  uint32_t dw3 = 0;
  uint32_t max_polls = 1000000u;
  uint32_t polls = 0;
  uint32_t last_dw3 = 0;
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.begin] path=typed cid=%llu cq_head=%llu phase=%llu max_polls=%llu", (unsigned long long) write_cid, (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) max_polls);
  for (uint32_t p = 0; p < max_polls; ++p) {
    polls += 1u;
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(io_cq_va + (uint64_t) cq_head * 16ULL);
    uint32_t rd_dw2 = sexos_volatile_read32(cqe_ptr + 2);
    uint32_t rd_dw3 = sexos_volatile_read32(cqe_ptr + 3);
    last_dw3 = rd_dw3;
    uint32_t cid_now = rd_dw3 & 0xFFFFu;
    uint32_t phase_now = rd_dw3 >> 16 & 0x1u;
    if (cid_now == (uint32_t) write_cid && phase_now == cq_phase) {
      done = true;
      dw3 = rd_dw3;
      sexos_line("[sexdrive.nvme.write.cqe] cid=%llu phase=%llu dw2=%#llx dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) rd_dw2, (unsigned long long) rd_dw3);
      sexos_line("[sexdrive.block.write.api.cqe] cid=%llu phase=%llu dw2=%#llx dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) rd_dw2, (unsigned long long) rd_dw3);
      break;
    }
  }
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.end] path=typed cid=%llu found=%llu status=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) write_cid, (unsigned long long)(done ? 1ULL : 0ULL), (unsigned long long)(dw3 >> 17 & 0x7FFFu), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
  if (!done) {
    sexos_line("[sexdrive.nvme.write.err] reason=cqe_timeout cid=%llu head=%llu phase=%llu", (unsigned long long) write_cid, (unsigned long long) cq_head, (unsigned long long) cq_phase);
    sexos_line("[sexdrive.nvme.poll.timeout.detail] path=typed cid=%llu expected_cid=%llu seen_cid=%llu seen_status=%llu seen_phase=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) write_cid, (unsigned long long) write_cid, (unsigned long long)(last_dw3 & 0xFFFFu), (unsigned long long)(last_dw3 >> 17 & 0x7FFFu), (unsigned long long)(last_dw3 >> 16 & 0x1u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint32_t sf = dw3 >> 17 & 0x7FFFu;
  uint32_t sc = sf & 0xFFu;
  uint32_t sct = sf >> 8 & 0x7u;
  if (sc != 0 || sct != 0) {
    sexos_line("[sexdrive.nvme.write.err] reason=status_fail cid=%llu sc=%llu sct=%llu dw3=%#llx", (unsigned long long) write_cid, (unsigned long long) sc, (unsigned long long) sct, (unsigned long long) dw3);
    return BLOCK_ERR_NO_DEVICE;
  }
  cq_head = (cq_head + 1u) % 16u;
  if (cq_head == 0u) cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + cq1hdbl), cq_head);
  sexdrive_state.nvme_io_state.sq_tail = sq_tail;
  sexdrive_state.nvme_io_state.cq_head = cq_head;
  sexdrive_state.nvme_io_state.cq_phase = cq_phase;
  sexos_line("[sexdrive.nvme.write.ok] cid=%llu slba=%llu", (unsigned long long) write_cid, (unsigned long long) slba);
  sexos_line("[sexdrive.block.write.api.ok] cid=%llu slba=%llu", (unsigned long long) write_cid, (unsigned long long) slba);
  return 0ULL;
}
static uint64_t nvme_write_readback_proof(uint64_t offset, uint64_t size, uint64_t src_va) {
  if (size != WRITE_PROOF_LEN || (src_va == 0 || src_va == UINT64_MAX)) {
    sexos_line("[sexdrive.storage100.rw.fail] reason=bad_args");
    sexos_line("[sexdrive.nvme.write.err] reason=bad_args offset=%#llx size=%llu src_va=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) src_va);
    return BLOCK_ERR_BAD_LEN;
  }
  uint64_t expected_offset = WRITE_PROOF_LBA * NVME_LBA_BYTES;
  if (offset != expected_offset) {
    sexos_line("[sexdrive.storage100.rw.fail] reason=guard_offset_mismatch");
    sexos_line("[sexdrive.nvme.write.err] reason=guard_offset_mismatch offset=%#llx expected=%#llx", (unsigned long long) offset, (unsigned long long) expected_offset);
    return BLOCK_ERR_BAD_LEN;
  }
  uint64_t lba = offset / NVME_LBA_BYTES;
  sexos_line("[sexdrive.storage100.rw.begin] lba=%llu bytes=%llu", (unsigned long long) lba, (unsigned long long) size);
  sexos_line("[sexdrive.nvme.write.begin] offset=%#llx size=%llu", (unsigned long long) offset, (unsigned long long) size);
  sexos_line("[sexdrive.block.write.api.nvme.submit] begin=1 offset=%#llx size=%llu", (unsigned long long) offset, (unsigned long long) size);
  if (!sexdrive_state.nvme_io_state.ready) {
    sexos_line("[sexdrive.nvme.write.err] reason=no_ioq_ready");
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t map_va = sexdrive_state.nvme_io_state.map_va;
  uint64_t io_sq_va = sexdrive_state.nvme_io_state.io_sq_va;
  uint64_t io_cq_va = sexdrive_state.nvme_io_state.io_cq_va;
  uint64_t sq1tdbl = sexdrive_state.nvme_io_state.sq1tdbl;
  uint64_t cq1hdbl = sexdrive_state.nvme_io_state.cq1hdbl;
  uint32_t sq_tail = sexdrive_state.nvme_io_state.sq_tail;
  uint32_t cq_head = sexdrive_state.nvme_io_state.cq_head;
  uint32_t cq_phase = sexdrive_state.nvme_io_state.cq_phase;
  uint16_t write_cid = sexdrive_state.nvme_io_state.next_cid;
  sexdrive_state.nvme_io_state.next_cid = (uint16_t)(write_cid + 1u);
  uint16_t read_cid = sexdrive_state.nvme_io_state.next_cid;
  sexdrive_state.nvme_io_state.next_cid = (uint16_t)(read_cid + 1u);
  uint64_t write_phys = sys_alloc_phys(PAGE_SIZE);
  uint64_t write_va = sys_map_phys(write_phys, PAGE_SIZE);
  if (write_phys == 0 || (write_phys == UINT64_MAX || (write_phys % PAGE_SIZE != 0 || (write_va == 0 || write_va == UINT64_MAX)))) {
    sexos_line("[sexdrive.storage100.rw.fail] reason=write_buf_invalid");
    sexos_line("[sexdrive.nvme.write.err] reason=write_buf_invalid phys=%#llx va=%#llx", (unsigned long long) write_phys, (unsigned long long) write_va);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint8_t *write_ptr = (uint8_t *)(uintptr_t) write_va;
  for (size_t i = 0; i < (size_t) PAGE_SIZE; ++i) sexos_volatile_write8(write_ptr + i, 0u);
  for (size_t i = 0; i < (size_t) WRITE_PROOF_LEN; ++i) {
    uint8_t b = (uint8_t)((0xA5u ^ (uint8_t) i ^ 0x3Cu) & 0xFFu);
    sexos_volatile_write8(write_ptr + i, b);
  }
  uint64_t slba = lba;
  uint32_t nlb = 0u;
  uint32_t *sqe_ptr = (uint32_t *)(uintptr_t)(io_sq_va + (uint64_t) sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(sqe_ptr + i, 0u);
  sexos_volatile_write32(sqe_ptr + 0, 0x01u | (uint32_t) write_cid << 16);
  sexos_volatile_write32(sqe_ptr + 1, 1u);
  sexos_volatile_write32(sqe_ptr + 6, (uint32_t)(write_phys & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 7, (uint32_t)(write_phys >> 32));
  sexos_volatile_write32(sqe_ptr + 10, (uint32_t)(slba & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 11, (uint32_t)(slba >> 32));
  sexos_volatile_write32(sqe_ptr + 12, nlb);
  sexos_line("[sexdrive.storage100.write.submit] lba=%llu bytes=%llu", (unsigned long long) slba, (unsigned long long) size);
  sexos_line("[sexdrive.nvme.write.submit] cid=%llu nsid=1 slba=%llu nlb=0 prp1=%#llx sq_tail=%llu", (unsigned long long) write_cid, (unsigned long long) slba, (unsigned long long) write_phys, (unsigned long long) sq_tail);
  sexos_line("[sexdrive.block.write.api.nvme.submit] cid=%llu nsid=1 slba=%llu nlb=0 prp1=%#llx", (unsigned long long) write_cid, (unsigned long long) slba, (unsigned long long) write_phys);
  if (hot_log()) sexos_line("[sexdrive.nvme.cmd.shape] path=selftest op=WRITE cid=%llu nsid=%llu slba=%llu nlb=%llu bytes=%llu prp1=%#llx prp2=%#llx buf_va=%#llx buf_phys=%#llx buffer_kind=internal", (unsigned long long) write_cid, 1ULL, (unsigned long long) slba, (unsigned long long) nlb, (unsigned long long) size, (unsigned long long) write_phys, 0ULL, (unsigned long long) write_va, (unsigned long long) write_phys);
  if (hot_log()) sexos_line("[sexdrive.nvme.submit.detail] path=selftest op=WRITE cid=%llu sq_tail_before=%llu sq_tail_after=%llu cq_head=%llu cq_phase=%llu lba=%llu bytes=%llu", (unsigned long long) write_cid, (unsigned long long) sq_tail, (unsigned long long)((sq_tail + 1u) % 16u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) slba, (unsigned long long) size);
  sexos_compiler_fence_seq_cst();
  sq_tail = (sq_tail + 1u) % 16u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + sq1tdbl), sq_tail);
  if (hot_log()) sexos_line("[sexdrive.nvme.doorbell] path=selftest qid=1 tail=%llu", (unsigned long long) sq_tail);
  bool done = false;
  uint32_t dw3 = 0;
  uint32_t max_polls = 1000000u;
  uint32_t polls = 0;
  uint32_t last_dw3 = 0;
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.begin] path=selftest cid=%llu cq_head=%llu phase=%llu max_polls=%llu", (unsigned long long) write_cid, (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) max_polls);
  for (uint32_t p = 0; p < max_polls; ++p) {
    polls += 1u;
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(io_cq_va + (uint64_t) cq_head * 16ULL);
    uint32_t rd_dw2 = sexos_volatile_read32(cqe_ptr + 2);
    uint32_t rd_dw3 = sexos_volatile_read32(cqe_ptr + 3);
    last_dw3 = rd_dw3;
    uint32_t cid_now = rd_dw3 & 0xFFFFu;
    uint32_t phase_now = rd_dw3 >> 16 & 0x1u;
    if (cid_now == (uint32_t) write_cid && phase_now == cq_phase) {
      done = true;
      dw3 = rd_dw3;
      sexos_line("[sexdrive.nvme.write.cqe] cid=%llu phase=%llu dw2=%#llx dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) rd_dw2, (unsigned long long) rd_dw3);
      sexos_line("[sexdrive.block.write.api.cqe] cid=%llu phase=%llu dw2=%#llx dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) rd_dw2, (unsigned long long) rd_dw3);
      break;
    }
  }
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.end] path=selftest cid=%llu found=%llu status=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) write_cid, (unsigned long long)(done ? 1ULL : 0ULL), (unsigned long long)(dw3 >> 17 & 0x7FFFu), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
  if (!done) {
    sexos_line("[sexdrive.storage100.rw.fail] reason=write_cqe_timeout");
    sexos_line("[sexdrive.nvme.write.err] reason=cqe_timeout cid=%llu head=%llu phase=%llu", (unsigned long long) write_cid, (unsigned long long) cq_head, (unsigned long long) cq_phase);
    sexos_line("[sexdrive.nvme.poll.timeout.detail] path=selftest cid=%llu expected_cid=%llu seen_cid=%llu seen_status=%llu seen_phase=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) write_cid, (unsigned long long) write_cid, (unsigned long long)(last_dw3 & 0xFFFFu), (unsigned long long)(last_dw3 >> 17 & 0x7FFFu), (unsigned long long)(last_dw3 >> 16 & 0x1u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint32_t sf = dw3 >> 17 & 0x7FFFu;
  uint32_t sc = sf & 0xFFu;
  uint32_t sct = sf >> 8 & 0x7u;
  if (sc != 0 || sct != 0) {
    sexos_line("[sexdrive.storage100.write.complete] status=1 bytes=%llu", (unsigned long long) size);
    sexos_line("[sexdrive.storage100.rw.fail] reason=write_status_fail");
    sexos_line("[sexdrive.nvme.write.err] reason=status_fail cid=%llu sc=%llu sct=%llu dw3=%#llx", (unsigned long long) write_cid, (unsigned long long) sc, (unsigned long long) sct, (unsigned long long) dw3);
    return BLOCK_ERR_NO_DEVICE;
  }
  cq_head = (cq_head + 1u) % 16u;
  if (cq_head == 0u) cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + cq1hdbl), cq_head);
  sexos_line("[sexdrive.nvme.write.ok] cid=%llu slba=%llu", (unsigned long long) write_cid, (unsigned long long) slba);
  sexos_line("[sexdrive.block.write.api.ok] cid=%llu slba=%llu", (unsigned long long) write_cid, (unsigned long long) slba);
  sexos_line("[sexdrive.storage100.write.complete] status=0 bytes=%llu", (unsigned long long) size);
  sexos_line("[sexdrive.storage100.read.submit] lba=%llu bytes=%llu", (unsigned long long) slba, (unsigned long long) size);
  sexos_line("[sexdrive.nvme.write.readback.begin] slba=%llu", (unsigned long long) slba);
  uint64_t read_phys = sys_alloc_phys(PAGE_SIZE);
  uint64_t read_va = sys_map_phys(read_phys, PAGE_SIZE);
  if (read_phys == 0 || (read_phys == UINT64_MAX || (read_phys % PAGE_SIZE != 0 || (read_va == 0 || read_va == UINT64_MAX)))) {
    sexos_line("[sexdrive.storage100.rw.fail] reason=readback_buf_invalid");
    sexos_line("[sexdrive.nvme.write.err] reason=readback_buf_invalid phys=%#llx va=%#llx", (unsigned long long) read_phys, (unsigned long long) read_va);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint8_t *read_ptr = (uint8_t *)(uintptr_t) read_va;
  for (size_t i = 0; i < (size_t) PAGE_SIZE; ++i) sexos_volatile_write8(read_ptr + i, 0u);
  uint32_t *sqe_ptr2 = (uint32_t *)(uintptr_t)(io_sq_va + (uint64_t) sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(sqe_ptr2 + i, 0u);
  sexos_volatile_write32(sqe_ptr2 + 0, 0x02u | (uint32_t) read_cid << 16);
  sexos_volatile_write32(sqe_ptr2 + 1, 1u);
  sexos_volatile_write32(sqe_ptr2 + 6, (uint32_t)(read_phys & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr2 + 7, (uint32_t)(read_phys >> 32));
  sexos_volatile_write32(sqe_ptr2 + 10, (uint32_t)(slba & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr2 + 11, (uint32_t)(slba >> 32));
  sexos_volatile_write32(sqe_ptr2 + 12, 0u);
  if (hot_log()) sexos_line("[sexdrive.nvme.cmd.shape] path=selftest op=READ cid=%llu nsid=%llu slba=%llu nlb=%llu bytes=%llu prp1=%#llx prp2=%#llx buf_va=%#llx buf_phys=%#llx buffer_kind=internal", (unsigned long long) read_cid, 1ULL, (unsigned long long) slba, 0ULL, (unsigned long long) size, (unsigned long long) read_phys, 0ULL, (unsigned long long) read_va, (unsigned long long) read_phys);
  sexos_compiler_fence_seq_cst();
  if (hot_log()) sexos_line("[sexdrive.nvme.submit.detail] path=selftest op=READ cid=%llu sq_tail_before=%llu sq_tail_after=%llu cq_head=%llu cq_phase=%llu lba=%llu bytes=%llu", (unsigned long long) read_cid, (unsigned long long) sq_tail, (unsigned long long)((sq_tail + 1u) % 16u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) slba, (unsigned long long) size);
  sq_tail = (sq_tail + 1u) % 16u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + sq1tdbl), sq_tail);
  if (hot_log()) sexos_line("[sexdrive.nvme.doorbell] path=selftest qid=1 tail=%llu", (unsigned long long) sq_tail);
  bool rb_done = false;
  uint32_t rb_dw3 = 0;
  polls = 0;
  last_dw3 = 0;
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.begin] path=selftest cid=%llu cq_head=%llu phase=%llu max_polls=%llu", (unsigned long long) read_cid, (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) max_polls);
  for (uint32_t p = 0; p < max_polls; ++p) {
    polls += 1u;
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(io_cq_va + (uint64_t) cq_head * 16ULL);
    uint32_t rd_dw2 = sexos_volatile_read32(cqe_ptr + 2);
    uint32_t rd_dw3 = sexos_volatile_read32(cqe_ptr + 3);
    last_dw3 = rd_dw3;
    uint32_t cid_now = rd_dw3 & 0xFFFFu;
    uint32_t phase_now = rd_dw3 >> 16 & 0x1u;
    if (cid_now == (uint32_t) read_cid && phase_now == cq_phase) {
      rb_done = true;
      rb_dw3 = rd_dw3;
      sexos_line("[sexdrive.nvme.write.readback.cqe] cid=%llu phase=%llu dw2=%#llx dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) rd_dw2, (unsigned long long) rd_dw3);
      break;
    }
  }
  if (hot_log()) sexos_line("[sexdrive.nvme.poll.end] path=selftest cid=%llu found=%llu status=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) read_cid, (unsigned long long)(rb_done ? 1ULL : 0ULL), (unsigned long long)(rb_dw3 >> 17 & 0x7FFFu), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
  if (!rb_done) {
    sexos_line("[sexdrive.storage100.rw.fail] reason=read_cqe_timeout");
    sexos_line("[sexdrive.nvme.write.err] reason=readback_timeout cid=%llu head=%llu phase=%llu", (unsigned long long) read_cid, (unsigned long long) cq_head, (unsigned long long) cq_phase);
    sexos_line("[sexdrive.nvme.poll.timeout.detail] path=selftest cid=%llu expected_cid=%llu seen_cid=%llu seen_status=%llu seen_phase=%llu cq_head=%llu phase=%llu polls=%llu", (unsigned long long) read_cid, (unsigned long long) read_cid, (unsigned long long)(last_dw3 & 0xFFFFu), (unsigned long long)(last_dw3 >> 17 & 0x7FFFu), (unsigned long long)(last_dw3 >> 16 & 0x1u), (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) polls);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint32_t rb_sf = rb_dw3 >> 17 & 0x7FFFu;
  uint32_t rb_sc = rb_sf & 0xFFu;
  uint32_t rb_sct = rb_sf >> 8 & 0x7u;
  if (rb_sc != 0 || rb_sct != 0) {
    sexos_line("[sexdrive.storage100.read.complete] status=1 bytes=%llu", (unsigned long long) size);
    sexos_line("[sexdrive.storage100.rw.fail] reason=read_status_fail");
    sexos_line("[sexdrive.nvme.write.err] reason=readback_status_fail cid=%llu sc=%llu sct=%llu dw3=%#llx", (unsigned long long) read_cid, (unsigned long long) rb_sc, (unsigned long long) rb_sct, (unsigned long long) rb_dw3);
    return BLOCK_ERR_NO_DEVICE;
  }
  cq_head = (cq_head + 1u) % 16u;
  if (cq_head == 0u) cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + cq1hdbl), cq_head);
  sexdrive_state.nvme_io_state.sq_tail = sq_tail;
  sexdrive_state.nvme_io_state.cq_head = cq_head;
  sexdrive_state.nvme_io_state.cq_phase = cq_phase;
  sexos_line("[sexdrive.storage100.read.complete] status=0 bytes=%llu", (unsigned long long) size);
  bool mismatch = false;
  for (size_t i = 0; i < (size_t) WRITE_PROOF_LEN; ++i) {
    uint8_t expect = (uint8_t)((0xA5u ^ (uint8_t) i ^ 0x3Cu) & 0xFFu);
    uint8_t got = sexos_volatile_read8((const uint8_t *)(uintptr_t) read_va + i);
    if (got != expect) {
      mismatch = true;
      break;
    }
  }
  if (mismatch) {
    sexos_line("[sexdrive.storage100.rw.fail] reason=byte_mismatch");
    sexos_line("[sexdrive.storage100.read.match] lba=%llu bytes=%llu ok=0", (unsigned long long) slba, (unsigned long long) size);
    return BLOCK_ERR_NO_DEVICE;
  }
  sexos_line("[sexdrive.storage100.read.match] lba=%llu bytes=%llu ok=1", (unsigned long long) slba, (unsigned long long) size);
  sexos_line("[sexdrive.storage100.rw.done] ok=1");
  return 0ULL;
}
static uint64_t nvme_multiblock_write_readback_proof(void) {
  sexos_line("[sexdrive.storage100.multi.begin] base_lba=%llu blocks=%llu bytes_per_block=%llu", (unsigned long long) AP4_MULTI_BASE_LBA, (unsigned long long) AP4_MULTI_BLOCKS, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
  for (uint64_t b = 0; b < AP4_MULTI_BLOCKS; ++b) {
    uint64_t lba = AP4_MULTI_BASE_LBA + b;
    sexos_line("[sexdrive.storage100.multi.block.begin] idx=%llu lba=%llu bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
    uint64_t write_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t write_va = sys_map_phys(write_phys, PAGE_SIZE);
    if (write_phys == 0 || (write_phys == UINT64_MAX || (write_phys % PAGE_SIZE != 0 || (write_va == 0 || write_va == UINT64_MAX)))) {
      sexos_line("[sexdrive.storage100.multi.fail] reason=write_buf_invalid idx=%llu lba=%llu", (unsigned long long) b, (unsigned long long) lba);
      return BLOCK_ERR_NO_DEVICE;
    }
    uint8_t *write_ptr = (uint8_t *)(uintptr_t) write_va;
    for (size_t i = 0; i < (size_t) AP4_MULTI_BLOCK_BYTES; ++i) {
      uint8_t val = (uint8_t)((0xA5u ^ (uint8_t) i ^ (uint8_t)((uint8_t) b * 0x33u) ^ 0x3Cu) & 0xFFu);
      sexos_volatile_write8(write_ptr + i, val);
    }
    sexos_line("[sexdrive.storage100.multi.write.submit] idx=%llu lba=%llu bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
    uint64_t write_status = nvme_write_one_block(lba * NVME_LBA_BYTES, AP4_MULTI_BLOCK_BYTES, write_va);
    if (write_status != 0) {
      sexos_line("[sexdrive.storage100.multi.write.complete] idx=%llu lba=%llu status=1 bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
      sexos_line("[sexdrive.storage100.multi.fail] reason=write_status_fail idx=%llu lba=%llu", (unsigned long long) b, (unsigned long long) lba);
      return BLOCK_ERR_NO_DEVICE;
    }
    sexos_line("[sexdrive.storage100.multi.write.complete] idx=%llu lba=%llu status=0 bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
    uint64_t read_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t read_va = sys_map_phys(read_phys, PAGE_SIZE);
    if (read_phys == 0 || (read_phys == UINT64_MAX || (read_phys % PAGE_SIZE != 0 || (read_va == 0 || read_va == UINT64_MAX)))) {
      sexos_line("[sexdrive.storage100.multi.fail] reason=read_buf_invalid idx=%llu lba=%llu", (unsigned long long) b, (unsigned long long) lba);
      return BLOCK_ERR_NO_DEVICE;
    }
    sexos_line("[sexdrive.storage100.multi.read.submit] idx=%llu lba=%llu bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
    uint64_t read_status = nvme_read_into_mapped_va(lba * NVME_LBA_BYTES, AP4_MULTI_BLOCK_BYTES, read_va);
    if (read_status != 0) {
      sexos_line("[sexdrive.storage100.multi.read.complete] idx=%llu lba=%llu status=1 bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
      sexos_line("[sexdrive.storage100.multi.fail] reason=read_status_fail idx=%llu lba=%llu", (unsigned long long) b, (unsigned long long) lba);
      return BLOCK_ERR_NO_DEVICE;
    }
    sexos_line("[sexdrive.storage100.multi.read.complete] idx=%llu lba=%llu status=0 bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
    bool mismatch = false;
    for (size_t i = 0; i < (size_t) AP4_MULTI_BLOCK_BYTES; ++i) {
      uint8_t expected = (uint8_t)((0xA5u ^ (uint8_t) i ^ (uint8_t)((uint8_t) b * 0x33u) ^ 0x3Cu) & 0xFFu);
      uint8_t got = sexos_volatile_read8((const uint8_t *)(uintptr_t) read_va + i);
      if (got != expected) {
        mismatch = true;
        sexos_line("[sexdrive.storage100.multi.read.match] idx=%llu lba=%llu ok=0 first_bad=%llu expected=%llu got=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) i, (unsigned long long) expected, (unsigned long long) got);
        sexos_line("[sexdrive.storage100.multi.fail] reason=byte_mismatch idx=%llu lba=%llu first_bad=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) i);
        break;
      }
    }
    if (mismatch) return BLOCK_ERR_NO_DEVICE;
    sexos_line("[sexdrive.storage100.multi.read.match] idx=%llu lba=%llu bytes=%llu ok=1", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP4_MULTI_BLOCK_BYTES);
    sexos_line("[sexdrive.storage100.multi.block.done] idx=%llu lba=%llu ok=1", (unsigned long long) b, (unsigned long long) lba);
  }
  sexos_line("[sexdrive.storage100.multi.done] blocks=%llu ok=1", (unsigned long long) AP4_MULTI_BLOCKS);
  return 0ULL;
}
static uint64_t nvme_persist_write_proof(void) {
  sexos_line("[sexdrive.storage100.persist.write.begin] base_lba=%llu blocks=%llu bytes_per_block=%llu", (unsigned long long) AP5A_PERSIST_BASE_LBA, (unsigned long long) AP5A_PERSIST_BLOCKS, (unsigned long long) AP5A_PERSIST_BLOCK_BYTES);
  for (uint64_t b = 0; b < AP5A_PERSIST_BLOCKS; ++b) {
    uint64_t lba = AP5A_PERSIST_BASE_LBA + b;
    uint64_t write_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t write_va = sys_map_phys(write_phys, PAGE_SIZE);
    if (write_phys == 0 || (write_phys == UINT64_MAX || (write_phys % PAGE_SIZE != 0 || (write_va == 0 || write_va == UINT64_MAX)))) {
      sexos_line("[sexdrive.storage100.persist.fail] phase=write reason=write_buf_invalid idx=%llu lba=%llu", (unsigned long long) b, (unsigned long long) lba);
      return BLOCK_ERR_NO_DEVICE;
    }
    uint8_t *write_ptr = (uint8_t *)(uintptr_t) write_va;
    for (size_t i = 0; i < (size_t) AP5A_PERSIST_BLOCK_BYTES; ++i) {
      uint8_t val = (uint8_t)((0x5Au ^ (uint8_t) i ^ (uint8_t)((uint8_t) b * 0x21u) ^ 0xC3u) & 0xFFu);
      sexos_volatile_write8(write_ptr + i, val);
    }
    uint64_t write_status = nvme_write_one_block(lba * NVME_LBA_BYTES, AP5A_PERSIST_BLOCK_BYTES, write_va);
    if (write_status != 0) {
      sexos_line("[sexdrive.storage100.persist.fail] phase=write reason=write_status_fail idx=%llu lba=%llu status=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) write_status);
      return BLOCK_ERR_NO_DEVICE;
    }
    sexos_line("[sexdrive.storage100.persist.write.block] idx=%llu lba=%llu status=0 bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP5A_PERSIST_BLOCK_BYTES);
  }
  sexos_line("[sexdrive.storage100.persist.write.done] blocks=%llu ok=1", (unsigned long long) AP5A_PERSIST_BLOCKS);
  return 0ULL;
}
static uint64_t nvme_persist_read_proof(void) {
  sexos_line("[sexdrive.storage100.persist.read.begin] base_lba=%llu blocks=%llu bytes_per_block=%llu", (unsigned long long) AP5A_PERSIST_BASE_LBA, (unsigned long long) AP5A_PERSIST_BLOCKS, (unsigned long long) AP5A_PERSIST_BLOCK_BYTES);
  for (uint64_t b = 0; b < AP5A_PERSIST_BLOCKS; ++b) {
    uint64_t lba = AP5A_PERSIST_BASE_LBA + b;
    uint64_t read_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t read_va = sys_map_phys(read_phys, PAGE_SIZE);
    if (read_phys == 0 || (read_phys == UINT64_MAX || (read_phys % PAGE_SIZE != 0 || (read_va == 0 || read_va == UINT64_MAX)))) {
      sexos_line("[sexdrive.storage100.persist.fail] phase=read reason=read_buf_invalid idx=%llu lba=%llu", (unsigned long long) b, (unsigned long long) lba);
      return BLOCK_ERR_NO_DEVICE;
    }
    uint64_t read_status = nvme_read_into_mapped_va(lba * NVME_LBA_BYTES, AP5A_PERSIST_BLOCK_BYTES, read_va);
    if (read_status != 0) {
      sexos_line("[sexdrive.storage100.persist.fail] phase=read reason=read_status_fail idx=%llu lba=%llu status=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) read_status);
      return BLOCK_ERR_NO_DEVICE;
    }
    sexos_line("[sexdrive.storage100.persist.read.block] idx=%llu lba=%llu status=0 bytes=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP5A_PERSIST_BLOCK_BYTES);
    for (size_t i = 0; i < (size_t) AP5A_PERSIST_BLOCK_BYTES; ++i) {
      uint8_t expected = (uint8_t)((0x5Au ^ (uint8_t) i ^ (uint8_t)((uint8_t) b * 0x21u) ^ 0xC3u) & 0xFFu);
      uint8_t got = sexos_volatile_read8((const uint8_t *)(uintptr_t) read_va + i);
      if (got != expected) {
        sexos_line("[sexdrive.storage100.persist.read.match] idx=%llu lba=%llu ok=0 first_bad=%llu expected=%llu got=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) i, (unsigned long long) expected, (unsigned long long) got);
        sexos_line("[sexdrive.storage100.persist.fail] phase=read reason=byte_mismatch idx=%llu lba=%llu first_bad=%llu", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) i);
        return BLOCK_ERR_NO_DEVICE;
      }
    }
    sexos_line("[sexdrive.storage100.persist.read.match] idx=%llu lba=%llu bytes=%llu ok=1", (unsigned long long) b, (unsigned long long) lba, (unsigned long long) AP5A_PERSIST_BLOCK_BYTES);
  }
  sexos_line("[sexdrive.storage100.persist.read.done] blocks=%llu ok=1", (unsigned long long) AP5A_PERSIST_BLOCKS);
  return 0ULL;
}
static uint64_t nvme_flush(void) {
  if (!sexdrive_state.nvme_io_state.ready) {
    sexos_line("[sexdrive.nvme.flush.err] reason=no_ioq_ready");
    return BLOCK_ERR_NO_DEVICE;
  }
  uint64_t map_va = sexdrive_state.nvme_io_state.map_va;
  uint64_t io_sq_va = sexdrive_state.nvme_io_state.io_sq_va;
  uint64_t io_cq_va = sexdrive_state.nvme_io_state.io_cq_va;
  uint64_t sq1tdbl = sexdrive_state.nvme_io_state.sq1tdbl;
  uint64_t cq1hdbl = sexdrive_state.nvme_io_state.cq1hdbl;
  uint32_t sq_tail = sexdrive_state.nvme_io_state.sq_tail;
  uint32_t cq_head = sexdrive_state.nvme_io_state.cq_head;
  uint32_t cq_phase = sexdrive_state.nvme_io_state.cq_phase;
  uint16_t flush_cid = sexdrive_state.nvme_io_state.next_cid;
  sexdrive_state.nvme_io_state.next_cid = (uint16_t)(flush_cid + 1u);
  uint32_t *sqe_ptr = (uint32_t *)(uintptr_t)(io_sq_va + (uint64_t) sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(sqe_ptr + i, 0u);
  sexos_volatile_write32(sqe_ptr + 0, (uint32_t) flush_cid << 16);
  sexos_volatile_write32(sqe_ptr + 1, 1u);
  sexos_line("[sexdrive.nvme.flush.submit] cid=%llu nsid=1 sq_tail=%llu", (unsigned long long) flush_cid, (unsigned long long) sq_tail);
  sexos_compiler_fence_seq_cst();
  sq_tail = (sq_tail + 1u) % 16u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + sq1tdbl), sq_tail);
  bool done = false;
  uint32_t dw3 = 0;
  for (uint32_t _poll = 0; _poll < 1000u; ++_poll) {
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(io_cq_va + (uint64_t) cq_head * 16ULL);
    uint32_t rd_dw3 = sexos_volatile_read32(cqe_ptr + 3);
    uint32_t cid_now = rd_dw3 & 0xFFFFu;
    uint32_t phase_now = rd_dw3 >> 16 & 0x1u;
    if (cid_now == (uint32_t) flush_cid && phase_now == cq_phase) {
      done = true;
      dw3 = rd_dw3;
      sexos_line("[sexdrive.nvme.flush.cqe] cid=%llu phase=%llu dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) rd_dw3);
      break;
    }
    sexos_spin_loop_hint();
  }
  if (!done) {
    sexos_line("[sexdrive.nvme.flush.err] reason=cqe_timeout cid=%llu head=%llu phase=%llu", (unsigned long long) flush_cid, (unsigned long long) cq_head, (unsigned long long) cq_phase);
    return BLOCK_ERR_NO_DEVICE;
  }
  uint32_t sf = dw3 >> 17 & 0x7FFFu;
  uint32_t sc = sf & 0xFFu;
  uint32_t sct = sf >> 8 & 0x7u;
  if (sc != 0 || sct != 0) {
    sexos_line("[sexdrive.nvme.flush.err] reason=status_fail cid=%llu sc=%llu sct=%llu dw3=%#llx", (unsigned long long) flush_cid, (unsigned long long) sc, (unsigned long long) sct, (unsigned long long) dw3);
    return BLOCK_ERR_NO_DEVICE;
  }
  cq_head = (cq_head + 1u) % 16u;
  if (cq_head == 0u) cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + cq1hdbl), cq_head);
  sexdrive_state.nvme_io_state.sq_tail = sq_tail;
  sexdrive_state.nvme_io_state.cq_head = cq_head;
  sexdrive_state.nvme_io_state.cq_phase = cq_phase;
  sexos_line("[sexdrive.nvme.flush.ok] cid=%llu", (unsigned long long) flush_cid);
  return 0ULL;
}
static uint64_t nvme_storage100_flush_audit(void) {
  sexos_line("[sexdrive.storage100.flush.begin] nsid=1");
  if (!sexdrive_state.nvme_io_state.ready) {
    sexos_line("[sexdrive.storage100.flush.fail] reason=no_ioq_ready");
    return BLOCK_ERR_NO_DEVICE;
  }
  sexos_line("[sexdrive.storage100.flush.submit] opcode=0x00 nsid=1");
  uint64_t status = nvme_flush();
  if (status == 0ULL) {
    sexos_line("[sexdrive.storage100.flush.complete] status=0");
    sexos_line("[sexdrive.storage100.flush.done] ok=1");
    return 0ULL;
  }
  sexos_line("[sexdrive.storage100.flush.skip] reason=flush_not_completed_or_not_supported status=%llu", (unsigned long long) status);
  return BLOCK_ERR_NO_DEVICE;
}
static uint64_t nvme_storage100_negative_mismatch(void) {
  uint64_t lba = AP6_NEG_MISMATCH_LBA;
  uint64_t size = AP6_NEG_MISMATCH_BYTES;
  uint64_t write_phys = sys_alloc_phys(PAGE_SIZE);
  uint64_t write_va = sys_map_phys(write_phys, PAGE_SIZE);
  uint64_t read_phys = sys_alloc_phys(PAGE_SIZE);
  uint64_t read_va = sys_map_phys(read_phys, PAGE_SIZE);
  if (write_phys == 0 || (write_phys == UINT64_MAX || (write_phys % PAGE_SIZE != 0 || (write_va == 0 || (write_va == UINT64_MAX || (read_phys == 0 || (read_phys == UINT64_MAX || (read_phys % PAGE_SIZE != 0 || (read_va == 0 || read_va == UINT64_MAX))))))))) {
    sexos_line("[sexdrive.storage100.neg.mismatch.fail] reason=buf_alloc_invalid");
    return BLOCK_ERR_NO_DEVICE;
  }
  sexos_line("[sexdrive.storage100.neg.mismatch.begin] lba=%llu bytes=%llu", (unsigned long long) lba, (unsigned long long) size);
  uint8_t *write_ptr = (uint8_t *)(uintptr_t) write_va;
  uint8_t *read_ptr = (uint8_t *)(uintptr_t) read_va;
  for (size_t i = 0; i < (size_t) size; ++i) {
    uint8_t b = (uint8_t)((0x5Au ^ (uint8_t) i ^ 0xC3u) & 0xFFu);
    sexos_volatile_write8(write_ptr + i, b);
    sexos_volatile_write8(read_ptr + i, 0u);
  }
  uint64_t write_status = nvme_write_one_block(lba * NVME_LBA_BYTES, size, write_va);
  if (write_status != 0ULL) {
    sexos_line("[sexdrive.storage100.neg.mismatch.fail] reason=write_status status=%llu", (unsigned long long) write_status);
    return write_status;
  }
  uint64_t read_status = nvme_read_into_mapped_va(lba * NVME_LBA_BYTES, size, read_va);
  if (read_status != 0ULL) {
    sexos_line("[sexdrive.storage100.neg.mismatch.fail] reason=read_status status=%llu", (unsigned long long) read_status);
    return read_status;
  }
  uint8_t expected = sexos_volatile_read8((const uint8_t *)(uintptr_t) write_va);
  expected ^= 0x01u;
  uint8_t got = sexos_volatile_read8((const uint8_t *)(uintptr_t) read_va);
  if (expected != got) {
    sexos_line("[sexdrive.storage100.neg.mismatch.detected] ok=1 first_bad=0 expected=%llu got=%llu", (unsigned long long) expected, (unsigned long long) got);
    return 0ULL;
  }
  sexos_line("[sexdrive.storage100.neg.mismatch.fail] reason=unexpected_match");
  return BLOCK_ERR_BAD_LEN;
}
static void nvme_probe_bar(void) {
  uint64_t map_va = sexos_map_pci_bar(SLOT_NVME_HOST, 0ULL, 0x4000ULL);
  if (map_va == UINT64_MAX || map_va == 0ULL) {
    sexos_line("[sexdrive.nvme.bar.resolve.begin] slot=%llu bar=%llu [sexdrive.nvme.bar.resolve.err] map_va=%#llx [sexdrive.device.no_nvme_cap]", (unsigned long long) SLOT_NVME_HOST, 0ULL, (unsigned long long) map_va);
    return;
  }
  uint64_t nvme_cap = sexos_volatile_read64((const uint64_t *)(uintptr_t)(map_va + 0x0000ULL));
  uint32_t nvme_vs = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x0008ULL));
  uint32_t nvme_cc = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x0014ULL));
  uint32_t nvme_csts = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x001CULL));
  uint32_t nvme_aqa = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x0024ULL));
  uint64_t nvme_asq = sexos_volatile_read64((const uint64_t *)(uintptr_t)(map_va + 0x0028ULL));
  uint64_t nvme_acq = sexos_volatile_read64((const uint64_t *)(uintptr_t)(map_va + 0x0030ULL));
  uint16_t cap_mqes = (uint16_t)(nvme_cap & 0xFFFFULL);
  uint8_t cap_dstrd = (uint8_t)(nvme_cap >> 32 & 0xFULL);
  uint16_t vs_major = (uint16_t)(nvme_vs >> 16 & 0xFFFFu);
  uint8_t vs_minor = (uint8_t)(nvme_vs >> 8 & 0xFFu);
  bool csts_rdy = (nvme_csts & 0x1u) != 0u;
  bool cc_en = (nvme_cc & 0x1u) != 0u;
  sexos_line("[sexdrive.nvme.bar.resolve.begin] slot=%llu bar=%llu [sexdrive.nvme.bar.resolve.ok] map_va=%#llx [sexdrive.device.nvme_cap.present] va=%#llx cap=%#llx", (unsigned long long) SLOT_NVME_HOST, 0ULL, (unsigned long long) map_va, (unsigned long long) map_va, (unsigned long long) nvme_cap);
  sexos_line("[sexdrive.nvme.reg.cap] cap=%#llx mqes=%llu dstrd=%llu", (unsigned long long) nvme_cap, (unsigned long long) cap_mqes, (unsigned long long) cap_dstrd);
  sexos_line("[sexdrive.nvme.reg.vs] vs=%#llx major=%llu minor=%llu", (unsigned long long) nvme_vs, (unsigned long long) vs_major, (unsigned long long) vs_minor);
  sexos_line("[sexdrive.nvme.reg.cc] cc=%#llx", (unsigned long long) nvme_cc);
  sexos_line("[sexdrive.nvme.reg.csts] csts=%#llx rdy=%llu", (unsigned long long) nvme_csts, (unsigned long long)(csts_rdy ? 1ULL : 0ULL));
  if (nvme_cap != 0ULL && nvme_vs != 0u) sexos_line("[sexdrive.nvme.identity.ok] cap=%#llx vs=%#llx mqes=%llu dstrd=%llu major=%llu minor=%llu csts_rdy=%llu", (unsigned long long) nvme_cap, (unsigned long long) nvme_vs, (unsigned long long) cap_mqes, (unsigned long long) cap_dstrd, (unsigned long long) vs_major, (unsigned long long) vs_minor, (unsigned long long)(csts_rdy ? 1ULL : 0ULL));
  else sexos_line("[sexdrive.nvme.identity.err] cap=%#llx vs=%#llx cc=%#llx csts=%#llx", (unsigned long long) nvme_cap, (unsigned long long) nvme_vs, (unsigned long long) nvme_cc, (unsigned long long) nvme_csts);
  sexos_line("[sexdrive.nvme.queue.inspect] aqa=%#llx asq=%#llx acq=%#llx cc=%#llx csts=%#llx", (unsigned long long) nvme_aqa, (unsigned long long) nvme_asq, (unsigned long long) nvme_acq, (unsigned long long) nvme_cc, (unsigned long long) nvme_csts);
  sexos_line("[sexdrive.nvme.queue.aqa] aqa=%#llx", (unsigned long long) nvme_aqa);
  sexos_line("[sexdrive.nvme.queue.asq] asq=%#llx", (unsigned long long) nvme_asq);
  sexos_line("[sexdrive.nvme.queue.acq] acq=%#llx", (unsigned long long) nvme_acq);
  bool queues_preconfigured = nvme_aqa != 0u && (nvme_asq != 0ULL && nvme_acq != 0ULL);
  if (queues_preconfigured) sexos_line("[sexdrive.nvme.queue.ready] mode=preconfigured cc_en=%llu csts_rdy=%llu aqa=%#llx asq=%#llx acq=%#llx", (unsigned long long)(cc_en ? 1ULL : 0ULL), (unsigned long long)(csts_rdy ? 1ULL : 0ULL), (unsigned long long) nvme_aqa, (unsigned long long) nvme_asq, (unsigned long long) nvme_acq);
  else {
    if (cc_en || csts_rdy) {
      sexos_line("[sexdrive.nvme.queue.err] reason=stop_first_controller_enabled_queue_program_requires_disable cc_en=%llu csts_rdy=%llu aqa=%#llx asq=%#llx acq=%#llx", (unsigned long long)(cc_en ? 1ULL : 0ULL), (unsigned long long)(csts_rdy ? 1ULL : 0ULL), (unsigned long long) nvme_aqa, (unsigned long long) nvme_asq, (unsigned long long) nvme_acq);
      return;
    }
    uint64_t asq_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t acq_phys = sys_alloc_phys(PAGE_SIZE);
    if (asq_phys == 0ULL || (asq_phys == UINT64_MAX || (acq_phys == 0ULL || acq_phys == UINT64_MAX))) {
      sexos_line("[sexdrive.nvme.queue.err] reason=alloc_failed asq_phys=%#llx acq_phys=%#llx", (unsigned long long) asq_phys, (unsigned long long) acq_phys);
      return;
    }
    if (asq_phys % PAGE_SIZE != 0ULL || acq_phys % PAGE_SIZE != 0ULL) {
      sexos_line("[sexdrive.nvme.queue.err] reason=alloc_unaligned asq_phys=%#llx acq_phys=%#llx", (unsigned long long) asq_phys, (unsigned long long) acq_phys);
      return;
    }
    uint64_t asq_va = sys_map_phys(asq_phys, PAGE_SIZE);
    uint64_t acq_va = sys_map_phys(acq_phys, PAGE_SIZE);
    if (asq_va == 0ULL || (asq_va == UINT64_MAX || (acq_va == 0ULL || acq_va == UINT64_MAX))) {
      sexos_line("[sexdrive.nvme.queue.err] reason=map_failed asq_va=%#llx acq_va=%#llx", (unsigned long long) asq_va, (unsigned long long) acq_va);
      return;
    }
    size_t asq_words = (size_t)(PAGE_SIZE / 8ULL);
    size_t acq_words = (size_t)(PAGE_SIZE / 8ULL);
    for (size_t i = 0; i < asq_words; ++i) sexos_volatile_write64((uint64_t *)(uintptr_t) asq_va + i, 0ULL);
    for (size_t i = 0; i < acq_words; ++i) sexos_volatile_write64((uint64_t *)(uintptr_t) acq_va + i, 0ULL);
    sexos_line("[sexdrive.nvme.queue.alloc.ok] asq_phys=%#llx asq_va=%#llx acq_phys=%#llx acq_va=%#llx", (unsigned long long) asq_phys, (unsigned long long) asq_va, (unsigned long long) acq_phys, (unsigned long long) acq_va);
    uint32_t aqa = (16u - 1u) << 16 | (16u - 1u);
    sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + 0x0024ULL), aqa);
    sexos_volatile_write64((uint64_t *)(uintptr_t)(map_va + 0x0028ULL), asq_phys);
    sexos_volatile_write64((uint64_t *)(uintptr_t)(map_va + 0x0030ULL), acq_phys);
    uint32_t aqa_rb = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x0024ULL));
    uint64_t asq_rb = sexos_volatile_read64((const uint64_t *)(uintptr_t)(map_va + 0x0028ULL));
    uint64_t acq_rb = sexos_volatile_read64((const uint64_t *)(uintptr_t)(map_va + 0x0030ULL));
    sexos_line("[sexdrive.nvme.queue.program.ok] aqa=%#llx asq=%#llx acq=%#llx", (unsigned long long) aqa_rb, (unsigned long long) asq_rb, (unsigned long long) acq_rb);
    sexos_line("[sexdrive.nvme.queue.ready] mode=programmed cc_en=0 csts_rdy=0 aqa=%#llx asq=%#llx acq=%#llx", (unsigned long long) aqa_rb, (unsigned long long) asq_rb, (unsigned long long) acq_rb);
  }
  uint32_t old_cc = nvme_cc;
  uint32_t old_csts = nvme_csts;
  uint32_t old_aqa = nvme_aqa;
  uint64_t old_asq = nvme_asq;
  uint64_t old_acq = nvme_acq;
  uint32_t cap_to = (uint32_t)(nvme_cap >> 24 & 0xFFULL);
  sexos_line("[sexdrive.nvme.reprovision.begin] old_cc=%#llx old_csts=%#llx old_aqa=%#llx old_asq=%#llx old_acq=%#llx cap_to=%llu dstrd=%llu", (unsigned long long) old_cc, (unsigned long long) old_csts, (unsigned long long) old_aqa, (unsigned long long) old_asq, (unsigned long long) old_acq, (unsigned long long) cap_to, (unsigned long long) cap_dstrd);
  sexos_line("[sexdrive.nvme.reprovision.disable.begin] cc_old=%#llx csts_old=%#llx", (unsigned long long) old_cc, (unsigned long long) old_csts);
  uint32_t cc_disabled = old_cc & ~1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + 0x0014ULL), cc_disabled);
  uint32_t disable_poll_limit = cap_to == 0u ? 1000000u : cap_to * 20000u > 200000u ? cap_to * 20000u : 200000u;
  uint32_t disable_polls = 0u;
  bool rdy0 = false;
  while (disable_polls < disable_poll_limit) {
    uint32_t csts_now = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x001CULL));
    if ((csts_now & 0x1u) == 0u) {
      rdy0 = true;
      sexos_line("[sexdrive.nvme.reprovision.rdy0] polls=%llu csts=%#llx cc=%#llx", (unsigned long long) disable_polls, (unsigned long long) csts_now, (unsigned long long) cc_disabled);
      break;
    }
    disable_polls += 1u;
  }
  if (!rdy0) {
    uint32_t csts_now = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x001CULL));
    sexos_line("[sexdrive.nvme.reprovision.err] reason=disable_rdy0_timeout polls=%llu limit=%llu csts=%#llx cc=%#llx", (unsigned long long) disable_polls, (unsigned long long) disable_poll_limit, (unsigned long long) csts_now, (unsigned long long) cc_disabled);
    return;
  }
  uint64_t asq_phys = sys_alloc_phys(PAGE_SIZE);
  uint64_t acq_phys = sys_alloc_phys(PAGE_SIZE);
  if (asq_phys == 0ULL || (asq_phys == UINT64_MAX || (acq_phys == 0ULL || acq_phys == UINT64_MAX))) {
    sexos_line("[sexdrive.nvme.reprovision.err] reason=alloc_failed asq_phys=%#llx acq_phys=%#llx", (unsigned long long) asq_phys, (unsigned long long) acq_phys);
    return;
  }
  if (asq_phys % PAGE_SIZE != 0ULL || acq_phys % PAGE_SIZE != 0ULL) {
    sexos_line("[sexdrive.nvme.reprovision.err] reason=alloc_unaligned asq_phys=%#llx acq_phys=%#llx", (unsigned long long) asq_phys, (unsigned long long) acq_phys);
    return;
  }
  uint64_t asq_va = sys_map_phys(asq_phys, PAGE_SIZE);
  uint64_t acq_va = sys_map_phys(acq_phys, PAGE_SIZE);
  if (asq_va == 0ULL || (asq_va == UINT64_MAX || (acq_va == 0ULL || acq_va == UINT64_MAX))) {
    sexos_line("[sexdrive.nvme.reprovision.err] reason=map_failed asq_va=%#llx acq_va=%#llx", (unsigned long long) asq_va, (unsigned long long) acq_va);
    return;
  }
  size_t words = (size_t)(PAGE_SIZE / 8ULL);
  for (size_t i = 0; i < words; ++i) {
    sexos_volatile_write64((uint64_t *)(uintptr_t) asq_va + i, 0ULL);
    sexos_volatile_write64((uint64_t *)(uintptr_t) acq_va + i, 0ULL);
  }
  sexos_line("[sexdrive.nvme.reprovision.alloc.ok] asq_phys=%#llx asq_va=%#llx acq_phys=%#llx acq_va=%#llx", (unsigned long long) asq_phys, (unsigned long long) asq_va, (unsigned long long) acq_phys, (unsigned long long) acq_va);
  uint32_t new_aqa = (16u - 1u) << 16 | (16u - 1u);
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + 0x0024ULL), new_aqa);
  sexos_volatile_write64((uint64_t *)(uintptr_t)(map_va + 0x0028ULL), asq_phys);
  sexos_volatile_write64((uint64_t *)(uintptr_t)(map_va + 0x0030ULL), acq_phys);
  sexos_line("[sexdrive.nvme.reprovision.program.aqa] aqa=%#llx", (unsigned long long) new_aqa);
  sexos_line("[sexdrive.nvme.reprovision.program.asq] asq=%#llx", (unsigned long long) asq_phys);
  sexos_line("[sexdrive.nvme.reprovision.program.acq] acq=%#llx", (unsigned long long) acq_phys);
  sexos_line("[sexdrive.nvme.reprovision.enable.begin] cc_before=%#llx", (unsigned long long) cc_disabled);
  uint32_t cc_enabled = cc_disabled | 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + 0x0014ULL), cc_enabled);
  uint32_t enable_poll_limit = cap_to == 0u ? 1000000u : cap_to * 20000u > 200000u ? cap_to * 20000u : 200000u;
  uint32_t enable_polls = 0u;
  bool rdy1 = false;
  while (enable_polls < enable_poll_limit) {
    uint32_t csts_now = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x001CULL));
    if ((csts_now & 0x1u) != 0u) {
      rdy1 = true;
      sexos_line("[sexdrive.nvme.reprovision.rdy1] polls=%llu csts=%#llx cc=%#llx", (unsigned long long) enable_polls, (unsigned long long) csts_now, (unsigned long long) cc_enabled);
      break;
    }
    enable_polls += 1u;
  }
  if (!rdy1) {
    uint32_t csts_now = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x001CULL));
    sexos_line("[sexdrive.nvme.reprovision.err] reason=enable_rdy1_timeout polls=%llu limit=%llu csts=%#llx cc=%#llx", (unsigned long long) enable_polls, (unsigned long long) enable_poll_limit, (unsigned long long) csts_now, (unsigned long long) cc_enabled);
    return;
  }
  uint32_t rb_cc = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x0014ULL));
  uint32_t rb_csts = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x001CULL));
  uint32_t rb_aqa = sexos_volatile_read32((const uint32_t *)(uintptr_t)(map_va + 0x0024ULL));
  uint64_t rb_asq = sexos_volatile_read64((const uint64_t *)(uintptr_t)(map_va + 0x0028ULL));
  uint64_t rb_acq = sexos_volatile_read64((const uint64_t *)(uintptr_t)(map_va + 0x0030ULL));
  if (rb_aqa != new_aqa || (rb_asq != asq_phys || (rb_acq != acq_phys || ((rb_cc & 1u) == 0u || (rb_csts & 1u) == 0u)))) {
    sexos_line("[sexdrive.nvme.reprovision.err] reason=readback_mismatch cc=%#llx csts=%#llx aqa=%#llx asq=%#llx acq=%#llx", (unsigned long long) rb_cc, (unsigned long long) rb_csts, (unsigned long long) rb_aqa, (unsigned long long) rb_asq, (unsigned long long) rb_acq);
    return;
  }
  sexos_line("[sexdrive.nvme.reprovision.ok] cc=%#llx csts=%#llx aqa=%#llx asq=%#llx acq=%#llx disable_polls=%llu enable_polls=%llu", (unsigned long long) rb_cc, (unsigned long long) rb_csts, (unsigned long long) rb_aqa, (unsigned long long) rb_asq, (unsigned long long) rb_acq, (unsigned long long) disable_polls, (unsigned long long) enable_polls);
  uint64_t asq_owned_phys = rb_asq;
  uint64_t acq_owned_phys = rb_acq;
  uint64_t asq_owned_va = asq_va;
  uint64_t acq_owned_va = acq_va;
  uint32_t q_entries = 16u;
  uint32_t sq_tail = 0u;
  uint32_t cq_head = 0u;
  uint32_t cq_phase = 1u;
  uint32_t cc_css = rb_cc >> 4 & 0x7u;
  uint32_t cc_mps = rb_cc >> 7 & 0xFu;
  uint32_t cc_ams = rb_cc >> 11 & 0x7u;
  uint32_t cc_iosqes = rb_cc >> 16 & 0xFu;
  uint32_t cc_iocqes = rb_cc >> 20 & 0xFu;
  sexos_line("[sexdrive.nvme.admin.identify.v2.begin] asq_phys=%#llx asq_va=%#llx acq_phys=%#llx acq_va=%#llx q_entries=%llu sq_head=%llu sq_tail=%llu cq_head=%llu cq_phase=%llu", (unsigned long long) asq_owned_phys, (unsigned long long) asq_owned_va, (unsigned long long) acq_owned_phys, (unsigned long long) acq_owned_va, (unsigned long long) q_entries, 0ULL, (unsigned long long) sq_tail, (unsigned long long) cq_head, (unsigned long long) cq_phase);
  sexos_line("[sexdrive.nvme.admin.identify.v2.cqe.decode] cc=%#llx en=%llu css=%llu mps=%llu ams=%llu iosqes=%llu iocqes=%llu dstrd=%llu sq0tdbl=%#llx cq0hdbl=%#llx", (unsigned long long) rb_cc, (unsigned long long)(rb_cc & 1u), (unsigned long long) cc_css, (unsigned long long) cc_mps, (unsigned long long) cc_ams, (unsigned long long) cc_iosqes, (unsigned long long) cc_iocqes, (unsigned long long) cap_dstrd, (unsigned long long) NVME_SQ0TDBL, (unsigned long long) NVME_CQ0HDBL);
  if (cc_iosqes != 6u || (cc_iocqes != 4u || cc_mps != 0u)) {
    sexos_line("[sexdrive.nvme.admin.identify.v2.err] reason=cc_layout_invalid cc=%#llx iosqes=%llu iocqes=%llu mps=%llu", (unsigned long long) rb_cc, (unsigned long long) cc_iosqes, (unsigned long long) cc_iocqes, (unsigned long long) cc_mps);
    return;
  }
  const uint32_t *acq0_ptr = (const uint32_t *)(uintptr_t) acq_owned_va;
  uint32_t acq0_dw0_before = sexos_volatile_read32(acq0_ptr + 0);
  uint32_t acq0_dw1_before = sexos_volatile_read32(acq0_ptr + 1);
  uint32_t acq0_dw2_before = sexos_volatile_read32(acq0_ptr + 2);
  uint32_t acq0_dw3_before = sexos_volatile_read32(acq0_ptr + 3);
  sexos_line("[sexdrive.nvme.admin.identify.v2.cqe.raw] stage=before dw0=%#llx dw1=%#llx dw2=%#llx dw3=%#llx phase=%llu", (unsigned long long) acq0_dw0_before, (unsigned long long) acq0_dw1_before, (unsigned long long) acq0_dw2_before, (unsigned long long) acq0_dw3_before, (unsigned long long)(acq0_dw3_before & 0x1u));
  uint64_t id_phys = sys_alloc_phys(PAGE_SIZE);
  if (id_phys == 0ULL || (id_phys == UINT64_MAX || id_phys % PAGE_SIZE != 0ULL)) {
    sexos_line("[sexdrive.nvme.admin.identify.v2.err] reason=id_alloc_invalid phys=%#llx", (unsigned long long) id_phys);
    return;
  }
  uint64_t id_va = sys_map_phys(id_phys, PAGE_SIZE);
  if (id_va == 0ULL || id_va == UINT64_MAX) {
    sexos_line("[sexdrive.nvme.admin.identify.v2.err] reason=id_map_failed va=%#llx", (unsigned long long) id_va);
    return;
  }
  for (size_t i = 0; i < (size_t)(PAGE_SIZE / 8ULL); ++i) sexos_volatile_write64((uint64_t *)(uintptr_t) id_va + i, 0ULL);
  uint16_t cid = 0x0042u;
  uint32_t *sqe_ptr = (uint32_t *)(uintptr_t)(asq_owned_va + (uint64_t) sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(sqe_ptr + i, 0u);
  sexos_volatile_write32(sqe_ptr + 0, 0x06u | (uint32_t) cid << 16);
  sexos_volatile_write32(sqe_ptr + 6, (uint32_t)(id_phys & 0xFFFFFFFFULL));
  sexos_volatile_write32(sqe_ptr + 7, (uint32_t)(id_phys >> 32));
  sexos_volatile_write32(sqe_ptr + 10, 1u);
  sexos_line("[sexdrive.nvme.admin.identify.v2.submit] cid=%llu opc=%#llx cns=%llu prp1=%#llx sq_tail=%llu", (unsigned long long) cid, 0x06ULL, 1ULL, (unsigned long long) id_phys, (unsigned long long) sq_tail);
  sexos_compiler_fence_seq_cst();
  sq_tail = (sq_tail + 1u) % q_entries;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + NVME_SQ0TDBL), sq_tail);
  sexos_line("[sexdrive.nvme.admin.identify.v2.submit] sq0tdbl=%#llx new_tail=%llu dstrd=%llu", (unsigned long long) NVME_SQ0TDBL, (unsigned long long) sq_tail, (unsigned long long) cap_dstrd);
  bool got_cqe = false;
  uint32_t cqe_dw0 = 0u;
  uint32_t cqe_dw1 = 0u;
  uint32_t cqe_dw2 = 0u;
  uint32_t cqe_dw3 = 0u;
  uint32_t cqe_phase = 0u;
  uint32_t cqe_cid = 0u;
  uint32_t cqe_polls = 0u;
  bool acq_changed = false;
  uint32_t first_change_poll = UINT32_MAX;
  uint32_t acq0_dw0_change = acq0_dw0_before;
  uint32_t acq0_dw1_change = acq0_dw1_before;
  uint32_t acq0_dw2_change = acq0_dw2_before;
  uint32_t acq0_dw3_change = acq0_dw3_before;
  bool saw_cid_ignore_phase = false;
  uint32_t ig_dw0 = 0u;
  uint32_t ig_dw1 = 0u;
  uint32_t ig_dw2 = 0u;
  uint32_t ig_dw3 = 0u;
  uint32_t ig_phase = 0u;
  while (cqe_polls < 1000000u) {
    uint32_t acq0_dw0 = sexos_volatile_read32(acq0_ptr + 0);
    uint32_t acq0_dw1 = sexos_volatile_read32(acq0_ptr + 1);
    uint32_t acq0_dw2 = sexos_volatile_read32(acq0_ptr + 2);
    uint32_t acq0_dw3 = sexos_volatile_read32(acq0_ptr + 3);
    if (!acq_changed && (acq0_dw0 != acq0_dw0_before || (acq0_dw1 != acq0_dw1_before || (acq0_dw2 != acq0_dw2_before || acq0_dw3 != acq0_dw3_before)))) {
      acq_changed = true;
      first_change_poll = cqe_polls;
      acq0_dw0_change = acq0_dw0;
      acq0_dw1_change = acq0_dw1;
      acq0_dw2_change = acq0_dw2;
      acq0_dw3_change = acq0_dw3;
      sexos_line("[sexdrive.nvme.admin.identify.v2.cqe.raw] stage=change poll=%llu dw0=%#llx dw1=%#llx dw2=%#llx dw3=%#llx phase=%llu", (unsigned long long) cqe_polls, (unsigned long long) acq0_dw0, (unsigned long long) acq0_dw1, (unsigned long long) acq0_dw2, (unsigned long long) acq0_dw3, (unsigned long long)(acq0_dw3 & 0x1u));
    }
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(acq_owned_va + (uint64_t) cq_head * 16ULL);
    uint32_t dw0 = sexos_volatile_read32(cqe_ptr + 0);
    uint32_t dw1 = sexos_volatile_read32(cqe_ptr + 1);
    uint32_t dw2 = sexos_volatile_read32(cqe_ptr + 2);
    uint32_t dw3 = sexos_volatile_read32(cqe_ptr + 3);
    uint32_t cmd_id = dw3 & 0xFFFFu;
    uint32_t phase = dw3 >> 16 & 0x1u;
    if (cmd_id == (uint32_t) cid) {
      saw_cid_ignore_phase = true;
      ig_dw0 = dw0;
      ig_dw1 = dw1;
      ig_dw2 = dw2;
      ig_dw3 = dw3;
      ig_phase = phase;
    }
    if (phase == cq_phase && cmd_id == (uint32_t) cid) {
      got_cqe = true;
      cqe_dw0 = dw0;
      cqe_dw1 = dw1;
      cqe_dw2 = dw2;
      cqe_dw3 = dw3;
      cqe_phase = phase;
      cqe_cid = cmd_id;
      sexos_line("[sexdrive.nvme.admin.identify.v2.cqe.decode] match=phase polls=%llu head=%llu phase=%llu cid=%llu dw0=%#llx dw1=%#llx dw2=%#llx dw3=%#llx", (unsigned long long) cqe_polls, (unsigned long long) cq_head, (unsigned long long) phase, (unsigned long long) cmd_id, (unsigned long long) dw0, (unsigned long long) dw1, (unsigned long long) dw2, (unsigned long long) dw3);
      break;
    }
    cqe_polls += 1u;
  }
  if (!got_cqe) {
    if (saw_cid_ignore_phase) sexos_line("[sexdrive.nvme.admin.identify.v2.cqe.raw] stage=ignore_phase cid=%llu phase=%llu dw0=%#llx dw1=%#llx dw2=%#llx dw3=%#llx", (unsigned long long) cid, (unsigned long long) ig_phase, (unsigned long long) ig_dw0, (unsigned long long) ig_dw1, (unsigned long long) ig_dw2, (unsigned long long) ig_dw3);
    sexos_line("[sexdrive.nvme.admin.identify.v2.err] reason=cqe_timeout cid=%llu head=%llu phase=%llu polls=%llu acq_changed=%llu first_change_poll=%llu acq0_dw0_before=%#llx acq0_dw1_before=%#llx acq0_dw2_before=%#llx acq0_dw3_before=%#llx acq0_dw0_now=%#llx acq0_dw1_now=%#llx acq0_dw2_now=%#llx acq0_dw3_now=%#llx", (unsigned long long) cid, (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) cqe_polls, (unsigned long long)(acq_changed ? 1ULL : 0ULL), (unsigned long long)(first_change_poll == UINT32_MAX ? UINT64_MAX : (uint64_t) first_change_poll), (unsigned long long) acq0_dw0_before, (unsigned long long) acq0_dw1_before, (unsigned long long) acq0_dw2_before, (unsigned long long) acq0_dw3_before, (unsigned long long) acq0_dw0_change, (unsigned long long) acq0_dw1_change, (unsigned long long) acq0_dw2_change, (unsigned long long) acq0_dw3_change);
    return;
  }
  sexos_line("[sexdrive.nvme.admin.identify.v2.cqe.raw] stage=phase_match cid=%llu phase=%llu dw0=%#llx dw1=%#llx dw2=%#llx dw3=%#llx", (unsigned long long) cqe_cid, (unsigned long long) cqe_phase, (unsigned long long) cqe_dw0, (unsigned long long) cqe_dw1, (unsigned long long) cqe_dw2, (unsigned long long) cqe_dw3);
  uint32_t sf = cqe_dw3 >> 17 & 0x7FFFu;
  uint32_t sc = sf & 0xFFu;
  uint32_t sct = sf >> 8 & 0x7u;
  uint32_t sqhd = cqe_dw2 >> 16 & 0xFFFFu;
  cq_head = (cq_head + 1u) % q_entries;
  if (cq_head == 0u) cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + NVME_CQ0HDBL), cq_head);
  if (sc != 0u || sct != 0u) {
    sexos_line("[sexdrive.nvme.admin.identify.v2.err] reason=cqe_status_fail cid=%llu sc=%llu sct=%llu sqhd=%llu head=%llu dw3=%#llx", (unsigned long long) cid, (unsigned long long) sc, (unsigned long long) sct, (unsigned long long) sqhd, (unsigned long long) cq_head, (unsigned long long) cqe_dw3);
    return;
  }
  uint64_t sn0 = sexos_volatile_read64((const uint64_t *)(uintptr_t)(id_va + 4ULL));
  uint64_t mn0 = sexos_volatile_read64((const uint64_t *)(uintptr_t)(id_va + 24ULL));
  uint32_t nn = sexos_volatile_read32((const uint32_t *)(uintptr_t)(id_va + 516ULL));
  sexos_line("[sexdrive.nvme.admin.identify.v2.ok] cid=%llu sc=%llu sct=%llu sqhd=%llu cqh=%llu cqp=%llu sn0=%#llx mn0=%#llx nn=%llu", (unsigned long long) cid, (unsigned long long) sc, (unsigned long long) sct, (unsigned long long) sqhd, (unsigned long long) cq_head, (unsigned long long) cq_phase, (unsigned long long) sn0, (unsigned long long) mn0, (unsigned long long) nn);
  uint64_t io_cq_phys = sys_alloc_phys(PAGE_SIZE);
  uint64_t io_sq_phys = sys_alloc_phys(PAGE_SIZE);
  if (io_cq_phys == 0ULL || (io_cq_phys == UINT64_MAX || (io_sq_phys == 0ULL || io_sq_phys == UINT64_MAX))) {
    sexos_line("[sexdrive.nvme.ioq.err] reason=alloc_failed io_cq_phys=%#llx io_sq_phys=%#llx", (unsigned long long) io_cq_phys, (unsigned long long) io_sq_phys);
    return;
  }
  if (io_cq_phys % PAGE_SIZE != 0ULL || io_sq_phys % PAGE_SIZE != 0ULL) {
    sexos_line("[sexdrive.nvme.ioq.err] reason=alloc_unaligned io_cq_phys=%#llx io_sq_phys=%#llx", (unsigned long long) io_cq_phys, (unsigned long long) io_sq_phys);
    return;
  }
  uint64_t io_cq_va = sys_map_phys(io_cq_phys, PAGE_SIZE);
  uint64_t io_sq_va = sys_map_phys(io_sq_phys, PAGE_SIZE);
  if (io_cq_va == 0ULL || (io_cq_va == UINT64_MAX || (io_sq_va == 0ULL || io_sq_va == UINT64_MAX))) {
    sexos_line("[sexdrive.nvme.ioq.err] reason=map_failed io_cq_va=%#llx io_sq_va=%#llx", (unsigned long long) io_cq_va, (unsigned long long) io_sq_va);
    return;
  }
  for (size_t i = 0; i < (size_t)(PAGE_SIZE / 8ULL); ++i) {
    sexos_volatile_write64((uint64_t *)(uintptr_t) io_cq_va + i, 0ULL);
    sexos_volatile_write64((uint64_t *)(uintptr_t) io_sq_va + i, 0ULL);
  }
  sexos_line("[sexdrive.nvme.ioq.alloc.ok] io_cq_phys=%#llx io_cq_va=%#llx io_sq_phys=%#llx io_sq_va=%#llx", (unsigned long long) io_cq_phys, (unsigned long long) io_cq_va, (unsigned long long) io_sq_phys, (unsigned long long) io_sq_va);
  uint32_t admin_sq_tail = sq_tail;
  uint32_t admin_cq_head = cq_head;
  uint32_t admin_cq_phase = cq_phase;
  uint16_t create_cq_cid = 0x0043u;
  uint32_t *cq_sqe_ptr = (uint32_t *)(uintptr_t)(asq_owned_va + (uint64_t) admin_sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(cq_sqe_ptr + i, 0u);
  sexos_volatile_write32(cq_sqe_ptr + 0, 0x05u | (uint32_t) create_cq_cid << 16);
  sexos_volatile_write32(cq_sqe_ptr + 6, (uint32_t)(io_cq_phys & 0xFFFFFFFFULL));
  sexos_volatile_write32(cq_sqe_ptr + 7, (uint32_t)(io_cq_phys >> 32));
  sexos_volatile_write32(cq_sqe_ptr + 10, 1u | 15u << 16);
  sexos_volatile_write32(cq_sqe_ptr + 11, 1u);
  sexos_line("[sexdrive.nvme.ioq.create_cq.submit] cid=%llu opc=%#llx qid=%llu qsize=%llu prp1=%#llx cdw11=%#llx", (unsigned long long) create_cq_cid, 0x05ULL, 1ULL, 15ULL, (unsigned long long) io_cq_phys, 1ULL);
  sexos_compiler_fence_seq_cst();
  admin_sq_tail = (admin_sq_tail + 1u) % q_entries;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + NVME_SQ0TDBL), admin_sq_tail);
  bool cq_done = false;
  uint32_t cq_cqe_dw3 = 0u;
  for (uint32_t _poll = 0; _poll < 1000000u; ++_poll) {
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(acq_owned_va + (uint64_t) admin_cq_head * 16ULL);
    uint32_t dw2 = sexos_volatile_read32(cqe_ptr + 2);
    uint32_t dw3 = sexos_volatile_read32(cqe_ptr + 3);
    uint32_t cid_now = dw3 & 0xFFFFu;
    uint32_t phase_now = dw3 >> 16 & 0x1u;
    if (cid_now == (uint32_t) create_cq_cid && phase_now == admin_cq_phase) {
      cq_done = true;
      cq_cqe_dw3 = dw3;
      sexos_line("[sexdrive.nvme.ioq.create_cq.cqe] cid=%llu dw2=%#llx dw3=%#llx phase=%llu", (unsigned long long) cid_now, (unsigned long long) dw2, (unsigned long long) dw3, (unsigned long long) phase_now);
      break;
    }
  }
  if (!cq_done) {
    sexos_line("[sexdrive.nvme.ioq.err] reason=create_cq_timeout cid=%llu", (unsigned long long) create_cq_cid);
    return;
  }
  uint32_t cq_sf = cq_cqe_dw3 >> 17 & 0x7FFFu;
  uint32_t cq_sc = cq_sf & 0xFFu;
  uint32_t cq_sct = cq_sf >> 8 & 0x7u;
  admin_cq_head = (admin_cq_head + 1u) % q_entries;
  if (admin_cq_head == 0u) admin_cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + NVME_CQ0HDBL), admin_cq_head);
  if (cq_sc != 0u || cq_sct != 0u) {
    sexos_line("[sexdrive.nvme.ioq.err] reason=create_cq_status_fail cid=%llu sc=%llu sct=%llu dw3=%#llx", (unsigned long long) create_cq_cid, (unsigned long long) cq_sc, (unsigned long long) cq_sct, (unsigned long long) cq_cqe_dw3);
    return;
  }
  sexos_line("[sexdrive.nvme.ioq.create_cq.ok] cid=%llu sc=%llu sct=%llu cqh=%llu cqp=%llu", (unsigned long long) create_cq_cid, (unsigned long long) cq_sc, (unsigned long long) cq_sct, (unsigned long long) admin_cq_head, (unsigned long long) admin_cq_phase);
  uint16_t create_sq_cid = 0x0044u;
  uint32_t *sq_sqe_ptr = (uint32_t *)(uintptr_t)(asq_owned_va + (uint64_t) admin_sq_tail * 64ULL);
  for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(sq_sqe_ptr + i, 0u);
  sexos_volatile_write32(sq_sqe_ptr + 0, 0x01u | (uint32_t) create_sq_cid << 16);
  sexos_volatile_write32(sq_sqe_ptr + 6, (uint32_t)(io_sq_phys & 0xFFFFFFFFULL));
  sexos_volatile_write32(sq_sqe_ptr + 7, (uint32_t)(io_sq_phys >> 32));
  sexos_volatile_write32(sq_sqe_ptr + 10, 1u | 15u << 16);
  sexos_volatile_write32(sq_sqe_ptr + 11, 1u << 16 | 1u);
  sexos_line("[sexdrive.nvme.ioq.create_sq.submit] cid=%llu opc=%#llx qid=%llu qsize=%llu cqid=%llu prp1=%#llx", (unsigned long long) create_sq_cid, 0x01ULL, 1ULL, 15ULL, 1ULL, (unsigned long long) io_sq_phys);
  sexos_compiler_fence_seq_cst();
  admin_sq_tail = (admin_sq_tail + 1u) % q_entries;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + NVME_SQ0TDBL), admin_sq_tail);
  bool sq_done = false;
  uint32_t sq_cqe_dw3 = 0u;
  for (uint32_t _poll = 0; _poll < 1000000u; ++_poll) {
    const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(acq_owned_va + (uint64_t) admin_cq_head * 16ULL);
    uint32_t dw3 = sexos_volatile_read32(cqe_ptr + 3);
    uint32_t cid_now = dw3 & 0xFFFFu;
    uint32_t phase_now = dw3 >> 16 & 0x1u;
    if (cid_now == (uint32_t) create_sq_cid && phase_now == admin_cq_phase) {
      sq_done = true;
      sq_cqe_dw3 = dw3;
      sexos_line("[sexdrive.nvme.ioq.create_sq.cqe] cid=%llu dw3=%#llx phase=%llu", (unsigned long long) cid_now, (unsigned long long) dw3, (unsigned long long) phase_now);
      break;
    }
  }
  if (!sq_done) {
    sexos_line("[sexdrive.nvme.ioq.err] reason=create_sq_timeout cid=%llu", (unsigned long long) create_sq_cid);
    return;
  }
  uint32_t sq_sf = sq_cqe_dw3 >> 17 & 0x7FFFu;
  uint32_t sq_sc = sq_sf & 0xFFu;
  uint32_t sq_sct = sq_sf >> 8 & 0x7u;
  admin_cq_head = (admin_cq_head + 1u) % q_entries;
  if (admin_cq_head == 0u) admin_cq_phase ^= 1u;
  sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + NVME_CQ0HDBL), admin_cq_head);
  if (sq_sc != 0u || sq_sct != 0u) {
    sexos_line("[sexdrive.nvme.ioq.err] reason=create_sq_status_fail cid=%llu sc=%llu sct=%llu dw3=%#llx", (unsigned long long) create_sq_cid, (unsigned long long) sq_sc, (unsigned long long) sq_sct, (unsigned long long) sq_cqe_dw3);
    return;
  }
  sexos_line("[sexdrive.nvme.ioq.create_sq.ok] cid=%llu sc=%llu sct=%llu cqh=%llu cqp=%llu", (unsigned long long) create_sq_cid, (unsigned long long) sq_sc, (unsigned long long) sq_sct, (unsigned long long) admin_cq_head, (unsigned long long) admin_cq_phase);
  uint64_t sq1tdbl = 0x1000ULL + (2ULL * 1ULL << (2u + (uint32_t) cap_dstrd));
  uint64_t cq1hdbl = 0x1000ULL + ((2ULL * 1ULL + 1ULL) << (2u + (uint32_t) cap_dstrd));
  sexos_line("[sexdrive.nvme.ioq.ready] qid=1 depth=16 sq_tail=%llu cq_head=%llu cq_phase=%llu sq1tdbl=%#llx cq1hdbl=%#llx", 0ULL, 0ULL, 1ULL, (unsigned long long) sq1tdbl, (unsigned long long) cq1hdbl);
  sexdrive_state.nvme_io_state.ready = true;
  sexdrive_state.nvme_io_state.map_va = map_va;
  sexdrive_state.nvme_io_state.io_sq_va = io_sq_va;
  sexdrive_state.nvme_io_state.io_cq_va = io_cq_va;
  sexdrive_state.nvme_io_state.sq1tdbl = sq1tdbl;
  sexdrive_state.nvme_io_state.cq1hdbl = cq1hdbl;
  sexdrive_state.nvme_io_state.sq_tail = 0u;
  sexdrive_state.nvme_io_state.cq_head = 0u;
  sexdrive_state.nvme_io_state.cq_phase = 1u;
  uint64_t ap3_buf_phys = sys_alloc_phys(PAGE_SIZE);
  uint64_t ap3_buf_va = sys_map_phys(ap3_buf_phys, PAGE_SIZE);
  if (ap3_buf_phys == 0ULL || (ap3_buf_phys == UINT64_MAX || (ap3_buf_phys % PAGE_SIZE != 0ULL || (ap3_buf_va == 0ULL || ap3_buf_va == UINT64_MAX)))) sexos_line("[sexdrive.storage100.rw.fail] reason=ap3_buf_alloc_invalid");
  else {
    uint64_t ap3_status = nvme_write_readback_proof(WRITE_PROOF_LBA * NVME_LBA_BYTES, WRITE_PROOF_LEN, ap3_buf_va);
    if (ap3_status != 0ULL) sexos_line("[sexdrive.storage100.rw.fail] reason=ap3_selftest_status status=%llu", (unsigned long long) ap3_status);
    else {
      uint64_t ap4_status = nvme_multiblock_write_readback_proof();
      if (ap4_status != 0ULL) sexos_line("[sexdrive.storage100.multi.fail] reason=ap4_selftest_status status=%llu", (unsigned long long) ap4_status);
      else if (STORAGE_100_PERSIST_WRITE_ENABLED) {
        uint64_t persist_write_status = nvme_persist_write_proof();
        if (persist_write_status != 0ULL) sexos_line("[sexdrive.storage100.persist.fail] phase=write reason=ap5a_selftest_status status=%llu", (unsigned long long) persist_write_status);
      } else if (STORAGE_100_PERSIST_READ_ENABLED) {
        uint64_t persist_read_status = nvme_persist_read_proof();
        if (persist_read_status != 0ULL) sexos_line("[sexdrive.storage100.persist.fail] phase=read reason=ap5a_selftest_status status=%llu", (unsigned long long) persist_read_status);
      } else if (STORAGE_100_FLUSH_AUDIT_ENABLED)(void) nvme_storage100_flush_audit();
      else if (STORAGE_100_NEGATIVE_ENABLED && STORAGE_100_NEG_MISMATCH_ENABLED) {
        uint64_t neg_status = nvme_storage100_negative_mismatch();
        if (neg_status != 0ULL) sexos_line("[sexdrive.storage100.neg.mismatch.fail] reason=ap6_negative_status status=%llu", (unsigned long long) neg_status);
      }
    }
  }
  if (STORAGE_100_IO_READ_PROBE_ENABLED) {
    uint32_t io_q_entries = 16u;
    uint32_t io_sq_tail = 0u;
    uint32_t io_cq_head = 0u;
    uint32_t io_cq_phase = 1u;
    uint64_t data_phys = sys_alloc_phys(PAGE_SIZE);
    if (data_phys == 0ULL || (data_phys == UINT64_MAX || data_phys % PAGE_SIZE != 0ULL)) {
      sexos_line("[sexdrive.nvme.io.read.err] reason=data_alloc_invalid phys=%#llx", (unsigned long long) data_phys);
      return;
    }
    uint64_t data_va = sys_map_phys(data_phys, PAGE_SIZE);
    if (data_va == 0ULL || data_va == UINT64_MAX) {
      sexos_line("[sexdrive.nvme.io.read.err] reason=data_map_failed va=%#llx", (unsigned long long) data_va);
      return;
    }
    for (size_t i = 0; i < (size_t)(PAGE_SIZE / 8ULL); ++i) sexos_volatile_write64((uint64_t *)(uintptr_t) data_va + i, 0ULL);
    sexos_line("[sexdrive.nvme.io.read.begin] data_phys=%#llx data_va=%#llx qid=%llu depth=%llu sq_tail=%llu cq_head=%llu cq_phase=%llu", (unsigned long long) data_phys, (unsigned long long) data_va, 1ULL, (unsigned long long) io_q_entries, (unsigned long long) io_sq_tail, (unsigned long long) io_cq_head, (unsigned long long) io_cq_phase);
    uint16_t read_cid = 0x0045u;
    uint32_t *io_sqe_ptr = (uint32_t *)(uintptr_t)(io_sq_va + (uint64_t) io_sq_tail * 64ULL);
    for (size_t i = 0; i < 16; ++i) sexos_volatile_write32(io_sqe_ptr + i, 0u);
    sexos_volatile_write32(io_sqe_ptr + 0, 0x02u | (uint32_t) read_cid << 16);
    sexos_volatile_write32(io_sqe_ptr + 1, 1u);
    sexos_volatile_write32(io_sqe_ptr + 6, (uint32_t)(data_phys & 0xFFFFFFFFULL));
    sexos_volatile_write32(io_sqe_ptr + 7, (uint32_t)(data_phys >> 32));
    sexos_volatile_write32(io_sqe_ptr + 10, 0u);
    sexos_volatile_write32(io_sqe_ptr + 11, 0u);
    sexos_volatile_write32(io_sqe_ptr + 12, 0u);
    sexos_line("[sexdrive.nvme.io.read.submit] cid=%llu opc=%#llx nsid=%llu slba=%llu nlb=%llu prp1=%#llx", (unsigned long long) read_cid, 0x02ULL, 1ULL, 0ULL, 0ULL, (unsigned long long) data_phys);
    sexos_compiler_fence_seq_cst();
    io_sq_tail = (io_sq_tail + 1u) % io_q_entries;
    sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + sq1tdbl), io_sq_tail);
    sexos_line("[sexdrive.nvme.io.read.doorbell] sq1tdbl=%#llx new_tail=%llu dstrd=%llu", (unsigned long long) sq1tdbl, (unsigned long long) io_sq_tail, (unsigned long long) cap_dstrd);
    bool read_done = false;
    uint32_t rd_dw2 = 0u;
    uint32_t rd_dw3 = 0u;
    for (uint32_t _poll = 0; _poll < 1000000u; ++_poll) {
      const uint32_t *cqe_ptr = (const uint32_t *)(uintptr_t)(io_cq_va + (uint64_t) io_cq_head * 16ULL);
      uint32_t dw0 = sexos_volatile_read32(cqe_ptr + 0);
      uint32_t dw1 = sexos_volatile_read32(cqe_ptr + 1);
      uint32_t dw2 = sexos_volatile_read32(cqe_ptr + 2);
      uint32_t dw3 = sexos_volatile_read32(cqe_ptr + 3);
      uint32_t cid_now = dw3 & 0xFFFFu;
      uint32_t phase_now = dw3 >> 16 & 0x1u;
      if (cid_now == (uint32_t) read_cid && phase_now == io_cq_phase) {
        read_done = true;
        rd_dw2 = dw2;
        rd_dw3 = dw3;
        sexos_line("[sexdrive.nvme.io.read.cqe] cid=%llu phase=%llu dw0=%#llx dw1=%#llx dw2=%#llx dw3=%#llx", (unsigned long long) cid_now, (unsigned long long) phase_now, (unsigned long long) dw0, (unsigned long long) dw1, (unsigned long long) dw2, (unsigned long long) dw3);
        break;
      }
    }
    if (!read_done) {
      sexos_line("[sexdrive.nvme.io.read.err] reason=cqe_timeout cid=%llu head=%llu phase=%llu", (unsigned long long) read_cid, (unsigned long long) io_cq_head, (unsigned long long) io_cq_phase);
      return;
    }
    uint32_t rd_sf = rd_dw3 >> 17 & 0x7FFFu;
    uint32_t rd_sc = rd_sf & 0xFFu;
    uint32_t rd_sct = rd_sf >> 8 & 0x7u;
    uint32_t rd_sqhd = rd_dw2 & 0xFFFFu;
    uint32_t rd_sqid = rd_dw2 >> 16 & 0xFFFFu;
    io_cq_head = (io_cq_head + 1u) % io_q_entries;
    if (io_cq_head == 0u) io_cq_phase ^= 1u;
    sexos_volatile_write32((uint32_t *)(uintptr_t)(map_va + cq1hdbl), io_cq_head);
    if (rd_sc != 0u || rd_sct != 0u) {
      sexos_line("[sexdrive.nvme.io.read.err] reason=status_fail cid=%llu sc=%llu sct=%llu sqhd=%llu sqid=%llu dw3=%#llx", (unsigned long long) read_cid, (unsigned long long) rd_sc, (unsigned long long) rd_sct, (unsigned long long) rd_sqhd, (unsigned long long) rd_sqid, (unsigned long long) rd_dw3);
      return;
    }
    uint64_t d0 = sexos_volatile_read64((const uint64_t *)(uintptr_t) data_va);
    uint64_t d1 = sexos_volatile_read64((const uint64_t *)(uintptr_t)(data_va + 8ULL));
    uint64_t d2 = sexos_volatile_read64((const uint64_t *)(uintptr_t)(data_va + 16ULL));
    uint64_t d3 = sexos_volatile_read64((const uint64_t *)(uintptr_t)(data_va + 24ULL));
    sexos_line("[sexdrive.nvme.io.read.ok] cid=%llu sc=%llu sct=%llu sqhd=%llu sqid=%llu cqh=%llu cqp=%llu d0=%#llx d1=%#llx d2=%#llx d3=%#llx", (unsigned long long) read_cid, (unsigned long long) rd_sc, (unsigned long long) rd_sct, (unsigned long long) rd_sqhd, (unsigned long long) rd_sqid, (unsigned long long) io_cq_head, (unsigned long long) io_cq_phase, (unsigned long long) d0, (unsigned long long) d1, (unsigned long long) d2, (unsigned long long) d3);
    sexdrive_state.nvme_io_state.sq_tail = io_sq_tail;
    sexdrive_state.nvme_io_state.cq_head = io_cq_head;
    sexdrive_state.nvme_io_state.cq_phase = io_cq_phase;
  } else sexos_line("[sexdrive.storage100.io_read_probe.skip] reason=not_requested");
}
static void xhci_probe_mmio(void) {
  uint64_t map_va = sexos_map_pci_bar(SLOT_USB_HOST, 0ULL, 0x1000ULL);
  if (map_va == UINT64_MAX || map_va == 0ULL) {
    sexos_line("[sexdrive] XHCI probe: no BAR lease/mapping available");
    return;
  }
  const uint32_t *regs = (const uint32_t *)(uintptr_t) map_va;
  uint32_t cap0 = sexos_volatile_read32(regs + 0);
  uint8_t caplength = (uint8_t)(cap0 & 0xFFu);
  uint16_t hciversion = (uint16_t)(cap0 >> 16 & 0xFFFFu);
  uint32_t hcsp1 = sexos_volatile_read32(regs + 1);
  uint32_t hcc1 = sexos_volatile_read32(regs + 4);
  sexos_line("[sexdrive] XHCI MMIO probe ok va=%#llx caplength=%#llx hciversion=%#llx hcsp1=%#llx hcc1=%#llx", (unsigned long long) map_va, (unsigned long long) caplength, (unsigned long long) hciversion, (unsigned long long) hcsp1, (unsigned long long) hcc1);
}
void sexdrive_main(void) {
  sexdrive_state_init(&sexdrive_state);
  sexos_line("[sexdrive.init.start]");
  nvme_probe_bar();
  xhci_probe_mmio();
  for (uint32_t _spin = 0; _spin < 10000000u; ++_spin) sexos_spin_loop_hint();
  uint64_t fb_size = 1024ULL * 768ULL * 4ULL;
  uint64_t shared_addr = sexos_alloc_shared_buffer(fb_size, 1ULL);
  if (shared_addr == 0ULL) while (1) sexos_spin_loop_hint();
  (void) sexos_pdx_call(SLOT_SHELL, OP_SHELL_BIND_BUFFER, shared_addr, 0ULL, 0ULL);
  uint32_t frame = 0u;
  if (hot_log()) sexos_line("[sexdrive.ready]");
  while (1) {
    SexosMessage msg = {
      0
    };
    if (sexos_try_listen(0ULL, &msg)) {
      uint64_t cmd = msg.type_id;
      uint64_t offset = msg.arg0;
      uint64_t size = msg.arg1;
      uint64_t _buf_cap = msg.arg2;
      uint64_t ready_snapshot = sexdrive_state.nvme_io_state.ready ? 1ULL : 0ULL;
      const char *op = cmd == BLOCK_READ ? "READ" : cmd == BLOCK_WRITE ? "WRITE" : "OTHER";
      uint64_t lba = BLOCK_SECTOR_SIZE != 0ULL ? offset / BLOCK_SECTOR_SIZE : 0ULL;
      uint64_t submit_cid_snapshot = (uint64_t) sexdrive_state.nvme_io_state.next_cid;
      uint64_t submit_tail_snapshot = (uint64_t) sexdrive_state.nvme_io_state.sq_tail;
      if (hot_log()) sexos_line("[sexdrive.block.typed.recv] cmd=%llu offset=%#llx size=%llu buf_cap=%#llx caller=%u", (unsigned long long) cmd, (unsigned long long) offset, (unsigned long long) size, (unsigned long long) _buf_cap, (unsigned) msg.caller_pd);
      if (cmd == BLOCK_READ || cmd == BLOCK_WRITE) if (hot_log()) sexos_line("[sexdrive.block.req] op=%s ready=%llu lba=%llu bytes=%llu buffer_cap=%#llx device_cap=%#llx", op, (unsigned long long) ready_snapshot, (unsigned long long) lba, (unsigned long long) size, (unsigned long long) _buf_cap, (unsigned long long) SLOT_NVME_HOST);
      uint64_t reply_val = 0ULL;
      if (cmd == BLOCK_READ) {
        if (hot_log()) sexos_line("[sexdrive.block.read.api.recv] offset=%#llx size=%llu buf_cap=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) _buf_cap);
        if (hot_log()) sexos_line("[sexdrive.block.read.handoff.begin] offset=%#llx size=%llu buf_cap=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) _buf_cap);
        if (size == 0ULL || size > BLOCK_MAX_XFER) {
          sexos_line("[sexdrive.block.read.api.err] reason=bad_len size=%llu max=%llu", (unsigned long long) size, (unsigned long long) BLOCK_MAX_XFER);
          sexos_line("[sexdrive.block.read.handoff.err] reason=bad_len size=%llu", (unsigned long long) size);
          reply_val = BLOCK_ERR_BAD_LEN;
        } else if (offset % BLOCK_SECTOR_SIZE != 0ULL) {
          sexos_line("[sexdrive.block.read.api.err] reason=unaligned_offset offset=%#llx sector=%llu", (unsigned long long) offset, (unsigned long long) BLOCK_SECTOR_SIZE);
          sexos_line("[sexdrive.block.read.handoff.err] reason=unaligned_offset offset=%#llx", (unsigned long long) offset);
          reply_val = BLOCK_ERR_BAD_LEN;
        } else if (_buf_cap == SLOT_BUF_LEND) if (size != 512ULL) {
          sexos_line("[sexdrive.block.read.handoff.err] reason=bad_len_phase_b size=%llu expect=512", (unsigned long long) size);
          reply_val = BLOCK_ERR_BAD_LEN;
        } else {
          uint64_t fill_va = sexos_map_mem_lend(SLOT_BUF_LEND);
          if (fill_va == 0ULL || fill_va == UINT64_MAX) {
            sexos_line("[sexdrive.block.read.handoff.err] reason=map_failed va=%#llx", (unsigned long long) fill_va);
            if (hot_log()) sexos_line("[sexdrive.nvme.cmd.shape.err] path=typed reason=memlend_map_failed buffer_cap=%#llx", (unsigned long long) SLOT_BUF_LEND);
            reply_val = BLOCK_ERR_NO_DEVICE;
          } else {
            if (hot_log()) sexos_line("[sexdrive.bufcap.map.ok] fill_va=%#llx", (unsigned long long) fill_va);
            if (hot_log()) sexos_line("[sexdrive.block.nvme.submit] op=%s lba=%llu bytes=%llu cid=%llu tail=%llu ready=%llu", op, (unsigned long long) lba, (unsigned long long) size, (unsigned long long) submit_cid_snapshot, (unsigned long long) submit_tail_snapshot, (unsigned long long) ready_snapshot);
            reply_val = nvme_read_into_mapped_va(offset, size, fill_va);
          }
        } else {
          sexos_line("[sexdrive.block.read.api.validate.ok] offset=%#llx size=%llu lba_bytes=%llu", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) NVME_LBA_BYTES);
          if (_buf_cap == 0ULL) sexos_line("[sexdrive.block.read.handoff.err] reason=buffer_cap_not_real buf_cap=%#llx", (unsigned long long) _buf_cap);
          else sexos_line("[sexdrive.block.read.handoff.err] reason=buffer_cap_unverifiable buf_cap=%#llx", (unsigned long long) _buf_cap);
          if (hot_log()) sexos_line("[sexdrive.block.nvme.submit] op=%s lba=%llu bytes=%llu cid=%llu tail=%llu ready=%llu", op, (unsigned long long) lba, (unsigned long long) size, (unsigned long long) submit_cid_snapshot, (unsigned long long) submit_tail_snapshot, (unsigned long long) ready_snapshot);
          reply_val = nvme_read_into_bounce(offset, size);
        }
      } else if (cmd == BLOCK_WRITE) {
        if (hot_log()) sexos_line("[sexdrive.block.write.api.recv] offset=%#llx size=%llu buf_cap=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) _buf_cap);
        if (write_guard_allows(offset, size, _buf_cap)) {
          uint64_t fill_va = sexos_map_mem_lend(SLOT_BUF_LEND);
          if (fill_va == 0ULL || fill_va == UINT64_MAX) {
            sexos_line("[sexdrive.nvme.write.err] reason=map_failed va=%#llx", (unsigned long long) fill_va);
            if (hot_log()) sexos_line("[sexdrive.nvme.cmd.shape.err] path=typed reason=memlend_map_failed buffer_cap=%#llx", (unsigned long long) SLOT_BUF_LEND);
            reply_val = BLOCK_ERR_NO_DEVICE;
          } else {
            if (hot_log()) sexos_line("[sexdrive.bufcap.map.ok] fill_va=%#llx", (unsigned long long) fill_va);
            if (hot_log()) sexos_line("[sexdrive.block.nvme.submit] op=%s lba=%llu bytes=%llu cid=%llu tail=%llu ready=%llu", op, (unsigned long long) lba, (unsigned long long) size, (unsigned long long) submit_cid_snapshot, (unsigned long long) submit_tail_snapshot, (unsigned long long) ready_snapshot);
            uint64_t proof_offset = WRITE_PROOF_LBA * NVME_LBA_BYTES;
            if (offset == proof_offset) reply_val = nvme_write_readback_proof(offset, size, fill_va);
            else reply_val = nvme_write_one_block(offset, size, fill_va);
          }
        } else {
          sexos_line("[sexdrive.block.typed] cmd=%llu ERR_NO_DEVICE honest=write_not_implemented_guard_only", (unsigned long long) cmd);
          reply_val = BLOCK_ERR_NO_DEVICE;
        }
      } else if (cmd == BLOCK_SYNC) {
        sexos_line("[sexdrive.sync.recv] cmd=%llu honest=flush_not_emulated_by_qemu_nvme", (unsigned long long) cmd);
        reply_val = BLOCK_ERR_NO_DEVICE;
      } else {
        sexos_line("[sexdrive.block.typed] cmd=%llu ERR_BAD_CMD unknown", (unsigned long long) cmd);
        reply_val = BLOCK_ERR_BAD_CMD;
      }
      if (hot_log()) sexos_line("[sexblock.abi.reply.encode] caller=%u status=%llu", (unsigned) msg.caller_pd, (unsigned long long) reply_val);
      (void) sexos_reply(msg.caller_pd, reply_val);
      if (hot_log()) sexos_line("[sexdrive.block.typed.reply] cmd=%llu caller=%u status=%llu", (unsigned long long) cmd, (unsigned) msg.caller_pd, (unsigned long long) reply_val);
      if (cmd == BLOCK_READ || cmd == BLOCK_WRITE) {
        if (reply_val == 0ULL) {
          if (hot_log()) sexos_line("[sexdrive.block.nvme.cqe] op=%s cid=%llu status=0", op, (unsigned long long) submit_cid_snapshot);
          if (hot_log()) sexos_line("[sexdrive.block.reply] op=%s status=0 bytes=%llu ready=1", op, (unsigned long long) size);
        } else if (reply_val == BLOCK_ERR_NO_DEVICE) {
          if (hot_log()) sexos_line("[sexdrive.block.nvme.cqe.timeout] op=%s cid=%llu polls=%llu", op, (unsigned long long) submit_cid_snapshot, 0ULL);
          const char *reason = ready_snapshot == 0ULL ? "no_ioq_ready" : "no_device_other";
          if (hot_log()) sexos_line("[sexdrive.block.reply] op=%s status=4 reason=%s ready=%llu", op, reason, (unsigned long long) ready_snapshot);
        } else if (hot_log()) sexos_line("[sexdrive.block.reply] op=%s status=%llu reason=other ready=%llu", op, (unsigned long long) reply_val, (unsigned long long) ready_snapshot);
      }
    }
    frame += 1u;
    uint32_t *ptr = (uint32_t *)(uintptr_t) shared_addr;
    for (size_t y = 0; y < 768; ++y) for (size_t x = 0; x < 1024; ++x) {
      uint32_t color = ((uint32_t) x ^ (uint32_t) y) + frame;
      ptr[y * 1024 + x] = color;
    }
    for (uint32_t _spin = 0; _spin < 2000000u; ++_spin) sexos_spin_loop_hint();
  }
}
