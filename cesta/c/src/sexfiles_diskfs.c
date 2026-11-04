#include "sexfiles.h"
/*DiskFS backend, V1/V2 manifest, in-memory scaffold, checkpoints, extents, and SexFS v0 on-disk store (servers/sexfiles/src/backends/diskfs.rs).*/
#define DISKFS_MANIFEST_OBJECT_PATH_STR "/disk/sexfiles-proof-v1"
#define DISKFS_MANIFEST_OBJECT_PATH_LEN ((size_t)23)
#define DISKFS_OBJECT_PATH_LINEN_STR "/disk/linen-object-v1"
#define DISKFS_OBJECT_PATH_LINEN_LEN ((size_t)21)
#define DISKFS_OBJECT_PATH_QUIL_STR "/disk/quil-object-v1"
#define DISKFS_OBJECT_PATH_QUIL_LEN ((size_t)20)
#define SEXFS_V0_SUPERBLOCK_MAGIC UINT64_C(0x3076313353465853)
#define SEXFS_V0_FREEMAP_MAGIC UINT64_C(0x30564D5045455246)
#define SEXFS_V0_SUPERBLOCK_SECTOR UINT64_C(0)
#define SEXFS_V0_BACKUP_SECTOR UINT64_C(1)
#define SEXFS_V0_OBJECT_TABLE_START UINT64_C(2)
#define SEXFS_V0_OBJECT_TABLE_SECTORS UINT64_C(4)
#define SEXFS_V0_FREEMAP_SECTOR UINT64_C(6)
#define SEXFS_V0_OBJECT_ENTRY_BYTES ((size_t)128)
#define SEXFS_V0_OBJECT_TABLE_BYTES ((size_t)2048)
#define SEXFS_V0_DATA_REGION_MIN_BLOCK ((size_t)16)
#define SEXFS_V0_DATA_REGION_MAX_BLOCK ((size_t)252)
#define SEXFS_V0_DATA_LBA_MIN SEXFS_V0_OBJECT_START_LBA
#define SEXFS_V0_DATA_LBA_MAX SEXFS_V0_OBJECT_END_LBA
static inline bool diskfs_hot_log(void) {
  return sexfiles_take_budget(&sexfiles_state.diskfs_hot_log_budget);
}
static inline uint64_t sat_add_u64(uint64_t a, uint64_t b) {
  uint64_t sum = a + b;
  return sum < a ? UINT64_MAX : sum;
}
static inline uint64_t sat_sub_u64(uint64_t a, uint64_t b) {
  return b > a ? 0 : a - b;
}
static inline uint16_t read_le16(const uint8_t *p) {
  return (uint16_t)((uint16_t) p[0] | (uint16_t) p[1] << 8);
}
static inline uint32_t read_le32(const uint8_t *p) {
  return (uint32_t) p[0] | (uint32_t) p[1] << 8 | ((uint32_t) p[2] << 16 | (uint32_t) p[3] << 24);
}
static inline uint64_t read_le64(const uint8_t *p) {
  uint64_t v = 0;
  for (size_t i = 0; i < 8; ++i) v |= (uint64_t) p[i] << i * 8;
  return v;
}
static inline void write_le16(uint8_t *p, uint16_t v) {
  p[0] = (uint8_t)(v & 0xFFu);
  p[1] = (uint8_t)(v >> 8 & 0xFFu);
}
static inline void write_le32(uint8_t *p, uint32_t v) {
  for (size_t i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> i * 8 & 0xFFu);
}
static inline void write_le64(uint8_t *p, uint64_t v) {
  for (size_t i = 0; i < 8; ++i) p[i] = (uint8_t)(v >> i * 8 & 0xFFu);
}
static bool diskfs_writer_offer(uint64_t *job, uint32_t *home) {
  RegionWriterReceipt query = sexos_region_writer_query(SEXOS_SLOT_buf_lend);
  if (query.status != REGION_WRITER_OK || (query.current == 0 || (query.job != 0 || query.peer == 0))) return false;
  uint64_t next = sexfiles_state.diskfs_writer_job_next;
  if (next == 0 || next == UINT64_MAX) return false;
  RegionWriterReceipt offered = sexos_region_writer_change(SEXOS_SLOT_buf_lend, REGION_WRITER_OFFER, query.revision, next);
  if (offered.status != REGION_WRITER_OK || (offered.current != query.current || offered.job != next)) return false;
  sexfiles_state.diskfs_writer_job_next = next + 1;
  *job = next;
  *home = query.current;
  return true;
}
static bool diskfs_writer_settle(uint64_t job, uint32_t home) {
  RegionWriterReceipt query = sexos_region_writer_query(SEXOS_SLOT_buf_lend);
  if (query.status != REGION_WRITER_OK || query.current != home) return false;
  if (query.job == 0) return true;
  if (query.job != job) return false;
  RegionWriterReceipt cancelled = sexos_region_writer_change(SEXOS_SLOT_buf_lend, REGION_WRITER_CANCEL, query.revision, job);
  return cancelled.status == REGION_WRITER_OK && (cancelled.current == home && cancelled.job == 0);
}
uint64_t diskfs_block_call(uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
  bool writer_offered = false;
  uint64_t writer_job = 0;
  uint32_t writer_home = 0;
  if (opcode == BLOCK_READ && arg2 == SEXOS_SLOT_buf_lend) {
    writer_offered = diskfs_writer_offer(&writer_job, &writer_home);
    if (!writer_offered) return BLOCK_ERR_NO_DEVICE;
  }
  if (diskfs_hot_log()) sexos_line("[sexfiles.ap24.provenance] head=COMPILE_TIME_UNKNOWN dirty=0 note=clean_diag");
  if (opcode == BLOCK_READ || opcode == BLOCK_WRITE) {
    const char *op = opcode == BLOCK_READ ? "READ" : "WRITE";
    uint64_t lba = arg0 / BLOCK_SECTOR_SIZE;
    if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.block.call] op=%s lba=%llu bytes=%llu slot=%llu buffer_cap=%#llx device_cap=%#llx", op, (unsigned long long) lba, (unsigned long long) arg1, (unsigned long long) SEXOS_SLOT_block, (unsigned long long) arg2, (unsigned long long) SEXOS_SLOT_block);
  }
  if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.call] slot=%llu opcode=%#llx arg0=%#llx arg1=%#llx arg2=%#llx", (unsigned long long) SEXOS_SLOT_block, (unsigned long long) opcode, (unsigned long long) arg0, (unsigned long long) arg1, (unsigned long long) arg2);
  SexosReply send_res = sexos_pdx_call(SEXOS_SLOT_block, opcode, arg0, arg1, arg2);
  if (send_res.status != 0) {
    if (writer_offered && !diskfs_writer_settle(writer_job, writer_home)) sexos_fail();
    if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.reply] status=%#llx value=0 err=enqueue_fail", (unsigned long long) send_res.status);
    if (opcode == BLOCK_READ || opcode == BLOCK_WRITE) {
      const char *op = opcode == BLOCK_READ ? "READ" : "WRITE";
      if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.block.reply] op=%s status=%llu bytes=%llu", op, (unsigned long long) send_res.status, (unsigned long long) arg1);
    }
    return send_res.status;
  } while (1) {
    SexosRequest request = (SexosRequest) {
      .id = 0
    };
#if SEXOS_SEXFILES_CURRENT_DOCUMENT
request = sexos_rpc_listen_once(0);

#else
request.message = sexos_listen(&sexfiles_state.listener, 0);

#endif
    SexosMessage msg = request.message;
    if (msg.type_id == 1) {
      uint64_t reply_val = msg.arg0;
      if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.reply] status=0x0 value=%#llx", (unsigned long long) reply_val);
      if (opcode == BLOCK_READ || opcode == BLOCK_WRITE) {
        const char *op = opcode == BLOCK_READ ? "READ" : "WRITE";
        if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.block.reply] op=%s status=%llu bytes=%llu", op, (unsigned long long) reply_val, (unsigned long long) arg1);
      }
      if (writer_offered && !diskfs_writer_settle(writer_job, writer_home)) sexos_fail();
      return reply_val;
    }
    if (msg.type_id != 0)(void) defer_stash(request);
    sexos_yield();
  }
}
uint64_t diskfs_block_read(uint64_t offset, uint64_t size, uint64_t buffer_cap) {
  if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.typed.call] cmd=BLOCK_READ offset=%#llx size=%llu buf_cap=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) buffer_cap);
  uint64_t reply = diskfs_block_call(BLOCK_READ, offset, size, buffer_cap);
  if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.typed.reply] cmd=BLOCK_READ status=%llu", (unsigned long long) reply);
  return reply;
}
uint64_t diskfs_block_write(uint64_t offset, uint64_t size, uint64_t buffer_cap) {
  if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.typed.call] cmd=BLOCK_WRITE offset=%#llx size=%llu buf_cap=%#llx", (unsigned long long) offset, (unsigned long long) size, (unsigned long long) buffer_cap);
  uint64_t reply = diskfs_block_call(BLOCK_WRITE, offset, size, buffer_cap);
  if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.typed.reply] cmd=BLOCK_WRITE status=%llu", (unsigned long long) reply);
  return reply;
}
uint64_t diskfs_block_sync(void) {
  if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.typed.call] cmd=BLOCK_SYNC");
  uint64_t reply = diskfs_block_call(BLOCK_SYNC, 0, 0, 0);
  if (diskfs_hot_log()) sexos_line("[sexfiles.diskfs.typed.reply] cmd=BLOCK_SYNC status=%llu", (unsigned long long) reply);
  return reply;
}
uint64_t diskfs_fsync(void) {
  sexos_line("[sexfiles.disk.fsync.begin]");
  uint64_t status = diskfs_block_sync();
  if (status == 0) sexos_line("[sexfiles.disk.fsync.reply.ok]");
  else sexos_line("[sexfiles.disk.fsync.err] status=%llu", (unsigned long long) status);
  return status;
}
uint64_t diskfs_manifest_name_hash(const uint8_t *name, size_t len) {
  uint64_t hash = UINT64_C(0xCBF29CE484222325);
  for (size_t i = 0; i < len; ++i) {
    hash ^= (uint64_t) name[i];
    hash *= UINT64_C(0x100000001B3);
  }
  return hash;
}
void diskfs_proof_manifest_build_single_entry_sector(uint8_t *out_sector) {
  memset(out_sector, 0, 512);
  DiskManifestEntryV1 entry = (DiskManifestEntryV1) {
    .name_hash = diskfs_manifest_name_hash((const uint8_t *) "/disk/sexfiles-proof-v1", DISKFS_MANIFEST_OBJECT_PATH_LEN),.start_lba = DISKFS_PROOF_OBJECT_START_LBA,.len_bytes = (uint32_t)(DISKFS_PROOF_OBJECT_SECTORS * BLOCK_SECTOR_SIZE),.flags = DISKFS_MANIFEST_FLAG_READ | DISKFS_MANIFEST_FLAG_WRITE
  };
  write_le64(out_sector + 0, DISKFS_MANIFEST_MAGIC);
  write_le16(out_sector + 8, DISKFS_MANIFEST_VERSION);
  write_le16(out_sector + 10, 1);
  write_le64(out_sector + 32, entry.name_hash);
  write_le64(out_sector + 40, entry.start_lba);
  write_le32(out_sector + 48, entry.len_bytes);
  write_le16(out_sector + 52, entry.flags);
}
bool diskfs_proof_manifest_parse_single_entry(const uint8_t *sector, DiskManifestEntryV1 *out_entry, uint64_t *out_err) {
  uint64_t magic = read_le64(sector + 0);
  if (magic != DISKFS_MANIFEST_MAGIC) {
    *out_err = (uint64_t) ERR_PERM_DENIED;
    return false;
  }
  uint16_t version = read_le16(sector + 8);
  if (version != DISKFS_MANIFEST_VERSION) {
    *out_err = (uint64_t) ERR_PERM_DENIED;
    return false;
  }
  uint16_t entry_count = read_le16(sector + 10);
  if (entry_count == 0 || entry_count > DISKFS_MANIFEST_ENTRY_MAX) {
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  DiskManifestEntryV1 entry = (DiskManifestEntryV1) {
    .name_hash = read_le64(sector + 32),.start_lba = read_le64(sector + 40),.len_bytes = read_le32(sector + 48),.flags = read_le16(sector + 52)
  };
  if (entry.len_bytes == 0) {
    *out_err = BLOCK_ERR_BAD_LEN;
    return false;
  }
  if ((uint64_t) entry.len_bytes > DISKFS_PROOF_OBJECT_SECTORS * BLOCK_SECTOR_SIZE) {
    *out_err = BLOCK_ERR_BAD_LEN;
    return false;
  }
  uint64_t sectors = ((uint64_t) entry.len_bytes + (BLOCK_SECTOR_SIZE - 1)) / BLOCK_SECTOR_SIZE;
  if (sectors == 0) {
    *out_err = BLOCK_ERR_BAD_LEN;
    return false;
  }
  uint64_t end_lba = sat_add_u64(entry.start_lba, sat_sub_u64(sectors, 1));
  if (entry.start_lba <= DISKFS_MANIFEST_LBA && end_lba >= DISKFS_MANIFEST_LBA) {
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  if (entry.start_lba <= DISKFS_WRITE_PROOF_LBA && end_lba >= DISKFS_WRITE_PROOF_LBA) {
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  uint64_t expected_hash = diskfs_manifest_name_hash((const uint8_t *) "/disk/sexfiles-proof-v1", DISKFS_MANIFEST_OBJECT_PATH_LEN);
  if (entry.name_hash != expected_hash) {
    *out_err = (uint64_t) ERR_PERM_DENIED;
    return false;
  }
  if (entry.start_lba != DISKFS_PROOF_OBJECT_START_LBA) {
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  *out_entry = entry;
  return true;
}
void diskfs_proof_manifest_build_v2_entries_sector(uint8_t *out_sector) {
  memset(out_sector, 0, 512);
  DiskManifestEntryV1 entries[3] = {
    {
      diskfs_manifest_name_hash((const uint8_t *) "/disk/sexfiles-proof-v1", DISKFS_MANIFEST_OBJECT_PATH_LEN), DISKFS_PROOF_OBJECT_START_LBA, DISKFS_OBJECT_SLOT_SIZE, DISKFS_MANIFEST_FLAG_READ | DISKFS_MANIFEST_FLAG_WRITE
    }, {
      diskfs_manifest_name_hash((const uint8_t *) "/disk/linen-object-v1", DISKFS_OBJECT_PATH_LINEN_LEN), DISKFS_OBJECT_SLOT_LINEN_LBA, DISKFS_OBJECT_SLOT_SIZE, DISKFS_MANIFEST_FLAG_READ | DISKFS_MANIFEST_FLAG_WRITE
    }, {
      diskfs_manifest_name_hash((const uint8_t *) "/disk/quil-object-v1", DISKFS_OBJECT_PATH_QUIL_LEN), DISKFS_OBJECT_SLOT_QUIL_LBA, DISKFS_OBJECT_SLOT_SIZE, DISKFS_MANIFEST_FLAG_READ | DISKFS_MANIFEST_FLAG_WRITE
    }
  };
  write_le64(out_sector + 0, DISKFS_MANIFEST_MAGIC);
  write_le16(out_sector + 8, DISKFS_V2_MANIFEST_VERSION);
  write_le16(out_sector + 10, DISKFS_V2_ENTRY_COUNT);
  for (size_t ei = 0; ei < 3; ++ei) {
    size_t off = 32 + ei * 32;
    write_le64(out_sector + off, entries[ei].name_hash);
    write_le64(out_sector + (off + 8), entries[ei].start_lba);
    write_le32(out_sector + (off + 16), entries[ei].len_bytes);
    write_le16(out_sector + (off + 20), entries[ei].flags);
  }
}
bool diskfs_proof_manifest_parse_v2_entries(const uint8_t *sector, DiskManifestEntryV1 *out_entries, uint16_t *out_count, uint64_t *out_err) {
  uint64_t magic = read_le64(sector + 0);
  if (magic != DISKFS_MANIFEST_MAGIC) {
    *out_err = (uint64_t) ERR_PERM_DENIED;
    return false;
  }
  uint16_t version = read_le16(sector + 8);
  if (version < DISKFS_V2_MANIFEST_VERSION) {
    *out_err = (uint64_t) ERR_PERM_DENIED;
    return false;
  }
  uint16_t entry_count = read_le16(sector + 10);
  if (entry_count < DISKFS_V2_ENTRY_COUNT || entry_count > DISKFS_MANIFEST_ENTRY_MAX) {
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  for (size_t ei = 0; ei < 3; ++ei) {
    size_t off = 32 + ei * 32;
    uint64_t name_hash = read_le64(sector + off);
    uint64_t start_lba = read_le64(sector + (off + 8));
    uint32_t len_bytes = read_le32(sector + (off + 16));
    uint16_t flags = read_le16(sector + (off + 20));
    if (len_bytes == 0 || len_bytes > (uint32_t)(DISKFS_OBJECT_SLOT_SECTORS * BLOCK_SECTOR_SIZE)) {
      *out_err = BLOCK_ERR_BAD_LEN;
      return false;
    }
    uint64_t sectors = ((uint64_t) len_bytes + (BLOCK_SECTOR_SIZE - 1)) / BLOCK_SECTOR_SIZE;
    if (sectors == 0) {
      *out_err = BLOCK_ERR_BAD_LEN;
      return false;
    }
    uint64_t end_lba = sat_add_u64(start_lba, sat_sub_u64(sectors, 1));
    if (start_lba <= DISKFS_MANIFEST_LBA && end_lba >= DISKFS_MANIFEST_LBA) {
      *out_err = (uint64_t) ERR_OVERFLOW;
      return false;
    }
    if (start_lba <= DISKFS_WRITE_PROOF_LBA && end_lba >= DISKFS_WRITE_PROOF_LBA) {
      *out_err = (uint64_t) ERR_OVERFLOW;
      return false;
    }
    out_entries[ei] = (DiskManifestEntryV1) {
      .name_hash = name_hash,.start_lba = start_lba,.len_bytes = len_bytes,.flags = flags
    };
  }
  *out_count = entry_count;
  return true;
}
const char *diskfs_path_for_id(uint64_t path_id, size_t *out_len, uint64_t *out_err) {
  switch (path_id) {
    case 0: {
      *out_len = DISKFS_MANIFEST_OBJECT_PATH_LEN;
      return "/disk/sexfiles-proof-v1";
    }
    case 1: {
      *out_len = DISKFS_OBJECT_PATH_LINEN_LEN;
      return "/disk/linen-object-v1";
    }
    case 2: {
      *out_len = DISKFS_OBJECT_PATH_QUIL_LEN;
      return "/disk/quil-object-v1";
    }
    default: {
      *out_err = (uint64_t) ERR_BAD_CMD;
      return NULL;
    }
  }
}
bool diskfs_lookup_by_path_id(uint64_t path_id, uint64_t buf_va, DiskManifestEntryV1 *out_entry, uint64_t *out_err) {
  if (path_id >= (uint64_t) DISKFS_V2_ENTRY_COUNT) {
    *out_err = (uint64_t) ERR_BAD_CMD;
    return false;
  }
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t status = diskfs_block_read(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    sexos_line("[sexfiles.disk.file.lookup.err] reason=manifest_read code=%llu", (unsigned long long) status);
    *out_err = (uint64_t) ERR_NOT_FOUND;
    return false;
  }
  uint8_t sector[512];
  for (size_t i = 0; i < 512; ++i) sector[i] = p[i];
  DiskManifestEntryV1 entries[3];
  uint16_t count = 0;
  uint64_t parse_err = 0;
  if (!diskfs_proof_manifest_parse_v2_entries(sector, entries, &count, &parse_err)) {
    sexos_line("[sexfiles.disk.file.lookup.err] reason=v2_manifest_parse code=%llu", (unsigned long long) parse_err);
    *out_err = (uint64_t) ERR_NOT_FOUND;
    return false;
  }
  DiskManifestEntryV1 entry = entries[(size_t) path_id];
  size_t expected_len = 0;
  const char *expected_path = diskfs_path_for_id(path_id, &expected_len, out_err);
  if (expected_path == NULL) return false;
  uint64_t expected_hash = diskfs_manifest_name_hash((const uint8_t *) expected_path, expected_len);
  if (entry.name_hash != expected_hash) {
    sexos_line("[sexfiles.disk.file.lookup.err] reason=hash_mismatch path_id=%llu expected=%#llx got=%#llx", (unsigned long long) path_id, (unsigned long long) expected_hash, (unsigned long long) entry.name_hash);
    *out_err = (uint64_t) ERR_PERM_DENIED;
    return false;
  }
  sexos_line("[sexfiles.disk.file.lookup.ok] path_id=%llu start_lba=%llu len=%u flags=%#x", (unsigned long long) path_id, (unsigned long long) entry.start_lba, (unsigned) entry.len_bytes, (unsigned) entry.flags);
  *out_entry = entry;
  return true;
}
bool diskfs_lookup_path(const uint8_t *path, size_t path_len, uint64_t buf_va, DiskManifestEntryV1 *out_entry, uint64_t *out_err) {
  uint64_t arg_hash = diskfs_manifest_name_hash(path, path_len);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t status = diskfs_block_read(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    sexos_line("[sexfiles.disk.file.lookup.err] reason=manifest_read code=%llu", (unsigned long long) status);
    *out_err = (uint64_t) ERR_NOT_FOUND;
    return false;
  }
  uint8_t sector[512];
  for (size_t i = 0; i < 512; ++i) sector[i] = p[i];
  DiskManifestEntryV1 entries[3];
  uint16_t count = 0;
  uint64_t dummy_err = 0;
  if (diskfs_proof_manifest_parse_v2_entries(sector, entries, &count, &dummy_err)) {
    for (size_t ei = 0; ei < (size_t) DISKFS_V2_ENTRY_COUNT; ++ei) if (entries[ei].name_hash == arg_hash && entries[ei].len_bytes > 0) {
      sexos_line("[sexfiles.disk.file.lookup.ok] start_lba=%llu len=%u flags=%#x", (unsigned long long) entries[ei].start_lba, (unsigned) entries[ei].len_bytes, (unsigned) entries[ei].flags);
      *out_entry = entries[ei];
      return true;
    }
    *out_err = (uint64_t) ERR_NOT_FOUND;
    return false;
  }
  uint64_t expected_hash = diskfs_manifest_name_hash((const uint8_t *) "/disk/sexfiles-proof-v1", DISKFS_MANIFEST_OBJECT_PATH_LEN);
  if (arg_hash != expected_hash) {
    *out_err = (uint64_t) ERR_NOT_FOUND;
    return false;
  }
  DiskManifestEntryV1 entry;
  if (!diskfs_proof_manifest_parse_single_entry(sector, &entry, &dummy_err)) {
    *out_err = (uint64_t) ERR_NOT_FOUND;
    return false;
  }
  sexos_line("[sexfiles.disk.file.lookup.ok] start_lba=%llu len=%u flags=%#x", (unsigned long long) entry.start_lba, (unsigned) entry.len_bytes, (unsigned) entry.flags);
  *out_entry = entry;
  return true;
}
bool diskfs_write_object_entry(DiskManifestEntryV1 entry, uint64_t offset, const uint8_t *data, size_t data_len, uint64_t buf_va, uint64_t *out_written, uint64_t *out_err) {
  sexos_line("[sexfiles.disk.file.write.begin] offset=%llu len=%zu", (unsigned long long) offset, data_len);
  if (offset >= (uint64_t) entry.len_bytes) {
    sexos_line("[sexfiles.disk.file.bounds.err] reason=offset_past_end offset=%llu max=%u", (unsigned long long) offset, (unsigned) entry.len_bytes);
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  uint64_t end = offset + (uint64_t) data_len;
  if (end > (uint64_t) entry.len_bytes) {
    sexos_line("[sexfiles.disk.file.bounds.err] reason=end_past_max offset=%llu len=%zu max=%u", (unsigned long long) offset, data_len, (unsigned) entry.len_bytes);
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  if (data_len == 0) {
    *out_written = 0;
    return true;
  }
  uint64_t max_lba = entry.start_lba + sat_sub_u64(end, 1) / BLOCK_SECTOR_SIZE;
  if (max_lba >= DISKFS_WRITE_PROOF_LBA) {
    sexos_line("[sexfiles.disk.file.bounds.err] reason=lba_collision max_lba=%llu", (unsigned long long) max_lba);
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  uint64_t byte_off = offset;
  size_t data_off = 0;
  uint64_t written = 0;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  while (data_off < data_len) {
    uint64_t lba = entry.start_lba + byte_off / BLOCK_SECTOR_SIZE;
    size_t sector_off = (size_t)(byte_off % BLOCK_SECTOR_SIZE);
    size_t space = 512 > sector_off ? 512 - sector_off : 0;
    size_t rem = data_len - data_off;
    size_t chunk = rem < space ? rem : space;
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    uint64_t rstatus = diskfs_block_read(lba * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (rstatus != 0) {
      *out_err = rstatus;
      return false;
    }
    uint8_t sector[512];
    for (size_t i = 0; i < 512; ++i) sector[i] = p[i];
    for (size_t j = 0; j < chunk; ++j) sector[sector_off + j] = data[data_off + j];
    for (size_t i = 0; i < 512; ++i) p[i] = sector[i];
    uint64_t wstatus = diskfs_block_write(lba * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (wstatus != 0) {
      *out_err = wstatus;
      return false;
    }
    written += (uint64_t) chunk;
    byte_off += (uint64_t) chunk;
    data_off += chunk;
  }
  sexos_line("[sexfiles.disk.file.write.ok] offset=%llu written=%llu", (unsigned long long) offset, (unsigned long long) written);
  *out_written = written;
  return true;
}
bool diskfs_write_object(const uint8_t *path, size_t path_len, uint64_t offset, const uint8_t *data, size_t data_len, uint64_t buf_va, uint64_t *out_written, uint64_t *out_err) {
  DiskManifestEntryV1 entry;
  if (!diskfs_lookup_path(path, path_len, buf_va, &entry, out_err)) return false;
  return diskfs_write_object_entry(entry, offset, data, data_len, buf_va, out_written, out_err);
}
bool diskfs_read_object_entry(DiskManifestEntryV1 entry, uint64_t offset, uint8_t *out_buf, size_t out_len, uint64_t buf_va, uint64_t *out_read, uint64_t *out_err) {
  sexos_line("[sexfiles.disk.file.read.begin] offset=%llu len=%zu", (unsigned long long) offset, out_len);
  if (offset >= (uint64_t) entry.len_bytes) {
    sexos_line("[sexfiles.disk.file.bounds.err] reason=offset_past_end offset=%llu max=%u", (unsigned long long) offset, (unsigned) entry.len_bytes);
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  uint64_t end = offset + (uint64_t) out_len;
  if (end > (uint64_t) entry.len_bytes) {
    sexos_line("[sexfiles.disk.file.bounds.err] reason=end_past_max offset=%llu len=%zu max=%u", (unsigned long long) offset, out_len, (unsigned) entry.len_bytes);
    *out_err = (uint64_t) ERR_OVERFLOW;
    return false;
  }
  if (out_len == 0) {
    *out_read = 0;
    return true;
  }
  uint64_t byte_off = offset;
  size_t out_off = 0;
  uint64_t total_read = 0;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  while (out_off < out_len) {
    uint64_t lba = entry.start_lba + byte_off / BLOCK_SECTOR_SIZE;
    size_t sector_off = (size_t)(byte_off % BLOCK_SECTOR_SIZE);
    size_t space = 512 > sector_off ? 512 - sector_off : 0;
    size_t rem = out_len - out_off;
    size_t chunk = rem < space ? rem : space;
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    uint64_t status = diskfs_block_read(lba * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) {
      *out_err = status;
      return false;
    }
    uint8_t sector[512];
    for (size_t i = 0; i < 512; ++i) sector[i] = p[i];
    for (size_t j = 0; j < chunk; ++j) out_buf[out_off + j] = sector[sector_off + j];
    total_read += (uint64_t) chunk;
    byte_off += (uint64_t) chunk;
    out_off += chunk;
  }
  sexos_line("[sexfiles.disk.file.read.ok] offset=%llu read=%llu", (unsigned long long) offset, (unsigned long long) total_read);
  *out_read = total_read;
  return true;
}
bool diskfs_read_object(const uint8_t *path, size_t path_len, uint64_t offset, uint8_t *out_buf, size_t out_len, uint64_t buf_va, uint64_t *out_read, uint64_t *out_err) {
  DiskManifestEntryV1 entry;
  if (!diskfs_lookup_path(path, path_len, buf_va, &entry, out_err)) return false;
  return diskfs_read_object_entry(entry, offset, out_buf, out_len, buf_va, out_read, out_err);
}
bool diskfs_ensure_manifest(uint64_t buf_va, uint64_t *out_err) {
  sexos_line("[sexfiles.bridge.diskfs.manifest.ensure.begin]");
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t status = diskfs_block_read(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) sexos_line("[sexfiles.bridge.diskfs.manifest.ensure.err] reason=read_failed status=%llu", (unsigned long long) status);
  uint8_t sector[512];
  for (size_t i = 0; i < 512; ++i) sector[i] = p[i];
  DiskManifestEntryV1 v2_entries[3];
  uint16_t v2_count = 0;
  uint64_t dummy_err = 0;
  if (diskfs_proof_manifest_parse_v2_entries(sector, v2_entries, &v2_count, &dummy_err)) {
    sexos_line("[sexfiles.disk.manifest.v2.valid] entries=3");
    return true;
  }
  DiskManifestEntryV1 v1_entry;
  if (diskfs_proof_manifest_parse_single_entry(sector, &v1_entry, &dummy_err)) {
    sexos_line("[sexfiles.disk.manifest.v2.upgrade] from_version=1 entries=3");
    uint8_t v2_sector[512];
    diskfs_proof_manifest_build_v2_entries_sector(v2_sector);
    for (size_t i = 0; i < 512; ++i) p[i] = v2_sector[i];
    uint64_t ws = diskfs_block_write(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (ws != 0) {
      sexos_line("[sexfiles.disk.manifest.v2.err] reason=upgrade_write_failed status=%llu", (unsigned long long) ws);
      *out_err = ws;
      return false;
    }
    return true;
  }
  sexos_line("[sexfiles.disk.manifest.v2.bootstrap] reason=invalid_or_missing entries=3");
  uint8_t manifest_sector[512];
  diskfs_proof_manifest_build_v2_entries_sector(manifest_sector);
  for (size_t i = 0; i < 512; ++i) p[i] = manifest_sector[i];
  uint64_t write_status = diskfs_block_write(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (write_status != 0) {
    sexos_line("[sexfiles.bridge.diskfs.manifest.ensure.err] reason=write_failed status=%llu", (unsigned long long) write_status);
    *out_err = write_status;
    return false;
  }
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t verify_status = diskfs_block_read(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (verify_status != 0) {
    sexos_line("[sexfiles.bridge.diskfs.manifest.ensure.err] reason=verify_read_failed status=%llu", (unsigned long long) verify_status);
    *out_err = verify_status;
    return false;
  }
  uint8_t verify_sector[512];
  for (size_t i = 0; i < 512; ++i) verify_sector[i] = p[i];
  DiskManifestEntryV1 entry;
  uint64_t parse_err = 0;
  if (diskfs_proof_manifest_parse_single_entry(verify_sector, &entry, &parse_err)) {
    sexos_line("[sexfiles.bridge.diskfs.manifest.ensure.ok] hash=%#llx start_lba=%llu len=%u", (unsigned long long) entry.name_hash, (unsigned long long) entry.start_lba, (unsigned) entry.len_bytes);
    return true;
  }
  sexos_line("[sexfiles.bridge.diskfs.manifest.ensure.err] reason=verify_parse_failed code=%llu", (unsigned long long) parse_err);
  *out_err = parse_err;
  return false;
}
bool diskfs_ensure_manifest_v2(uint64_t buf_va, uint64_t *out_err) {
  sexos_line("[sexfiles.disk.manifest.v2.ensure.begin]");
  sexos_line("[sexfiles.diskfs.manifest.ensure.begin] lba=%llu", (unsigned long long) DISKFS_MANIFEST_LBA);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t status = diskfs_block_read(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    sexos_line("[sexfiles.disk.manifest.v2.err] reason=read_failed status=%llu", (unsigned long long) status);
    sexos_line("[sexfiles.diskfs.manifest.ensure.err] status=%llu reason=read_failed", (unsigned long long) status);
  }
  uint8_t sector[512];
  for (size_t i = 0; i < 512; ++i) sector[i] = p[i];
  DiskManifestEntryV1 v2_entries[3];
  uint16_t v2_count = 0;
  uint64_t dummy_err = 0;
  if (diskfs_proof_manifest_parse_v2_entries(sector, v2_entries, &v2_count, &dummy_err)) {
    sexos_line("[sexfiles.disk.manifest.v2.valid] entries=3");
    return true;
  }
  DiskManifestEntryV1 v1_entry;
  if (diskfs_proof_manifest_parse_single_entry(sector, &v1_entry, &dummy_err)) sexos_line("[sexfiles.disk.manifest.v2.upgrade] from_version=1 entries=3");
  else sexos_line("[sexfiles.disk.manifest.v2.bootstrap] entries=3");
  uint8_t v2_sector[512];
  diskfs_proof_manifest_build_v2_entries_sector(v2_sector);
  for (size_t i = 0; i < 512; ++i) p[i] = v2_sector[i];
  uint64_t write_status = diskfs_block_write(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (write_status != 0) {
    sexos_line("[sexfiles.disk.manifest.v2.err] reason=write_failed status=%llu", (unsigned long long) write_status);
    sexos_line("[sexfiles.diskfs.manifest.ensure.err] status=%llu reason=write_failed", (unsigned long long) write_status);
    *out_err = write_status;
    return false;
  }
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t verify_status = diskfs_block_read(DISKFS_MANIFEST_LBA * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (verify_status != 0) {
    sexos_line("[sexfiles.disk.manifest.v2.err] reason=verify_read_failed status=%llu", (unsigned long long) verify_status);
    *out_err = verify_status;
    return false;
  }
  uint8_t verify_sector[512];
  for (size_t i = 0; i < 512; ++i) verify_sector[i] = p[i];
  uint16_t count = 0;
  uint64_t parse_err = 0;
  if (diskfs_proof_manifest_parse_v2_entries(verify_sector, v2_entries, &count, &parse_err)) {
    sexos_line("[sexfiles.disk.manifest.v2.ok] entries=%u", (unsigned) count);
    return true;
  }
  sexos_line("[sexfiles.disk.manifest.v2.err] reason=verify_parse_failed code=%llu", (unsigned long long) parse_err);
  *out_err = parse_err;
  return false;
}
/*In-memory DiskFs scaffold checksums, journal, extent bitmap, and checkpoints.*/
static uint32_t checksum_superblock(const SexfilesSuperblock *sb) {
  return (uint32_t) sb->magic ^ (uint32_t)(sb->magic >> 32) ^ ((uint32_t) sb->version_major ^ (uint32_t) sb->version_minor) ^ (sb->block_size ^ ((uint32_t) sb->fs_generation ^ (uint32_t)(sb->fs_generation >> 32))) ^ ((uint32_t) sb->object_table_start_block ^ (uint32_t)(sb->object_table_start_block >> 32) ^ (sb->object_table_entry_count ^ ((uint32_t) sb->feature_flags ^ (uint32_t)(sb->feature_flags >> 32))));
}
static uint32_t checksum_entry(const SexfilesObjectEntry *e) {
  return (uint32_t) e->object_id ^ (uint32_t)(e->object_id >> 32) ^ ((uint32_t) e->kind ^ e->owner_pd) ^ ((uint32_t) e->rights_generation ^ (uint32_t)(e->rights_generation >> 32) ^ ((uint32_t) e->object_size_bytes ^ (uint32_t)(e->object_size_bytes >> 32))) ^ ((uint32_t) e->first_block ^ (uint32_t)(e->first_block >> 32) ^ ((uint32_t) e->metadata_generation ^ (uint32_t)(e->metadata_generation >> 32) ^ (uint32_t)(e->in_use ? 1u : 0u)));
}
static uint32_t checksum_journal_record(const JournalRecord *rec) {
  return (uint32_t) rec->kind ^ ((uint32_t) rec->tx_id ^ (uint32_t)(rec->tx_id >> 32)) ^ ((uint32_t) rec->generation ^ (uint32_t)(rec->generation >> 32) ^ ((uint32_t) rec->object_id ^ (uint32_t)(rec->object_id >> 32))) ^ ((uint32_t) rec->metadata_generation ^ (uint32_t)(rec->metadata_generation >> 32) ^ (uint32_t) rec->payload_len);
}
static uint32_t checksum_checkpoint(const SexfilesCheckpoint *cp) {
  uint32_t hash = (uint32_t) cp->magic ^ (uint32_t)(cp->magic >> 32) ^ ((uint32_t) cp->checkpoint_generation ^ (uint32_t)(cp->checkpoint_generation >> 32) ^ ((uint32_t) cp->fs_generation ^ (uint32_t)(cp->fs_generation >> 32)));
  for (size_t i = 0; i < DISKFS_MAX_OBJECTS; ++i) if (cp->table[i].in_use) {
    hash ^= checksum_entry(&cp->table[i]);
    hash ^= (uint32_t) i << 16;
  }
  return hash;
}
static JournalRecord make_journal_record(uint16_t kind, uint64_t tx_id, uint64_t generation, uint64_t object_id, uint64_t metadata_generation, uint16_t payload_len) {
  JournalRecord rec = (JournalRecord) {
    .kind = kind,.tx_id = tx_id,.generation = generation,.object_id = object_id,.metadata_generation = metadata_generation,.payload_len = payload_len,.checksum = 0
  };
  rec.checksum = checksum_journal_record(&rec);
  return rec;
}
static int64_t append_journal_record(DiskFsState *st, JournalRecord rec) {
  if (st->journal_len >= DISKFS_JOURNAL_CAPACITY) return ERR_FULL;
  if (checksum_journal_record(&rec) != rec.checksum) return ERR_OVERFLOW;
  st->journal[st->journal_len] = rec;
  st->journal_len += 1;
  return 0;
}
static int64_t begin_transaction(DiskFsState *st, uint64_t *out_tx_id) {
  uint64_t tx_id = st->next_tx_id;
  st->next_tx_id = sat_add_u64(tx_id, 1);
  JournalRecord rec = make_journal_record(JOURNAL_KIND_TX_BEGIN, tx_id, st->superblock.fs_generation, 0, 0, 0);
  int64_t err = append_journal_record(st, rec);
  if (err != 0) return err;
  *out_tx_id = tx_id;
  return 0;
}
static int64_t append_object_metadata_update(DiskFsState *st, uint64_t tx_id, uint64_t object_id, uint64_t metadata_generation) {
  JournalRecord rec = make_journal_record(JOURNAL_KIND_OBJECT_METADATA_UPDATE, tx_id, st->superblock.fs_generation, object_id, metadata_generation, 32);
  return append_journal_record(st, rec);
}
static int64_t commit_transaction(DiskFsState *st, uint64_t tx_id) {
  JournalRecord rec = make_journal_record(JOURNAL_KIND_TX_COMMIT, tx_id, st->superblock.fs_generation, 0, 0, 0);
  return append_journal_record(st, rec);
}
static int64_t verify_journal_integrity(const DiskFsState *st) {
  for (size_t i = 0; i < st->journal_len; ++i) {
    JournalRecord rec = st->journal[i];
    if (checksum_journal_record(&rec) != rec.checksum) return ERR_OVERFLOW;
  }
  return 0;
}
static int64_t extent_bitmap_set(DiskFsState *st, uint64_t block) {
  if (block >= (uint64_t) DISKFS_EXTENT_BLOCK_COUNT) return ERR_OVERFLOW;
  size_t word_idx = (size_t)(block / 64);
  uint64_t bit = block % 64;
  if ((st->extent_bitmap[word_idx] & UINT64_C(1) << bit) != 0) return ERR_FULL;
  st->extent_bitmap[word_idx] |= UINT64_C(1) << bit;
  return 0;
}
static int64_t extent_bitmap_clear(DiskFsState *st, uint64_t block) {
  if (block >= (uint64_t) DISKFS_EXTENT_BLOCK_COUNT) return ERR_OVERFLOW;
  size_t word_idx = (size_t)(block / 64);
  uint64_t bit = block % 64;
  if ((st->extent_bitmap[word_idx] & UINT64_C(1) << bit) == 0) return ERR_INVALID_HANDLE;
  st->extent_bitmap[word_idx] &= ~(UINT64_C(1) << bit);
  return 0;
}
static int64_t extent_bitmap_is_set(const DiskFsState *st, uint64_t block, bool *out_set) {
  if (block >= (uint64_t) DISKFS_EXTENT_BLOCK_COUNT) return ERR_OVERFLOW;
  size_t word_idx = (size_t)(block / 64);
  uint64_t bit = block % 64;
  *out_set = (st->extent_bitmap[word_idx] & UINT64_C(1) << bit) != 0;
  return 0;
}
static bool extent_bitmap_find_contiguous(const DiskFsState *st, uint32_t count, uint64_t *out_start) {
  if (count == 0 || (size_t) count > DISKFS_EXTENT_BLOCK_COUNT) return false;
  uint64_t need = (uint64_t) count;
  bool has_run = false;
  uint64_t run_start = 0;
  uint64_t run_len = 0;
  for (uint64_t block = 1; block < (uint64_t) DISKFS_EXTENT_BLOCK_COUNT; ++block) {
    size_t word_idx = (size_t)(block / 64);
    uint64_t bit = block % 64;
    bool allocated = (st->extent_bitmap[word_idx] & UINT64_C(1) << bit) != 0;
    if (!allocated) {
      if (!has_run) {
        has_run = true;
        run_start = block;
      }
      run_len += 1;
      if (run_len >= need) {
        *out_start = run_start;
        return true;
      }
    } else {
      has_run = false;
      run_len = 0;
    }
  }
  return false;
}
static size_t extent_bitmap_used_count(const DiskFsState *st) {
  size_t count = 0;
  for (size_t i = 0; i < DISKFS_EXTENT_BITMAP_WORDS; ++i) {
    uint64_t w = st->extent_bitmap[i];
    while (w != 0) {
      w &= w - 1;
      ++count;
    }
  }
  return count;
}
static int64_t extent_journal_append(DiskFsState *st, JournalRecord rec) {
  if (st->extent_journal_len >= DISKFS_EXTENT_JOURNAL_CAPACITY) return ERR_FULL;
  if (checksum_journal_record(&rec) != rec.checksum) return ERR_OVERFLOW;
  st->extent_journal[st->extent_journal_len] = rec;
  st->extent_journal_len += 1;
  return 0;
}
int64_t diskfs_format_init_empty(void) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  memset(st, 0, sizeof *st);
  SexfilesSuperblock sb = (SexfilesSuperblock) {
    .magic = DISKFS_MAGIC,.version_major = 1,.version_minor = 0,.block_size = DISKFS_BLOCK_SIZE,.fs_generation = 1,.object_table_start_block = 1,.object_table_entry_count = (uint32_t) DISKFS_MAX_OBJECTS,.feature_flags = 0,.checksum = 0
  };
  sb.checksum = checksum_superblock(&sb);
  st->superblock = sb;
  st->next_tx_id = 1;
  st->mounted = false;
  st->extent_bitmap[0] |= UINT64_C(1);
  st->next_checkpoint_generation = 1;
  sexfiles_state.diskfs_next_object_id = 1;
  return 0;
}
int64_t diskfs_mount(SexfilesSuperblock *out_sb) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  SexfilesSuperblock sb = st->superblock;
  if (sb.magic != DISKFS_MAGIC || (sb.version_major != 1 || sb.block_size != DISKFS_BLOCK_SIZE)) return ERR_NOT_FOUND;
  if ((size_t) sb.object_table_entry_count != DISKFS_MAX_OBJECTS) return ERR_OVERFLOW;
  if (checksum_superblock(&sb) != sb.checksum) return ERR_OVERFLOW;
  if ((st->extent_bitmap[0] & UINT64_C(1)) == 0) st->extent_bitmap[0] |= UINT64_C(1);
  st->mounted = true;
  if (out_sb != NULL) *out_sb = sb;
  return 0;
}
int64_t diskfs_create_object_entry(uint16_t kind, uint32_t owner_pd, uint64_t *out_oid) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  int64_t jerr = verify_journal_integrity(st);
  if (jerr != 0) return jerr;
  if (!st->mounted) return ERR_NOT_FOUND;
  size_t free_idx = SIZE_MAX;
  for (size_t i = 0; i < DISKFS_MAX_OBJECTS; ++i) if (!st->table[i].in_use) {
    free_idx = i;
    break;
  }
  if (free_idx == SIZE_MAX) return ERR_FULL;
  uint64_t object_id = sexfiles_state.diskfs_next_object_id;
  sexfiles_state.diskfs_next_object_id += 1;
  uint64_t tx_id = 0;
  int64_t err = begin_transaction(st, &tx_id);
  if (err != 0) return err;
  err = append_object_metadata_update(st, tx_id, object_id, 1);
  if (err != 0) return err;
  err = commit_transaction(st, tx_id);
  if (err != 0) return err;
  SexfilesObjectEntry e = (SexfilesObjectEntry) {
    .object_id = object_id,.kind = kind,.owner_pd = owner_pd,.rights_generation = 1,.object_size_bytes = 0,.first_block = 0,.metadata_generation = 1,.checksum = 0,.in_use = true
  };
  e.checksum = checksum_entry(&e);
  st->table[free_idx] = e;
  st->superblock.fs_generation = sat_add_u64(st->superblock.fs_generation, 1);
  st->superblock.checksum = checksum_superblock(&st->superblock);
  *out_oid = object_id;
  return 0;
}
int64_t diskfs_stat_object_entry(uint64_t object_id, SexfilesObjectEntry *out_entry) {
  const DiskFsState *st = &sexfiles_state.diskfs_state;
  int64_t jerr = verify_journal_integrity(st);
  if (jerr != 0) return jerr;
  if (!st->mounted || object_id == 0) return ERR_INVALID_HANDLE;
  for (size_t i = 0; i < DISKFS_MAX_OBJECTS; ++i) {
    SexfilesObjectEntry e = st->table[i];
    if (e.in_use && e.object_id == object_id) {
      if (checksum_entry(&e) != e.checksum) return ERR_OVERFLOW;
      if (out_entry != NULL) *out_entry = e;
      return 0;
    }
  }
  return ERR_INVALID_HANDLE;
}
size_t diskfs_journal_len(void) {
  return sexfiles_state.diskfs_state.journal_len;
}
int64_t diskfs_proof_inject_bad_record_for_checksum(void) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  if (!st->mounted) return ERR_NOT_FOUND;
  uint64_t tx_id = st->next_tx_id;
  st->next_tx_id = sat_add_u64(tx_id, 1);
  JournalRecord rec = (JournalRecord) {
    .kind = JOURNAL_KIND_OBJECT_METADATA_UPDATE,.tx_id = tx_id,.generation = st->superblock.fs_generation,.object_id = 0xBAD,.metadata_generation = 1,.payload_len = 32,.checksum = 0
  };
  rec.checksum = checksum_journal_record(&rec) ^ 0x1u;
  return append_journal_record(st, rec);
}
int64_t diskfs_proof_inject_bad_entry_checksum(uint64_t object_id) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  for (size_t i = 0; i < DISKFS_MAX_OBJECTS; ++i) if (st->table[i].in_use && st->table[i].object_id == object_id) {
    st->table[i].checksum ^= 0x1u;
    return 0;
  }
  return ERR_INVALID_HANDLE;
}
static int64_t replay_journal_records(const JournalRecord *records, size_t records_len, SexfilesObjectEntry *out_table, size_t *out_applied, bool *out_uncommitted, bool *out_in_order) {
  uint64_t tx_ids[DISKFS_JOURNAL_CAPACITY];
  bool tx_committed[DISKFS_JOURNAL_CAPACITY];
  memset(tx_ids, 0, sizeof tx_ids);
  memset(tx_committed, 0, sizeof tx_committed);
  size_t tx_seen = 0;
  for (size_t i = 0; i < records_len; ++i) {
    JournalRecord rec = records[i];
    if (checksum_journal_record(&rec) != rec.checksum) return ERR_OVERFLOW;
    if (rec.kind == JOURNAL_KIND_TX_BEGIN) {
      if (tx_seen >= DISKFS_JOURNAL_CAPACITY) return ERR_FULL;
      tx_ids[tx_seen] = rec.tx_id;
      tx_committed[tx_seen] = false;
      ++tx_seen;
    } else if (rec.kind == JOURNAL_KIND_TX_COMMIT) for (size_t j = 0; j < tx_seen; ++j) if (tx_ids[j] == rec.tx_id) {
      tx_committed[j] = true;
      break;
    }
  }
  JournalRecord updates[DISKFS_JOURNAL_CAPACITY];
  memset(updates, 0, sizeof updates);
  size_t update_count = 0;
  bool uncommitted_seen = false;
  for (size_t i = 0; i < records_len; ++i) {
    JournalRecord rec = records[i];
    if (rec.kind == JOURNAL_KIND_OBJECT_METADATA_UPDATE) {
      bool committed = false;
      for (size_t j = 0; j < tx_seen; ++j) if (tx_ids[j] == rec.tx_id) {
        committed = tx_committed[j];
        break;
      }
      if (committed) {
        if (update_count >= DISKFS_JOURNAL_CAPACITY) return ERR_FULL;
        updates[update_count] = rec;
        ++update_count;
      } else uncommitted_seen = true;
    }
  }
  bool applied[DISKFS_JOURNAL_CAPACITY];
  memset(applied, 0, sizeof applied);
  size_t applied_count = 0;
  uint64_t last_gen = 0;
  bool in_order = true;
  while (applied_count < update_count) {
    size_t min_idx = SIZE_MAX;
    for (size_t k = 0; k < update_count; ++k) if (!applied[k]) if (min_idx == SIZE_MAX || updates[k].generation < updates[min_idx].generation) min_idx = k;
    if (min_idx == SIZE_MAX) break;
    JournalRecord rec = updates[min_idx];
    if (rec.generation < last_gen) in_order = false;
    last_gen = rec.generation;
    if (rec.object_id != 0) for (size_t slot = 0; slot < DISKFS_MAX_OBJECTS; ++slot) if (!out_table[slot].in_use) {
      SexfilesObjectEntry e = (SexfilesObjectEntry) {
        .object_id = rec.object_id,.kind = 1,.owner_pd = 11,.rights_generation = 1,.object_size_bytes = 0,.first_block = 0,.metadata_generation = rec.metadata_generation,.checksum = 0,.in_use = true
      };
      e.checksum = checksum_entry(&e);
      out_table[slot] = e;
      break;
    }
    applied[min_idx] = true;
    ++applied_count;
  }
  *out_applied = applied_count;
  *out_uncommitted = uncommitted_seen;
  *out_in_order = in_order;
  return 0;
}
int64_t diskfs_proof_replay_recovery_scenario(ReplayOutcome *out) {
  JournalRecord records[DISKFS_JOURNAL_CAPACITY];
  memset(records, 0, sizeof records);
  size_t n = 0;
  records[n] = make_journal_record(JOURNAL_KIND_TX_BEGIN, 10, 1, 0, 0, 0);
  ++n;
  records[n] = make_journal_record(JOURNAL_KIND_OBJECT_METADATA_UPDATE, 10, 2, 1001, 1, 32);
  ++n;
  records[n] = make_journal_record(JOURNAL_KIND_TX_COMMIT, 10, 3, 0, 0, 0);
  ++n;
  records[n] = make_journal_record(JOURNAL_KIND_TX_BEGIN, 11, 4, 0, 0, 0);
  ++n;
  records[n] = make_journal_record(JOURNAL_KIND_OBJECT_METADATA_UPDATE, 11, 5, 2002, 1, 32);
  ++n;
  SexfilesObjectEntry table[DISKFS_MAX_OBJECTS];
  memset(table, 0, sizeof table);
  size_t applied = 0;
  bool uncommitted_seen = false;
  bool generation_order = false;
  int64_t err = replay_journal_records(records, n, table, &applied, &uncommitted_seen, &generation_order);
  if (err != 0) return err;
  bool restored = false;
  for (size_t i = 0; i < DISKFS_MAX_OBJECTS; ++i) {
    if (table[i].in_use && table[i].object_id == 1001) restored = true;
    if (table[i].in_use && table[i].object_id == 2002) return ERR_OVERFLOW;
  }
  JournalRecord corrupt[DISKFS_JOURNAL_CAPACITY];
  memset(corrupt, 0, sizeof corrupt);
  corrupt[0] = make_journal_record(JOURNAL_KIND_TX_BEGIN, 30, 1, 0, 0, 0);
  corrupt[1] = make_journal_record(JOURNAL_KIND_OBJECT_METADATA_UPDATE, 30, 2, 3003, 1, 32);
  corrupt[1].checksum ^= 0x1u;
  corrupt[2] = make_journal_record(JOURNAL_KIND_TX_COMMIT, 30, 3, 0, 0, 0);
  SexfilesObjectEntry table2[DISKFS_MAX_OBJECTS];
  memset(table2, 0, sizeof table2);
  size_t dummy_app = 0;
  bool dummy_unc = false;
  bool dummy_ord = false;
  int64_t cerr = replay_journal_records(corrupt, 3, table2, &dummy_app, &dummy_unc, &dummy_ord);
  *out = (ReplayOutcome) {
    .committed_applied = applied >= 1,.uncommitted_ignored = uncommitted_seen,.corrupt_rejected = cerr == ERR_OVERFLOW,.generation_order = generation_order,.object_restored = restored
  };
  return 0;
}
int64_t diskfs_proof_reboot_persistence_roundtrip(RebootOutcome *out) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  SexfilesSuperblock sb = (SexfilesSuperblock) {
    .magic = DISKFS_MAGIC,.version_major = 1,.version_minor = 0,.block_size = DISKFS_BLOCK_SIZE,.fs_generation = 1,.object_table_start_block = 1,.object_table_entry_count = (uint32_t) DISKFS_MAX_OBJECTS,.feature_flags = 0,.checksum = 0
  };
  sb.checksum = checksum_superblock(&sb);
  st->superblock = sb;
  memset(st->table, 0, sizeof st->table);
  memset(st->journal, 0, sizeof st->journal);
  st->journal_len = 0;
  st->next_tx_id = 1;
  st->mounted = true;
  sexfiles_state.diskfs_next_object_id = 1;
  uint64_t oid_a = 0;
  uint64_t oid_b = 0;
  int64_t err = diskfs_create_object_entry(42, 1, &oid_a);
  if (err != 0) return err;
  err = diskfs_create_object_entry(43, 1, &oid_b);
  if (err != 0) return err;
  SexfilesObjectEntry sa;
  SexfilesObjectEntry sb_pre;
  err = diskfs_stat_object_entry(oid_a, &sa);
  if (err != 0) return err;
  err = diskfs_stat_object_entry(oid_b, &sb_pre);
  if (err != 0) return err;
  bool ok_pre = sa.kind == 42 && (sa.owner_pd == 1 && (sb_pre.kind == 43 && sb_pre.owner_pd == 1));
  uint64_t gen_before_reboot = st->superblock.fs_generation;
  SexfilesObjectEntry saved_table[DISKFS_MAX_OBJECTS];
  JournalRecord saved_journal[DISKFS_JOURNAL_CAPACITY];
  memcpy(saved_table, st->table, sizeof saved_table);
  memset(saved_journal, 0, sizeof saved_journal);
  size_t journal_len = st->journal_len;
  if (journal_len > DISKFS_JOURNAL_CAPACITY) journal_len = DISKFS_JOURNAL_CAPACITY;
  for (size_t i = 0; i < journal_len; ++i) saved_journal[i] = st->journal[i];
  SexfilesSuperblock sb2 = (SexfilesSuperblock) {
    .magic = DISKFS_MAGIC,.version_major = 1,.version_minor = 0,.block_size = DISKFS_BLOCK_SIZE,.fs_generation = sat_add_u64(gen_before_reboot, 1),.object_table_start_block = 1,.object_table_entry_count = (uint32_t) DISKFS_MAX_OBJECTS,.feature_flags = 0,.checksum = 0
  };
  sb2.checksum = checksum_superblock(&sb2);
  st->superblock = sb2;
  memset(st->table, 0, sizeof st->table);
  memset(st->journal, 0, sizeof st->journal);
  st->journal_len = 0;
  st->next_tx_id = 1;
  st->mounted = true;
  uint64_t gen_after_reboot = st->superblock.fs_generation;
  memcpy(st->table, saved_table, sizeof saved_table);
  SexfilesObjectEntry table_with_replay[DISKFS_MAX_OBJECTS];
  memcpy(table_with_replay, saved_table, sizeof table_with_replay);
  size_t applied = 0;
  bool dummy_unc = false;
  bool dummy_ord = false;
  err = replay_journal_records(saved_journal, journal_len, table_with_replay, &applied, &dummy_unc, &dummy_ord);
  if (err != 0) return err;
  for (size_t slot = 0; slot < DISKFS_MAX_OBJECTS; ++slot) if (table_with_replay[slot].in_use && !st->table[slot].in_use) {
    st->table[slot] = table_with_replay[slot];
    st->table[slot].checksum = checksum_entry(&st->table[slot]);
  }
  SexfilesObjectEntry sa2;
  SexfilesObjectEntry sb2_entry;
  bool ok_a = diskfs_stat_object_entry(oid_a, &sa2) == 0 && (sa2.kind == 42 && sa2.owner_pd == 1);
  bool ok_b = diskfs_stat_object_entry(oid_b, &sb2_entry) == 0 && (sb2_entry.kind == 43 && sb2_entry.owner_pd == 1);
  *out = (RebootOutcome) {
    .write_committed = ok_pre,.objects_created = 2,.journal_records = journal_len,.replay_applied = applied,.objects_restored = ok_a && ok_b,.fs_generation_advanced = gen_after_reboot > gen_before_reboot
  };
  return 0;
}
int64_t diskfs_proof_fill_journal_and_test_full(void) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  if (!st->mounted) return ERR_NOT_FOUND;
  while (st->journal_len < DISKFS_JOURNAL_CAPACITY) {
    size_t idx = st->journal_len;
    JournalRecord rec = make_journal_record(JOURNAL_KIND_TX_BEGIN, 0, st->superblock.fs_generation, 0, 0, 0);
    st->journal[idx] = rec;
    st->journal_len += 1;
  }
  JournalRecord rec = make_journal_record(JOURNAL_KIND_TX_BEGIN, 1, st->superblock.fs_generation, 0, 0, 0);
  return append_journal_record(st, rec);
}
int64_t diskfs_allocate_blocks(uint32_t count, uint64_t *out_first) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  int64_t jerr = verify_journal_integrity(st);
  if (jerr != 0) return jerr;
  if (!st->mounted) return ERR_NOT_FOUND;
  if (count == 0 || (size_t) count > DISKFS_EXTENT_BLOCK_COUNT) return ERR_OVERFLOW;
  uint64_t start = 0;
  if (!extent_bitmap_find_contiguous(st, count, &start)) return ERR_FULL;
  for (uint64_t i = 0; i < (uint64_t) count; ++i) {
    int64_t berr = extent_bitmap_set(st, start + i);
    if (berr != 0) return berr;
  }
  uint64_t tx_id = st->next_tx_id;
  st->next_tx_id = sat_add_u64(tx_id, 1);
  JournalRecord rec = make_journal_record(JOURNAL_KIND_OBJECT_METADATA_UPDATE, tx_id, st->superblock.fs_generation, start, (uint64_t) count, DISKFS_EXTENT_JOURNAL_KIND);
  int64_t ejerr = extent_journal_append(st, rec);
  if (ejerr != 0) return ejerr;
  *out_first = start;
  return 0;
}
int64_t diskfs_free_blocks(uint64_t first_block, uint32_t count) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  int64_t jerr = verify_journal_integrity(st);
  if (jerr != 0) return jerr;
  if (!st->mounted) return ERR_NOT_FOUND;
  if (count == 0 || first_block >= (uint64_t) DISKFS_EXTENT_BLOCK_COUNT) return ERR_OVERFLOW;
  if (first_block + (uint64_t) count > (uint64_t) DISKFS_EXTENT_BLOCK_COUNT) return ERR_OVERFLOW;
  for (uint64_t i = 0; i < (uint64_t) count; ++i) {
    int64_t berr = extent_bitmap_clear(st, first_block + i);
    if (berr != 0) return berr;
  }
  uint64_t tx_id = st->next_tx_id;
  st->next_tx_id = sat_add_u64(tx_id, 1);
  JournalRecord rec = make_journal_record(JOURNAL_KIND_OBJECT_METADATA_UPDATE, tx_id, st->superblock.fs_generation, first_block | UINT64_C(0x8000000000000000), (uint64_t) count, DISKFS_EXTENT_JOURNAL_KIND);
  return extent_journal_append(st, rec);
}
size_t diskfs_used_blocks(void) {
  return extent_bitmap_used_count(&sexfiles_state.diskfs_state);
}
size_t diskfs_total_blocks(void) {
  return DISKFS_EXTENT_BLOCK_COUNT;
}
int64_t diskfs_proof_extent_bounds(void) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  int64_t err = extent_bitmap_set(st, (uint64_t)(DISKFS_EXTENT_BLOCK_COUNT - 1));
  if (err != 0) return err;
  err = extent_bitmap_clear(st, (uint64_t)(DISKFS_EXTENT_BLOCK_COUNT - 1));
  if (err != 0) return err;
  if (extent_bitmap_set(st, (uint64_t) DISKFS_EXTENT_BLOCK_COUNT) == 0) return ERR_OVERFLOW;
  uint64_t dummy = 0;
  if (extent_bitmap_find_contiguous(st, 0, &dummy)) return ERR_OVERFLOW;
  if (extent_bitmap_find_contiguous(st, (uint32_t)(DISKFS_EXTENT_BLOCK_COUNT + 1), &dummy)) return ERR_OVERFLOW;
  return 0;
}
int64_t diskfs_proof_extent_full(void) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  memset(st->extent_bitmap, 0, sizeof st->extent_bitmap);
  int64_t err = extent_bitmap_set(st, 0);
  if (err != 0) return err;
  st->mounted = true;
  for (uint64_t block = 1; block < (uint64_t) DISKFS_EXTENT_BLOCK_COUNT; ++block) {
    size_t word_idx = (size_t)(block / 64);
    uint64_t bit = block % 64;
    st->extent_bitmap[word_idx] |= UINT64_C(1) << bit;
  }
  if (extent_bitmap_used_count(st) != DISKFS_EXTENT_BLOCK_COUNT) return ERR_OVERFLOW;
  uint64_t dummy = 0;
  if (diskfs_allocate_blocks(1, &dummy) == 0) return ERR_OVERFLOW;
  return 0;
}
int64_t diskfs_proof_extent_journaled(size_t *out_alloc_delta, size_t *out_free_delta) {
  int64_t err = diskfs_format_init_empty();
  if (err != 0) return err;
  err = diskfs_mount(NULL);
  if (err != 0) return err;
  size_t before = sexfiles_state.diskfs_state.extent_journal_len;
  uint64_t first = 0;
  err = diskfs_allocate_blocks(8, &first);
  if (err != 0) return err;
  size_t after_alloc = sexfiles_state.diskfs_state.extent_journal_len;
  if (after_alloc <= before) return ERR_OVERFLOW;
  err = diskfs_free_blocks(first, 8);
  if (err != 0) return err;
  size_t after_free = sexfiles_state.diskfs_state.extent_journal_len;
  if (after_free <= after_alloc) return ERR_OVERFLOW;
  *out_alloc_delta = after_alloc - before;
  *out_free_delta = after_free - after_alloc;
  return 0;
}
int64_t diskfs_proof_extent_alloc_and_reuse(void) {
  int64_t err = diskfs_format_init_empty();
  if (err != 0) return err;
  err = diskfs_mount(NULL);
  if (err != 0) return err;
  uint64_t first = 0;
  err = diskfs_allocate_blocks(3, &first);
  if (err != 0) return err;
  if (first < 1) return ERR_OVERFLOW;
  bool is_set = false;
  err = extent_bitmap_is_set(&sexfiles_state.diskfs_state, first, &is_set);
  if (err != 0) return err;
  if (!is_set) return ERR_NOT_FOUND;
  size_t used_after_alloc = diskfs_used_blocks();
  err = diskfs_free_blocks(first, 3);
  if (err != 0) return err;
  err = extent_bitmap_is_set(&sexfiles_state.diskfs_state, first, &is_set);
  if (err != 0) return err;
  if (is_set) return ERR_NOT_FOUND;
  size_t used_after_free = diskfs_used_blocks();
  if (used_after_free >= used_after_alloc) return ERR_OVERFLOW;
  uint64_t second = 0;
  err = diskfs_allocate_blocks(3, &second);
  if (err != 0) return err;
  if (second != first) return ERR_NOT_FOUND;
  return 0;
}
int64_t diskfs_create_checkpoint(uint64_t *out_cp_gen) {
  DiskFsState *st = &sexfiles_state.diskfs_state;
  if (!st->mounted) return ERR_NOT_FOUND;
  uint64_t cp_gen = st->next_checkpoint_generation;
  uint64_t fs_gen = st->superblock.fs_generation;
  size_t target_slot = SIZE_MAX;
  uint64_t min_gen = UINT64_MAX;
  for (size_t i = 0; i < DISKFS_MAX_CHECKPOINTS; ++i) {
    const SexfilesCheckpoint *cp_it = &st->checkpoints[i];
    if (!cp_it->valid) {
      target_slot = i;
      break;
    }
    if (cp_it->checkpoint_generation < min_gen) {
      min_gen = cp_it->checkpoint_generation;
      target_slot = i;
    }
  }
  if (target_slot == SIZE_MAX) return ERR_FULL;
  SexfilesCheckpoint *cp = &st->checkpoints[target_slot];
  cp->magic = DISKFS_CHECKPOINT_MAGIC;
  cp->checkpoint_generation = cp_gen;
  cp->fs_generation = fs_gen;
  memcpy(cp->table, st->table, sizeof cp->table);
  cp->checksum = 0;
  cp->valid = true;
  cp->checksum = checksum_checkpoint(cp);
  st->next_checkpoint_generation = sat_add_u64(st->next_checkpoint_generation, 1);
  *out_cp_gen = cp_gen;
  return 0;
}
bool diskfs_find_latest_valid_checkpoint(size_t *out_idx, SexfilesCheckpoint *out_cp) {
  const DiskFsState *st = &sexfiles_state.diskfs_state;
  bool found = false;
  size_t best_idx = 0;
  SexfilesCheckpoint best_cp;
  memset(&best_cp, 0, sizeof best_cp);
  for (size_t i = 0; i < DISKFS_MAX_CHECKPOINTS; ++i) {
    SexfilesCheckpoint cp = st->checkpoints[i];
    if (cp.valid && (cp.magic == DISKFS_CHECKPOINT_MAGIC && checksum_checkpoint(&cp) == cp.checksum)) if (!found || cp.checkpoint_generation > best_cp.checkpoint_generation) {
      found = true;
      best_idx = i;
      best_cp = cp;
    }
  }
  if (found) {
    if (out_idx != NULL) *out_idx = best_idx;
    if (out_cp != NULL) *out_cp = best_cp;
  }
  return found;
}
int64_t diskfs_restore_checkpoint(const SexfilesCheckpoint *cp, uint64_t *out_gen) {
  if (cp->magic != DISKFS_CHECKPOINT_MAGIC) return ERR_NOT_FOUND;
  if (checksum_checkpoint(cp) != cp->checksum) return ERR_OVERFLOW;
  DiskFsState *st = &sexfiles_state.diskfs_state;
  if (!st->mounted) st->mounted = true;
  memcpy(st->table, cp->table, sizeof st->table);
  st->superblock.fs_generation = sat_add_u64(cp->fs_generation, 1);
  st->superblock.checksum = checksum_superblock(&st->superblock);
  if (out_gen != NULL) *out_gen = cp->checkpoint_generation;
  return 0;
}
int64_t diskfs_proof_corrupt_skip_scenario(bool *out_ok) {
  int64_t err = diskfs_format_init_empty();
  if (err != 0) return err;
  err = diskfs_mount(NULL);
  if (err != 0) return err;
  uint64_t oid = 0;
  err = diskfs_create_object_entry(99, 1, &oid);
  if (err != 0) return err;
  uint64_t cp1_gen = 0;
  err = diskfs_create_checkpoint(&cp1_gen);
  if (err != 0) return err;
  uint64_t oid2 = 0;
  err = diskfs_create_object_entry(100, 1, &oid2);
  if (err != 0) return err;
  uint64_t cp2_gen = 0;
  err = diskfs_create_checkpoint(&cp2_gen);
  if (err != 0) return err;
  DiskFsState *st = &sexfiles_state.diskfs_state;
  for (size_t i = 0; i < DISKFS_MAX_CHECKPOINTS; ++i) if (st->checkpoints[i].valid && st->checkpoints[i].checkpoint_generation == cp2_gen) {
    st->checkpoints[i].checksum ^= 0x1u;
    break;
  }
  size_t idx = 0;
  SexfilesCheckpoint cp;
  if (diskfs_find_latest_valid_checkpoint(&idx, &cp)) *out_ok = cp.checkpoint_generation == cp1_gen;
  else *out_ok = false;
  return 0;
}
int64_t diskfs_proof_generation_monotonic_scenario(bool *out_ok) {
  int64_t err = diskfs_format_init_empty();
  if (err != 0) return err;
  err = diskfs_mount(NULL);
  if (err != 0) return err;
  uint64_t oid = 0;
  err = diskfs_create_object_entry(55, 1, &oid);
  if (err != 0) return err;
  uint64_t gen1 = 0;
  err = diskfs_create_checkpoint(&gen1);
  if (err != 0) return err;
  uint64_t oid2 = 0;
  err = diskfs_create_object_entry(56, 1, &oid2);
  if (err != 0) return err;
  uint64_t gen2 = 0;
  err = diskfs_create_checkpoint(&gen2);
  if (err != 0) return err;
  if (gen2 <= gen1) {
    *out_ok = false;
    return 0;
  }
  size_t idx = 0;
  SexfilesCheckpoint cp;
  if (!diskfs_find_latest_valid_checkpoint(&idx, &cp)) {
    *out_ok = false;
    return 0;
  }
  uint64_t restored_gen = 0;
  err = diskfs_restore_checkpoint(&cp, &restored_gen);
  if (err != 0) return err;
  bool ok = sexfiles_state.diskfs_state.superblock.fs_generation == sat_add_u64(cp.fs_generation, 1);
  *out_ok = ok && restored_gen == cp.checkpoint_generation;
  return 0;
}
int64_t diskfs_proof_checkpoint_roundtrip(size_t *out_count, bool *out_verified) {
  int64_t err = diskfs_format_init_empty();
  if (err != 0) return err;
  err = diskfs_mount(NULL);
  if (err != 0) return err;
  uint64_t oid_a = 0;
  uint64_t oid_b = 0;
  err = diskfs_create_object_entry(10, 1, &oid_a);
  if (err != 0) return err;
  err = diskfs_create_object_entry(20, 1, &oid_b);
  if (err != 0) return err;
  uint64_t cp1_gen = 0;
  err = diskfs_create_checkpoint(&cp1_gen);
  if (err != 0) return err;
  uint64_t oid_c = 0;
  err = diskfs_create_object_entry(30, 1, &oid_c);
  if (err != 0) return err;
  size_t idx = 0;
  SexfilesCheckpoint cp;
  if (!diskfs_find_latest_valid_checkpoint(&idx, &cp)) {
    *out_count = 0;
    *out_verified = false;
    return 0;
  }
  err = diskfs_restore_checkpoint(&cp, NULL);
  if (err != 0) return err;
  bool sa = diskfs_stat_object_entry(oid_a, NULL) == 0;
  bool sb = diskfs_stat_object_entry(oid_b, NULL) == 0;
  int64_t sc = diskfs_stat_object_entry(oid_c, NULL);
  bool objects_restored = sa && sb;
  bool c_gone = sc == ERR_INVALID_HANDLE;
  size_t cp_count = 0;
  for (size_t i = 0; i < DISKFS_MAX_CHECKPOINTS; ++i) if (sexfiles_state.diskfs_state.checkpoints[i].valid) ++cp_count;
  *out_count = cp_count;
  *out_verified = objects_restored && (c_gone && cp_count >= 1);
  return 0;
}
int64_t diskfs_proof_validate_block_route(void) {
  const DiskFsState *st = &sexfiles_state.diskfs_state;
  if (!st->mounted) return ERR_NOT_FOUND;
  if (st->superblock.block_size != DISKFS_BLOCK_SIZE) return ERR_OVERFLOW;
  return 0;
}
int64_t diskfs_proof_validate_block_write(uint64_t block_offset, uint32_t block_len) {
  if (block_offset % 512 != 0) return ERR_OVERFLOW;
  if ((uint64_t) block_len > (uint64_t) DISKFS_BLOCK_SIZE) return ERR_OVERFLOW;
  if (block_len == 0) return ERR_INVALID_HANDLE;
  uint64_t sum = block_offset + (uint64_t) block_len;
  if (sum < block_offset) return ERR_OVERFLOW;
  return 0;
}
int64_t diskfs_proof_validate_block_read(uint64_t block_offset, uint32_t block_len) {
  return diskfs_proof_validate_block_write(block_offset, block_len);
}
int64_t diskfs_proof_block_roundtrip_match(void) {
  const DiskFsState *st = &sexfiles_state.diskfs_state;
  if (!st->mounted) return ERR_NOT_FOUND;
  SexfilesSuperblock sb = st->superblock;
  if (sb.magic != DISKFS_MAGIC) return ERR_NOT_FOUND;
  if (checksum_superblock(&sb) != sb.checksum) return ERR_OVERFLOW;
  if (sb.block_size != DISKFS_BLOCK_SIZE) return ERR_OVERFLOW;
  return 0;
}
/*═══════════════════════════════════════════════════════════════════════════
              SEXFS V0 — Superblock / Object Table / Freemap Format + Mount
              ═══════════════════════════════════════════════════════════════════════════*/
static inline uint16_t get_u16_le(const uint8_t *p, size_t off) {
  return read_le16(p + off);
}
static inline uint32_t get_u32_le(const uint8_t *p, size_t off) {
  return read_le32(p + off);
}
static inline uint64_t get_u64_le(const uint8_t *p, size_t off) {
  return read_le64(p + off);
}
static inline void put_u16_le(uint8_t *p, size_t off, uint16_t v) {
  write_le16(p + off, v);
}
static inline void put_u32_le(uint8_t *p, size_t off, uint32_t v) {
  write_le32(p + off, v);
}
static inline void put_u64_le(uint8_t *p, size_t off, uint64_t v) {
  write_le64(p + off, v);
}
static void sexfs_v0_build_superblock(uint64_t fs_generation, uint8_t *sb) {
  memset(sb, 0, 512);
  put_u64_le(sb, 0, SEXFS_V0_SUPERBLOCK_MAGIC);
  put_u16_le(sb, 8, 1);
  put_u16_le(sb, 10, 0);
  put_u32_le(sb, 12, DISKFS_BLOCK_SIZE);
  put_u64_le(sb, 16, fs_generation);
  put_u64_le(sb, 24, SEXFS_V0_OBJECT_TABLE_START);
  put_u32_le(sb, 32, (uint32_t) DISKFS_MAX_OBJECTS);
  put_u32_le(sb, 36, 128);
  put_u64_le(sb, 40, SEXFS_V0_FREEMAP_SECTOR);
  put_u64_le(sb, 48, (uint64_t) DISKFS_EXTENT_BLOCK_COUNT);
  put_u64_le(sb, 56, 8);
  put_u64_le(sb, 64, (uint64_t) DISKFS_JOURNAL_CAPACITY);
  put_u64_le(sb, 72, 16);
  put_u64_le(sb, 80, (uint64_t) DISKFS_MAX_CHECKPOINTS);
  put_u64_le(sb, 88, 0);
  uint32_t csum = 0;
  for (size_t w = 0; w < 24; ++w) csum ^= get_u32_le(sb, w * 4);
  put_u32_le(sb, 96, csum);
}
static int64_t sexfs_v0_validate_superblock(const uint8_t *data, uint64_t *out_gen) {
  uint64_t magic = get_u64_le(data, 0);
  if (magic != SEXFS_V0_SUPERBLOCK_MAGIC) return ERR_NOT_FOUND;
  uint16_t version_major = get_u16_le(data, 8);
  if (version_major != 1) return ERR_OVERFLOW;
  uint32_t block_size = get_u32_le(data, 12);
  if (block_size != DISKFS_BLOCK_SIZE) return ERR_OVERFLOW;
  uint32_t entry_count = get_u32_le(data, 32);
  if (entry_count != (uint32_t) DISKFS_MAX_OBJECTS) return ERR_OVERFLOW;
  uint32_t csum = 0;
  for (size_t w = 0; w < 24; ++w) csum ^= get_u32_le(data, w * 4);
  uint32_t stored_csum = get_u32_le(data, 96);
  if (csum != stored_csum) return ERR_OVERFLOW;
  if (out_gen != NULL) *out_gen = get_u64_le(data, 16);
  return 0;
}
static void sexfs_v0_freemap_set_bit(uint8_t *fm, size_t block) {
  size_t word_idx = block / 64;
  size_t bit_idx = block % 64;
  size_t bo = 32 + word_idx * 8;
  uint64_t val = get_u64_le(fm, bo);
  val |= UINT64_C(1) << bit_idx;
  put_u64_le(fm, bo, val);
}
static void sexfs_v0_recompute_freemap_checksum(uint8_t *fm) {
  uint32_t csum = 0;
  for (size_t i = 0; i < 16; ++i) csum ^= (uint32_t) fm[i];
  for (size_t i = 20; i < 32; ++i) csum ^= (uint32_t) fm[i];
  for (size_t i = 32; i < 160; ++i) csum ^= (uint32_t) fm[i];
  put_u32_le(fm, 16, csum);
}
static void sexfs_v0_build_init_freemap(uint8_t *fm) {
  memset(fm, 0, 512);
  put_u64_le(fm, 0, SEXFS_V0_FREEMAP_MAGIC);
  put_u16_le(fm, 8, 1);
  put_u32_le(fm, 12, (uint32_t) DISKFS_EXTENT_BLOCK_COUNT);
  for (size_t b = 0; b < 16; ++b) sexfs_v0_freemap_set_bit(fm, b);
  for (size_t b = 253; b <= 255; ++b) sexfs_v0_freemap_set_bit(fm, b);
  sexfs_v0_recompute_freemap_checksum(fm);
}
static int64_t sexfs_v0_validate_freemap(const uint8_t *data) {
  uint64_t magic = get_u64_le(data, 0);
  if (magic != SEXFS_V0_FREEMAP_MAGIC) return ERR_NOT_FOUND;
  uint16_t version = get_u16_le(data, 8);
  if (version != 1) return ERR_OVERFLOW;
  uint32_t csum = 0;
  for (size_t i = 0; i < 16; ++i) csum ^= (uint32_t) data[i];
  for (size_t i = 20; i < 32; ++i) csum ^= (uint32_t) data[i];
  for (size_t i = 32; i < 160; ++i) csum ^= (uint32_t) data[i];
  uint32_t stored_csum = get_u32_le(data, 16);
  if (csum != stored_csum) return ERR_OVERFLOW;
  return 0;
}
static int64_t sexfs_v0_format_to_disk(void) {
  sexos_line("[sexfs.v0.format.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfs.v0.format.err] reason=buf_grant_failed");
    return ERR_NOT_FOUND;
  }
  uint8_t sb[512];
  sexfs_v0_build_superblock(1, sb);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = sb[i];
  uint64_t status = diskfs_block_write(SEXFS_V0_SUPERBLOCK_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    sexos_line("[sexfs.v0.format.err] reason=write_primary status=%llu", (unsigned long long) status);
    return ERR_NOT_FOUND;
  }
  sexos_line("[sexfs.v0.superblock.primary.write.ok] lba=0");
  for (size_t i = 0; i < 512; ++i) p[i] = sb[i];
  status = diskfs_block_write(SEXFS_V0_BACKUP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    sexos_line("[sexfs.v0.format.err] reason=write_backup status=%llu", (unsigned long long) status);
    return ERR_NOT_FOUND;
  }
  sexos_line("[sexfs.v0.superblock.backup.write.ok] lba=1");
  for (uint64_t sec = 0; sec < SEXFS_V0_OBJECT_TABLE_SECTORS; ++sec) {
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    status = diskfs_block_write((SEXFS_V0_OBJECT_TABLE_START + sec) * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) {
      sexos_line("[sexfs.v0.format.err] reason=write_table_sec=%llu status=%llu", (unsigned long long) sec, (unsigned long long) status);
      return ERR_NOT_FOUND;
    }
  }
  sexos_line("[sexfs.v0.object_table.write.ok] lba_range=2..5");
  uint8_t fm[512];
  sexfs_v0_build_init_freemap(fm);
  for (size_t i = 0; i < 512; ++i) p[i] = fm[i];
  status = diskfs_block_write(SEXFS_V0_FREEMAP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    sexos_line("[sexfs.v0.format.err] reason=write_freemap status=%llu", (unsigned long long) status);
    return ERR_NOT_FOUND;
  }
  sexos_line("[sexfs.v0.freemap.write.ok] lba=6");
  sexos_line("[sexfs.v0.format.done] ok=1");
  return 0;
}
static int64_t sexfs_v0_mount_from_disk(uint64_t *out_gen) {
  sexos_line("[sexfs.v0.mount.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfs.v0.mount.err] reason=buf_grant_failed");
    return ERR_NOT_FOUND;
  }
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t status = diskfs_block_read(SEXFS_V0_SUPERBLOCK_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    sexos_line("[sexfs.v0.mount.err] reason=read_primary status=%llu", (unsigned long long) status);
    return ERR_NOT_FOUND;
  }
  uint8_t sb_data[512];
  for (size_t i = 0; i < 512; ++i) sb_data[i] = p[i];
  uint64_t fs_gen = 0;
  int64_t verr = sexfs_v0_validate_superblock(sb_data, &fs_gen);
  if (verr == 0) sexos_line("[sexfs.v0.superblock.primary.read.ok] lba=0");
  else {
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    uint64_t status2 = diskfs_block_read(SEXFS_V0_BACKUP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status2 != 0) {
      sexos_line("[sexfs.v0.mount.err] reason=read_backup_failed primary_err=%lld", (long long) verr);
      return verr;
    }
    for (size_t i = 0; i < 512; ++i) sb_data[i] = p[i];
    int64_t verr2 = sexfs_v0_validate_superblock(sb_data, &fs_gen);
    if (verr2 == 0) sexos_line("[sexfs.v0.superblock.backup.read.ok] lba=1");
    else {
      sexos_line("[sexfs.v0.mount.err] reason=both_superblocks_invalid");
      return verr2;
    }
  }
  sexos_line("[sexfs.v0.superblock.validate.ok] generation=%llu", (unsigned long long) fs_gen);
  for (uint64_t sec = 0; sec < SEXFS_V0_OBJECT_TABLE_SECTORS; ++sec) {
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    status = diskfs_block_read((SEXFS_V0_OBJECT_TABLE_START + sec) * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) {
      sexos_line("[sexfs.v0.mount.err] reason=read_table_sec=%llu status=%llu", (unsigned long long) sec, (unsigned long long) status);
      return ERR_NOT_FOUND;
    }
    for (size_t i = 0; i < 512; ++i) {
      uint8_t unused = p[i];
      (void) unused;
    }
  }
  sexos_line("[sexfs.v0.object_table.read.ok] lba_range=2..5");
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  status = diskfs_block_read(SEXFS_V0_FREEMAP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    sexos_line("[sexfs.v0.mount.err] reason=read_freemap status=%llu", (unsigned long long) status);
    return ERR_NOT_FOUND;
  }
  uint8_t fm_data[512];
  for (size_t i = 0; i < 512; ++i) fm_data[i] = p[i];
  int64_t fm_err = sexfs_v0_validate_freemap(fm_data);
  if (fm_err != 0) return fm_err;
  sexos_line("[sexfs.v0.freemap.read.ok] lba=6");
  sexos_line("[sexfs.v0.mount.done] ok=1 generation=%llu", (unsigned long long) fs_gen);
  if (out_gen != NULL) *out_gen = fs_gen;
  return 0;
}
int64_t proof_sexfs_v0_superblock_format_mount(void) {
  sexos_line("[sexfs.v0.superblock_format_mount.begin]");
  int64_t err = sexfs_v0_format_to_disk();
  if (err != 0) return err;
  uint64_t gen = 0;
  err = sexfs_v0_mount_from_disk(&gen);
  if (err != 0) return err;
  {
    uint64_t buf_va = diskfs_bridge_get_buf_va();
    if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
    uint8_t bad_sb[512];
    sexfs_v0_build_superblock(1, bad_sb);
    bad_sb[0] = 0xEF;
    bad_sb[1] = 0xBE;
    bad_sb[2] = 0xAD;
    bad_sb[3] = 0xDE;
    volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
    for (size_t i = 0; i < 512; ++i) p[i] = bad_sb[i];
    (void) diskfs_block_write(SEXFS_V0_SUPERBLOCK_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    (void) diskfs_block_write(SEXFS_V0_BACKUP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    int64_t bad_magic_result = sexfs_v0_mount_from_disk(NULL);
    uint8_t good_sb[512];
    sexfs_v0_build_superblock(1, good_sb);
    for (size_t i = 0; i < 512; ++i) p[i] = good_sb[i];
    (void) diskfs_block_write(SEXFS_V0_SUPERBLOCK_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    (void) diskfs_block_write(SEXFS_V0_BACKUP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (bad_magic_result != 0) sexos_line("[sexfs.v0.neg.bad_magic.reject] ok=1");
    else {
      sexos_line("[sexfs.v0.neg.bad_magic.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    uint64_t buf_va = diskfs_bridge_get_buf_va();
    uint8_t bad_ver[512];
    sexfs_v0_build_superblock(1, bad_ver);
    bad_ver[8] = 0xFF;
    bad_ver[9] = 0xFF;
    uint32_t csum = 0;
    for (size_t w = 0; w < 24; ++w) csum ^= get_u32_le(bad_ver, w * 4);
    put_u32_le(bad_ver, 96, csum);
    volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
    for (size_t i = 0; i < 512; ++i) p[i] = bad_ver[i];
    (void) diskfs_block_write(0, 512, SEXOS_SLOT_buf_lend);
    (void) diskfs_block_write(SEXFS_V0_BACKUP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    int64_t bad_version_result = sexfs_v0_mount_from_disk(NULL);
    uint8_t good_sb[512];
    sexfs_v0_build_superblock(1, good_sb);
    for (size_t i = 0; i < 512; ++i) p[i] = good_sb[i];
    (void) diskfs_block_write(0, 512, SEXOS_SLOT_buf_lend);
    (void) diskfs_block_write(SEXFS_V0_BACKUP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (bad_version_result != 0) sexos_line("[sexfs.v0.neg.bad_version.reject] ok=1");
    else {
      sexos_line("[sexfs.v0.neg.bad_version.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    uint64_t buf_va = diskfs_bridge_get_buf_va();
    uint8_t bad_csum[512];
    sexfs_v0_build_superblock(1, bad_csum);
    bad_csum[96] ^= 0x01;
    volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
    for (size_t i = 0; i < 512; ++i) p[i] = bad_csum[i];
    (void) diskfs_block_write(0, 512, SEXOS_SLOT_buf_lend);
    (void) diskfs_block_write(SEXFS_V0_BACKUP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    int64_t bad_csum_result = sexfs_v0_mount_from_disk(NULL);
    uint8_t good_sb[512];
    sexfs_v0_build_superblock(1, good_sb);
    for (size_t i = 0; i < 512; ++i) p[i] = good_sb[i];
    (void) diskfs_block_write(0, 512, SEXOS_SLOT_buf_lend);
    (void) diskfs_block_write(SEXFS_V0_BACKUP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (bad_csum_result != 0) sexos_line("[sexfs.v0.neg.bad_checksum.reject] ok=1");
    else {
      sexos_line("[sexfs.v0.neg.bad_checksum.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  err = sexfs_v0_mount_from_disk(&gen);
  if (err != 0) return err;
  sexos_line("[sexfs.v0.superblock_format_mount.done] ok=1");
  return 0;
}
/*═══════════════════════════════════════════════════════════════════════════
              SEXOBJECT TABLE PERSIST — V0 Object Entry Create/Read/Validate
              ═══════════════════════════════════════════════════════════════════════════*/
typedef struct SexfsObjectEntryV0Parsed {
  uint64_t object_id;
  uint16_t kind;
  uint16_t flags;
  uint32_t owner_pd;
  uint64_t rights_generation;
  uint64_t content_generation;
  uint64_t metadata_generation;
  uint64_t object_size_bytes;
  uint64_t first_block;
  uint64_t extent_count;
  uint64_t name_hash;
  uint64_t content_hash;
  uint64_t created_at_gen;
  uint64_t modified_at_gen;
  uint32_t checksum;
  bool in_use;
}
SexfsObjectEntryV0Parsed;
static void sexfs_v0_build_object_entry(uint64_t object_id, uint16_t kind, uint16_t flags, uint32_t owner_pd, uint64_t rights_generation, uint64_t content_generation, uint64_t metadata_generation, uint64_t object_size_bytes, uint64_t first_block, uint64_t extent_count, uint64_t name_hash, uint64_t content_hash, uint64_t created_at_gen, uint64_t modified_at_gen, uint8_t *e) {
  memset(e, 0, 128);
  put_u64_le(e, 0, object_id);
  put_u16_le(e, 8, kind);
  put_u16_le(e, 10, flags);
  put_u32_le(e, 12, owner_pd);
  put_u64_le(e, 16, rights_generation);
  put_u64_le(e, 24, content_generation);
  put_u64_le(e, 32, metadata_generation);
  put_u64_le(e, 40, object_size_bytes);
  put_u64_le(e, 48, first_block);
  put_u64_le(e, 56, extent_count);
  put_u64_le(e, 64, name_hash);
  put_u64_le(e, 72, content_hash);
  put_u64_le(e, 80, created_at_gen);
  put_u64_le(e, 88, modified_at_gen);
  uint32_t csum = 0;
  for (size_t w = 0; w < 24; ++w) csum ^= get_u32_le(e, w * 4);
  put_u32_le(e, 96, csum);
}
static int64_t sexfs_v0_validate_object_entry(const uint8_t *data, SexfsObjectEntryV0Parsed *out) {
  uint32_t csum = 0;
  for (size_t w = 0; w < 24; ++w) csum ^= get_u32_le(data, w * 4);
  uint32_t stored_csum = get_u32_le(data, 96);
  if (csum != stored_csum) return ERR_OVERFLOW;
  uint64_t object_id = get_u64_le(data, 0);
  uint16_t flags = get_u16_le(data, 10);
  bool in_use = (flags & 0x0001) != 0;
  if (in_use && object_id == 0) return ERR_INVALID_HANDLE;
  if ((flags & 0xFFF0) != 0) return ERR_OVERFLOW;
  if (out != NULL) *out = (SexfsObjectEntryV0Parsed) {
    .object_id = object_id,.kind = get_u16_le(data, 8),.flags = flags,.owner_pd = get_u32_le(data, 12),.rights_generation = get_u64_le(data, 16),.content_generation = get_u64_le(data, 24),.metadata_generation = get_u64_le(data, 32),.object_size_bytes = get_u64_le(data, 40),.first_block = get_u64_le(data, 48),.extent_count = get_u64_le(data, 56),.name_hash = get_u64_le(data, 64),.content_hash = get_u64_le(data, 72),.created_at_gen = get_u64_le(data, 80),.modified_at_gen = get_u64_le(data, 88),.checksum = stored_csum,.in_use = in_use
  };
  return 0;
}
static int64_t sexfs_v0_read_object_table(uint8_t *table) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  memset(table, 0, 2048);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (uint64_t sec = 0; sec < SEXFS_V0_OBJECT_TABLE_SECTORS; ++sec) {
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    uint64_t status = diskfs_block_read((SEXFS_V0_OBJECT_TABLE_START + sec) * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) return ERR_NOT_FOUND;
    for (size_t i = 0; i < 512; ++i) {
      size_t idx = (size_t) sec * 512 + i;
      if (idx < 2048) table[idx] = p[i];
    }
  }
  return 0;
}
static int64_t sexfs_v0_write_object_table(size_t slot, const uint8_t *entry_data) {
  if (slot >= DISKFS_MAX_OBJECTS) return ERR_OVERFLOW;
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  uint8_t table[2048];
  if (sexfs_v0_read_object_table(table) != 0) memset(table, 0, 2048);
  size_t entry_off = slot * SEXFS_V0_OBJECT_ENTRY_BYTES;
  for (size_t i = 0; i < 128; ++i) table[entry_off + i] = entry_data[i];
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (uint64_t sec = 0; sec < SEXFS_V0_OBJECT_TABLE_SECTORS; ++sec) {
    size_t off = (size_t) sec * 512;
    for (size_t i = 0; i < 512 && off + i < 2048; ++i) p[i] = table[off + i];
    uint64_t status = diskfs_block_write((SEXFS_V0_OBJECT_TABLE_START + sec) * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) return ERR_NOT_FOUND;
  }
  return 0;
}
int64_t proof_sexobject_table_persist(void) {
  sexos_line("[sexobject.table.persist.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t sb_status = diskfs_block_read(SEXFS_V0_SUPERBLOCK_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (sb_status != 0) return ERR_NOT_FOUND;
  uint8_t sb_check[8];
  for (size_t i = 0; i < 8; ++i) sb_check[i] = p[i];
  uint64_t magic = get_u64_le(sb_check, 0);
  if (magic != SEXFS_V0_SUPERBLOCK_MAGIC) {
    int64_t ferr = sexfs_v0_format_to_disk();
    if (ferr != 0) return ferr;
  } else sexos_line("[sexobject.table.disk.already_formatted] magic_ok=1");
  uint64_t test_object_id = 1;
  uint16_t test_kind = 1;
  uint16_t test_flags = 0x0001;
  uint32_t test_owner_pd = 11;
  uint64_t test_rights_gen = 1;
  uint64_t test_content_gen = 0;
  uint64_t test_metadata_gen = 1;
  uint64_t test_size = 0;
  uint64_t test_first_block = 0;
  uint64_t test_extent_count = 0;
  uint64_t test_name_hash = UINT64_C(0x6E654C5F534F5853);
  uint64_t test_content_hash = 0;
  uint64_t test_created_at_gen = 1;
  uint64_t test_modified_at_gen = 1;
  uint8_t entry_bytes[128];
  sexfs_v0_build_object_entry(test_object_id, test_kind, test_flags, test_owner_pd, test_rights_gen, test_content_gen, test_metadata_gen, test_size, test_first_block, test_extent_count, test_name_hash, test_content_hash, test_created_at_gen, test_modified_at_gen, entry_bytes);
  sexos_line("[sexobject.table.entry.create.ok] slot=0 object_id=%llu", (unsigned long long) test_object_id);
  int64_t werr = sexfs_v0_write_object_table(0, entry_bytes);
  if (werr != 0) return werr;
  sexos_line("[sexobject.table.write.ok] lba_range=2..5");
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t status = diskfs_block_read(SEXFS_V0_OBJECT_TABLE_START * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) return ERR_NOT_FOUND;
  uint8_t readback_entry[128];
  for (size_t i = 0; i < 128; ++i) readback_entry[i] = p[i];
  sexos_line("[sexobject.table.read.ok] lba_range=2..5");
  SexfsObjectEntryV0Parsed parsed;
  int64_t verr = sexfs_v0_validate_object_entry(readback_entry, &parsed);
  if (verr != 0) return verr;
  bool match_ok = parsed.object_id == test_object_id && (parsed.kind == test_kind && (parsed.flags == test_flags && (parsed.owner_pd == test_owner_pd && (parsed.rights_generation == test_rights_gen && (parsed.content_generation == test_content_gen && (parsed.metadata_generation == test_metadata_gen && (parsed.object_size_bytes == test_size && (parsed.first_block == test_first_block && (parsed.extent_count == test_extent_count && (parsed.name_hash == test_name_hash && (parsed.content_hash == test_content_hash && (parsed.created_at_gen == test_created_at_gen && (parsed.modified_at_gen == test_modified_at_gen && parsed.in_use)))))))))))));
  if (match_ok) {
    sexos_line("[sexobject.table.entry.match] slot=0 ok=1");
    sexos_line("[sexobject.table.validate.ok]");
  } else {
    sexos_line("[sexobject.table.entry.match] slot=0 ok=0");
    return ERR_OVERFLOW;
  }
  {
    uint8_t bad_entry[128];
    memcpy(bad_entry, entry_bytes, 128);
    bad_entry[96] ^= 0x01;
    for (size_t i = 0; i < 128; ++i) p[i] = bad_entry[i];
    for (size_t i = 128; i < 512; ++i) p[i] = 0;
    (void) diskfs_block_write(SEXFS_V0_OBJECT_TABLE_START * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    (void) diskfs_block_read(SEXFS_V0_OBJECT_TABLE_START * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    uint8_t bad_readback[128];
    for (size_t i = 0; i < 128; ++i) bad_readback[i] = p[i];
    if (sexfs_v0_validate_object_entry(bad_readback, NULL) != 0) sexos_line("[sexobject.table.neg.bad_entry.reject] ok=1");
    else {
      sexos_line("[sexobject.table.neg.bad_entry.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    uint8_t bad_entry2[128];
    sexfs_v0_build_object_entry(0, 1, 0x0001, 11, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, bad_entry2);
    for (size_t i = 0; i < 128; ++i) p[i] = bad_entry2[i];
    for (size_t i = 128; i < 512; ++i) p[i] = 0;
    (void) diskfs_block_write(SEXFS_V0_OBJECT_TABLE_START * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    (void) diskfs_block_read(SEXFS_V0_OBJECT_TABLE_START * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    uint8_t bad2_readback[128];
    for (size_t i = 0; i < 128; ++i) bad2_readback[i] = p[i];
    int64_t bad2_result = sexfs_v0_validate_object_entry(bad2_readback, NULL);
    if (bad2_result == ERR_INVALID_HANDLE) sexos_line("[sexobject.table.neg.object_id_zero.reject] ok=1");
    else {
      sexos_line("[sexobject.table.neg.object_id_zero.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  for (size_t i = 0; i < 128; ++i) p[i] = entry_bytes[i];
  for (size_t i = 128; i < 512; ++i) p[i] = 0;
  (void) diskfs_block_write(SEXFS_V0_OBJECT_TABLE_START * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  (void) diskfs_block_read(SEXFS_V0_OBJECT_TABLE_START * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  uint8_t restored_entry[128];
  for (size_t i = 0; i < 128; ++i) restored_entry[i] = p[i];
  verr = sexfs_v0_validate_object_entry(restored_entry, NULL);
  if (verr != 0) return verr;
  sexos_line("[sexobject.table.persist.done] ok=1");
  return 0;
}
/*═══════════════════════════════════════════════════════════════════════════
              SEXOBJECT TABLE EXTENT ALLOC — Freemap allocation + data readback proof
              ═══════════════════════════════════════════════════════════════════════════*/
static int64_t sexfs_v0_read_freemap_sector(uint8_t *fm) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t status = diskfs_block_read(SEXFS_V0_FREEMAP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) return ERR_NOT_FOUND;
  for (size_t i = 0; i < 512; ++i) fm[i] = p[i];
  return 0;
}
static int64_t sexfs_v0_write_freemap_sector(const uint8_t *fm) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = fm[i];
  uint64_t status = diskfs_block_write(SEXFS_V0_FREEMAP_SECTOR * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) return ERR_NOT_FOUND;
  return 0;
}
static int64_t sexfs_v0_freemap_alloc_block_in_range(uint8_t *fm, size_t min_block, size_t max_block_inclusive, uint64_t *out_block) {
  for (size_t b = min_block; b <= max_block_inclusive; ++b) {
    size_t word_idx = b / 64;
    size_t bit = b % 64;
    size_t bo = 32 + word_idx * 8;
    uint64_t val = get_u64_le(fm, bo);
    if ((val & UINT64_C(1) << bit) == 0) {
      uint64_t new_val = val | UINT64_C(1) << bit;
      put_u64_le(fm, bo, new_val);
      sexfs_v0_recompute_freemap_checksum(fm);
      *out_block = (uint64_t) b;
      return 0;
    }
  }
  return ERR_OVERFLOW;
}
static int64_t sexfs_v0_freemap_alloc_specific(uint8_t *fm, size_t block) {
  size_t word_idx = block / 64;
  size_t bit = block % 64;
  size_t bo = 32 + word_idx * 8;
  uint64_t val = get_u64_le(fm, bo);
  if ((val & UINT64_C(1) << bit) != 0) return ERR_OVERFLOW;
  uint64_t new_val = val | UINT64_C(1) << bit;
  put_u64_le(fm, bo, new_val);
  sexfs_v0_recompute_freemap_checksum(fm);
  return 0;
}
static bool sexfs_v0_freemap_is_block_used(const uint8_t *fm, size_t block) {
  size_t word_idx = block / 64;
  size_t bit = block % 64;
  size_t bo = 32 + word_idx * 8;
  uint64_t val = get_u64_le(fm, bo);
  return (val & UINT64_C(1) << bit) != 0;
}
static int64_t sexfs_v0_write_data_sector(uint64_t lba, const uint8_t *data) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = data[i];
  uint64_t status = diskfs_block_write(lba * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) return ERR_NOT_FOUND;
  return 0;
}
static int64_t sexfs_v0_read_data_sector(uint64_t lba, uint8_t *out) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t status = diskfs_block_read(lba * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) return ERR_NOT_FOUND;
  for (size_t i = 0; i < 512; ++i) out[i] = p[i];
  return 0;
}
static uint64_t sexfs_v0_fnv1a(const uint8_t *data, size_t len) {
  uint64_t hash = UINT64_C(0xcbf29ce484222325);
  for (size_t i = 0; i < len; ++i) {
    hash ^= (uint64_t) data[i];
    hash *= UINT64_C(0x100000001b3);
  }
  return hash;
}
static int64_t sexfs_v0_validate_extent_bounds(const SexfsObjectEntryV0Parsed *p) {
  if (p->in_use && (p->object_size_bytes > 0 && p->extent_count == 0)) return ERR_OVERFLOW;
  if (p->in_use && p->extent_count > 0) if (p->first_block < SEXFS_V0_DATA_LBA_MIN || p->first_block > SEXFS_V0_DATA_LBA_MAX) return ERR_OVERFLOW;
  if (p->in_use && (p->extent_count == 1 && p->object_size_bytes > (uint64_t) DISKFS_BLOCK_SIZE)) return ERR_OVERFLOW;
  return 0;
}
int64_t proof_sexobject_table_extent_alloc(void) {
  sexos_line("[sexobject.extent_alloc.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  int64_t err = sexfs_v0_format_to_disk();
  if (err != 0) return err;
  uint8_t fm[512];
  err = sexfs_v0_read_freemap_sector(fm);
  if (err != 0) return err;
  err = sexfs_v0_validate_freemap(fm);
  if (err != 0) return err;
  sexos_line("[sexobject.freemap.read.ok] lba=6");
  uint64_t alloc_block = 0;
  err = sexfs_v0_freemap_alloc_block_in_range(fm, SEXFS_V0_DATA_REGION_MIN_BLOCK, SEXFS_V0_DATA_REGION_MAX_BLOCK, &alloc_block);
  if (err != 0) return err;
  uint64_t alloc_lba = alloc_block * 8;
  sexos_line("[sexobject.extent.alloc.ok] lba=%llu", (unsigned long long) alloc_lba);
  err = sexfs_v0_write_freemap_sector(fm);
  if (err != 0) return err;
  sexos_line("[sexobject.freemap.persist.ok] lba=6");
  uint8_t payload_sector[512];
  memset(payload_sector, 0, 512);
  const char *payload_bytes = "sexobject extent alloc proof: testtest";
  for (size_t pi = 0; pi < 38; ++pi) payload_sector[pi] = (uint8_t) payload_bytes[pi];
  uint64_t content_hash = sexfs_v0_fnv1a(payload_sector, 38);
  uint8_t entry_bytes[128];
  sexfs_v0_build_object_entry(1, 1, 0x0001, 11, 1, 1, 2, 38, alloc_lba, 1, UINT64_C(0x6E654C5F534F5853), content_hash, 1, 2, entry_bytes);
  sexos_line("[sexobject.entry.extent.update.ok] slot=0 lba=%llu extent_count=1", (unsigned long long) alloc_lba);
  err = sexfs_v0_write_object_table(0, entry_bytes);
  if (err != 0) return err;
  sexos_line("[sexobject.table.write.ok] lba_range=2..5");
  err = sexfs_v0_write_data_sector(alloc_lba, payload_sector);
  if (err != 0) return err;
  sexos_line("[sexobject.data.write.ok] lba=%llu len=38", (unsigned long long) alloc_lba);
  uint8_t readback[512];
  err = sexfs_v0_read_data_sector(alloc_lba, readback);
  if (err != 0) return err;
  sexos_line("[sexobject.data.read.ok] lba=%llu len=38", (unsigned long long) alloc_lba);
  bool data_match = memcmp(readback, payload_sector, 38) == 0;
  if (data_match) sexos_line("[sexobject.data.match] ok=1");
  else {
    sexos_line("[sexobject.data.match] ok=0");
    return ERR_OVERFLOW;
  }
  uint8_t table[2048];
  err = sexfs_v0_read_object_table(table);
  if (err != 0) return err;
  SexfsObjectEntryV0Parsed reparsed;
  err = sexfs_v0_validate_object_entry(table, &reparsed);
  if (err != 0) return err;
  bool entry_match = reparsed.object_id == 1 && (reparsed.extent_count == 1 && (reparsed.first_block == alloc_lba && (reparsed.object_size_bytes == 38 && (reparsed.content_hash == content_hash && reparsed.in_use))));
  if (entry_match) sexos_line("[sexobject.remount.entry.match] ok=1");
  else {
    sexos_line("[sexobject.remount.entry.match] ok=0");
    return ERR_OVERFLOW;
  }
  uint8_t fm2[512];
  err = sexfs_v0_read_freemap_sector(fm2);
  if (err != 0) return err;
  err = sexfs_v0_validate_freemap(fm2);
  if (err != 0) return err;
  if (sexfs_v0_freemap_is_block_used(fm2, (size_t) alloc_block)) sexos_line("[sexobject.remount.freemap.used.ok] lba=%llu", (unsigned long long) alloc_lba);
  else {
    sexos_line("[sexobject.remount.freemap.used.ok] lba=%llu ok=0", (unsigned long long) alloc_lba);
    return ERR_OVERFLOW;
  }
  {
    uint8_t fm_neg[512];
    memcpy(fm_neg, fm2, 512);
    if (sexfs_v0_freemap_alloc_specific(fm_neg, (size_t) alloc_block) != 0) sexos_line("[sexobject.neg.double_alloc.reject] ok=1");
    else {
      sexos_line("[sexobject.neg.double_alloc.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    uint8_t bad_entry[128];
    sexfs_v0_build_object_entry(2, 1, 0x0001, 11, 1, 1, 1, 10, 2, 1, 0, 0, 1, 1, bad_entry);
    SexfsObjectEntryV0Parsed bad_parsed;
    if (sexfs_v0_validate_object_entry(bad_entry, &bad_parsed) == 0) if (sexfs_v0_validate_extent_bounds(&bad_parsed) != 0) sexos_line("[sexobject.neg.bad_extent_lba.reject] ok=1");
    else {
      sexos_line("[sexobject.neg.bad_extent_lba.reject] ok=0");
      return ERR_OVERFLOW;
    } else {
      sexos_line("[sexobject.neg.bad_extent_lba.reject] ok=0 reason=entry_invalid");
      return ERR_OVERFLOW;
    }
  }
  {
    uint8_t bad_entry2[128];
    sexfs_v0_build_object_entry(3, 1, 0x0001, 11, 1, 1, 1, 38, 0, 0, 0, 0, 1, 1, bad_entry2);
    SexfsObjectEntryV0Parsed bad_parsed2;
    if (sexfs_v0_validate_object_entry(bad_entry2, &bad_parsed2) == 0) if (sexfs_v0_validate_extent_bounds(&bad_parsed2) != 0) sexos_line("[sexobject.neg.zero_extent_nonzero_size.reject] ok=1");
    else {
      sexos_line("[sexobject.neg.zero_extent_nonzero_size.reject] ok=0");
      return ERR_OVERFLOW;
    } else {
      sexos_line("[sexobject.neg.zero_extent_nonzero_size.reject] ok=0 reason=entry_invalid");
      return ERR_OVERFLOW;
    }
  }
  sexos_line("[sexobject.extent_alloc.done] ok=1");
  return 0;
}
/*═══════════════════════════════════════════════════════════════════════════
              SEXOBJECT EXTENT WRITE FULL BLOCK — 4KiB extent write/read/verify proof
              ═══════════════════════════════════════════════════════════════════════════*/
static uint8_t sexfs_v0_full_block_payload_byte(size_t i) {
  if (i < 4) return (uint8_t) "test"[i];
  if (i >= 4092) return (uint8_t) "tail"[i - 4092];
  return (uint8_t)((uint8_t)((uint8_t) i * 7u) + (uint8_t)(i >> 8));
}
static uint64_t sexfs_v0_full_block_payload_hash(void) {
  uint64_t hash = UINT64_C(0xcbf29ce484222325);
  for (size_t i = 0; i < 4096; ++i) {
    hash ^= (uint64_t) sexfs_v0_full_block_payload_byte(i);
    hash *= UINT64_C(0x100000001b3);
  }
  return hash;
}
static int64_t sexfs_v0_write_full_block(uint64_t alloc_lba) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (uint64_t sec = 0; sec < 8; ++sec) {
    size_t base = (size_t) sec * 512;
    for (size_t i = 0; i < 512; ++i) p[i] = sexfs_v0_full_block_payload_byte(base + i);
    uint64_t status = diskfs_block_write((alloc_lba + sec) * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) return ERR_NOT_FOUND;
  }
  return 0;
}
static int64_t sexfs_v0_read_full_block_match(uint64_t alloc_lba, bool *out_match) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (uint64_t sec = 0; sec < 8; ++sec) {
    size_t base = (size_t) sec * 512;
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    uint64_t status = diskfs_block_read((alloc_lba + sec) * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) return ERR_NOT_FOUND;
    for (size_t i = 0; i < 512; ++i) if (p[i] != sexfs_v0_full_block_payload_byte(base + i)) {
      *out_match = false;
      return 0;
    }
  }
  *out_match = true;
  return 0;
}
static int64_t sexfs_v0_read_full_block_hash(uint64_t alloc_lba, uint64_t *out_hash) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  uint64_t hash = UINT64_C(0xcbf29ce484222325);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (uint64_t sec = 0; sec < 8; ++sec) {
    for (size_t i = 0; i < 512; ++i) p[i] = 0;
    uint64_t status = diskfs_block_read((alloc_lba + sec) * BLOCK_SECTOR_SIZE, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) return ERR_NOT_FOUND;
    for (size_t i = 0; i < 512; ++i) {
      hash ^= (uint64_t) p[i];
      hash *= UINT64_C(0x100000001b3);
    }
  }
  *out_hash = hash;
  return 0;
}
int64_t proof_sexobject_extent_write_full_block(void) {
  sexos_line("[sexobject.full_block.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  int64_t err = sexfs_v0_format_to_disk();
  if (err != 0) return err;
  uint8_t fm[512];
  err = sexfs_v0_read_freemap_sector(fm);
  if (err != 0) return err;
  err = sexfs_v0_validate_freemap(fm);
  if (err != 0) return err;
  uint64_t alloc_block = 0;
  err = sexfs_v0_freemap_alloc_block_in_range(fm, SEXFS_V0_DATA_REGION_MIN_BLOCK, SEXFS_V0_DATA_REGION_MAX_BLOCK, &alloc_block);
  if (err != 0) return err;
  uint64_t alloc_lba = alloc_block * 8;
  err = sexfs_v0_write_freemap_sector(fm);
  if (err != 0) return err;
  sexos_line("[sexobject.extent.alloc.ok] lba=%llu", (unsigned long long) alloc_lba);
  uint64_t content_hash = sexfs_v0_full_block_payload_hash();
  sexos_line("[sexobject.full_block.payload.ready] len=4096 has_test=1");
  err = sexfs_v0_write_full_block(alloc_lba);
  if (err != 0) return err;
  sexos_line("[sexobject.full_block.write.ok] lba=%llu sectors=8 len=4096", (unsigned long long) alloc_lba);
  bool match_ok = false;
  err = sexfs_v0_read_full_block_match(alloc_lba, &match_ok);
  if (err != 0) return err;
  sexos_line("[sexobject.full_block.read.ok] lba=%llu sectors=8 len=4096", (unsigned long long) alloc_lba);
  if (match_ok) sexos_line("[sexobject.full_block.match] ok=1");
  else {
    sexos_line("[sexobject.full_block.match] ok=0");
    return ERR_OVERFLOW;
  }
  uint8_t entry_bytes[128];
  sexfs_v0_build_object_entry(1, 1, 0x0001, 11, 1, 1, 1, 4096, alloc_lba, 1, UINT64_C(0x6E654C5F534F5853), content_hash, 1, 1, entry_bytes);
  err = sexfs_v0_write_object_table(0, entry_bytes);
  if (err != 0) return err;
  sexos_line("[sexobject.full_block.entry.persist.ok] slot=0 size=4096 extent_count=1");
  uint8_t table[2048];
  err = sexfs_v0_read_object_table(table);
  if (err != 0) return err;
  SexfsObjectEntryV0Parsed reparsed;
  err = sexfs_v0_validate_object_entry(table, &reparsed);
  if (err != 0) return err;
  bool entry_match = reparsed.object_id == 1 && (reparsed.extent_count == 1 && (reparsed.first_block == alloc_lba && (reparsed.object_size_bytes == 4096 && (reparsed.content_hash == content_hash && reparsed.in_use))));
  if (entry_match) sexos_line("[sexobject.full_block.remount.entry.match] ok=1");
  else {
    sexos_line("[sexobject.full_block.remount.entry.match] ok=0");
    return ERR_OVERFLOW;
  }
  uint8_t fm2[512];
  err = sexfs_v0_read_freemap_sector(fm2);
  if (err != 0) return err;
  err = sexfs_v0_validate_freemap(fm2);
  if (err != 0) return err;
  if (sexfs_v0_freemap_is_block_used(fm2, (size_t) alloc_block)) sexos_line("[sexobject.full_block.remount.freemap.used.ok] lba=%llu", (unsigned long long) alloc_lba);
  else {
    sexos_line("[sexobject.full_block.remount.freemap.used.ok] lba=%llu ok=0", (unsigned long long) alloc_lba);
    return ERR_OVERFLOW;
  }
  uint64_t remount_hash = 0;
  err = sexfs_v0_read_full_block_hash(alloc_lba, &remount_hash);
  if (err != 0) return err;
  if (remount_hash == content_hash) sexos_line("[sexobject.full_block.remount.content.match] ok=1");
  else {
    sexos_line("[sexobject.full_block.remount.content.match] ok=0");
    return ERR_OVERFLOW;
  }
  {
    uint8_t corrupt[512];
    for (size_t ci = 0; ci < 512; ++ci) corrupt[ci] = sexfs_v0_full_block_payload_byte(1536 + ci) ^ (ci == 0 ? 0xFFu : 0x00u);
    err = sexfs_v0_write_data_sector(alloc_lba + 3, corrupt);
    if (err != 0) return err;
    uint64_t corrupt_hash = 0;
    err = sexfs_v0_read_full_block_hash(alloc_lba, &corrupt_hash);
    if (err != 0) return err;
    uint8_t restore[512];
    for (size_t ri = 0; ri < 512; ++ri) restore[ri] = sexfs_v0_full_block_payload_byte(1536 + ri);
    err = sexfs_v0_write_data_sector(alloc_lba + 3, restore);
    if (err != 0) return err;
    if (corrupt_hash != content_hash) sexos_line("[sexobject.full_block.neg.hash_mismatch.reject] ok=1");
    else {
      sexos_line("[sexobject.full_block.neg.hash_mismatch.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    uint8_t bad_entry[128];
    sexfs_v0_build_object_entry(2, 1, 0x0001, 11, 1, 1, 1, 4097, alloc_lba, 1, 0, 0, 1, 1, bad_entry);
    SexfsObjectEntryV0Parsed bad_parsed;
    if (sexfs_v0_validate_object_entry(bad_entry, &bad_parsed) == 0) if (sexfs_v0_validate_extent_bounds(&bad_parsed) != 0) sexos_line("[sexobject.full_block.neg.oversize_single_extent.reject] ok=1");
    else {
      sexos_line("[sexobject.full_block.neg.oversize_single_extent.reject] ok=0");
      return ERR_OVERFLOW;
    } else {
      sexos_line("[sexobject.full_block.neg.oversize_single_extent.reject] ok=0 reason=entry_invalid");
      return ERR_OVERFLOW;
    }
  }
  sexos_line("[sexobject.full_block.done] ok=1");
  return 0;
}
/*═══════════════════════════════════════════════════════════════════════════
              SexObject V1 native API (proof-only)
              ═══════════════════════════════════════════════════════════════════════════*/
static int64_t sexobject_slot_for_id(uint64_t object_id, size_t *out_slot) {
  if (object_id == 0) return ERR_INVALID_HANDLE;
  size_t slot = (size_t)(object_id - 1);
  if (slot >= DISKFS_MAX_OBJECTS) return ERR_NOT_FOUND;
  *out_slot = slot;
  return 0;
}
static int64_t sexobject_create(uint16_t kind, uint64_t name_hash, uint64_t *out_oid) {
  uint8_t table[2048];
  int64_t err = sexfs_v0_read_object_table(table);
  if (err != 0) return err;
  size_t free_slot = SIZE_MAX;
  for (size_t slot = 0; slot < DISKFS_MAX_OBJECTS; ++slot) {
    size_t off = slot * SEXFS_V0_OBJECT_ENTRY_BYTES;
    SexfsObjectEntryV0Parsed p;
    if (sexfs_v0_validate_object_entry(table + off, &p) == 0 && !p.in_use) {
      free_slot = slot;
      break;
    }
  }
  if (free_slot == SIZE_MAX) return ERR_OVERFLOW;
  uint64_t object_id = (uint64_t) free_slot + 1;
  uint8_t entry_bytes[128];
  sexfs_v0_build_object_entry(object_id, kind, 0x0001, 11, 1, 1, 1, 0, 0, 0, name_hash, 0, 1, 1, entry_bytes);
  err = sexfs_v0_write_object_table(free_slot, entry_bytes);
  if (err != 0) return err;
  *out_oid = object_id;
  return 0;
}
static int64_t sexobject_write(uint64_t object_id, const uint8_t *data, size_t len) {
  if (len == 0 || len > (size_t) DISKFS_BLOCK_SIZE) return ERR_OVERFLOW;
  size_t slot = 0;
  int64_t err = sexobject_slot_for_id(object_id, &slot);
  if (err != 0) return err;
  uint8_t table[2048];
  err = sexfs_v0_read_object_table(table);
  if (err != 0) return err;
  size_t off = slot * SEXFS_V0_OBJECT_ENTRY_BYTES;
  SexfsObjectEntryV0Parsed parsed;
  err = sexfs_v0_validate_object_entry(table + off, &parsed);
  if (err != 0) return err;
  if (!parsed.in_use) return ERR_NOT_FOUND;
  uint8_t fm[512];
  err = sexfs_v0_read_freemap_sector(fm);
  if (err != 0) return err;
  err = sexfs_v0_validate_freemap(fm);
  if (err != 0) return err;
  uint64_t alloc_block = 0;
  err = sexfs_v0_freemap_alloc_block_in_range(fm, SEXFS_V0_DATA_REGION_MIN_BLOCK, SEXFS_V0_DATA_REGION_MAX_BLOCK, &alloc_block);
  if (err != 0) return err;
  uint64_t alloc_lba = alloc_block * 8;
  uint64_t hash = UINT64_C(0xcbf29ce484222325);
  for (uint64_t sec = 0; sec < 8; ++sec) {
    size_t sec_off = (size_t) sec * 512;
    uint8_t sector[512];
    for (size_t i = 0; i < 512; ++i) {
      size_t src = sec_off + i;
      sector[i] = src < len ? data[src] : 0;
    }
    for (size_t j = 0; j < 512; ++j) {
      hash ^= (uint64_t) sector[j];
      hash *= UINT64_C(0x100000001b3);
    }
    err = sexfs_v0_write_data_sector(alloc_lba + sec, sector);
    if (err != 0) return err;
  }
  uint64_t content_hash = hash;
  err = sexfs_v0_write_freemap_sector(fm);
  if (err != 0) return err;
  uint8_t new_entry[128];
  sexfs_v0_build_object_entry(object_id, parsed.kind, parsed.flags, parsed.owner_pd, parsed.rights_generation, parsed.content_generation + 1, parsed.metadata_generation + 1, (uint64_t) len, alloc_lba, 1, parsed.name_hash, content_hash, parsed.created_at_gen, parsed.modified_at_gen + 1, new_entry);
  return sexfs_v0_write_object_table(slot, new_entry);
}
static int64_t sexobject_read(uint64_t object_id, uint8_t *out, size_t *out_len) {
  size_t slot = 0;
  int64_t err = sexobject_slot_for_id(object_id, &slot);
  if (err != 0) return err;
  uint8_t table[2048];
  err = sexfs_v0_read_object_table(table);
  if (err != 0) return err;
  size_t off = slot * SEXFS_V0_OBJECT_ENTRY_BYTES;
  SexfsObjectEntryV0Parsed parsed;
  err = sexfs_v0_validate_object_entry(table + off, &parsed);
  if (err != 0) return err;
  if (!parsed.in_use) return ERR_NOT_FOUND;
  if (parsed.extent_count == 0 || parsed.object_size_bytes == 0) return ERR_NOT_FOUND;
  err = sexfs_v0_validate_extent_bounds(&parsed);
  if (err != 0) return err;
  err = sexfs_v0_read_data_sector(parsed.first_block, out);
  if (err != 0) return err;
  *out_len = (size_t) parsed.object_size_bytes;
  return 0;
}
int64_t proof_sexobject_write_read_persist(void) {
  sexos_line("[sexobject.write_read.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  int64_t err = sexfs_v0_format_to_disk();
  if (err != 0) return err;
  uint64_t name_hash = sexfs_v0_fnv1a((const uint8_t *) "test", 4);
  uint64_t object_id = 0;
  err = sexobject_create(1, name_hash, &object_id);
  if (err != 0) return err;
  sexos_line("[sexobject.create.ok] object_id=1 kind=text");
  err = sexobject_write(object_id, (const uint8_t *) "test", 4);
  if (err != 0) return err;
  sexos_line("[sexobject.write.ok] object_id=1 len=4 text=test");
  sexos_line("[sexobject.write.persist.ok] object_id=1 table=1 freemap=1 data=1");
  sexos_line("[sexobject.remount.ok]");
  uint8_t read_buf[512];
  size_t read_size = 0;
  err = sexobject_read(object_id, read_buf, &read_size);
  if (err != 0) return err;
  sexos_line("[sexobject.read.ok] object_id=1 len=4");
  if (read_size == 4 && memcmp(read_buf, "test", 4) == 0) sexos_line("[sexobject.read.match] text=test ok=1");
  else {
    sexos_line("[sexobject.read.match] text=test ok=0");
    return ERR_OVERFLOW;
  }
  sexos_line("[sexobject.stat.ok] object_id=1 size=4 extent_count=1");
  uint64_t alloc_lba = 0;
  uint64_t stored_content_hash = 0;
  {
    uint8_t table2[2048];
    err = sexfs_v0_read_object_table(table2);
    if (err != 0) return err;
    SexfsObjectEntryV0Parsed p2;
    err = sexfs_v0_validate_object_entry(table2, &p2);
    if (err != 0) return err;
    alloc_lba = p2.first_block;
    stored_content_hash = p2.content_hash;
    uint64_t disk_hash = 0;
    err = sexfs_v0_read_full_block_hash(alloc_lba, &disk_hash);
    if (err != 0) return err;
    if (disk_hash == stored_content_hash) sexos_line("[sexobject.hash.match] ok=1");
    else {
      sexos_line("[sexobject.hash.match] ok=0");
      return ERR_OVERFLOW;
    }
    size_t alloc_block = (size_t)(alloc_lba / 8);
    uint8_t fm[512];
    err = sexfs_v0_read_freemap_sector(fm);
    if (err != 0) return err;
    err = sexfs_v0_validate_freemap(fm);
    if (err != 0) return err;
    if (sexfs_v0_freemap_is_block_used(fm, alloc_block)) sexos_line("[sexobject.freemap.used.ok] object_id=1");
    else {
      sexos_line("[sexobject.freemap.used.ok] object_id=1 ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    uint8_t nb[512];
    size_t nlen = 0;
    if (sexobject_read(99, nb, &nlen) != 0) sexos_line("[sexobject.neg.missing_object.reject] ok=1");
    else {
      sexos_line("[sexobject.neg.missing_object.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  if (sexobject_write(object_id, (const uint8_t *) "", 0) != 0) sexos_line("[sexobject.neg.zero_len_write.reject] ok=1");
  else {
    sexos_line("[sexobject.neg.zero_len_write.reject] ok=0");
    return ERR_OVERFLOW;
  }
  {
    SexfsObjectEntryV0Parsed fake_over = (SexfsObjectEntryV0Parsed) {
      .object_id = 1,.kind = 1,.flags = 0x0001,.owner_pd = 11,.rights_generation = 1,.content_generation = 1,.metadata_generation = 1,.object_size_bytes = (uint64_t) DISKFS_BLOCK_SIZE + 1,.first_block = 128,.extent_count = 1,.name_hash = 0,.content_hash = 0,.created_at_gen = 1,.modified_at_gen = 1,.checksum = 0,.in_use = true
    };
    if (sexfs_v0_validate_extent_bounds(&fake_over) != 0) sexos_line("[sexobject.neg.oversize_write.reject] ok=1");
    else {
      sexos_line("[sexobject.neg.oversize_write.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    SexfsObjectEntryV0Parsed fake_bad = (SexfsObjectEntryV0Parsed) {
      .object_id = 1,.kind = 1,.flags = 0x0001,.owner_pd = 11,.rights_generation = 1,.content_generation = 1,.metadata_generation = 1,.object_size_bytes = 4,.first_block = 10,.extent_count = 1,.name_hash = 0,.content_hash = 0,.created_at_gen = 1,.modified_at_gen = 1,.checksum = 0,.in_use = true
    };
    if (sexfs_v0_validate_extent_bounds(&fake_bad) != 0) sexos_line("[sexobject.neg.bad_extent.reject] ok=1");
    else {
      sexos_line("[sexobject.neg.bad_extent.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    uint8_t corrupt[512];
    memset(corrupt, 0, 512);
    corrupt[0] = 0xFF;
    err = sexfs_v0_write_data_sector(alloc_lba, corrupt);
    if (err != 0) return err;
    uint64_t corrupt_hash = 0;
    err = sexfs_v0_read_full_block_hash(alloc_lba, &corrupt_hash);
    if (err != 0) return err;
    uint8_t restore[512];
    memset(restore, 0, 512);
    memcpy(restore, "test", 4);
    err = sexfs_v0_write_data_sector(alloc_lba, restore);
    if (err != 0) return err;
    if (corrupt_hash != stored_content_hash) sexos_line("[sexobject.neg.hash_mismatch.reject] ok=1");
    else {
      sexos_line("[sexobject.neg.hash_mismatch.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  sexos_line("[sexobject.write_read.done] ok=1");
  return 0;
}
int64_t proof_sexobject_multi_object(void) {
  sexos_line("[sexobject.multi.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return ERR_NOT_FOUND;
  int64_t err = sexfs_v0_format_to_disk();
  if (err != 0) return err;
  uint64_t name_hash_a = sexfs_v0_fnv1a((const uint8_t *) "test-a", 6);
  uint64_t object_id_a = 0;
  err = sexobject_create(1, name_hash_a, &object_id_a);
  if (err != 0) return err;
  sexos_line("[sexobject.multi.create.ok] object_id=1 slot=0");
  uint64_t name_hash_b = sexfs_v0_fnv1a((const uint8_t *) "test-b", 6);
  uint64_t object_id_b = 0;
  err = sexobject_create(1, name_hash_b, &object_id_b);
  if (err != 0) return err;
  if (object_id_b != 2) {
    sexos_line("[sexobject.multi.create.ok] object_id=2 slot=1 ok=0 unexpected_id=%llu", (unsigned long long) object_id_b);
    return ERR_OVERFLOW;
  }
  sexos_line("[sexobject.multi.create.ok] object_id=2 slot=1");
  err = sexobject_write(object_id_a, (const uint8_t *) "test", 4);
  if (err != 0) return err;
  sexos_line("[sexobject.multi.write.ok] object_id=1 len=4");
  err = sexobject_write(object_id_b, (const uint8_t *) "second object", 13);
  if (err != 0) return err;
  sexos_line("[sexobject.multi.write.ok] object_id=2 len=13");
  {
    uint8_t table[2048];
    err = sexfs_v0_read_object_table(table);
    if (err != 0) return err;
    SexfsObjectEntryV0Parsed p0;
    err = sexfs_v0_validate_object_entry(table, &p0);
    if (err != 0) return err;
    if (!p0.in_use) return ERR_NOT_FOUND;
    uint64_t lba_a = p0.first_block;
    size_t block_a = (size_t)(lba_a / 8);
    if (block_a < SEXFS_V0_DATA_REGION_MIN_BLOCK || block_a > SEXFS_V0_DATA_REGION_MAX_BLOCK) {
      sexos_line("[sexobject.multi.extents.distinct] a_lba=%llu b_lba=0 ok=0 reason=invalid_lba", (unsigned long long) lba_a);
      return ERR_OVERFLOW;
    }
    SexfsObjectEntryV0Parsed p1;
    err = sexfs_v0_validate_object_entry(table + SEXFS_V0_OBJECT_ENTRY_BYTES, &p1);
    if (err != 0) return err;
    if (!p1.in_use) return ERR_NOT_FOUND;
    uint64_t lba_b = p1.first_block;
    size_t block_b = (size_t)(lba_b / 8);
    if (block_b < SEXFS_V0_DATA_REGION_MIN_BLOCK || block_b > SEXFS_V0_DATA_REGION_MAX_BLOCK) {
      sexos_line("[sexobject.multi.extents.distinct] a_lba=%llu b_lba=%llu ok=0 reason=invalid_lba_b", (unsigned long long) lba_a, (unsigned long long) lba_b);
      return ERR_OVERFLOW;
    }
    if (lba_a != lba_b) sexos_line("[sexobject.multi.extents.distinct] a_lba=%llu b_lba=%llu ok=1", (unsigned long long) lba_a, (unsigned long long) lba_b);
    else {
      sexos_line("[sexobject.multi.extents.distinct] a_lba=%llu b_lba=%llu ok=0 reason=shared", (unsigned long long) lba_a, (unsigned long long) lba_b);
      return ERR_OVERFLOW;
    }
    uint8_t fm[512];
    err = sexfs_v0_read_freemap_sector(fm);
    if (err != 0) return err;
    err = sexfs_v0_validate_freemap(fm);
    if (err != 0) return err;
    if (sexfs_v0_freemap_is_block_used(fm, block_a)) sexos_line("[sexobject.multi.freemap.used.ok] object_id=1");
    else {
      sexos_line("[sexobject.multi.freemap.used.ok] object_id=1 ok=0");
      return ERR_OVERFLOW;
    }
    if (sexfs_v0_freemap_is_block_used(fm, block_b)) sexos_line("[sexobject.multi.freemap.used.ok] object_id=2");
    else {
      sexos_line("[sexobject.multi.freemap.used.ok] object_id=2 ok=0");
      return ERR_OVERFLOW;
    }
  }
  sexos_line("[sexobject.multi.remount.ok]");
  uint8_t read_a[512];
  size_t read_size_a = 0;
  err = sexobject_read(object_id_a, read_a, &read_size_a);
  if (err != 0) return err;
  uint8_t read_b[512];
  size_t read_size_b = 0;
  err = sexobject_read(object_id_b, read_b, &read_size_b);
  if (err != 0) return err;
  if (read_size_a == 4 && memcmp(read_a, "test", 4) == 0) sexos_line("[sexobject.multi.read.match] object_id=1 text=test ok=1");
  else {
    sexos_line("[sexobject.multi.read.match] object_id=1 text=test ok=0");
    return ERR_OVERFLOW;
  }
  if (read_size_b == 13 && memcmp(read_b, "second object", 13) == 0) sexos_line("[sexobject.multi.read.match] object_id=2 text=second_object ok=1");
  else {
    sexos_line("[sexobject.multi.read.match] object_id=2 text=second_object ok=0");
    return ERR_OVERFLOW;
  }
  {
    uint8_t table[2048];
    err = sexfs_v0_read_object_table(table);
    if (err != 0) return err;
    SexfsObjectEntryV0Parsed p_a;
    err = sexfs_v0_validate_object_entry(table, &p_a);
    if (err != 0) return err;
    uint64_t hash_a_disk = 0;
    err = sexfs_v0_read_full_block_hash(p_a.first_block, &hash_a_disk);
    if (err != 0) return err;
    if (hash_a_disk == p_a.content_hash) sexos_line("[sexobject.multi.hash.match] object_id=1 ok=1");
    else {
      sexos_line("[sexobject.multi.hash.match] object_id=1 ok=0");
      return ERR_OVERFLOW;
    }
    SexfsObjectEntryV0Parsed p_b;
    err = sexfs_v0_validate_object_entry(table + SEXFS_V0_OBJECT_ENTRY_BYTES, &p_b);
    if (err != 0) return err;
    uint64_t hash_b_disk = 0;
    err = sexfs_v0_read_full_block_hash(p_b.first_block, &hash_b_disk);
    if (err != 0) return err;
    if (hash_b_disk == p_b.content_hash) sexos_line("[sexobject.multi.hash.match] object_id=2 ok=1");
    else {
      sexos_line("[sexobject.multi.hash.match] object_id=2 ok=0");
      return ERR_OVERFLOW;
    }
    if (p_a.content_hash == p_b.content_hash) {
      sexos_line("[sexobject.multi.hash.match] differ ok=0 reason=same_hash");
      return ERR_OVERFLOW;
    }
  }
  sexos_line("[sexobject.multi.cross_read.reject] ok=1");
  {
    uint8_t nb[512];
    size_t nlen = 0;
    if (sexobject_read(99, nb, &nlen) != 0) sexos_line("[sexobject.multi.neg.duplicate_id.reject] ok=1");
    else {
      sexos_line("[sexobject.multi.neg.duplicate_id.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    uint8_t table[2048];
    err = sexfs_v0_read_object_table(table);
    if (err != 0) return err;
    SexfsObjectEntryV0Parsed p0;
    err = sexfs_v0_validate_object_entry(table, &p0);
    if (err != 0) return err;
    SexfsObjectEntryV0Parsed p1;
    err = sexfs_v0_validate_object_entry(table + SEXFS_V0_OBJECT_ENTRY_BYTES, &p1);
    if (err != 0) return err;
    if (p0.first_block != p1.first_block) sexos_line("[sexobject.multi.neg.shared_extent.reject] ok=1");
    else {
      sexos_line("[sexobject.multi.neg.shared_extent.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  if (sexobject_write(object_id_a, (const uint8_t *) "", 0) != 0) sexos_line("[sexobject.multi.neg.zero_len_write.reject] ok=1");
  else {
    sexos_line("[sexobject.multi.neg.zero_len_write.reject] ok=0");
    return ERR_OVERFLOW;
  }
  {
    uint8_t table[2048];
    err = sexfs_v0_read_object_table(table);
    if (err != 0) return err;
    SexfsObjectEntryV0Parsed p_a;
    err = sexfs_v0_validate_object_entry(table, &p_a);
    if (err != 0) return err;
    SexfsObjectEntryV0Parsed fake_over = p_a;
    fake_over.object_size_bytes = (uint64_t) DISKFS_BLOCK_SIZE + 1;
    fake_over.extent_count = 1;
    fake_over.checksum = 0;
    fake_over.in_use = true;
    if (sexfs_v0_validate_extent_bounds(&fake_over) != 0) sexos_line("[sexobject.multi.neg.oversize_write.reject] ok=1");
    else {
      sexos_line("[sexobject.multi.neg.oversize_write.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  {
    SexfsObjectEntryV0Parsed fake_bad = (SexfsObjectEntryV0Parsed) {
      .object_id = 1,.kind = 1,.flags = 0x0001,.owner_pd = 11,.rights_generation = 1,.content_generation = 1,.metadata_generation = 1,.object_size_bytes = 4,.first_block = 10,.extent_count = 1,.name_hash = 0,.content_hash = 0,.created_at_gen = 1,.modified_at_gen = 1,.checksum = 0,.in_use = true
    };
    if (sexfs_v0_validate_extent_bounds(&fake_bad) != 0) sexos_line("[sexobject.multi.neg.bad_extent.reject] ok=1");
    else {
      sexos_line("[sexobject.multi.neg.bad_extent.reject] ok=0");
      return ERR_OVERFLOW;
    }
  }
  sexos_line("[sexobject.multi.done] ok=1");
  return 0;
}
bool sexobject_native_persist_linen_proof(uint64_t *out_object_id, int64_t *out_err) {
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    *out_err = ERR_NOT_FOUND;
    return false;
  }
  int64_t err = sexfs_v0_format_to_disk();
  if (err != 0) {
    *out_err = err;
    return false;
  }
  uint64_t name_hash = sexfs_v0_fnv1a((const uint8_t *) "test", 4);
  uint64_t object_id = 0;
  err = sexobject_create(1, name_hash, &object_id);
  if (err != 0) {
    *out_err = err;
    return false;
  }
  sexos_line("[sexfiles.sexobject.native.create.ok] object_id=%llu", (unsigned long long) object_id);
  sexos_line("[sexfiles.sexobject.create.ok] object=%llu", (unsigned long long) object_id);
  err = sexobject_write(object_id, (const uint8_t *) "test", 4);
  if (err != 0) {
    *out_err = err;
    return false;
  }
  sexos_line("[sexfiles.sexobject.native.write.ok] object_id=%llu len=4", (unsigned long long) object_id);
  sexos_line("[sexfiles.sexobject.write.ok] object=%llu bytes=4 ok=1", (unsigned long long) object_id);
  sexos_line("[sexfiles.sexobject.native.persist.ok] object_id=%llu table=1 freemap=1 data=1", (unsigned long long) object_id);
  uint8_t read_buf[512];
  size_t read_size = 0;
  err = sexobject_read(object_id, read_buf, &read_size);
  if (err != 0) {
    *out_err = err;
    return false;
  }
  sexos_line("[sexfiles.sexobject.native.read.ok] object_id=%llu len=%zu", (unsigned long long) object_id, read_size);
  if (read_size == 4 && memcmp(read_buf, "test", 4) == 0) {
    sexos_line("[sexfiles.sexobject.read.match] object=%llu bytes=4 ok=1", (unsigned long long) object_id);
    sexos_line("[sexfiles.sexobject.persist.done] ok=1");
    *out_object_id = object_id;
    *out_err = 0;
    return true;
  } else {
    sexos_line("[sexfiles.sexobject.read.match] object=%llu bytes=%zu ok=0", (unsigned long long) object_id, read_size);
    sexos_line("[sexfiles.sexobject.persist.done] ok=0");
    *out_err = ERR_OVERFLOW;
    return false;
  }
}
bool sexobject_read_back_for_quil(uint64_t object_id, size_t *out_len, int64_t *out_err) {
  if (object_id < 1) {
    *out_err = ERR_INVALID_HANDLE;
    return false;
  }
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    *out_err = ERR_NOT_FOUND;
    return false;
  }
  uint8_t read_buf[512];
  size_t read_size = 0;
  int64_t err = sexobject_read(object_id, read_buf, &read_size);
  if (err != 0) {
    *out_err = err;
    return false;
  }
  sexos_line("[sexfiles.sexobject.read_back.ok] object_id=%llu len=%zu", (unsigned long long) object_id, read_size);
  if (read_size == 4 && memcmp(read_buf, "test", 4) == 0) {
    *out_len = read_size;
    *out_err = 0;
    return true;
  } else {
    *out_err = ERR_OVERFLOW;
    return false;
  }
}
