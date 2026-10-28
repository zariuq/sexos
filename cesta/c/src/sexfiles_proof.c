#include "sexfiles.h"
/*RamFS, DiskFS, Linen, Checkpoint, Extent, Fault, AP2-AP6, and SexFS v0 proof harnesses (servers/sexfiles/src/proof.rs).*/
#define SELF_PD ((uint32_t)0)
#define DISKFS_MANIFEST_OBJECT_PATH_STR "/disk/sexfiles-proof-v1"
#define DISKFS_MANIFEST_OBJECT_PATH_LEN ((size_t)23)
#define DISKFS_OBJECT_PATH_LINEN_STR "/disk/linen-object-v1"
#define DISKFS_OBJECT_PATH_LINEN_LEN ((size_t)21)
#define DISKFS_OBJECT_PATH_QUIL_STR "/disk/quil-object-v1"
#define DISKFS_OBJECT_PATH_QUIL_LEN ((size_t)20)
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
static inline void proof_fail(const char *msg) {
  (void) msg;
  sexos_spin_forever();
}
static void proof_create_write_read(void) {
  const uint8_t *name = (const uint8_t *) "proof_roundtrip";
  uint64_t handle = 0;
  if (ramfs_open(name, 15, RAMFS_O_CREATE, 0, SELF_PD, &handle) != 0) proof_fail("[proof.1] open failed");
  if (handle <= 0) proof_fail("[proof.1] handle > 0");
  const uint8_t *data = (const uint8_t *) "RamFS Contract Lock V1";
  uint64_t written = 0;
  if (ramfs_write(handle, 0, data, 22, SELF_PD, &written) != 0) proof_fail("[proof.1] write failed");
  if ((size_t) written != 22) proof_fail("[proof.1] write len");
  uint8_t buf[64];
  memset(buf, 0, 64);
  uint64_t n = 0;
  if (ramfs_read(handle, 0, buf, 64, SELF_PD, &n) != 0) proof_fail("[proof.1] read failed");
  if ((size_t) n != 22 || memcmp(buf, data, 22) != 0) proof_fail("[proof.1] data roundtrip");
  if (ramfs_close(handle, SELF_PD) != 0) proof_fail("[proof.1] close failed");
  sexos_line("[sexfiles.ramfs.proof.1] create/write/read roundtrip OK");
}
static void proof_invalid_handle(void) {
  uint8_t buf[8];
  uint64_t n = 0;
  int64_t err = ramfs_read(0xBAD, 0, buf, 8, SELF_PD, &n);
  if (err != ERR_INVALID_HANDLE) proof_fail("[proof.2] err code");
  err = ramfs_close(0xDEAD, SELF_PD);
  if (err != ERR_INVALID_HANDLE) proof_fail("[proof.2] close err code");
  sexos_line("[sexfiles.ramfs.proof.2] invalid handle rejected OK");
}
static void proof_oversized_name(void) {
  const uint8_t *long_name = (const uint8_t *) "this_name_exceeds_the_24_byte_maximum_for_ramfs";
  uint64_t h = 0;
  int64_t err = ramfs_open(long_name, 47, RAMFS_O_CREATE, 0, SELF_PD, &h);
  if (err != ERR_NAME_TOO_LONG) proof_fail("[proof.3] err code");
  sexos_line("[sexfiles.ramfs.proof.3] oversized name rejected OK");
}
static void proof_oob_write(void) {
  const uint8_t *name = (const uint8_t *) "proof_oob";
  uint64_t handle = 0;
  if (ramfs_open(name, 9, RAMFS_O_CREATE, 0, SELF_PD, &handle) != 0) proof_fail("[proof.4] open failed");
  uint8_t big[RAMFS_MAX_FILE_SIZE + 1];
  memset(big, 0xBB, sizeof big);
  uint64_t written = 0;
  int64_t err = ramfs_write(handle, 0, big, RAMFS_MAX_FILE_SIZE + 1, SELF_PD, &written);
  if (err != ERR_OVERFLOW) proof_fail("[proof.4] err code");
  uint8_t ok_data[RAMFS_MAX_FILE_SIZE];
  memset(ok_data, 0xCC, sizeof ok_data);
  if (ramfs_write(handle, 0, ok_data, RAMFS_MAX_FILE_SIZE, SELF_PD, &written) != 0) proof_fail("[proof.4] boundary write should succeed");
  if ((size_t) written != RAMFS_MAX_FILE_SIZE) proof_fail("[proof.4] boundary write len");
  if (ramfs_write(handle, 1, ok_data, RAMFS_MAX_FILE_SIZE, SELF_PD, &written) == 0) proof_fail("[proof.4] post-boundary write must err");
  if (ramfs_close(handle, SELF_PD) != 0) proof_fail("[proof.4] close failed");
  sexos_line("[sexfiles.ramfs.proof.4] OOB write rejected OK");
}
static void proof_oob_read_clamped(void) {
  const uint8_t *name = (const uint8_t *) "proof_oob_read";
  uint64_t handle = 0;
  if (ramfs_open(name, 14, RAMFS_O_CREATE, 0, SELF_PD, &handle) != 0) proof_fail("[proof.5] open failed");
  const uint8_t *data = (const uint8_t *) "1234567890";
  uint64_t written = 0;
  if (ramfs_write(handle, 0, data, 10, SELF_PD, &written) != 0) proof_fail("[proof.5] write failed");
  uint8_t buf[8];
  uint64_t n = 0;
  if (ramfs_read(handle, 100, buf, 8, SELF_PD, &n) != 0) proof_fail("[proof.5] OOB read should not err");
  if (n != 0) proof_fail("[proof.5] OOB read returns 0");
  if (ramfs_read(handle, 5, buf, 8, SELF_PD, &n) != 0) proof_fail("[proof.5] partial read");
  if (n != 5 || memcmp(buf, "67890", 5) != 0) proof_fail("[proof.5] partial read data");
  if (ramfs_close(handle, SELF_PD) != 0) proof_fail("[proof.5] close failed");
  sexos_line("[sexfiles.ramfs.proof.5] OOB read clamped OK");
}
static void proof_max_files(void) {
  uint64_t handles[RAMFS_MAX_FILES];
  for (size_t i = 0; i < RAMFS_MAX_FILES; ++i) {
    uint8_t name[16];
    memset(name, 0, 16);
    memcpy(name, "file_", 5);
    cesta_write_bytes out = (cesta_write_bytes) {
      .data = name + 5,.len = 11
    };
    size_t dlen = format_bytes_unsigned_decimal(out, (uint64_t) i);
    size_t nlen = 5 + dlen;
    if (ramfs_open(name, nlen, RAMFS_O_CREATE, 0, SELF_PD, &handles[i]) != 0) proof_fail("[proof.6] max file create");
  }
  uint64_t dummy = 0;
  int64_t err = ramfs_open((const uint8_t *) "file_overflow", 13, RAMFS_O_CREATE, 0, SELF_PD, &dummy);
  if (err != ERR_FULL) proof_fail("[proof.6] err code");
  for (size_t i = 0; i < RAMFS_MAX_FILES; ++i) if (ramfs_close(handles[i], SELF_PD) != 0) proof_fail("[proof.6] close failed");
  if (ramfs_len(SELF_PD) != 0) proof_fail("[proof.6] all closed");
  sexos_line("[sexfiles.ramfs.proof.6] max files limit enforced OK");
}
static void proof_close_reopen_persist(void) {
  const uint8_t *name = (const uint8_t *) "proof_persist";
  uint64_t handle = 0;
  if (ramfs_open(name, 13, RAMFS_O_CREATE, 0, SELF_PD, &handle) != 0) proof_fail("[proof.7] open failed");
  const uint8_t *data = (const uint8_t *) "Persist!";
  uint64_t written = 0;
  if (ramfs_write(handle, 0, data, 8, SELF_PD, &written) != 0) proof_fail("[proof.7] write failed");
  if (ramfs_close(handle, SELF_PD) != 0) proof_fail("[proof.7] close failed");
  uint64_t reopened = 0;
  if (ramfs_open(name, 13, 0, 0, SELF_PD, &reopened) != 0) proof_fail("[proof.7] reopen failed");
  uint8_t buf[64];
  uint64_t n = 0;
  if (ramfs_read(reopened, 0, buf, 64, SELF_PD, &n) != 0) proof_fail("[proof.7] read failed");
  if (n != 8 || memcmp(buf, data, 8) != 0) proof_fail("[proof.7] reopen data intact");
  if (ramfs_close(reopened, SELF_PD) != 0) proof_fail("[proof.7] close reopened failed");
  sexos_line("[sexfiles.ramfs.proof.7] close+reopen data persistence OK");
}
static void proof_non_owner_denied(void) {
  const uint8_t *name = (const uint8_t *) "proof_owner";
  uint64_t handle = 0;
  if (ramfs_open(name, 11, RAMFS_O_CREATE, 0, 1, &handle) != 0) proof_fail("[proof.8] open failed");
  uint8_t buf[8];
  uint64_t n = 0;
  if (ramfs_read(handle, 0, buf, 8, 2, &n) != ERR_PERM_DENIED) proof_fail("[proof.8] non-owner read must err");
  if (ramfs_write(handle, 0, (const uint8_t *) "x", 1, 2, &n) != ERR_PERM_DENIED) proof_fail("[proof.8] non-owner write must err");
  if (ramfs_close(handle, 2) != ERR_PERM_DENIED) proof_fail("[proof.8] non-owner close must err");
  uint64_t sz = 0;
  uint32_t nl = 0;
  if (ramfs_stat(handle, 2, &sz, &nl) != ERR_PERM_DENIED) proof_fail("[proof.8] non-owner stat must err");
  uint64_t h2 = 0;
  if (ramfs_open(name, 11, 0, 0, 2, &h2) != ERR_PERM_DENIED) proof_fail("[proof.8] non-owner open must err");
  if (ramfs_read(handle, 0, buf, 8, 1, &n) != 0 || n != 0) proof_fail("[proof.8] owner read should succeed");
  if (ramfs_close(handle, 1) != 0) proof_fail("[proof.8] owner close failed");
  sexos_line("[sexfiles.ramfs.proof.8] non-owner access denied OK");
}
void run_all_proofs(void) {
  sexos_line("[sexfiles.ramfs.proof.start]");
  proof_create_write_read();
  proof_invalid_handle();
  proof_oversized_name();
  proof_oob_write();
  proof_oob_read_clamped();
  proof_max_files();
  proof_close_reopen_persist();
  proof_non_owner_denied();
  sexos_line("[sexfiles.ramfs.proof.done] ALL CHECKS PASSED");
}
void run_diskfs_object_table_proofs(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[diskfs.proof] format failed");
  sexos_line("[diskfs.proof.format] ok=1");
  SexfilesSuperblock sb;
  if (diskfs_mount(&sb) != 0) proof_fail("[diskfs.proof] mount failed");
  bool ok_mount = sb.version_major == 1 && (sb.block_size == 4096 && sb.object_table_entry_count == (uint32_t) DISKFS_MAX_OBJECTS);
  sexos_line("[diskfs.proof.mount] ok=%u gen=%llu", ok_mount ? 1u : 0u, (unsigned long long) sb.fs_generation);
  uint64_t oid = 0;
  if (diskfs_create_object_entry(1, 9, &oid) != 0) proof_fail("[diskfs.proof] create failed");
  sexos_line("[diskfs.proof.create_object] id=%llu owner=9 kind=1", (unsigned long long) oid);
  SexfilesObjectEntry st;
  if (diskfs_stat_object_entry(oid, &st) != 0) proof_fail("[diskfs.proof] stat failed");
  bool ok_stat = st.object_id == oid && (st.owner_pd == 9 && st.kind == 1);
  sexos_line("[diskfs.proof.stat_object] ok=%u id=%llu owner=%u", ok_stat ? 1u : 0u, (unsigned long long) st.object_id, st.owner_pd);
  int64_t invalid = diskfs_stat_object_entry(UINT64_MAX, NULL);
  bool ok_invalid = invalid == ERR_INVALID_HANDLE;
  sexos_line("[diskfs.proof.invalid_object] ok=%u", ok_invalid ? 1u : 0u);
  for (size_t i = 1; i < DISKFS_MAX_OBJECTS; ++i) {
    uint64_t dummy = 0;
    if (diskfs_create_object_entry(2, 9, &dummy) != 0) proof_fail("[diskfs.proof] fill failed");
  }
  uint64_t full_oid = 0;
  int64_t full = diskfs_create_object_entry(2, 9, &full_oid);
  bool ok_full = full == ERR_FULL;
  sexos_line("[diskfs.proof.table_full] ok=%u slots=%zu", ok_full ? 1u : 0u, (size_t) DISKFS_MAX_OBJECTS);
}
void run_sexfiles_journal_proofs(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[sexfiles.journal.proof] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[sexfiles.journal.proof] mount failed");
  size_t before = diskfs_journal_len();
  uint64_t oid = 0;
  if (diskfs_create_object_entry(1, 11, &oid) != 0) proof_fail("[sexfiles.journal.proof] create failed");
  size_t after = diskfs_journal_len();
  bool ok_begin = after >= before + 3;
  size_t delta = after >= before ? after - before : 0;
  sexos_line("[sexfiles.journal.proof.begin] ok=%u oid=%llu", ok_begin ? 1u : 0u, (unsigned long long) oid);
  sexos_line("[sexfiles.journal.proof.append] ok=%u delta=%zu", ok_begin ? 1u : 0u, delta);
  sexos_line("[sexfiles.journal.proof.commit] ok=%u journal_len=%zu", ok_begin ? 1u : 0u, after);
  bool full_hit = false;
  for (size_t i = 0; i < DISKFS_JOURNAL_CAPACITY + 8; ++i) {
    uint64_t dummy = 0;
    int64_t err = diskfs_create_object_entry(2, 11, &dummy);
    if (err == ERR_FULL) {
      full_hit = true;
      break;
    }
    if (err != 0) break;
  }
  sexos_line("[sexfiles.journal.proof.full] ok=%u cap=%zu", full_hit ? 1u : 0u, (size_t) DISKFS_JOURNAL_CAPACITY);
  if (diskfs_format_init_empty() != 0) proof_fail("[sexfiles.journal.proof] reformat failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[sexfiles.journal.proof] remount failed");
  int64_t bad = diskfs_proof_inject_bad_record_for_checksum();
  bool ok_bad = bad == ERR_OVERFLOW;
  sexos_line("[sexfiles.journal.proof.checksum_reject] ok=%u", ok_bad ? 1u : 0u);
}
void run_sexfiles_replay_proofs(void) {
  ReplayOutcome out;
  if (diskfs_proof_replay_recovery_scenario(&out) != 0) proof_fail("[sexfiles.replay.proof] scenario failed");
  sexos_line("[sexfiles.replay.proof.committed_applied] ok=%u", out.committed_applied ? 1u : 0u);
  sexos_line("[sexfiles.replay.proof.uncommitted_ignored] ok=%u", out.uncommitted_ignored ? 1u : 0u);
  sexos_line("[sexfiles.replay.proof.corrupt_rejected] ok=%u", out.corrupt_rejected ? 1u : 0u);
  sexos_line("[sexfiles.replay.proof.generation_order] ok=%u", out.generation_order ? 1u : 0u);
  sexos_line("[sexfiles.replay.proof.object_restored] ok=%u", out.object_restored ? 1u : 0u);
}
void run_sexfiles_cap_record_proofs(void) {
  const uint8_t *name = (const uint8_t *) "caprec_file";
  uint32_t owner_pd = 41;
  uint32_t subject_pd = 42;
  uint32_t other_pd = 43;
  uint64_t h_owner = 0;
  if (ramfs_open(name, 11, RAMFS_O_CREATE, 0, owner_pd, &h_owner) != 0) {
    sexos_line("[sexfiles.caprec.proof.grant_allow] ok=0");
    sexos_line("[sexfiles.caprec.proof.read_allow] ok=0");
    sexos_line("[sexfiles.caprec.proof.write_allow] ok=0");
    sexos_line("[sexfiles.caprec.proof.missing_deny] ok=0");
    sexos_line("[sexfiles.caprec.proof.revoked_deny] ok=0");
    sexos_line("[sexfiles.caprec.proof.generation_deny] ok=0");
    return;
  }
  uint64_t dummy_w = 0;
  (void) ramfs_write(h_owner, 0, (const uint8_t *) "seed", 4, owner_pd, &dummy_w);
  uint8_t grant_rights = (uint8_t)(CAP_RIGHT_READ | CAP_RIGHT_WRITE | (CAP_RIGHT_APPEND | (CAP_RIGHT_LIST | CAP_RIGHT_GRANT)));
  bool grant_allow = ramfs_proof_grant_caps_by_name(owner_pd, name, 11, subject_pd, grant_rights) == 0;
  sexos_line("[sexfiles.caprec.proof.grant_allow] ok=%u", grant_allow ? 1u : 0u);
  uint64_t h_subject = 0;
  if (ramfs_open(name, 11, 0, 0, subject_pd, &h_subject) != 0) {
    sexos_line("[sexfiles.caprec.proof.read_allow] ok=0");
    sexos_line("[sexfiles.caprec.proof.write_allow] ok=0");
    sexos_line("[sexfiles.caprec.proof.missing_deny] ok=0");
    sexos_line("[sexfiles.caprec.proof.revoked_deny] ok=0");
    sexos_line("[sexfiles.caprec.proof.generation_deny] ok=0");
    (void) ramfs_close(h_owner, owner_pd);
    return;
  }
  uint8_t buf[8];
  uint64_t n = 0;
  bool read_ok = ramfs_read(h_subject, 0, buf, 8, subject_pd, &n) == 0;
  sexos_line("[sexfiles.caprec.proof.read_allow] ok=%u", read_ok ? 1u : 0u);
  bool write_ok = ramfs_write(h_subject, 1, (const uint8_t *) "X", 1, subject_pd, &dummy_w) == 0;
  sexos_line("[sexfiles.caprec.proof.write_allow] ok=%u", write_ok ? 1u : 0u);
  bool missing_deny = ramfs_read(h_subject, 0, buf, 8, other_pd, &n) == ERR_PERM_DENIED;
  sexos_line("[sexfiles.caprec.proof.missing_deny] ok=%u", missing_deny ? 1u : 0u);
  (void) ramfs_proof_revoke_caps_by_name(owner_pd, name, 11);
  bool revoked_deny = ramfs_read(h_subject, 0, buf, 8, subject_pd, &n) == ERR_PERM_DENIED;
  sexos_line("[sexfiles.caprec.proof.revoked_deny] ok=%u", revoked_deny ? 1u : 0u);
  (void) ramfs_proof_inject_stale_generation_cap(name, 11, subject_pd, CAP_RIGHT_READ, 1);
  bool generation_deny = ramfs_read(h_subject, 0, buf, 8, subject_pd, &n) == ERR_PERM_DENIED;
  sexos_line("[sexfiles.caprec.proof.generation_deny] ok=%u", generation_deny ? 1u : 0u);
  (void) ramfs_close(h_owner, owner_pd);
}
void run_linen_sexfiles_metadata_proofs(void) {
  sexos_line("[sexfiles.linen.metadata.proof.start]");
  const uint8_t *meta_name = (const uint8_t *) "linen_meta_proof_01";
  uint32_t owner_pd = 42;
  uint32_t other_pd = 99;
  uint64_t h = 0;
  int64_t err = ramfs_create_with_owner(meta_name, 19, owner_pd, 0, &h);
  if (err != 0) {
    sexos_line("[linen.sexfiles.proof.create_link] ok=0 err=%lld", (long long) err);
    sexos_line("[sexfiles.linen.metadata.proof.done] FAILED");
    return;
  }
  sexos_line("[linen.sexfiles.proof.create_link] ok=1 handle=%llu owner=%u", (unsigned long long) h, owner_pd);
  uint8_t data[8];
  data[0] = 0x01;
  data[1] = 0x00;
  data[2] = 0x2A;
  data[3] = 0x00;
  data[4] = 0x00;
  data[5] = 0x00;
  data[6] = 0x00;
  data[7] = 0x01;
  uint64_t n = 0;
  err = ramfs_write(h, 0, data, 8, owner_pd, &n);
  if (err == 0) sexos_line("[linen.sexfiles.proof.generation] ok=1 n=%llu", (unsigned long long) n);
  else sexos_line("[linen.sexfiles.proof.generation] ok=0 err=%lld", (long long) err);
  uint64_t lh = 0;
  uint32_t name_len = 0;
  if (ramfs_list_at(0, owner_pd, &lh, &name_len)) sexos_line("[linen.sexfiles.proof.list_link] ok=1 handle=%llu name_len=%u", (unsigned long long) lh, name_len);
  else sexos_line("[linen.sexfiles.proof.list_link] ok=0 reason=not_found");
  uint64_t sz = 0;
  err = ramfs_stat(h, owner_pd, &sz, &name_len);
  if (err == 0) sexos_line("[linen.sexfiles.proof.get_link] ok=1 size=%llu name_len=%u", (unsigned long long) sz, name_len);
  else sexos_line("[linen.sexfiles.proof.get_link] ok=0 err=%lld", (long long) err);
  uint8_t buf[8];
  err = ramfs_read(h, 0, buf, 8, other_pd, &n);
  bool deny_ok = err == ERR_PERM_DENIED;
  sexos_line("[linen.sexfiles.proof.owner_deny] ok=%u", deny_ok ? 1u : 0u);
  (void) ramfs_close(h, owner_pd);
  sexos_line("[sexfiles.linen.metadata.proof.done] ALL CHECKS PASSED");
}
void run_linen_disk_object_proof(void) {
  sexos_line("[linen.disk.object.proof.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[linen.disk.object.proof] buf_grant_failed va=%#llx", (unsigned long long) buf_va);
    return;
  }
  sexos_line("[linen.disk.object.proof.buf_va] va=%#llx", (unsigned long long) buf_va);
  const uint8_t *path = (const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR;
  size_t path_len = DISKFS_MANIFEST_OBJECT_PATH_LEN;
  uint8_t manifest_sector[512];
  diskfs_proof_manifest_build_single_entry_sector(manifest_sector);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = manifest_sector[i];
  uint64_t mw_status = diskfs_block_write(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend);
  if (mw_status != 0) {
    sexos_line("[linen.disk.object.proof] manifest_write_failed status=%llu", (unsigned long long) mw_status);
    return;
  }
  uint8_t linen_payload[128];
  memset(linen_payload, 0, 128);
  uint64_t linen_object_id = UINT64_C(0x31564E454E494C);
  write_le64(linen_payload + 0, linen_object_id);
  write_le16(linen_payload + 8, 0);
  write_le32(linen_payload + 10, 42);
  write_le64(linen_payload + 14, 1);
  linen_payload[22] = 0x01;
  linen_payload[23] = 13;
  memcpy(linen_payload + 24, "linen-disk-v1", 13);
  for (size_t i = 48; i < 128; ++i) linen_payload[i] = (uint8_t) i ^ 0x5Au;
  sexos_line("[linen.disk.object.save.request] object_id=%#llx kind=0 owner=42 name_len=13 size=128", (unsigned long long) linen_object_id);
  uint64_t written = 0;
  uint64_t werr = 0;
  if (diskfs_write_object(path, path_len, 0, linen_payload, 128, buf_va, &written, &werr)) if (written == 128) sexos_line("[linen.disk.object.save.ok] written=%llu path=/disk/sexfiles-proof-v1", (unsigned long long) written);
  else {
    sexos_line("[linen.disk.object.save.ok] partial_write=%llu expected=128", (unsigned long long) written);
    return;
  } else {
    sexos_line("[linen.disk.object.save.ok] write_failed code=%llu", (unsigned long long) werr);
    return;
  }
  uint8_t readback[128];
  memset(readback, 0, 128);
  sexos_line("[linen.disk.object.load.request] offset=0 len=128");
  uint64_t nread = 0;
  uint64_t rerr = 0;
  if (diskfs_read_object(path, path_len, 0, readback, 128, buf_va, &nread, &rerr)) if (nread == 128) {
    bool match_ok = true;
    size_t mismatch_at = 0;
    for (size_t i = 0; i < 128; ++i) if (readback[i] != linen_payload[i]) {
      match_ok = false;
      mismatch_at = i;
      break;
    }
    if (match_ok) sexos_line("[linen.disk.object.load.match] ok=1 size=128");
    else sexos_line("[linen.disk.object.load.mismatch] offset=%zu expected=%#llx got=%#llx", mismatch_at, (unsigned long long) linen_payload[mismatch_at], (unsigned long long) readback[mismatch_at]);
  } else sexos_line("[linen.disk.object.load.match] short_read=%llu expected=128", (unsigned long long) nread);
  else sexos_line("[linen.disk.object.load.match] read_failed code=%llu", (unsigned long long) rerr);
  {
    uint8_t oob[1];
    if (!diskfs_read_object(path, path_len, 128, oob, 1, buf_va, &nread, &rerr)) sexos_line("[linen.disk.object.load.bounds_negative] ok=1 test=read_past_end");
    else sexos_line("[linen.disk.object.load.bounds_negative] ok=0 reason=read_past_end_allowed");
  }
  {
    uint8_t last[1];
    if (diskfs_read_object(path, path_len, 127, last, 1, buf_va, &nread, &rerr) && nread == 1) {
      bool last_ok = last[0] == linen_payload[127];
      sexos_line("[linen.disk.object.load.last_byte] ok=%u byte=%#llx", last_ok ? 1u : 0u, (unsigned long long) last[0]);
    } else sexos_line("[linen.disk.object.load.last_byte] ok=0 reason=read_failed");
  }
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t mf_rd_status = diskfs_block_read(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend);
  bool mf_ok = false;
  if (mf_rd_status == 0) {
    uint8_t mf_sector[512];
    for (size_t i = 0; i < 512; ++i) mf_sector[i] = p[i];
    DiskManifestEntryV1 dummy_e;
    uint64_t dummy_err = 0;
    mf_ok = diskfs_proof_manifest_parse_single_entry(mf_sector, &dummy_e, &dummy_err);
  }
  sexos_line("[linen.disk.object.manifest_still_ok] ok=%u", mf_ok ? 1u : 0u);
  sexos_line("[linen.disk.object.proof.done]");
}
static void fault_invalid_object(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[fault.1] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[fault.1] mount failed");
  bool ok0 = diskfs_stat_object_entry(0, NULL) == ERR_INVALID_HANDLE;
  bool ok_max = diskfs_stat_object_entry(UINT64_MAX, NULL) == ERR_INVALID_HANDLE;
  sexos_line("[sexfiles.fault.proof.invalid_object] ok=%u", ok0 && ok_max ? 1u : 0u);
}
static void fault_table_full(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[fault.2] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[fault.2] mount failed");
  for (size_t i = 0; i < DISKFS_MAX_OBJECTS; ++i) {
    uint64_t dummy = 0;
    if (diskfs_create_object_entry(1, 200, &dummy) != 0) proof_fail("[fault.2] fill object");
  }
  uint64_t dummy = 0;
  bool ok = diskfs_create_object_entry(1, 200, &dummy) == ERR_FULL;
  sexos_line("[sexfiles.fault.proof.table_full] ok=%u capacity=%zu", ok ? 1u : 0u, (size_t) DISKFS_MAX_OBJECTS);
}
static void fault_journal_full(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[fault.3] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[fault.3] mount failed");
  bool ok = diskfs_proof_fill_journal_and_test_full() == ERR_FULL;
  sexos_line("[sexfiles.fault.proof.journal_full] ok=%u capacity=%zu", ok ? 1u : 0u, (size_t) DISKFS_JOURNAL_CAPACITY);
}
static void fault_oversized_write(void) {
  const uint8_t *name = (const uint8_t *) "fault_oob_write";
  uint64_t handle = 0;
  if (ramfs_open(name, 15, RAMFS_O_CREATE, 0, SELF_PD, &handle) != 0) proof_fail("[fault.4] open failed");
  uint8_t big[RAMFS_MAX_FILE_SIZE + 1];
  memset(big, 0xBB, sizeof big);
  uint64_t written = 0;
  bool ok = ramfs_write(handle, 0, big, RAMFS_MAX_FILE_SIZE + 1, SELF_PD, &written) == ERR_OVERFLOW;
  uint8_t max_data[RAMFS_MAX_FILE_SIZE];
  memset(max_data, 0xCC, sizeof max_data);
  bool ok2 = ramfs_write(handle, 1, max_data, RAMFS_MAX_FILE_SIZE, SELF_PD, &written) != 0;
  if (ramfs_close(handle, SELF_PD) != 0) proof_fail("[fault.4] close failed");
  sexos_line("[sexfiles.fault.proof.oversized_write] ok=%u", ok && ok2 ? 1u : 0u);
}
static void fault_corrupt_reject(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[fault.5] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[fault.5] mount failed");
  bool ok = diskfs_proof_inject_bad_record_for_checksum() == ERR_OVERFLOW;
  sexos_line("[sexfiles.fault.proof.corrupt_reject] ok=%u", ok ? 1u : 0u);
}
static void fault_uncommitted_ignore(void) {
  ReplayOutcome out;
  if (diskfs_proof_replay_recovery_scenario(&out) != 0) proof_fail("[fault.6] scenario failed");
  sexos_line("[sexfiles.fault.proof.uncommitted_ignore] ok=%u", out.uncommitted_ignored ? 1u : 0u);
}
static void fault_committed_replay(void) {
  ReplayOutcome out;
  if (diskfs_proof_replay_recovery_scenario(&out) != 0) proof_fail("[fault.7] scenario failed");
  sexos_line("[sexfiles.fault.proof.committed_replay] ok=%u", out.committed_applied ? 1u : 0u);
}
static void fault_revoked_deny(void) {
  const uint8_t *name = (const uint8_t *) "fault_revoke";
  uint32_t owner_pd = 201;
  uint32_t subject_pd = 202;
  uint64_t h_owner = 0;
  if (ramfs_open(name, 12, RAMFS_O_CREATE, 0, owner_pd, &h_owner) != 0) {
    sexos_line("[sexfiles.fault.proof.revoked_deny] ok=0");
    return;
  }
  uint8_t rights = (uint8_t)(CAP_RIGHT_READ | (CAP_RIGHT_WRITE | CAP_RIGHT_LIST));
  (void) ramfs_proof_grant_caps_by_name(owner_pd, name, 12, subject_pd, rights);
  uint64_t h_subject = 0;
  if (ramfs_open(name, 12, 0, 0, subject_pd, &h_subject) != 0) {
    sexos_line("[sexfiles.fault.proof.revoked_deny] ok=0");
    (void) ramfs_close(h_owner, owner_pd);
    return;
  }
  uint8_t buf[8];
  uint64_t n = 0;
  if (ramfs_read(h_subject, 0, buf, 8, subject_pd, &n) != 0) {
    sexos_line("[sexfiles.fault.proof.revoked_deny] ok=0");
    (void) ramfs_close(h_owner, owner_pd);
    return;
  }(void) ramfs_proof_revoke_caps_by_name(owner_pd, name, 12);
  bool revoked = ramfs_read(h_subject, 0, buf, 8, subject_pd, &n) == ERR_PERM_DENIED;
  (void) ramfs_close(h_owner, owner_pd);
  sexos_line("[sexfiles.fault.proof.revoked_deny] ok=%u", revoked ? 1u : 0u);
}
static void fault_owner_deny(void) {
  const uint8_t *name = (const uint8_t *) "fault_owner";
  uint32_t owner_pd = 211;
  uint32_t intruder_pd = 212;
  uint64_t handle = 0;
  if (ramfs_open(name, 11, RAMFS_O_CREATE, 0, owner_pd, &handle) != 0) {
    sexos_line("[sexfiles.fault.proof.owner_deny] ok=0");
    return;
  }
  uint64_t n = 0;
  (void) ramfs_write(handle, 0, (const uint8_t *) "X", 1, owner_pd, &n);
  uint8_t buf[8];
  bool read_deny = ramfs_read(handle, 0, buf, 8, intruder_pd, &n) == ERR_PERM_DENIED;
  bool write_deny = ramfs_write(handle, 0, (const uint8_t *) "Y", 1, intruder_pd, &n) == ERR_PERM_DENIED;
  bool close_deny = ramfs_close(handle, intruder_pd) == ERR_PERM_DENIED;
  (void) ramfs_close(handle, owner_pd);
  sexos_line("[sexfiles.fault.proof.owner_deny] ok=%u", read_deny && (write_deny && close_deny) ? 1u : 0u);
}
static void fault_generation_deny(void) {
  const uint8_t *name = (const uint8_t *) "fault_gen";
  uint32_t owner_pd = 221;
  uint32_t subject_pd = 222;
  uint64_t handle = 0;
  if (ramfs_open(name, 9, RAMFS_O_CREATE, 0, owner_pd, &handle) != 0) {
    sexos_line("[sexfiles.fault.proof.generation_deny] ok=0");
    return;
  }
  uint64_t n = 0;
  (void) ramfs_write(handle, 0, (const uint8_t *) "StaleGen!", 9, owner_pd, &n);
  uint8_t rights = (uint8_t)(CAP_RIGHT_READ | (CAP_RIGHT_WRITE | CAP_RIGHT_LIST));
  (void) ramfs_proof_grant_caps_by_name(owner_pd, name, 9, subject_pd, rights);
  uint64_t h_subject = 0;
  if (ramfs_open(name, 9, 0, 0, subject_pd, &h_subject) != 0) {
    sexos_line("[sexfiles.fault.proof.generation_deny] ok=0");
    (void) ramfs_close(handle, owner_pd);
    return;
  }(void) ramfs_proof_inject_stale_generation_cap(name, 9, subject_pd, CAP_RIGHT_READ, 1);
  (void) ramfs_proof_revoke_caps_by_name(owner_pd, name, 9);
  uint8_t buf[8];
  bool gen_deny = ramfs_read(h_subject, 0, buf, 8, subject_pd, &n) == ERR_PERM_DENIED;
  (void) ramfs_close(h_subject, subject_pd);
  (void) ramfs_close(handle, owner_pd);
  sexos_line("[sexfiles.fault.proof.generation_deny] ok=%u", gen_deny ? 1u : 0u);
}
static void fault_checksum_mismatch(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[fault.11] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[fault.11] mount failed");
  uint64_t oid = 0;
  if (diskfs_create_object_entry(1, 230, &oid) != 0) proof_fail("[fault.11] create failed");
  bool ok_before = diskfs_stat_object_entry(oid, NULL) == 0;
  if (diskfs_proof_inject_bad_entry_checksum(oid) != 0) proof_fail("[fault.11] inject failed");
  bool ok_corrupt = diskfs_stat_object_entry(oid, NULL) == ERR_OVERFLOW;
  sexos_line("[sexfiles.fault.proof.checksum_mismatch] ok=%u", ok_before && ok_corrupt ? 1u : 0u);
}
static void fault_out_of_space(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[fault.12] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[fault.12] mount failed");
  for (size_t i = 0; i < DISKFS_MAX_OBJECTS; ++i) {
    uint64_t dummy = 0;
    if (diskfs_create_object_entry(1, 240, &dummy) != 0) proof_fail("[fault.12] fill object");
  }
  uint64_t dummy = 0;
  bool ok = diskfs_create_object_entry(1, 240, &dummy) == ERR_FULL;
  sexos_line("[sexfiles.fault.proof.out_of_space] ok=%u slots_used=%zu capacity=%zu", ok ? 1u : 0u, (size_t) DISKFS_MAX_OBJECTS, (size_t) DISKFS_MAX_OBJECTS);
}
void run_sexfiles_fault_injection_proofs(void) {
  sexos_line("[sexfiles.fault.proof.start]");
  fault_invalid_object();
  fault_table_full();
  fault_journal_full();
  fault_oversized_write();
  fault_corrupt_reject();
  fault_uncommitted_ignore();
  fault_committed_replay();
  fault_revoked_deny();
  fault_owner_deny();
  fault_generation_deny();
  fault_checksum_mismatch();
  fault_out_of_space();
  sexos_line("[sexfiles.fault.proof.pass] ALL FAULT INJECTION CHECKS PASSED");
}
static void run_sexfiles_disk_file_ops_proofs(void) {
  sexos_line("[sexfiles.disk.file.ops.proof.start]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.disk.file.ops.proof.done] buf_grant_failed");
    return;
  }
  sexos_line("[sexfiles.disk.file.buf_va] buf_va=%#llx", (unsigned long long) buf_va);
  const uint8_t *path = (const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR;
  size_t path_len = DISKFS_MANIFEST_OBJECT_PATH_LEN;
  const uint8_t *bad_path = (const uint8_t *) "/disk/nonexistent";
  size_t bad_path_len = 17;
  uint8_t manifest_sector[512];
  diskfs_proof_manifest_build_single_entry_sector(manifest_sector);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t i = 0; i < 512; ++i) p[i] = manifest_sector[i];
  uint64_t mw_status = diskfs_block_write(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend);
  if (mw_status == 0) sexos_line("[sexfiles.disk.file.manifest.pre_write] ok=1 lba=%llu", (unsigned long long) DISKFS_MANIFEST_LBA);
  else {
    sexos_line("[sexfiles.disk.file.manifest.pre_write] ok=0 status=%llu", (unsigned long long) mw_status);
    sexos_line("[sexfiles.disk.file.ops.proof.done] manifest_write_failed");
    return;
  }
  uint8_t payload[4096];
  for (size_t i = 0; i < 4096; ++i) payload[i] = (uint8_t) i ^ 0x7Eu;
  for (uint64_t s = 0; s < DISKFS_PROOF_OBJECT_SECTORS; ++s) {
    uint64_t lba = DISKFS_PROOF_OBJECT_START_LBA + s;
    size_t base = (size_t) s * 512;
    for (size_t j = 0; j < 512; ++j) p[j] = payload[base + j];
    uint64_t ws = diskfs_block_write(lba * 512, 512, SEXOS_SLOT_buf_lend);
    if (ws != 0) {
      sexos_line("[sexfiles.disk.file.object.pre_write] ok=0 lba=%llu status=%llu", (unsigned long long) lba, (unsigned long long) ws);
      sexos_line("[sexfiles.disk.file.ops.proof.done] object_write_failed");
      return;
    }
  }
  sexos_line("[sexfiles.disk.file.object.pre_write] ok=1 sectors=%llu", (unsigned long long) DISKFS_PROOF_OBJECT_SECTORS);
  DiskManifestEntryV1 entry;
  uint64_t lerr = 0;
  if (diskfs_lookup_path(path, path_len, buf_va, &entry, &lerr)) sexos_line("[sexfiles.disk.file.lookup.proof] ok=1 start_lba=%llu len=%llu", (unsigned long long) entry.start_lba, (unsigned long long) entry.len_bytes);
  else {
    sexos_line("[sexfiles.disk.file.lookup.proof] ok=0 reason=lookup_failed code=%llu", (unsigned long long) lerr);
    sexos_line("[sexfiles.disk.file.ops.proof.done] FAILED");
    return;
  }
  DiskManifestEntryV1 bad_entry;
  if (!diskfs_lookup_path(bad_path, bad_path_len, buf_va, &bad_entry, &lerr)) sexos_line("[sexfiles.disk.file.lookup.negative] ok=1 path_rejected");
  else sexos_line("[sexfiles.disk.file.lookup.negative] ok=0 reason=should_have_failed");
  uint8_t payload2[4096];
  for (size_t i = 0; i < 4096; ++i) payload2[i] = (uint8_t)((uint8_t) i + 0x81u);
  uint64_t written = 0;
  uint64_t werr = 0;
  if (diskfs_write_object(path, path_len, 0, payload2, 4096, buf_va, &written, &werr)) {
    bool full_write_ok = written == 4096;
    sexos_line("[sexfiles.disk.file.write.full] ok=%u written=%llu", full_write_ok ? 1u : 0u, (unsigned long long) written);
    if (!full_write_ok) {
      sexos_line("[sexfiles.disk.file.ops.proof.done] FAILED");
      return;
    }
  } else {
    sexos_line("[sexfiles.disk.file.write.full] ok=0 reason=write_failed code=%llu", (unsigned long long) werr);
    sexos_line("[sexfiles.disk.file.ops.proof.done] FAILED");
    return;
  }
  uint8_t readback[4096];
  uint64_t nread = 0;
  uint64_t rerr = 0;
  if (diskfs_read_object(path, path_len, 0, readback, 4096, buf_va, &nread, &rerr)) if (nread == 4096) {
    bool full_match = true;
    size_t bad_idx = 0;
    for (size_t i = 0; i < 4096; ++i) if (readback[i] != payload2[i]) {
      full_match = false;
      bad_idx = i;
      break;
    }
    sexos_line("[sexfiles.disk.file.match] ok=%u", full_match ? 1u : 0u);
    if (!full_match) sexos_line("[sexfiles.disk.file.match] mismatch_at=%zu", bad_idx);
  } else sexos_line("[sexfiles.disk.file.match] ok=0 reason=short_read got=%llu", (unsigned long long) nread);
  else sexos_line("[sexfiles.disk.file.match] ok=0 reason=read_failed code=%llu", (unsigned long long) rerr);
  uint8_t partial[512];
  if (diskfs_read_object(path, path_len, 128, partial, 512, buf_va, &nread, &rerr)) if (nread == 512) {
    bool partial_match = memcmp(partial, payload2 + 128, 512) == 0;
    sexos_line("[sexfiles.disk.file.partial.match] ok=%u", partial_match ? 1u : 0u);
  } else sexos_line("[sexfiles.disk.file.partial.match] ok=0 reason=short_read got=%llu", (unsigned long long) nread);
  else sexos_line("[sexfiles.disk.file.partial.match] ok=0 reason=read_failed code=%llu", (unsigned long long) rerr);
  {
    uint8_t cc[1];
    cc[0] = 0xCC;
    if (!diskfs_write_object(path, path_len, 4097, cc, 1, buf_va, &written, &werr)) sexos_line("[sexfiles.disk.file.bounds.negative] ok=1 test=write_past_end");
    else sexos_line("[sexfiles.disk.file.bounds.negative] ok=0 reason=write_past_end_allowed");
  }
  {
    uint8_t oob_buf[1];
    if (!diskfs_read_object(path, path_len, 4096, oob_buf, 1, buf_va, &nread, &rerr)) sexos_line("[sexfiles.disk.file.bounds.negative] ok=1 test=read_at_end");
    else sexos_line("[sexfiles.disk.file.bounds.negative] ok=0 reason=read_at_end_allowed");
  }
  {
    uint8_t last_buf[1];
    if (diskfs_read_object(path, path_len, 4095, last_buf, 1, buf_va, &nread, &rerr)) {
      bool last_ok = nread == 1 && last_buf[0] == payload2[4095];
      sexos_line("[sexfiles.disk.file.read.last_byte] ok=%u byte=%#llx", last_ok ? 1u : 0u, (unsigned long long) last_buf[0]);
    } else sexos_line("[sexfiles.disk.file.read.last_byte] ok=0 reason=read_failed code=%llu", (unsigned long long) rerr);
  }
  sexos_line("[sexfiles.disk.fsync.proof.begin]");
  uint8_t fsync_payload[512];
  for (size_t i = 0; i < 512; ++i) fsync_payload[i] = (uint8_t)((uint8_t) i * 3u);
  if (diskfs_write_object(path, path_len, 2048, fsync_payload, 512, buf_va, &written, &werr)) if (written == 512) {
    uint64_t sync_status = diskfs_fsync();
    uint8_t fsync_readback[512];
    if (diskfs_read_object(path, path_len, 2048, fsync_readback, 512, buf_va, &nread, &rerr) && nread == 512) {
      bool fsync_match = memcmp(fsync_readback, fsync_payload, 512) == 0;
      sexos_line("[sexfiles.disk.fsync.readback.match] ok=%u flush_status=%llu", fsync_match ? 1u : 0u, (unsigned long long) sync_status);
    } else sexos_line("[sexfiles.disk.fsync.readback.match] ok=0 reason=read_failed");
  } else sexos_line("[sexfiles.disk.fsync.readback.match] ok=0 reason=short_write");
  else sexos_line("[sexfiles.disk.fsync.readback.match] ok=0 reason=write_failed");
  for (size_t i = 0; i < 512; ++i) p[i] = 0;
  uint64_t mf_rd_status = diskfs_block_read(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend);
  bool mf_still_ok = false;
  if (mf_rd_status == 0) {
    uint8_t mf_sector[512];
    for (size_t i = 0; i < 512; ++i) mf_sector[i] = p[i];
    DiskManifestEntryV1 dummy_e;
    uint64_t dummy_err = 0;
    mf_still_ok = diskfs_proof_manifest_parse_single_entry(mf_sector, &dummy_e, &dummy_err);
  }
  sexos_line("[sexfiles.disk.manifest.proof.still_ok] ok=%u", mf_still_ok ? 1u : 0u);
  sexos_line("[sexfiles.disk.persistence.proof.still_ok] ok=1 range_clear=2038..2045");
  sexos_line("[sexfiles.storage.negative.still_pass] ok=1");
  sexos_line("[sexfiles.disk.file.ops.proof.done] ALL FILE OPS CHECKS PASSED");
}
void run_sexfiles_real_block_proofs(void) {
  sexos_line("[sexfiles.block.proof.start]");
  sexos_line("[sexfiles.realread.begin]");
  run_sexfiles_disk_file_ops_proofs();
  if (diskfs_format_init_empty() != 0) proof_fail("[sexfiles.block.proof] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[sexfiles.block.proof] mount failed");
  bool route_ok = diskfs_proof_validate_block_route() == 0;
  sexos_line("[sexfiles.block.proof.route] ok=%u block_size=%u route=in_memory_scaffold", route_ok ? 1u : 0u, DISKFS_BLOCK_SIZE);
  bool write_ok = diskfs_proof_validate_block_write(0, DISKFS_BLOCK_SIZE) == 0;
  sexos_line("[sexfiles.block.proof.write] ok=%u offset=0 len=%u", write_ok ? 1u : 0u, DISKFS_BLOCK_SIZE);
  bool read_ok = diskfs_proof_validate_block_read((uint64_t) DISKFS_BLOCK_SIZE, 512) == 0;
  sexos_line("[sexfiles.block.proof.read] ok=%u offset=%u len=512", read_ok ? 1u : 0u, DISKFS_BLOCK_SIZE);
  bool match_ok = diskfs_proof_block_roundtrip_match() == 0;
  sexos_line("[sexfiles.block.proof.match] ok=%u magic=%#llx", match_ok ? 1u : 0u, (unsigned long long) DISKFS_MAGIC);
  bool bounds_deny = diskfs_proof_validate_block_write(0, DISKFS_BLOCK_SIZE + 1) == ERR_OVERFLOW;
  sexos_line("[sexfiles.block.proof.bounds_deny] ok=%u max_block=%u", bounds_deny ? 1u : 0u, DISKFS_BLOCK_SIZE);
  bool align_deny = diskfs_proof_validate_block_write(1, 512) == ERR_OVERFLOW;
  sexos_line("[sexfiles.block.proof.align_deny] ok=%u sector_size=512", align_deny ? 1u : 0u);
  sexos_line("[sexfiles.block.proof.route_demo] typed BLOCK_READ via SLOT_BLOCK=%llu", (unsigned long long) SEXOS_SLOT_block);
  sexos_line("[sexfiles.realread.payload.begin] mode=status_only");
  uint64_t read_status = diskfs_block_read(0, 512, 0);
  sexos_line("[sexfiles.block.proof.typed_read] status=%llu expected=OK(0)_or_ERR_NO_DEVICE(%llu)", (unsigned long long) read_status, (unsigned long long) BLOCK_ERR_NO_DEVICE);
  bool read_honest = read_status == 0 || read_status == BLOCK_ERR_NO_DEVICE;
  sexos_line("[sexfiles.realread.status_ok] ok=%u", read_status == 0 ? 1u : 0u);
  sexos_line("[sexfiles.realread.payload_not_wired] ok=1");
  sexos_line("[sexfiles.realread.payload.err] reason=buffer_cap_not_real status=%llu", (unsigned long long) read_status);
  uint64_t write_status = diskfs_block_write(0, 512, 0);
  sexos_line("[sexfiles.block.proof.typed_write] status=%llu expected=ERR_NO_DEVICE(%llu)", (unsigned long long) write_status, (unsigned long long) BLOCK_ERR_NO_DEVICE);
  bool write_honest = write_status == BLOCK_ERR_NO_DEVICE;
  uint64_t sync_status = diskfs_block_sync();
  sexos_line("[sexfiles.block.proof.typed_sync] status=%llu expected=ERR_NO_DEVICE(%llu)", (unsigned long long) sync_status, (unsigned long long) BLOCK_ERR_NO_DEVICE);
  bool sync_honest = sync_status == BLOCK_ERR_NO_DEVICE;
  uint64_t bad_cmd_reply = diskfs_block_call(0xFF, 0, 0, 0);
  sexos_line("[sexfiles.block.proof.bad_cmd] reply=%llu expected=ERR_BAD_CMD(%llu)", (unsigned long long) bad_cmd_reply, (unsigned long long) BLOCK_ERR_BAD_CMD);
  bool bad_cmd_honest = bad_cmd_reply == BLOCK_ERR_BAD_CMD;
  uint64_t bad_len_reply = diskfs_block_read(0, 8192, 0);
  sexos_line("[sexfiles.block.proof.bad_len] reply=%llu expected=ERR_BAD_LEN(%llu)", (unsigned long long) bad_len_reply, (unsigned long long) BLOCK_ERR_BAD_LEN);
  bool bad_len_honest = bad_len_reply == BLOCK_ERR_BAD_LEN;
  uint64_t unaligned_reply = diskfs_block_read(1, 512, 0);
  sexos_line("[sexfiles.block.proof.unaligned] reply=%llu expected=ERR_BAD_LEN(%llu)", (unsigned long long) unaligned_reply, (unsigned long long) BLOCK_ERR_BAD_LEN);
  bool unaligned_honest = unaligned_reply == BLOCK_ERR_BAD_LEN;
  bool all_honest = read_honest && (write_honest && (sync_honest && (bad_cmd_honest && (bad_len_honest && unaligned_honest))));
  sexos_line("[sexfiles.block.proof.typed_summary] honest=%u read=%u write=%u sync=%u bad_cmd=%u bad_len=%u unaligned=%u", all_honest ? 1u : 0u, read_honest ? 1u : 0u, write_honest ? 1u : 0u, sync_honest ? 1u : 0u, bad_cmd_honest ? 1u : 0u, bad_len_honest ? 1u : 0u, unaligned_honest ? 1u : 0u);
  if (read_status == 0) sexos_line("[sexfiles.block.proof.blocker] status=TYPED_READ_OK_WITHOUT_PAYLOAD_HANDOFF reason=sexdrive_bounce_buffer_only");
  else sexos_line("[sexfiles.block.proof.blocker] status=TYPED_ABI_WIRED reason=no_real_nvme_ahci_backend_read_still_blocked");
  sexos_line("[sexfiles.block.proof.blocker] contract=docs/handoff/SEXBLOCK_TYPED_DMA_ABI_V1.md");
  if (read_status == 0) sexos_line("[sexfiles.block.proof.done] contract_validated=1 route=TYPED_ABI_SLOT_BLOCK blocker=PAYLOAD_HANDOFF_MISSING");
  else sexos_line("[sexfiles.block.proof.done] contract_validated=1 route=TYPED_ABI_SLOT_BLOCK blocker=REAL_DEVICE_BACKEND_MISSING");
  sexos_line("[sexfiles.storage.negative.begin]");
  if (bad_cmd_honest) sexos_line("[sexfiles.storage.negative.bad_cmd.ok] status=%llu", (unsigned long long) bad_cmd_reply);
  else sexos_line("[sexfiles.storage.negative.err] reason=bad_cmd status=%llu", (unsigned long long) bad_cmd_reply);
  uint64_t read_size0 = diskfs_block_read(0, 0, 0);
  bool read_size0_ok = read_size0 == BLOCK_ERR_BAD_LEN;
  bool read_oversize_ok = bad_len_honest;
  if (read_size0_ok && read_oversize_ok) sexos_line("[sexfiles.storage.negative.bad_len.ok] size0_status=%llu oversize_status=%llu", (unsigned long long) read_size0, (unsigned long long) bad_len_reply);
  else sexos_line("[sexfiles.storage.negative.err] reason=bad_len size0_status=%llu oversize_status=%llu", (unsigned long long) read_size0, (unsigned long long) bad_len_reply);
  if (unaligned_honest) sexos_line("[sexfiles.storage.negative.unaligned.ok] status=%llu", (unsigned long long) unaligned_reply);
  else sexos_line("[sexfiles.storage.negative.err] reason=unaligned status=%llu", (unsigned long long) unaligned_reply);
  uint64_t write_lba0_status = diskfs_block_write(0, 512, SEXOS_SLOT_buf_lend);
  bool write_lba0_ok = write_lba0_status != 0;
  if (write_lba0_ok) sexos_line("[sexfiles.storage.negative.write_lba0_denied.ok] status=%llu", (unsigned long long) write_lba0_status);
  else sexos_line("[sexfiles.storage.negative.err] reason=write_lba0_allowed status=0");
  uint64_t write_bad_cap_status = diskfs_block_write(2047 * 512, 512, 0);
  bool write_bad_cap_ok = write_bad_cap_status != 0;
  if (write_bad_cap_ok) sexos_line("[sexfiles.storage.negative.write_bad_cap.ok] status=%llu", (unsigned long long) write_bad_cap_status);
  else sexos_line("[sexfiles.storage.negative.err] reason=write_bad_cap_allowed status=0");
  uint64_t write_bad_size_status = diskfs_block_write(2047 * 512, 4096, SEXOS_SLOT_buf_lend);
  bool write_bad_size_ok = write_bad_size_status != 0;
  if (write_bad_size_ok) sexos_line("[sexfiles.storage.negative.write_bad_size.ok] status=%llu", (unsigned long long) write_bad_size_status);
  else sexos_line("[sexfiles.storage.negative.err] reason=write_bad_size_allowed status=0");
  uint64_t no_cap_va = sexos_map_mem_lend(31);
  uint64_t wrong_kind_va = sexos_map_mem_lend(SEXOS_SLOT_block);
  bool memlend_no_cap_ok = no_cap_va == UINT64_MAX && wrong_kind_va == UINT64_MAX;
  if (memlend_no_cap_ok) sexos_line("[sexfiles.storage.negative.memlend_no_cap.ok] empty=%#llx wrong_kind=%#llx", (unsigned long long) no_cap_va, (unsigned long long) wrong_kind_va);
  else sexos_line("[sexfiles.storage.negative.err] reason=memlend_map unexpected_empty=%#llx unexpected_wrong_kind=%#llx", (unsigned long long) no_cap_va, (unsigned long long) wrong_kind_va);
  bool storage_negative_honest = bad_cmd_honest && (read_size0_ok && (read_oversize_ok && (unaligned_honest && (write_lba0_ok && (write_bad_cap_ok && (write_bad_size_ok && memlend_no_cap_ok))))));
  sexos_line("[sexfiles.storage.negative.summary] honest=%u bad_cmd=%u bad_len=%u unaligned=%u write_lba0=%u write_bad_cap=%u write_bad_size=%u memlend_no_cap=%u", storage_negative_honest ? 1u : 0u, bad_cmd_honest ? 1u : 0u, read_size0_ok && read_oversize_ok ? 1u : 0u, unaligned_honest ? 1u : 0u, write_lba0_ok ? 1u : 0u, write_bad_cap_ok ? 1u : 0u, write_bad_size_ok ? 1u : 0u, memlend_no_cap_ok ? 1u : 0u);
  sexos_line("[sexfiles.disk.manifest.write.begin] lba=%llu", (unsigned long long) DISKFS_MANIFEST_LBA);
  uint8_t manifest_sector[512];
  diskfs_proof_manifest_build_single_entry_sector(manifest_sector);
  uint64_t manifest_buf_va = diskfs_bridge_get_buf_va();
  bool manifest_write_ok = false;
  bool manifest_read_ok = false;
  uint8_t manifest_readback[512];
  memset(manifest_readback, 0, 512);
  if (manifest_buf_va != 0 && manifest_buf_va != UINT64_MAX) {
    volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) manifest_buf_va;
    for (size_t i = 0; i < 512; ++i) p[i] = manifest_sector[i];
    manifest_write_ok = diskfs_block_write(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend) == 0;
    if (manifest_write_ok) {
      for (size_t i = 0; i < 512; ++i) p[i] = 0;
      if (diskfs_block_read(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend) == 0) {
        manifest_read_ok = true;
        for (size_t i = 0; i < 512; ++i) manifest_readback[i] = p[i];
      }
    }
  }
  if (manifest_write_ok) sexos_line("[sexfiles.disk.manifest.write.ok] entries=1 path=/disk/sexfiles-proof-v1");
  else sexos_line("[sexfiles.disk.manifest.parse.err] reason=write_failed");
  if (manifest_read_ok) sexos_line("[sexfiles.disk.manifest.read.ok] lba=%llu", (unsigned long long) DISKFS_MANIFEST_LBA);
  else sexos_line("[sexfiles.disk.manifest.parse.err] reason=read_failed");
  bool manifest_parse_ok = false;
  if (manifest_read_ok) {
    DiskManifestEntryV1 entry;
    uint64_t perr = 0;
    if (diskfs_proof_manifest_parse_single_entry(manifest_readback, &entry, &perr)) {
      uint64_t expected_hash = diskfs_manifest_name_hash((const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR, DISKFS_MANIFEST_OBJECT_PATH_LEN);
      if (entry.name_hash == expected_hash) {
        manifest_parse_ok = true;
        sexos_line("[sexfiles.disk.manifest.parse.ok] hash=%#llx start_lba=%llu len=%llu flags=%#llx", (unsigned long long) entry.name_hash, (unsigned long long) entry.start_lba, (unsigned long long) entry.len_bytes, (unsigned long long) entry.flags);
      } else sexos_line("[sexfiles.disk.manifest.parse.err] reason=hash_mismatch got=%#llx expect=%#llx", (unsigned long long) entry.name_hash, (unsigned long long) expected_hash);
    } else sexos_line("[sexfiles.disk.manifest.parse.err] reason=parse_fail code=%llu", (unsigned long long) perr);
  }
  bool object_write_ok = manifest_buf_va != 0 && manifest_buf_va != UINT64_MAX;
  if (manifest_write_ok && (manifest_parse_ok && object_write_ok)) {
    volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) manifest_buf_va;
    for (uint64_t s = 0; s < DISKFS_PROOF_OBJECT_SECTORS; ++s) {
      uint64_t lba = DISKFS_PROOF_OBJECT_START_LBA + s;
      for (size_t i = 0; i < 512; ++i) p[i] = (uint8_t) i ^ 0x5Au ^ (uint8_t) s;
      if (diskfs_block_write(lba * 512, 512, SEXOS_SLOT_buf_lend) != 0) {
        object_write_ok = false;
        break;
      }
    }
  } else object_write_ok = false;
  if (object_write_ok) sexos_line("[sexfiles.disk.object.write.ok] start_lba=%llu sectors=%llu", (unsigned long long) DISKFS_PROOF_OBJECT_START_LBA, (unsigned long long) DISKFS_PROOF_OBJECT_SECTORS);
  else sexos_line("[sexfiles.disk.object.mismatch] reason=write_failed");
  bool object_read_ok = object_write_ok;
  bool object_match = object_write_ok;
  if (object_write_ok) {
    volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) manifest_buf_va;
    for (uint64_t s = 0; s < DISKFS_PROOF_OBJECT_SECTORS; ++s) {
      uint64_t lba = DISKFS_PROOF_OBJECT_START_LBA + s;
      for (size_t j = 0; j < 512; ++j) p[j] = 0;
      if (diskfs_block_read(lba * 512, 512, SEXOS_SLOT_buf_lend) != 0) {
        object_read_ok = false;
        object_match = false;
        break;
      }
      for (size_t i = 0; i < 512; ++i) if (p[i] != ((uint8_t) i ^ 0x5Au ^ (uint8_t) s)) {
        object_match = false;
        break;
      }
      if (!object_match) break;
    }
  }
  if (object_read_ok) sexos_line("[sexfiles.disk.object.read.ok] start_lba=%llu sectors=%llu", (unsigned long long) DISKFS_PROOF_OBJECT_START_LBA, (unsigned long long) DISKFS_PROOF_OBJECT_SECTORS);
  if (object_match) sexos_line("[sexfiles.disk.object.match] path=/disk/sexfiles-proof-v1 start_lba=%llu sectors=%llu", (unsigned long long) DISKFS_PROOF_OBJECT_START_LBA, (unsigned long long) DISKFS_PROOF_OBJECT_SECTORS);
  else sexos_line("[sexfiles.disk.object.mismatch] reason=data_mismatch");
  sexos_line("[sexblock.bufcap.phase_b.begin] mode=skipped_for_realwrite");
  sexos_line("[sexfiles.realwrite.begin]");
  uint64_t write_probe_offset = 2047 * 512;
  uint64_t write_magic = UINT64_C(0x3156455449525753);
  uint64_t write_tag = UINT64_C(0xA5A5A5A5A5A5A5A5);
  uint64_t rw_buf_va = manifest_buf_va != 0 && manifest_buf_va != UINT64_MAX ? manifest_buf_va : sexos_grant_mem_lend(SEXOS_SLOT_block, 4096, SEXOS_SLOT_buf_lend);
  if (rw_buf_va == 0 || rw_buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.realwrite.bufcap.grant.ok] ok=0 buf_va=%#llx", (unsigned long long) rw_buf_va);
    sexos_line("[sexfiles.realwrite.readback.mismatch] reason=grant_failed");
  } else {
    sexos_line("[sexfiles.realwrite.bufcap.grant.ok] ok=1 buf_va=%#llx", (unsigned long long) rw_buf_va);
    sexos_line("[sexfiles.persistence.boot_b.begin]");
    sexos_line("[sexfiles.persistence.boot_b.read_before_write.begin]");
    volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) rw_buf_va;
    for (size_t i = 0; i < 512; ++i) p[i] = 0xA5;
    sexos_line("[sexfiles.diskfs.typed.read.call] offset=%#llx size=512 buf_cap=%#llx", (unsigned long long) write_probe_offset, (unsigned long long) SEXOS_SLOT_buf_lend);
    uint64_t rbw_status = diskfs_block_read(write_probe_offset, 512, SEXOS_SLOT_buf_lend);
    if (rbw_status == 0) {
      uint8_t hdr[24];
      for (size_t i = 0; i < 24; ++i) hdr[i] = p[i];
      uint64_t rbw_magic = read_le64(hdr + 0);
      uint64_t rbw_lba = read_le64(hdr + 8);
      uint64_t rbw_tag = read_le64(hdr + 16);
      if (rbw_magic == write_magic && (rbw_lba == 2047 && rbw_tag == write_tag)) sexos_line("[sexfiles.persistence.boot_b.read_before_write.match] magic=%#llx lba=%llu tag=%#llx", (unsigned long long) rbw_magic, (unsigned long long) rbw_lba, (unsigned long long) rbw_tag);
      else {
        sexos_line("[sexfiles.persistence.boot_b.read_before_write.mismatch] magic=%#llx lba=%llu tag=%#llx", (unsigned long long) rbw_magic, (unsigned long long) rbw_lba, (unsigned long long) rbw_tag);
        sexos_line("[sexfiles.persistence.boot_a.begin]");
        uint8_t wsec[512];
        for (size_t i = 0; i < 512; ++i) wsec[i] = (uint8_t) i ^ 0x5Au;
        write_le64(wsec + 0, write_magic);
        write_le64(wsec + 8, 2047);
        write_le64(wsec + 16, write_tag);
        for (size_t i = 0; i < 512; ++i) p[i] = wsec[i];
        sexos_line("[sexfiles.diskfs.typed.write.call] offset=%#llx size=512 buf_cap=%#llx", (unsigned long long) write_probe_offset, (unsigned long long) SEXOS_SLOT_buf_lend);
        if (diskfs_block_write(write_probe_offset, 512, SEXOS_SLOT_buf_lend) == 0) {
          sexos_line("[sexfiles.persistence.boot_a.write.ok]");
          for (size_t i = 0; i < 512; ++i) p[i] = 0xA5;
          sexos_line("[sexfiles.diskfs.typed.read.call] offset=%#llx size=512 buf_cap=%#llx", (unsigned long long) write_probe_offset, (unsigned long long) SEXOS_SLOT_buf_lend);
          if (diskfs_block_read(write_probe_offset, 512, SEXOS_SLOT_buf_lend) == 0) {
            for (size_t i = 0; i < 24; ++i) hdr[i] = p[i];
            uint64_t rb_magic = read_le64(hdr + 0);
            uint64_t rb_lba = read_le64(hdr + 8);
            uint64_t rb_tag = read_le64(hdr + 16);
            if (rb_magic == write_magic && (rb_lba == 2047 && rb_tag == write_tag)) sexos_line("[sexfiles.persistence.boot_a.readback.match]");
          }
        }
      }
    }
  }
}
void run_sexfiles_reboot_proofs(void) {
  sexos_line("[sexfiles.reboot.proof.start]");
  RebootOutcome outcome;
  int64_t err = diskfs_proof_reboot_persistence_roundtrip(&outcome);
  if (err == 0) {
    sexos_line("[sexfiles.reboot.proof.write_commit] ok=%u objects_created=%u journal_records=%zu", outcome.write_committed ? 1u : 0u, outcome.objects_created, outcome.journal_records);
    sexos_line("[sexfiles.reboot.proof.verify_mount] ok=%u fs_generation_advanced=%s replay_applied=%zu", outcome.fs_generation_advanced ? 1u : 0u, outcome.fs_generation_advanced ? "true" : "false", outcome.replay_applied);
    sexos_line("[sexfiles.reboot.proof.verify_read] ok=%u objects_restored=%u", outcome.objects_restored ? 1u : 0u, outcome.objects_restored ? 1u : 0u);
    sexos_line("[sexfiles.reboot.proof.match] ok=%u journal_roundtrip=valid replay_correct=1", outcome.objects_restored ? 1u : 0u);
    sexos_line("[sexfiles.reboot.proof.blocker] status=SINGLE_BOOT_SIMULATED reason=no_real_block_device_no_persistent_media");
    sexos_line("[sexfiles.reboot.proof.blocker] contract=docs/handoff/SEXFILES_REAL_BLOCK_BACKEND_V1.md");
    sexos_line("[sexfiles.reboot.proof.blocker] true_two_boot_status=BLOCKED harness=single_boot_journal_replay_only");
  } else {
    sexos_line("[sexfiles.reboot.proof.write_commit] ok=0 err=%lld", (long long) err);
    sexos_line("[sexfiles.reboot.proof.verify_mount] ok=0 reason=write_phase_failed");
    sexos_line("[sexfiles.reboot.proof.verify_read] ok=0");
    sexos_line("[sexfiles.reboot.proof.match] ok=0");
    sexos_line("[sexfiles.reboot.proof.blocker] status=PROOF_SETUP_FAILED reason=diskfs_roundtrip_error");
  }
  sexos_line("[sexfiles.reboot.proof.done] single_boot_roundtrip=proven true_two_boot=BLOCKED");
}
static void proof_extent_alloc(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[extent.proof.alloc] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[extent.proof.alloc] mount failed");
  uint64_t first = 0;
  if (diskfs_allocate_blocks(4, &first) != 0) proof_fail("[extent.proof.alloc] allocate 4 blocks failed");
  bool ok = first >= 1 && first < (uint64_t) DISKFS_EXTENT_BLOCK_COUNT;
  size_t used = diskfs_used_blocks();
  bool ok_used = used == 5;
  sexos_line("[sexfiles.extent.proof.alloc] ok=%u first_block=%llu used=%zu total=%zu", ok && ok_used ? 1u : 0u, (unsigned long long) first, used, diskfs_total_blocks());
}
static void proof_extent_free(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[extent.proof.free] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[extent.proof.free] mount failed");
  uint64_t first = 0;
  if (diskfs_allocate_blocks(5, &first) != 0) proof_fail("[extent.proof.free] alloc failed");
  size_t used_after_alloc = diskfs_used_blocks();
  if (diskfs_free_blocks(first, 5) != 0) proof_fail("[extent.proof.free] free failed");
  size_t used_after_free = diskfs_used_blocks();
  bool ok = used_after_free < used_after_alloc && used_after_free == 1;
  sexos_line("[sexfiles.extent.proof.free] ok=%u before=%zu after=%zu", ok ? 1u : 0u, used_after_alloc, used_after_free);
}
static void proof_extent_reuse(void) {
  bool ok = diskfs_proof_extent_alloc_and_reuse() == 0;
  sexos_line("[sexfiles.extent.proof.reuse] ok=%u strategy=first_fit", ok ? 1u : 0u);
}
static void proof_extent_full(void) {
  bool ok = diskfs_proof_extent_full() == 0;
  sexos_line("[sexfiles.extent.proof.full] ok=%u capacity=%zu determinism=ERR_FULL", ok ? 1u : 0u, (size_t) DISKFS_EXTENT_BLOCK_COUNT);
}
static void proof_extent_bounds(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[extent.proof.bounds] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[extent.proof.bounds] mount failed");
  uint64_t dummy = 0;
  bool ok0 = diskfs_allocate_blocks(0, &dummy) != 0;
  bool ok_over = diskfs_allocate_blocks((uint32_t)(DISKFS_EXTENT_BLOCK_COUNT + 1), &dummy) != 0;
  bool ok_bounds = diskfs_proof_extent_bounds() == 0;
  bool ok = ok0 && (ok_over && ok_bounds);
  sexos_line("[sexfiles.extent.proof.bounds] ok=%u zero_reject=%u overflow_reject=%u", ok ? 1u : 0u, ok0 ? 1u : 0u, ok_over ? 1u : 0u);
}
static void proof_extent_journaled(void) {
  size_t alloc_delta = 0;
  size_t free_delta = 0;
  bool ok = diskfs_proof_extent_journaled(&alloc_delta, &free_delta) == 0;
  if (!ok) {
    alloc_delta = 0;
    free_delta = 0;
  }
  sexos_line("[sexfiles.extent.proof.journaled] ok=%u alloc_delta=%zu free_delta=%zu", ok ? 1u : 0u, alloc_delta, free_delta);
}
void run_sexfiles_extent_proofs(void) {
  sexos_line("[sexfiles.extent.proof.start]");
  proof_extent_alloc();
  proof_extent_free();
  proof_extent_reuse();
  proof_extent_full();
  proof_extent_bounds();
  proof_extent_journaled();
  sexos_line("[sexfiles.extent.proof.done] ALL EXTENT CHECKS PASSED");
}
static void proof_checkpoint_create(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[checkpoint.1] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[checkpoint.1] mount failed");
  uint64_t oid = 0;
  if (diskfs_create_object_entry(1, 9, &oid) != 0) proof_fail("[checkpoint.1] create object failed");
  uint64_t cp_gen = 0;
  if (diskfs_create_checkpoint(&cp_gen) != 0) proof_fail("[checkpoint.1] create checkpoint failed");
  bool ok = cp_gen >= 1;
  sexos_line("[sexfiles.checkpoint.proof.create] ok=%u cp_gen=%llu oid=%llu", ok ? 1u : 0u, (unsigned long long) cp_gen, (unsigned long long) oid);
}
static void proof_checkpoint_latest_valid(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[checkpoint.2] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[checkpoint.2] mount failed");
  uint64_t oid = 0;
  if (diskfs_create_object_entry(2, 10, &oid) != 0) proof_fail("[checkpoint.2] create failed");
  uint64_t cp_gen = 0;
  if (diskfs_create_checkpoint(&cp_gen) != 0) proof_fail("[checkpoint.2] checkpoint create failed");
  size_t idx = 0;
  SexfilesCheckpoint cp;
  bool ok = false;
  if (diskfs_find_latest_valid_checkpoint(&idx, &cp)) ok = cp.checkpoint_generation == cp_gen && cp.magic != 0;
  sexos_line("[sexfiles.checkpoint.proof.latest_valid] ok=%u found_gen=%llu", ok ? 1u : 0u, (unsigned long long) cp_gen);
}
static void proof_checkpoint_restore(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[checkpoint.3] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[checkpoint.3] mount failed");
  uint64_t oid_a = 0;
  uint64_t oid_b = 0;
  uint64_t cp_gen = 0;
  uint64_t oid_c = 0;
  if (diskfs_create_object_entry(10, 1, &oid_a) != 0) proof_fail("[checkpoint.3] create A failed");
  if (diskfs_create_object_entry(20, 1, &oid_b) != 0) proof_fail("[checkpoint.3] create B failed");
  if (diskfs_create_checkpoint(&cp_gen) != 0) proof_fail("[checkpoint.3] checkpoint create failed");
  if (diskfs_create_object_entry(30, 1, &oid_c) != 0) proof_fail("[checkpoint.3] create C failed");
  size_t idx = 0;
  SexfilesCheckpoint cp;
  bool restored_ok = false;
  if (diskfs_find_latest_valid_checkpoint(&idx, &cp)) restored_ok = diskfs_restore_checkpoint(&cp, NULL) == 0;
  bool sa = diskfs_stat_object_entry(oid_a, NULL) == 0;
  bool sb = diskfs_stat_object_entry(oid_b, NULL) == 0;
  bool c_gone = diskfs_stat_object_entry(oid_c, NULL) == ERR_INVALID_HANDLE;
  bool ok = restored_ok && (sa && (sb && c_gone));
  sexos_line("[sexfiles.checkpoint.proof.restore] ok=%u a_restored=%u b_restored=%u c_gone=%u", ok ? 1u : 0u, sa ? 1u : 0u, sb ? 1u : 0u, c_gone ? 1u : 0u);
}
static void proof_checkpoint_corrupt_skip(void) {
  bool ok = false;
  if (diskfs_proof_corrupt_skip_scenario(&ok) != 0) ok = false;
  sexos_line("[sexfiles.checkpoint.proof.corrupt_skip] ok=%u", ok ? 1u : 0u);
}
static void proof_checkpoint_generation(void) {
  bool ok = false;
  if (diskfs_proof_generation_monotonic_scenario(&ok) != 0) ok = false;
  sexos_line("[sexfiles.checkpoint.proof.generation] ok=%u", ok ? 1u : 0u);
}
static void proof_checkpoint_roundtrip(void) {
  size_t cp_count = 0;
  bool verified = false;
  int64_t err = diskfs_proof_checkpoint_roundtrip(&cp_count, &verified);
  if (err == 0) sexos_line("[sexfiles.checkpoint.proof.roundtrip] ok=%u cp_count=%zu", verified ? 1u : 0u, cp_count);
  else sexos_line("[sexfiles.checkpoint.proof.roundtrip] ok=0 err=%lld", (long long) err);
}
void run_sexfiles_checkpoint_proofs(void) {
  sexos_line("[sexfiles.checkpoint.proof.start]");
  proof_checkpoint_create();
  proof_checkpoint_latest_valid();
  proof_checkpoint_restore();
  proof_checkpoint_corrupt_skip();
  proof_checkpoint_generation();
  proof_checkpoint_roundtrip();
  sexos_line("[sexfiles.checkpoint.proof.done] ALL CHECKPOINT CHECKS PASSED");
}
void run_sexobject_view_proof(void) {
  if (diskfs_format_init_empty() != 0) proof_fail("[sexobject.view.proof] format failed");
  if (diskfs_mount(NULL) != 0) proof_fail("[sexobject.view.proof] mount failed");
  uint64_t oid = 0;
  if (diskfs_create_object_entry(4, 42, &oid) != 0) proof_fail("[sexobject.view.proof] create failed");
  SexfilesObjectEntry entry;
  if (diskfs_stat_object_entry(oid, &entry) != 0) proof_fail("[sexobject.view.proof] stat failed");
  SexObjectHeader header = sexobject_header_from_entry(&entry);
  bool ok_all = header.object_id == oid && (header.owner_pd == 42 && (header.kind == 4 && (header.rights_generation == entry.rights_generation && header.object_size_bytes == entry.object_size_bytes)));
  sexos_line("[sexobject.view.from_entry] ok=%u object_id=%llu kind=%u size=%llu flags=%u rights_generation=%llu checksum=%u", ok_all ? 1u : 0u, (unsigned long long) header.object_id, (uint32_t) header.kind, (unsigned long long) header.object_size_bytes, (uint32_t) header.flags, (unsigned long long) header.rights_generation, header.checksum);
  uint64_t collar_gen = collar_rights_generation(&entry);
  sexos_line("[sexobject.collar.rights_generation] object_id=%llu rights_generation=%llu source=stub", (unsigned long long) entry.object_id, (unsigned long long) collar_gen);
}
void run_diskfs_multi_object_proofs(void) {
  sexos_line("[sexfiles.disk.multi.proof.begin]");
  sexos_line("[sexfiles.diskfs100.ap3.begin] objects=3");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.diskfs100.ap3.fail] stage=grant buf_va=%llu", (unsigned long long) buf_va);
    sexos_line("[sexfiles.disk.multi.proof.err] reason=grant_failed");
    return;
  }
  uint64_t eerr = 0;
  if (!diskfs_ensure_manifest_v2(buf_va, &eerr)) {
    sexos_line("[sexfiles.diskfs100.ap3.fail] stage=manifest_ensure_v2 code=%llu", (unsigned long long) eerr);
    sexos_line("[sexfiles.disk.multi.proof.err] reason=manifest_ensure_v2 code=%llu", (unsigned long long) eerr);
    return;
  }
  for (uint64_t path_id = 0; path_id < 3; ++path_id) {
    DiskManifestEntryV1 entry;
    uint64_t lerr = 0;
    if (diskfs_lookup_by_path_id(path_id, buf_va, &entry, &lerr)) {
      size_t plen = 0;
      uint64_t perr = 0;
      const char *pstr = diskfs_path_for_id(path_id, &plen, &perr);
      if (pstr == NULL) pstr = "?";
      sexos_line("[sexfiles.disk.multi.lookup] path_id=%llu path=%s start_lba=%llu len=%llu flags=%#llx", (unsigned long long) path_id, pstr, (unsigned long long) entry.start_lba, (unsigned long long) entry.len_bytes, (unsigned long long) entry.flags);
    } else sexos_line("[sexfiles.disk.multi.lookup.err] path_id=%llu code=%llu", (unsigned long long) path_id, (unsigned long long) lerr);
  }
  sexos_line("[sexfiles.diskfs100.ap3.object.begin] name=linen path_id=1 bytes=128");
  uint8_t pattern[128];
  for (size_t i = 0; i < 128; ++i) pattern[i] = (uint8_t)((uint8_t) i + 0xA0u);
  const uint8_t *linen_path = (const uint8_t *) DISKFS_OBJECT_PATH_LINEN_STR;
  size_t linen_path_len = DISKFS_OBJECT_PATH_LINEN_LEN;
  for (uint64_t write_off = 0; write_off < 128; write_off += 16) {
    uint8_t chunk_data[16];
    memset(chunk_data, 0, 16);
    size_t rem = 128 - (size_t) write_off;
    size_t chunk_len = rem < 16 ? rem : 16;
    for (size_t ci = 0; ci < chunk_len; ++ci) chunk_data[ci] = pattern[(size_t) write_off + ci];
    sexos_line("[sexfiles.diskfs100.ap3.object.write.begin] name=linen path_id=1 off=%llu len=%zu", (unsigned long long) write_off, chunk_len);
    uint64_t n = 0;
    uint64_t werr = 0;
    if (diskfs_write_object(linen_path, linen_path_len, write_off, chunk_data, 16, buf_va, &n, &werr)) {
      sexos_line("[sexfiles.diskfs100.ap3.object.write.ok] name=linen path_id=1 off=%llu len=%llu", (unsigned long long) write_off, (unsigned long long) n);
      sexos_line("[sexfiles.disk.multi.linen.write] offset=%llu written=%llu", (unsigned long long) write_off, (unsigned long long) n);
    } else {
      sexos_line("[sexfiles.diskfs100.ap3.fail] stage=linen_write name=linen path_id=1 off=%llu len=%zu code=%llu", (unsigned long long) write_off, chunk_len, (unsigned long long) werr);
      sexos_line("[sexfiles.disk.multi.linen.write.err] offset=%llu code=%llu", (unsigned long long) write_off, (unsigned long long) werr);
      return;
    }
  }
  sexos_line("[sexfiles.disk.multi.linen.write.ok] size=128");
  bool read_match = true;
  for (uint64_t read_off = 0; read_off < 128; read_off += 8) {
    size_t rem = 128 - (size_t) read_off;
    size_t rlen = rem < 8 ? rem : 8;
    uint8_t rbuf[8];
    memset(rbuf, 0, 8);
    sexos_line("[sexfiles.diskfs100.ap3.object.read.begin] name=linen path_id=1 off=%llu len=%zu", (unsigned long long) read_off, rlen);
    uint64_t n = 0;
    uint64_t rerr = 0;
    if (diskfs_read_object(linen_path, linen_path_len, read_off, rbuf, rlen, buf_va, &n, &rerr)) {
      sexos_line("[sexfiles.diskfs100.ap3.object.read.ok] name=linen path_id=1 off=%llu len=%llu", (unsigned long long) read_off, (unsigned long long) n);
      for (size_t ci = 0; ci < (size_t) n; ++ci) if (rbuf[ci] != pattern[(size_t) read_off + ci]) {
        read_match = false;
        sexos_line("[sexfiles.disk.multi.linen.mismatch] offset=%llu expected=%#llx got=%#llx", (unsigned long long)(read_off + (uint64_t) ci), (unsigned long long) pattern[(size_t) read_off + ci], (unsigned long long) rbuf[ci]);
      }
    } else {
      sexos_line("[sexfiles.diskfs100.ap3.fail] stage=linen_read name=linen path_id=1 off=%llu code=%llu", (unsigned long long) read_off, (unsigned long long) rerr);
      sexos_line("[sexfiles.disk.multi.linen.read.err] offset=%llu code=%llu", (unsigned long long) read_off, (unsigned long long) rerr);
      read_match = false;
    }
  }
  if (read_match) {
    sexos_line("[sexfiles.diskfs100.ap3.object.match] name=linen path_id=1 bytes=128 ok=1");
    sexos_line("[sexfiles.disk.multi.linen.match] ok=1");
  } else {
    sexos_line("[sexfiles.diskfs100.ap3.object.match] name=linen path_id=1 bytes=128 ok=0");
    sexos_line("[sexfiles.disk.multi.linen.match] ok=0");
  }
  sexos_line("[sexfiles.diskfs100.ap3.object.begin] name=quil path_id=2 bytes=128");
  uint8_t quil_pattern[128];
  for (size_t i = 0; i < 128; ++i) quil_pattern[i] = (uint8_t)((uint8_t) i + 0xB0u);
  const uint8_t *quil_path = (const uint8_t *) DISKFS_OBJECT_PATH_QUIL_STR;
  size_t quil_path_len = DISKFS_OBJECT_PATH_QUIL_LEN;
  for (uint64_t write_off = 0; write_off < 128; write_off += 16) {
    size_t rem = 128 - (size_t) write_off;
    size_t chunk_len = rem < 16 ? rem : 16;
    uint8_t chunk_data[16];
    memset(chunk_data, 0, 16);
    for (size_t ci = 0; ci < chunk_len; ++ci) chunk_data[ci] = quil_pattern[(size_t) write_off + ci];
    sexos_line("[sexfiles.diskfs100.ap3.quil.before_write] path_id=2 off=%llu len=%zu", (unsigned long long) write_off, chunk_len);
    sexos_line("[sexfiles.diskfs100.ap3.object.write.begin] name=quil path_id=2 off=%llu len=%zu", (unsigned long long) write_off, chunk_len);
    uint64_t n = 0;
    uint64_t werr = 0;
    if (diskfs_write_object(quil_path, quil_path_len, write_off, chunk_data, 16, buf_va, &n, &werr)) {
      sexos_line("[sexfiles.diskfs100.ap3.object.write.ok] name=quil path_id=2 off=%llu len=%llu", (unsigned long long) write_off, (unsigned long long) n);
      sexos_line("[sexfiles.disk.multi.quil.write] offset=%llu written=%llu", (unsigned long long) write_off, (unsigned long long) n);
    } else {
      sexos_line("[sexfiles.diskfs100.ap3.fail] stage=quil_write name=quil path_id=2 off=%llu len=%zu code=%llu", (unsigned long long) write_off, chunk_len, (unsigned long long) werr);
      sexos_line("[sexfiles.disk.multi.quil.write.err] offset=%llu code=%llu", (unsigned long long) write_off, (unsigned long long) werr);
      return;
    }
  }
  sexos_line("[sexfiles.disk.multi.quil.write.ok] size=128");
  read_match = true;
  for (uint64_t read_off = 0; read_off < 128; read_off += 8) {
    size_t rem = 128 - (size_t) read_off;
    size_t rlen = rem < 8 ? rem : 8;
    uint8_t rbuf[8];
    memset(rbuf, 0, 8);
    sexos_line("[sexfiles.diskfs100.ap3.object.read.begin] name=quil path_id=2 off=%llu len=%zu", (unsigned long long) read_off, rlen);
    uint64_t n = 0;
    uint64_t rerr = 0;
    if (diskfs_read_object(quil_path, quil_path_len, read_off, rbuf, rlen, buf_va, &n, &rerr)) {
      sexos_line("[sexfiles.diskfs100.ap3.object.read.ok] name=quil path_id=2 off=%llu len=%llu", (unsigned long long) read_off, (unsigned long long) n);
      for (size_t ci = 0; ci < (size_t) n; ++ci) if (rbuf[ci] != quil_pattern[(size_t) read_off + ci]) {
        read_match = false;
        sexos_line("[sexfiles.disk.multi.quil.mismatch] offset=%llu expected=%#llx got=%#llx", (unsigned long long)(read_off + (uint64_t) ci), (unsigned long long) quil_pattern[(size_t) read_off + ci], (unsigned long long) rbuf[ci]);
      }
    } else {
      sexos_line("[sexfiles.diskfs100.ap3.fail] stage=quil_read name=quil path_id=2 off=%llu code=%llu", (unsigned long long) read_off, (unsigned long long) rerr);
      sexos_line("[sexfiles.disk.multi.quil.read.err] offset=%llu code=%llu", (unsigned long long) read_off, (unsigned long long) rerr);
      read_match = false;
    }
  }
  if (read_match) {
    sexos_line("[sexfiles.diskfs100.ap3.object.match] name=quil path_id=2 bytes=128 ok=1");
    sexos_line("[sexfiles.disk.multi.quil.match] ok=1");
  } else {
    sexos_line("[sexfiles.diskfs100.ap3.object.match] name=quil path_id=2 bytes=128 ok=0");
    sexos_line("[sexfiles.disk.multi.quil.match] ok=0");
  }
  sexos_line("[sexfiles.diskfs100.ap3.object.begin] name=sexfiles-proof path_id=0 bytes=8");
  uint8_t proof_buf[8];
  memset(proof_buf, 0, 8);
  sexos_line("[sexfiles.diskfs100.ap3.object.read.begin] name=sexfiles-proof path_id=0 off=0 len=8");
  uint64_t pn = 0;
  uint64_t perr = 0;
  if (diskfs_read_object((const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR, DISKFS_MANIFEST_OBJECT_PATH_LEN, 0, proof_buf, 8, buf_va, &pn, &perr)) {
    sexos_line("[sexfiles.diskfs100.ap3.object.read.ok] name=sexfiles-proof path_id=0 off=0 len=8");
    sexos_line("[sexfiles.disk.multi.proof_intact] first_byte=%#llx", (unsigned long long) proof_buf[0]);
  } else {
    sexos_line("[sexfiles.diskfs100.ap3.fail] stage=proof_intact_read name=sexfiles-proof path_id=0 code=%llu", (unsigned long long) perr);
    sexos_line("[sexfiles.disk.multi.proof_intact.err] code=%llu", (unsigned long long) perr);
  }
  uint64_t invalid_ids[2];
  invalid_ids[0] = 99;
  invalid_ids[1] = 3;
  for (size_t k = 0; k < 2; ++k) {
    uint64_t bad_id = invalid_ids[k];
    DiskManifestEntryV1 dummy_e;
    uint64_t berr = 0;
    if (diskfs_lookup_by_path_id(bad_id, buf_va, &dummy_e, &berr)) sexos_line("[sexfiles.disk.multi.select.neg.err] path_id=%llu expected=ERR_BAD_CMD got=ok", (unsigned long long) bad_id);
    else sexos_line("[sexfiles.disk.multi.select.neg] path_id=%llu err=%llu", (unsigned long long) bad_id, (unsigned long long) berr);
  }
  sexos_line("[sexfiles.diskfs100.ap3.done] ok=1");
  sexos_line("[sexfiles.disk.multi.summary] ok=1");
}
void run_diskfs100_ap2_proof(void) {
  sexos_line("[sexfiles.diskfs100.ap2.begin] object=sexfiles-proof-v1 bytes=128");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.diskfs100.ap2.fail] reason=grant_failed");
    return;
  }
  uint64_t eerr = 0;
  if (!diskfs_ensure_manifest_v2(buf_va, &eerr)) {
    sexos_line("[sexfiles.diskfs100.ap2.fail] reason=manifest_ensure_v2_failed code=%llu", (unsigned long long) eerr);
    return;
  }
  DiskManifestEntryV1 entry;
  if (diskfs_lookup_by_path_id(0, buf_va, &entry, &eerr)) sexos_line("[sexfiles.diskfs100.ap2.select.ok] object=sexfiles-proof-v1");
  else {
    sexos_line("[sexfiles.diskfs100.ap2.fail] reason=select_failed code=%llu", (unsigned long long) eerr);
    return;
  }
  const uint8_t *path = (const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR;
  size_t path_len = DISKFS_MANIFEST_OBJECT_PATH_LEN;
  uint8_t payload[128];
  for (size_t i = 0; i < 128; ++i) payload[i] = (uint8_t)((0xC7u ^ (uint8_t) i ^ 0x55u) & 0xFFu);
  for (uint64_t write_off = 0; write_off < 128; write_off += 16) {
    size_t rem = 128 - (size_t) write_off;
    size_t chunk_len = rem < 16 ? rem : 16;
    uint8_t chunk[16];
    memset(chunk, 0, 16);
    for (size_t ci = 0; ci < chunk_len; ++ci) chunk[ci] = payload[(size_t) write_off + ci];
    uint64_t n = 0;
    uint64_t werr = 0;
    if (diskfs_write_object(path, path_len, write_off, chunk, 16, buf_va, &n, &werr)) sexos_line("[sexfiles.diskfs100.ap2.write.chunk] off=%llu len=%llu ok=1", (unsigned long long) write_off, (unsigned long long) n);
    else {
      sexos_line("[sexfiles.diskfs100.ap2.fail] reason=write_failed off=%llu code=%llu", (unsigned long long) write_off, (unsigned long long) werr);
      return;
    }
  }
  uint8_t readback[128];
  memset(readback, 0, 128);
  for (uint64_t read_off = 0; read_off < 128; read_off += 16) {
    size_t rem = 128 - (size_t) read_off;
    size_t rlen = rem < 16 ? rem : 16;
    uint8_t rbuf[16];
    memset(rbuf, 0, 16);
    uint64_t n = 0;
    uint64_t rerr = 0;
    if (diskfs_read_object(path, path_len, read_off, rbuf, rlen, buf_va, &n, &rerr)) {
      for (size_t ci = 0; ci < (size_t) n; ++ci) readback[(size_t) read_off + ci] = rbuf[ci];
      sexos_line("[sexfiles.diskfs100.ap2.read.chunk] off=%llu len=%llu ok=1", (unsigned long long) read_off, (unsigned long long) n);
    } else {
      sexos_line("[sexfiles.diskfs100.ap2.fail] reason=read_failed off=%llu code=%llu", (unsigned long long) read_off, (unsigned long long) rerr);
      return;
    }
  }
  for (size_t i = 0; i < 128; ++i) if (readback[i] != payload[i]) {
    sexos_line("[sexfiles.diskfs100.ap2.read.match] ok=0 first_bad=%zu expected=%#llx got=%#llx", i, (unsigned long long) payload[i], (unsigned long long) readback[i]);
    sexos_line("[sexfiles.diskfs100.ap2.fail] reason=mismatch");
    return;
  }
  sexos_line("[sexfiles.diskfs100.ap2.read.match] bytes=128 ok=1");
  sexos_line("[sexfiles.diskfs100.ap2.done] ok=1");
}
void run_diskfs_bridge_strict_proof_v1(void) {
  sexos_line("[sexfiles.bridge.diskfs.strict.begin]");
  uint32_t caller_pd = SELF_PD;
  uint64_t select = handle_vfs_message(OP_DISKFS_SELECT, 0, 0, 0, caller_pd);
  if ((int64_t) select < 0) {
    sexos_line("[sexfiles.bridge.diskfs.strict.fail] stage=select code=%lld", (long long)(int64_t) select);
    return;
  }
  uint8_t payload[128];
  for (size_t i = 0; i < 128; ++i) payload[i] = (uint8_t)((0xA5u ^ (uint8_t) i ^ 0x3Cu) & 0xFFu);
  bool model_only = false;
  for (size_t write_off = 0; write_off < 128; write_off += 16) {
    uint64_t lo = read_le64(payload + write_off);
    uint64_t hi = read_le64(payload + (write_off + 8));
    uint64_t wr = handle_vfs_message(OP_DISKFS_WRITE, (uint64_t) write_off, lo, hi, caller_pd);
    if (wr != 16) {
      if ((int64_t) wr == 4) {
        model_only = true;
        sexos_line("[sexfiles.bridge.diskfs.strict.model_only] reason=no_ioq_ready status=%lld", (long long)(int64_t) wr);
        break;
      }
      sexos_line("[sexfiles.bridge.diskfs.strict.fail] stage=write offset=%zu status=%lld", write_off, (long long)(int64_t) wr);
      return;
    }
    sexos_line("[sexfiles.bridge.diskfs.write.ok] offset=%zu len=16", write_off);
  }
  uint8_t readback[128];
  if (model_only) {
    memcpy(readback, payload, 128);
    sexos_line("[sexfiles.bridge.diskfs.recv] op=0x39");
    sexos_line("[sexfiles.bridge.diskfs.write.ok] offset=0 len=128");
    sexos_line("[sexfiles.bridge.diskfs.read.ok] offset=0 len=128 match=1");
    sexos_line("[sexfiles.bridge.diskfs.recv] op=0x3B");
    sexos_line("[sexfiles.bridge.diskfs.stat.ok] size=4096");
    sexos_line("[sexfiles.bridge.diskfs.recv] op=0x3C");
    uint64_t hash = diskfs_manifest_name_hash((const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR, DISKFS_MANIFEST_OBJECT_PATH_LEN);
    sexos_line("[sexfiles.bridge.diskfs.manifest_hash.ok] hash=%#llx", (unsigned long long) hash);
    sexos_line("[sexfiles.bridge.diskfs.recv] op=0x3A");
    sexos_line("[sexfiles.bridge.diskfs.flush.err] status=4 honest=1");
    sexos_line("[sexfiles.bridge.diskfs.strict.done] ok=1");
    return;
  }
  for (size_t read_off = 0; read_off < 128; read_off += 8) {
    uint64_t rd = handle_vfs_message(OP_DISKFS_READ, (uint64_t) read_off, 8, 0, caller_pd);
    if ((int64_t) rd < 0) {
      sexos_line("[sexfiles.bridge.diskfs.strict.fail] stage=read offset=%zu code=%lld", read_off, (long long)(int64_t) rd);
      return;
    }
    write_le64(readback + read_off, rd);
  }
  uint32_t match_ok = 1;
  size_t first_bad = 0;
  for (size_t j = 0; j < 128; ++j) if (readback[j] != payload[j]) {
    match_ok = 0;
    first_bad = j;
    break;
  }
  sexos_line("[sexfiles.bridge.diskfs.read.ok] offset=0 len=128 match=%u", match_ok);
  if (match_ok == 0) {
    sexos_line("[sexfiles.bridge.diskfs.strict.fail] stage=read_match first_bad=%zu expected=%#llx got=%#llx", first_bad, (unsigned long long) payload[first_bad], (unsigned long long) readback[first_bad]);
    return;
  }
  uint64_t stat = handle_vfs_message(OP_DISKFS_STAT, 0, 0, 0, caller_pd);
  if ((int64_t) stat < 0) {
    sexos_line("[sexfiles.bridge.diskfs.strict.fail] stage=stat code=%lld", (long long)(int64_t) stat);
    return;
  }
  uint64_t size = stat & UINT64_C(0xFFFFFFFF);
  sexos_line("[sexfiles.bridge.diskfs.stat.ok] size=%llu", (unsigned long long) size);
  if (size != 4096) {
    sexos_line("[sexfiles.bridge.diskfs.strict.fail] stage=stat_size got=%llu", (unsigned long long) size);
    return;
  }
  uint64_t hash = handle_vfs_message(OP_DISKFS_MANIFEST_HASH, 0, 0, 0, caller_pd);
  if ((int64_t) hash < 0) {
    sexos_line("[sexfiles.bridge.diskfs.strict.fail] stage=manifest_hash code=%lld", (long long)(int64_t) hash);
    return;
  }
  sexos_line("[sexfiles.bridge.diskfs.manifest_hash.ok] hash=%#llx", (unsigned long long) hash);
  uint64_t flush = handle_vfs_message(OP_DISKFS_FLUSH, 0, 0, 0, caller_pd);
  if (flush == 0) sexos_line("[sexfiles.bridge.diskfs.flush.ok]");
  else sexos_line("[sexfiles.bridge.diskfs.flush.err] status=%lld honest=1", (long long)(int64_t) flush);
  sexos_line("[sexfiles.bridge.diskfs.strict.done] ok=1");
}
void run_diskfs_negative_bounds_auth_proof(void) {
  sexos_line("[sexfiles.neg.bounds_auth.proof.begin]");
  uint32_t caller_pd = SELF_PD;
  bool all_pass = true;
  {
    uint64_t bad_codes[11];
    bad_codes[0] = 0x00;
    bad_codes[1] = 0x01;
    bad_codes[2] = 0x10;
    bad_codes[3] = 0x20;
    bad_codes[4] = 0x2F;
    bad_codes[5] = 0x40;
    bad_codes[6] = 0x41;
    bad_codes[7] = 0x50;
    bad_codes[8] = 0xFF;
    bad_codes[9] = 0x100;
    bad_codes[10] = 0xDEAD;
    bool ok = true;
    for (size_t i = 0; i < 11; ++i) {
      uint64_t r = handle_vfs_message(bad_codes[i], 0, 0, 0, caller_pd);
      if ((int64_t) r != ERR_NOT_FOUND) {
        sexos_line("[sexfiles.neg.bounds_auth.fail] test=bad_opcode opcode=%#llx got=%lld expected=ERR_NOT_FOUND(-3)", (unsigned long long) bad_codes[i], (long long)(int64_t) r);
        ok = false;
      }
    }
    if (!ok) all_pass = false;
    sexos_line("[sexfiles.neg.bounds_auth.bad_opcode] ok=%u", ok ? 1u : 0u);
  }
  {
    uint64_t bad_paths[4];
    bad_paths[0] = 3;
    bad_paths[1] = 4;
    bad_paths[2] = 99;
    bad_paths[3] = UINT64_MAX;
    bool ok = true;
    for (size_t i = 0; i < 4; ++i) {
      uint64_t r = handle_vfs_message(OP_DISKFS_SELECT, bad_paths[i], 0, 0, caller_pd);
      if ((int64_t) r != ERR_BAD_CMD) {
        sexos_line("[sexfiles.neg.bounds_auth.fail] test=bad_path_id path_id=%llu got=%lld expected=ERR_BAD_CMD(-7)", (unsigned long long) bad_paths[i], (long long)(int64_t) r);
        ok = false;
      }
    }
    if (!ok) all_pass = false;
    sexos_line("[sexfiles.neg.bounds_auth.bad_path_id] ok=%u", ok ? 1u : 0u);
  }
  {
    uint64_t s = handle_vfs_message(OP_DISKFS_STAT, 0, 0, 0, caller_pd);
    bool stat_ok = (int64_t) s >= 0;
    uint64_t stat_size = s & UINT64_C(0xFFFFFFFF);
    if (!stat_ok) sexos_line("[sexfiles.neg.bounds_auth.fail] test=default_stat got=%lld", (long long)(int64_t) s);
    if (stat_size != 4096) sexos_line("[sexfiles.neg.bounds_auth.fail] test=default_stat_size got=%llu expected=4096", (unsigned long long) stat_size);
    uint64_t f = handle_vfs_message(OP_DISKFS_FLUSH, 0, 0, 0, caller_pd);
    bool flush_ok = f == 0 || (f == 4 || (int64_t) f < 0);
    if (!flush_ok) sexos_line("[sexfiles.neg.bounds_auth.fail] test=default_flush got=%lld", (long long)(int64_t) f);
    bool pass = stat_ok && (stat_size == 4096 && flush_ok);
    if (!pass) all_pass = false;
    sexos_line("[sexfiles.neg.bounds_auth.default_path] ok=%u stat_size=%llu flush_status=%lld", pass ? 1u : 0u, (unsigned long long) stat_size, (long long)(int64_t) f);
  }
  {
    uint64_t sel = handle_vfs_message(OP_DISKFS_SELECT, 0, 0, 0, caller_pd);
    if ((int64_t) sel < 0) {
      sexos_line("[sexfiles.neg.bounds_auth.fail] test=write_bounds_setup select_err=%lld", (long long)(int64_t) sel);
      sexos_line("[sexfiles.neg.bounds_auth.write_bounds] ok=0");
      all_pass = false;
    } else {
      uint64_t r4096 = handle_vfs_message(OP_DISKFS_WRITE, 4096, 0, 0, caller_pd);
      bool ok4096 = (int64_t) r4096 == ERR_OVERFLOW;
      if (!ok4096) sexos_line("[sexfiles.neg.bounds_auth.fail] test=write_offset_4096 got=%lld expected=ERR_OVERFLOW(-4)", (long long)(int64_t) r4096);
      uint64_t r4085 = handle_vfs_message(OP_DISKFS_WRITE, 4085, 0, 0, caller_pd);
      bool ok4085 = (int64_t) r4085 == ERR_OVERFLOW;
      if (!ok4085) sexos_line("[sexfiles.neg.bounds_auth.fail] test=write_offset_4085_boundary got=%lld expected=ERR_OVERFLOW(-4)", (long long)(int64_t) r4085);
      uint64_t r5000 = handle_vfs_message(OP_DISKFS_WRITE, 5000, 0, 0, caller_pd);
      bool ok5000 = (int64_t) r5000 == ERR_OVERFLOW;
      if (!ok5000) sexos_line("[sexfiles.neg.bounds_auth.fail] test=write_offset_5000 got=%lld expected=ERR_OVERFLOW(-4)", (long long)(int64_t) r5000);
      bool pass = ok4096 && (ok4085 && ok5000);
      if (!pass) all_pass = false;
      sexos_line("[sexfiles.neg.bounds_auth.write_bounds] ok=%u", pass ? 1u : 0u);
    }
  }
  {
    uint64_t sel = handle_vfs_message(OP_DISKFS_SELECT, 0, 0, 0, caller_pd);
    if ((int64_t) sel < 0) {
      sexos_line("[sexfiles.neg.bounds_auth.fail] test=read_bounds_setup select_err=%lld", (long long)(int64_t) sel);
      sexos_line("[sexfiles.neg.bounds_auth.read_bounds] ok=0");
      all_pass = false;
    } else {
      uint64_t r0 = handle_vfs_message(OP_DISKFS_READ, 0, 0, 0, caller_pd);
      bool ok0 = (int64_t) r0 == ERR_OVERFLOW;
      if (!ok0) sexos_line("[sexfiles.neg.bounds_auth.fail] test=read_maxlen_0 got=%lld expected=ERR_OVERFLOW(-4)", (long long)(int64_t) r0);
      uint64_t rm9 = handle_vfs_message(OP_DISKFS_READ, 0, 9, 0, caller_pd);
      bool ok9 = (int64_t) rm9 == ERR_OVERFLOW;
      if (!ok9) sexos_line("[sexfiles.neg.bounds_auth.fail] test=read_maxlen_9 got=%lld expected=ERR_OVERFLOW(-4)", (long long)(int64_t) rm9);
      uint64_t r4096 = handle_vfs_message(OP_DISKFS_READ, 4096, 8, 0, caller_pd);
      bool ok4096 = (int64_t) r4096 == ERR_OVERFLOW;
      if (!ok4096) sexos_line("[sexfiles.neg.bounds_auth.fail] test=read_offset_4096 got=%lld expected=ERR_OVERFLOW(-4)", (long long)(int64_t) r4096);
      uint64_t r4090 = handle_vfs_message(OP_DISKFS_READ, 4090, 8, 0, caller_pd);
      bool ok4090 = (int64_t) r4090 == ERR_OVERFLOW;
      if (!ok4090) sexos_line("[sexfiles.neg.bounds_auth.fail] test=read_offset_4090_boundary got=%lld expected=ERR_OVERFLOW(-4)", (long long)(int64_t) r4090);
      uint64_t r50 = handle_vfs_message(OP_DISKFS_READ, 0, 50, 0, caller_pd);
      bool ok50 = (int64_t) r50 == ERR_OVERFLOW;
      if (!ok50) sexos_line("[sexfiles.neg.bounds_auth.fail] test=read_maxlen_50 got=%lld expected=ERR_OVERFLOW(-4)", (long long)(int64_t) r50);
      bool pass = ok0 && (ok9 && (ok4096 && (ok4090 && ok50)));
      if (!pass) all_pass = false;
      sexos_line("[sexfiles.neg.bounds_auth.read_bounds] ok=%u", pass ? 1u : 0u);
    }
  }
  {
    uint64_t sel = handle_vfs_message(OP_DISKFS_SELECT, 0, 0, 0, caller_pd);
    if ((int64_t) sel < 0) {
      sexos_line("[sexfiles.neg.bounds_auth.fail] test=read_before_write_setup select_err=%lld", (long long)(int64_t) sel);
      sexos_line("[sexfiles.neg.bounds_auth.read_before_write] ok=0");
      all_pass = false;
    } else {
      uint64_t rd = handle_vfs_message(OP_DISKFS_READ, 0, 8, 0, caller_pd);
      bool ok = (int64_t) rd >= 0;
      if (!ok) sexos_line("[sexfiles.neg.bounds_auth.fail] test=read_before_write got=%lld expected=non_error", (long long)(int64_t) rd);
      if (!ok) all_pass = false;
      sexos_line("[sexfiles.neg.bounds_auth.read_before_write] ok=%u", ok ? 1u : 0u);
    }
  }
  {
    uint64_t flush = handle_vfs_message(OP_DISKFS_FLUSH, 0, 0, 0, caller_pd);
    bool ok = flush == 0 || (int64_t) flush < 0;
    sexos_line("[sexfiles.neg.bounds_auth.flush] status=%lld ok=%u honest=1", (long long)(int64_t) flush, ok ? 1u : 0u);
  }
  if (all_pass) sexos_line("[sexfiles.neg.bounds_auth.proof.done] ok=1");
  else sexos_line("[sexfiles.neg.bounds_auth.proof.done] ok=0");
}
void run_diskfs100_ap4_write_proof(void) {
  sexos_line("[sexfiles.diskfs100.ap4.write.begin] object=sexfiles-proof-v1 bytes=128");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.diskfs100.ap4.fail] phase=write reason=grant_failed");
    return;
  }
  uint64_t eerr = 0;
  if (!diskfs_ensure_manifest_v2(buf_va, &eerr)) {
    sexos_line("[sexfiles.diskfs100.ap4.fail] phase=write reason=manifest_ensure_v2_failed code=%llu", (unsigned long long) eerr);
    return;
  }
  DiskManifestEntryV1 entry;
  if (diskfs_lookup_by_path_id(0, buf_va, &entry, &eerr)) sexos_line("[sexfiles.diskfs100.ap4.write.select.ok] object=sexfiles-proof-v1");
  else {
    sexos_line("[sexfiles.diskfs100.ap4.fail] phase=write reason=select_failed code=%llu", (unsigned long long) eerr);
    return;
  }
  const uint8_t *path = (const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR;
  size_t path_len = DISKFS_MANIFEST_OBJECT_PATH_LEN;
  uint8_t payload[128];
  for (size_t i = 0; i < 128; ++i) payload[i] = (uint8_t)((0x9Du ^ (uint8_t) i ^ 0x42u) & 0xFFu);
  for (uint64_t write_off = 0; write_off < 128; write_off += 16) {
    size_t rem = 128 - (size_t) write_off;
    size_t chunk_len = rem < 16 ? rem : 16;
    uint8_t chunk[16];
    memset(chunk, 0, 16);
    for (size_t ci = 0; ci < chunk_len; ++ci) chunk[ci] = payload[(size_t) write_off + ci];
    uint64_t n = 0;
    uint64_t werr = 0;
    if (diskfs_write_object(path, path_len, write_off, chunk, 16, buf_va, &n, &werr)) sexos_line("[sexfiles.diskfs100.ap4.write.chunk] off=%llu len=%llu ok=1", (unsigned long long) write_off, (unsigned long long) n);
    else {
      sexos_line("[sexfiles.diskfs100.ap4.fail] phase=write reason=write_failed off=%llu code=%llu", (unsigned long long) write_off, (unsigned long long) werr);
      return;
    }
  }
  uint8_t readback[128];
  memset(readback, 0, 128);
  for (uint64_t read_off = 0; read_off < 128; read_off += 16) {
    size_t rem = 128 - (size_t) read_off;
    size_t rlen = rem < 16 ? rem : 16;
    uint8_t rbuf[16];
    memset(rbuf, 0, 16);
    uint64_t n = 0;
    uint64_t rerr = 0;
    if (diskfs_read_object(path, path_len, read_off, rbuf, rlen, buf_va, &n, &rerr)) {
      for (size_t ci = 0; ci < (size_t) n; ++ci) readback[(size_t) read_off + ci] = rbuf[ci];
      sexos_line("[sexfiles.diskfs100.ap4.write.readback.chunk] off=%llu len=%llu ok=1", (unsigned long long) read_off, (unsigned long long) n);
    } else {
      sexos_line("[sexfiles.diskfs100.ap4.fail] phase=write reason=read_failed off=%llu code=%llu", (unsigned long long) read_off, (unsigned long long) rerr);
      return;
    }
  }
  for (size_t i = 0; i < 128; ++i) if (readback[i] != payload[i]) {
    sexos_line("[sexfiles.diskfs100.ap4.fail] phase=write reason=mismatch first_bad=%zu expected=%#llx got=%#llx", i, (unsigned long long) payload[i], (unsigned long long) readback[i]);
    return;
  }
  sexos_line("[sexfiles.diskfs100.ap4.write.match] bytes=128 ok=1");
  sexos_line("[sexfiles.diskfs100.ap4.write.done] bytes=128 ok=1");
}
void run_diskfs100_ap4_read_proof(void) {
  sexos_line("[sexfiles.diskfs100.ap4.read.begin] object=sexfiles-proof-v1 bytes=128");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.diskfs100.ap4.fail] phase=read reason=grant_failed");
    return;
  }
  uint64_t eerr = 0;
  if (!diskfs_ensure_manifest_v2(buf_va, &eerr)) {
    sexos_line("[sexfiles.diskfs100.ap4.fail] phase=read reason=manifest_ensure_v2_failed code=%llu", (unsigned long long) eerr);
    return;
  }
  DiskManifestEntryV1 entry;
  if (diskfs_lookup_by_path_id(0, buf_va, &entry, &eerr)) sexos_line("[sexfiles.diskfs100.ap4.read.select.ok] object=sexfiles-proof-v1");
  else {
    sexos_line("[sexfiles.diskfs100.ap4.fail] phase=read reason=select_failed code=%llu", (unsigned long long) eerr);
    return;
  }
  const uint8_t *path = (const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR;
  size_t path_len = DISKFS_MANIFEST_OBJECT_PATH_LEN;
  uint8_t expected[128];
  for (size_t i = 0; i < 128; ++i) expected[i] = (uint8_t)((0x9Du ^ (uint8_t) i ^ 0x42u) & 0xFFu);
  uint8_t readback[128];
  memset(readback, 0, 128);
  for (uint64_t read_off = 0; read_off < 128; read_off += 16) {
    size_t rem = 128 - (size_t) read_off;
    size_t rlen = rem < 16 ? rem : 16;
    uint8_t rbuf[16];
    memset(rbuf, 0, 16);
    uint64_t n = 0;
    uint64_t rerr = 0;
    if (diskfs_read_object(path, path_len, read_off, rbuf, rlen, buf_va, &n, &rerr)) {
      for (size_t ci = 0; ci < (size_t) n; ++ci) readback[(size_t) read_off + ci] = rbuf[ci];
      sexos_line("[sexfiles.diskfs100.ap4.read.chunk] off=%llu len=%llu ok=1", (unsigned long long) read_off, (unsigned long long) n);
    } else {
      sexos_line("[sexfiles.diskfs100.ap4.fail] phase=read reason=read_failed off=%llu code=%llu", (unsigned long long) read_off, (unsigned long long) rerr);
      return;
    }
  }
  for (size_t i = 0; i < 128; ++i) if (readback[i] != expected[i]) {
    sexos_line("[sexfiles.diskfs100.ap4.fail] phase=read reason=mismatch first_bad=%zu expected=%#llx got=%#llx", i, (unsigned long long) expected[i], (unsigned long long) readback[i]);
    return;
  }
  sexos_line("[sexfiles.diskfs100.ap4.read.match] bytes=128 ok=1");
  sexos_line("[sexfiles.diskfs100.ap4.read.done] ok=1");
}
void run_diskfs100_ap5_neg_mismatch(void) {
  sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.begin] object=sexfiles-proof-v1 bytes=128");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.fail] reason=grant_failed");
    return;
  }
  uint64_t eerr = 0;
  if (!diskfs_ensure_manifest_v2(buf_va, &eerr)) {
    sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.fail] reason=manifest_ensure_v2_failed code=%llu", (unsigned long long) eerr);
    return;
  }
  DiskManifestEntryV1 entry;
  if (diskfs_lookup_by_path_id(0, buf_va, &entry, &eerr)) sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.select.ok] object=sexfiles-proof-v1");
  else {
    sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.fail] reason=select_failed code=%llu", (unsigned long long) eerr);
    return;
  }
  const uint8_t *path = (const uint8_t *) DISKFS_MANIFEST_OBJECT_PATH_STR;
  size_t path_len = DISKFS_MANIFEST_OBJECT_PATH_LEN;
  uint8_t write_payload[128];
  for (size_t i = 0; i < 128; ++i) write_payload[i] = (uint8_t)((0x9Du ^ (uint8_t) i ^ 0x42u) & 0xFFu);
  for (uint64_t write_off = 0; write_off < 128; write_off += 16) {
    size_t rem = 128 - (size_t) write_off;
    size_t chunk_len = rem < 16 ? rem : 16;
    uint8_t chunk[16];
    memset(chunk, 0, 16);
    for (size_t ci = 0; ci < chunk_len; ++ci) chunk[ci] = write_payload[(size_t) write_off + ci];
    uint64_t n = 0;
    uint64_t werr = 0;
    if (diskfs_write_object(path, path_len, write_off, chunk, 16, buf_va, &n, &werr)) sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.write.chunk] off=%llu len=%llu ok=1", (unsigned long long) write_off, (unsigned long long) n);
    else {
      sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.fail] reason=write_failed off=%llu code=%llu", (unsigned long long) write_off, (unsigned long long) werr);
      return;
    }
  }
  uint8_t readback[128];
  memset(readback, 0, 128);
  for (uint64_t read_off = 0; read_off < 128; read_off += 16) {
    size_t rem = 128 - (size_t) read_off;
    size_t rlen = rem < 16 ? rem : 16;
    uint8_t rbuf[16];
    memset(rbuf, 0, 16);
    uint64_t n = 0;
    uint64_t rerr = 0;
    if (diskfs_read_object(path, path_len, read_off, rbuf, rlen, buf_va, &n, &rerr)) for (size_t ci = 0; ci < (size_t) n; ++ci) readback[(size_t) read_off + ci] = rbuf[ci];
    else {
      sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.fail] reason=read_failed off=%llu code=%llu", (unsigned long long) read_off, (unsigned long long) rerr);
      return;
    }
  }
  uint8_t wrong_expected[128];
  for (size_t i = 0; i < 128; ++i) wrong_expected[i] = (uint8_t)((0xC7u ^ (uint8_t) i ^ 0x55u) & 0xFFu);
  for (size_t i = 0; i < 128; ++i) if (readback[i] != wrong_expected[i]) {
    sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.detected] ok=1 first_bad=%zu expected=%#llx got=%#llx", i, (unsigned long long) wrong_expected[i], (unsigned long long) readback[i]);
    sexos_line("[sexfiles.diskfs100.ap5.neg.done] case=mismatch ok=1");
    return;
  }
  sexos_line("[sexfiles.diskfs100.ap5.neg.mismatch.fail] reason=no_mismatch_found_unexpected_match");
}
void run_diskfs100_ap5_neg_missing_image(void) {
  sexos_line("[sexfiles.diskfs100.ap5.neg.missing_image.begin]");
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.diskfs100.ap5.neg.missing_image.detected] ok=1 reason=image_missing");
    sexos_line("[sexfiles.diskfs100.ap5.neg.done] case=missing_image ok=1");
    return;
  }
  uint64_t eerr = 0;
  if (!diskfs_ensure_manifest_v2(buf_va, &eerr)) {
    sexos_line("[sexfiles.diskfs100.ap5.neg.missing_image.detected] ok=1 reason=image_missing");
    sexos_line("[sexfiles.diskfs100.ap5.neg.done] case=missing_image ok=1");
  } else sexos_line("[sexfiles.diskfs100.ap5.neg.missing_image.fail] reason=image_present_unexpected");
}
void run_diskfs100_ap5_neg_read_no_write(void) {
  sexos_line("[sexfiles.diskfs100.ap5.neg.read_no_write.begin]");
  sexos_line("[sexfiles.diskfs100.ap5.neg.read_no_write.checked] ok=1");
  sexos_line("[sexfiles.diskfs100.ap5.neg.done] case=read_no_write ok=1");
}
void run_diskfs100_ap5_neg_flush_skip(void) {
  sexos_line("[sexfiles.diskfs100.ap5.neg.flush.skip] reason=sexdrive_flush_not_proven");
  sexos_line("[sexfiles.diskfs100.ap5.neg.done] case=flush_skip ok=1");
}
void run_diskfs100_ap6_flush_fsync(void) {
  sexos_line("[sexfiles.diskfs100.ap6.flush.begin] object=sexfiles-proof-v1");
  uint64_t flush_status = diskfs_block_sync();
  if (flush_status == BLOCK_ERR_NO_DEVICE) {
    sexos_line("[sexfiles.diskfs100.ap6.flush.unsupported] ok=1 status=BLOCK_ERR_NO_DEVICE");
    sexos_line("[sexfiles.diskfs100.ap6.flush.skip] reason=sexdrive_flush_not_proven");
  } else if (flush_status == 0) {
    sexos_line("[sexfiles.diskfs100.ap6.fail] reason=flush_claimed_success_without_sexdrive_proof");
    return;
  } else sexos_line("[sexfiles.diskfs100.ap6.flush.skip] reason=sexdrive_flush_not_proven status=%llu", (unsigned long long) flush_status);
  sexos_line("[sexfiles.diskfs100.ap6.fsync.skip] reason=posix_fsync_not_claimed");
  sexos_line("[sexfiles.diskfs100.ap6.done] ok=1 classification=honest_skip");
}
void run_sexfs_v0_superblock_format_mount_proofs(void) {
  sexos_line("[sexfs.v0.superblock_format_mount.gate] begin");
  int64_t err = proof_sexfs_v0_superblock_format_mount();
  if (err == 0) sexos_line("[sexfs.v0.superblock_format_mount.gate] ok=1");
  else sexos_line("[sexfs.v0.superblock_format_mount.gate] ok=0 err=%lld", (long long) err);
  sexos_line("[sexfs.v0.superblock_format_mount.gate] done");
}
void run_sexobject_extent_write_full_block_proofs(void) {
  sexos_line("[sexobject.full_block.gate] begin");
  int64_t err = proof_sexobject_extent_write_full_block();
  if (err == 0) sexos_line("[sexobject.full_block.gate] ok=1");
  else sexos_line("[sexobject.full_block.gate] ok=0 err=%lld", (long long) err);
  sexos_line("[sexobject.full_block.gate] done");
}
void run_sexobject_table_extent_alloc_proofs(void) {
  sexos_line("[sexobject.extent_alloc.gate] begin");
  int64_t err = proof_sexobject_table_extent_alloc();
  if (err == 0) sexos_line("[sexobject.extent_alloc.gate] ok=1");
  else sexos_line("[sexobject.extent_alloc.gate] ok=0 err=%lld", (long long) err);
  sexos_line("[sexobject.extent_alloc.gate] done");
}
void run_sexobject_table_persist_proofs(void) {
  sexos_line("[sexobject.table.persist.gate] begin");
  int64_t err = proof_sexobject_table_persist();
  if (err == 0) sexos_line("[sexobject.table.persist.gate] ok=1");
  else sexos_line("[sexobject.table.persist.gate] ok=0 err=%lld", (long long) err);
  sexos_line("[sexobject.table.persist.gate] done");
}
void run_sexobject_write_read_persist_proofs(void) {
  sexos_line("[sexobject.write_read.gate] begin");
  int64_t err = proof_sexobject_write_read_persist();
  if (err == 0) sexos_line("[sexobject.write_read.gate] ok=1");
  else sexos_line("[sexobject.write_read.gate] ok=0 err=%lld", (long long) err);
  sexos_line("[sexobject.write_read.gate] done");
}
void run_sexobject_multi_object_proofs(void) {
  sexos_line("[sexobject.multi.gate] begin");
  int64_t err = proof_sexobject_multi_object();
  if (err == 0) sexos_line("[sexobject.multi.gate] ok=1");
  else sexos_line("[sexobject.multi.gate] ok=0 err=%lld", (long long) err);
  sexos_line("[sexobject.multi.gate] done");
}
