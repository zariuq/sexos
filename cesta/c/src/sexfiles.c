#include "sexfiles.h"
#if !defined(SEXOS_SEXFILES_CURRENT_DOCUMENT)
#define SEXOS_SEXFILES_CURRENT_DOCUMENT 1

#endif
/*SexFiles server: message defer ring, SexObject header adapter, RamFS backend, DiskFS V4 bridge, VFS dispatch, and trampoline entry (servers/sexfiles/src/{pdx,sexobject,backends/ramfs,vfs,trampoline,main}.rs).*/
/*Build-time proof flags and cfg gates (servers/sexfiles/src/trampoline.rs & build.rs).*/
#if defined(SEXFILES_RAMFS_PROOF)
#define RAMFS_PROOF_ENABLED 1

#else
#define RAMFS_PROOF_ENABLED 0

#endif
#if defined(SEXOS_DISKFS_OBJECT_TABLE_PROOF)
#define DISKFS_OBJECT_TABLE_PROOF_ENABLED 1

#else
#define DISKFS_OBJECT_TABLE_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SEXFILES_JOURNAL_PROOF)
#define SEXFILES_JOURNAL_PROOF_ENABLED 1

#else
#define SEXFILES_JOURNAL_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SEXFILES_REPLAY_PROOF)
#define SEXFILES_REPLAY_PROOF_ENABLED 1

#else
#define SEXFILES_REPLAY_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SEXFILES_CAP_RECORD_PROOF)
#define SEXFILES_CAP_RECORD_PROOF_ENABLED 1

#else
#define SEXFILES_CAP_RECORD_PROOF_ENABLED 0

#endif
#if defined(SEXOS_LINEN_SEXFILES_METADATA_PROOF)
#define LINEN_SEXFILES_METADATA_PROOF_ENABLED 1

#else
#define LINEN_SEXFILES_METADATA_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SEXFILES_FAULT_INJECTION_PROOF)
#define SEXFILES_FAULT_INJECTION_PROOF_ENABLED 1

#else
#define SEXFILES_FAULT_INJECTION_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SEXFILES_REAL_BLOCK_PROOF)
#define SEXFILES_REAL_BLOCK_PROOF_ENABLED 1

#else
#define SEXFILES_REAL_BLOCK_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SEXFILES_REBOOT_PROOF)
#define SEXFILES_REBOOT_PROOF_ENABLED 1

#else
#define SEXFILES_REBOOT_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SEXFILES_EXTENT_PROOF)
#define SEXFILES_EXTENT_PROOF_ENABLED 1

#else
#define SEXFILES_EXTENT_PROOF_ENABLED 0

#endif
#if defined(SEXOS_SEXFILES_CHECKPOINT_PROOF)
#define SEXFILES_CHECKPOINT_PROOF_ENABLED 1

#else
#define SEXFILES_CHECKPOINT_PROOF_ENABLED 0

#endif
#if defined(SEXFILES_ROUTE_AUDIT_ONLY)
#define SEXFILES_ROUTE_AUDIT_ONLY_ENABLED 1

#else
#define SEXFILES_ROUTE_AUDIT_ONLY_ENABLED 0

#endif
#if defined(SEXOS_SEXOBJECT_VIEW_PROOF)
#define SEXOBJECT_VIEW_PROOF_ENABLED 1

#else
#define SEXOBJECT_VIEW_PROOF_ENABLED 0

#endif
#if defined(SEXOS_LINEN_DISK_OBJECT_PROOF)
#define LINEN_DISK_OBJECT_PROOF_ENABLED 1

#else
#define LINEN_DISK_OBJECT_PROOF_ENABLED 0

#endif
#if defined(SEXFS_V0_SUPERBLOCK_FORMAT_MOUNT_PROOF)
#define SEXFS_V0_SUPERBLOCK_FORMAT_MOUNT_PROOF_ENABLED 1

#else
#define SEXFS_V0_SUPERBLOCK_FORMAT_MOUNT_PROOF_ENABLED 0

#endif
#if defined(SEXOBJECT_TABLE_PERSIST_PROOF)
#define SEXOBJECT_TABLE_PERSIST_PROOF_ENABLED 1

#else
#define SEXOBJECT_TABLE_PERSIST_PROOF_ENABLED 0

#endif
#if defined(SEXOBJECT_TABLE_EXTENT_ALLOC_PROOF)
#define SEXOBJECT_TABLE_EXTENT_ALLOC_PROOF_ENABLED 1

#else
#define SEXOBJECT_TABLE_EXTENT_ALLOC_PROOF_ENABLED 0

#endif
#if defined(SEXOBJECT_EXTENT_WRITE_FULL_BLOCK_PROOF)
#define SEXOBJECT_EXTENT_WRITE_FULL_BLOCK_PROOF_ENABLED 1

#else
#define SEXOBJECT_EXTENT_WRITE_FULL_BLOCK_PROOF_ENABLED 0

#endif
#if defined(SEXOBJECT_WRITE_READ_PERSIST_PROOF)
#define SEXOBJECT_WRITE_READ_PERSIST_PROOF_ENABLED 1

#else
#define SEXOBJECT_WRITE_READ_PERSIST_PROOF_ENABLED 0

#endif
#if defined(SEXOBJECT_MULTI_OBJECT_PROOF)
#define SEXOBJECT_MULTI_OBJECT_PROOF_ENABLED 1

#else
#define SEXOBJECT_MULTI_OBJECT_PROOF_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap4_write) || defined(SEXFILES_DISKFS_100_AP4_WRITE)
#define DISKFS_100_AP4_WRITE_PROOF_ENABLED 1

#else
#define DISKFS_100_AP4_WRITE_PROOF_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap4_read) || defined(SEXFILES_DISKFS_100_AP4_READ) || defined(sexfiles_diskfs100_ap5_neg_read_no_write) || defined(SEXFILES_DISKFS_100_AP5_NEG_READ_NO_WRITE)
#define DISKFS_100_AP4_READ_PROOF_ENABLED 1

#else
#define DISKFS_100_AP4_READ_PROOF_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap5_neg_read_no_write) || defined(SEXFILES_DISKFS_100_AP5_NEG_READ_NO_WRITE)
#define DISKFS_100_AP5_NEG_READ_NO_WRITE_ENABLED 1

#else
#define DISKFS_100_AP5_NEG_READ_NO_WRITE_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap5_neg_mismatch) || defined(SEXFILES_DISKFS_100_AP5_NEG_MISMATCH)
#define DISKFS_100_AP5_NEG_MISMATCH_ENABLED 1

#else
#define DISKFS_100_AP5_NEG_MISMATCH_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap5_neg_missing_image) || defined(SEXFILES_DISKFS_100_AP5_NEG_MISSING_IMAGE)
#define DISKFS_100_AP5_NEG_MISSING_IMAGE_ENABLED 1

#else
#define DISKFS_100_AP5_NEG_MISSING_IMAGE_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap5_neg_flush_skip) || defined(SEXFILES_DISKFS_100_AP5_NEG_FLUSH_SKIP)
#define DISKFS_100_AP5_NEG_FLUSH_SKIP_ENABLED 1

#else
#define DISKFS_100_AP5_NEG_FLUSH_SKIP_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap6_flush_fsync) || defined(SEXFILES_DISKFS_100_AP6_FLUSH_FSYNC)
#define DISKFS_100_AP6_FLUSH_FSYNC_ENABLED 1

#else
#define DISKFS_100_AP6_FLUSH_FSYNC_ENABLED 0

#endif
#if defined(sexfiles_diskfs_negative_bounds_auth_proof) || defined(SEXFILES_DISKFS_NEGATIVE_BOUNDS_AUTH_PROOF)
#define DISKFS_NEGATIVE_BOUNDS_AUTH_PROOF_ENABLED 1

#else
#define DISKFS_NEGATIVE_BOUNDS_AUTH_PROOF_ENABLED 0

#endif
#if defined(sexfiles_diskfs_bridge_strict_proof) || defined(SEXFILES_DISKFS_BRIDGE_STRICT_PROOF)
#define DISKFS_BRIDGE_STRICT_PROOF_ENABLED 1

#else
#define DISKFS_BRIDGE_STRICT_PROOF_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap2_proof) || defined(SEXFILES_DISKFS_100_PROOF)
#define DISKFS_100_AP2_PROOF_ENABLED 1

#else
#define DISKFS_100_AP2_PROOF_ENABLED 0

#endif
#if defined(sexfiles_diskfs100_ap3_proof) || defined(SEXFILES_DISKFS_100_AP3_PROOF)
#define DISKFS_100_AP3_PROOF_ENABLED 1

#else
#define DISKFS_100_AP3_PROOF_ENABLED 0

#endif
#if defined(sexfiles_diskfs_multi_object_proof) || defined(SEXFILES_DISKFS_100_AP3_PROOF)
#define DISKFS_MULTI_OBJECT_PROOF_ENABLED 1

#else
#define DISKFS_MULTI_OBJECT_PROOF_ENABLED 0

#endif
SexfilesState _Alignas(4096) sexfiles_state;
void sexfiles_state_init(SexfilesState *s) {
  memset(s, 0, sizeof *s);
  s->listener.wrapper_budget = 8;
  s->trampoline_hot_log_budget = 120;
  s->tramp_listen_budget = 8;
  s->vfs_hot_log_budget = 200;
  s->diskfs_hot_log_budget = 200;
  s->ramfs_next_object_id = 1;
  s->ramfs_next_handle = 1;
  s->diskfs_state.next_tx_id = 1;
  s->diskfs_state.next_checkpoint_generation = 1;
  s->diskfs_next_object_id = 1;
  s->diskfs_writer_job_next = 1;
}
bool sexfiles_take_budget(uint32_t *budget) {
  if (*budget > 0) {
    *budget -= 1;
    return true;
  }
  return false;
}
/*SEXFILES_DEFER_V1: defer ring for client messages arriving during diskfs_block_call (servers/sexfiles/src/pdx.rs).*/
bool defer_stash(SexosRequest request) {
  if (sexfiles_state.defer_len >= DEFER_CAP) {
    sexos_line("[sexfiles.defer.drop] type=%#llx caller=%u reason=stash_full", (unsigned long long) request.message.type_id, (unsigned) request.message.caller_pd);
    return false;
  }
  size_t i = sexfiles_state.defer_len;
  sexfiles_state.defer_ring[i] = (DeferEntry) {
    .request = request
  };
  sexfiles_state.defer_len = i + 1;
  sexos_line("[sexfiles.defer.stash] type=%#llx caller=%u depth=%zu", (unsigned long long) request.message.type_id, (unsigned) request.message.caller_pd, i + 1);
  return true;
}
bool defer_pop(DeferEntry *out) {
  if (sexfiles_state.defer_len == 0) return false;
  DeferEntry m = sexfiles_state.defer_ring[0];
  size_t n = sexfiles_state.defer_len;
  for (size_t i = 1; i < n; ++i) sexfiles_state.defer_ring[i - 1] = sexfiles_state.defer_ring[i];
  sexfiles_state.defer_len = n - 1;
  sexos_line("[sexfiles.defer.replay] type=%#llx caller=%u depth=%zu", (unsigned long long) m.request.message.type_id, (unsigned) m.request.message.caller_pd, n - 1);
  *out = m;
  return true;
}
/*SexObject header adapter (servers/sexfiles/src/sexobject.rs).*/
uint64_t collar_rights_generation(const SexfilesObjectEntry *entry) {
  return entry->rights_generation;
}
SexObjectHeader sexobject_header_from_entry(const SexfilesObjectEntry *entry) {
  return (SexObjectHeader) {
    .object_id = entry->object_id,.content_generation = entry->metadata_generation,.rights_generation = entry->rights_generation,.metadata_generation = entry->metadata_generation,.object_size_bytes = entry->object_size_bytes,.first_block = entry->first_block,.owner_pd = entry->owner_pd,.kind = (uint32_t) entry->kind,.checksum = entry->checksum,.flags = entry->in_use ? 0 : SEX_OBJECT_FLAG_TOMBSTONED,.reserved0 = 0,.reserved1 = 0
  };
}
/*RamFs backend (servers/sexfiles/src/backends/ramfs.rs).*/
static size_t ramfs_find_active_by_handle(uint64_t handle) {
  for (size_t i = 0; i < sexfiles_state.ramfs_files_len; ++i) {
    const RamfsFileEntry *f = &sexfiles_state.ramfs_files[i];
    if (f->in_use && (f->active && f->handle == handle)) return i;
  }
  return SIZE_MAX;
}
static size_t ramfs_find_any_by_name(const uint8_t *name, size_t name_len) {
  for (size_t i = 0; i < sexfiles_state.ramfs_files_len; ++i) {
    const RamfsFileEntry *f = &sexfiles_state.ramfs_files[i];
    if (f->in_use && ((size_t) f->name_len == name_len && memcmp(f->name, name, name_len) == 0)) return i;
  }
  return SIZE_MAX;
}
static int64_t ramfs_allocate(const uint8_t *name, size_t name_len, uint32_t owner_pd, uint64_t *out_handle) {
  size_t active_count = 0;
  for (size_t i = 0; i < sexfiles_state.ramfs_files_len; ++i) if (sexfiles_state.ramfs_files[i].in_use && sexfiles_state.ramfs_files[i].active) ++active_count;
  if (active_count >= RAMFS_MAX_FILES || sexfiles_state.ramfs_files_len >= RAMFS_SLOT_CAP) return ERR_FULL;
  uint64_t handle = sexfiles_state.ramfs_next_handle;
  sexfiles_state.ramfs_next_handle += 1;
  uint64_t object_id = sexfiles_state.ramfs_next_object_id;
  sexfiles_state.ramfs_next_object_id += 1;
  size_t idx = sexfiles_state.ramfs_files_len;
  sexfiles_state.ramfs_files_len += 1;
  RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  memset(entry, 0, sizeof *entry);
  entry->handle = handle;
  entry->object_id = object_id;
  entry->active = true;
  entry->in_use = true;
  entry->owner_pd = owner_pd;
  entry->cap_generation = 1;
  memcpy(entry->name, name, name_len);
  entry->name_len = (uint8_t) name_len;
  entry->data_len = 0;
  *out_handle = handle;
  return 0;
}
static int64_t ramfs_check_access(uint32_t caller_pd, const RamfsFileEntry *entry, uint8_t required) {
  if (caller_pd == 0 || caller_pd == entry->owner_pd) return 0;
  for (size_t i = 0; i < sexfiles_state.ramfs_caps_len; ++i) {
    RamfsCapRecord cap = sexfiles_state.ramfs_caps[i];
    if (cap.valid && (cap.object_id == entry->object_id && (cap.subject_pd == caller_pd && (cap.generation == entry->cap_generation && (cap.rights & required) == required)))) return 0;
  }
  return ERR_PERM_DENIED;
}
static int64_t ramfs_check_access_any(uint32_t caller_pd, const RamfsFileEntry *entry, uint8_t required_any) {
  if (caller_pd == 0 || caller_pd == entry->owner_pd) return 0;
  for (size_t i = 0; i < sexfiles_state.ramfs_caps_len; ++i) {
    RamfsCapRecord cap = sexfiles_state.ramfs_caps[i];
    if (cap.valid && (cap.object_id == entry->object_id && (cap.subject_pd == caller_pd && (cap.generation == entry->cap_generation && (cap.rights & required_any) != 0)))) return 0;
  }
  return ERR_PERM_DENIED;
}
static int64_t ramfs_grant_cap_internal(const RamfsFileEntry *entry, uint32_t subject_pd, uint8_t rights) {
  if (rights == 0) return ERR_PERM_DENIED;
  for (size_t i = 0; i < sexfiles_state.ramfs_caps_len; ++i) {
    RamfsCapRecord *rec = &sexfiles_state.ramfs_caps[i];
    if (rec->object_id == entry->object_id && rec->subject_pd == subject_pd) {
      rec->rights = rights;
      rec->generation = entry->cap_generation;
      rec->valid = true;
      return 0;
    }
  }
  if (sexfiles_state.ramfs_caps_len >= CAP_MAX_RECORDS) return ERR_FULL;
  size_t idx = sexfiles_state.ramfs_caps_len;
  sexfiles_state.ramfs_caps_len += 1;
  sexfiles_state.ramfs_caps[idx] = (RamfsCapRecord) {
    .object_id = entry->object_id,.subject_pd = subject_pd,.rights = rights,.generation = entry->cap_generation,.valid = true
  };
  return 0;
}
static void ramfs_revoke_caps_internal(uint64_t object_id) {
  for (size_t i = 0; i < sexfiles_state.ramfs_caps_len; ++i) if (sexfiles_state.ramfs_caps[i].object_id == object_id) sexfiles_state.ramfs_caps[i].valid = false;
}
int64_t ramfs_proof_grant_caps_by_name(uint32_t owner_pd, const uint8_t *name, size_t name_len, uint32_t subject_pd, uint8_t rights) {
  size_t idx = ramfs_find_any_by_name(name, name_len);
  if (idx == SIZE_MAX) return ERR_NOT_FOUND;
  const RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  if (owner_pd != 0 && owner_pd != entry->owner_pd) return ERR_PERM_DENIED;
  return ramfs_grant_cap_internal(entry, subject_pd, rights);
}
int64_t ramfs_proof_revoke_caps_by_name(uint32_t owner_pd, const uint8_t *name, size_t name_len) {
  size_t idx = ramfs_find_any_by_name(name, name_len);
  if (idx == SIZE_MAX) return ERR_NOT_FOUND;
  RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  if (owner_pd != 0 && owner_pd != entry->owner_pd) return ERR_PERM_DENIED;
  if (entry->cap_generation < UINT64_MAX) entry->cap_generation += 1;
  ramfs_revoke_caps_internal(entry->object_id);
  return 0;
}
int64_t ramfs_proof_inject_stale_generation_cap(const uint8_t *name, size_t name_len, uint32_t subject_pd, uint8_t rights, uint64_t stale_generation) {
  size_t idx = ramfs_find_any_by_name(name, name_len);
  if (idx == SIZE_MAX) return ERR_NOT_FOUND;
  const RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  if (sexfiles_state.ramfs_caps_len >= CAP_MAX_RECORDS) return ERR_FULL;
  size_t cidx = sexfiles_state.ramfs_caps_len;
  sexfiles_state.ramfs_caps_len += 1;
  sexfiles_state.ramfs_caps[cidx] = (RamfsCapRecord) {
    .object_id = entry->object_id,.subject_pd = subject_pd,.rights = rights,.generation = stale_generation,.valid = true
  };
  return 0;
}
int64_t ramfs_object_id_for_handle(uint64_t handle, uint32_t caller_pd, uint64_t *out_id) {
  size_t idx = ramfs_find_active_by_handle(handle);
  if (idx == SIZE_MAX) return ERR_INVALID_HANDLE;
  const RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  if (caller_pd != 0 && caller_pd != entry->owner_pd) return ERR_PERM_DENIED;
  *out_id = entry->object_id;
  return 0;
}
int64_t ramfs_open(const uint8_t *name, size_t name_len, uint32_t flags, uint32_t mode, uint32_t caller_pd, uint64_t *out_handle) {
  (void) mode;
  if (name_len > RAMFS_MAX_NAME) return ERR_NAME_TOO_LONG;
  if (name_len == 0) return ERR_NOT_FOUND;
  size_t idx = ramfs_find_any_by_name(name, name_len);
  if (idx != SIZE_MAX) {
    if ((flags & RAMFS_O_EXCL) != 0) return ERR_NOT_FOUND;
    RamfsFileEntry *existing = &sexfiles_state.ramfs_files[idx];
    int64_t err = ramfs_check_access_any(caller_pd, existing, CAP_RIGHT_OPEN_EXISTING);
    if (err != 0) return err;
    if (!existing->active) {
      existing->active = true;
      existing->handle = sexfiles_state.ramfs_next_handle;
      sexfiles_state.ramfs_next_handle += 1;
    }
    *out_handle = existing->handle;
    return 0;
  }
  if ((flags & RAMFS_O_CREATE) != 0) return ramfs_allocate(name, name_len, caller_pd, out_handle);
  return ERR_NOT_FOUND;
}
int64_t ramfs_read(uint64_t handle, uint64_t offset, uint8_t *buf, size_t buf_len, uint32_t caller_pd, uint64_t *out_n) {
  size_t idx = ramfs_find_active_by_handle(handle);
  if (idx == SIZE_MAX) return ERR_INVALID_HANDLE;
  const RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  int64_t err = ramfs_check_access(caller_pd, entry, CAP_RIGHT_READ);
  if (err != 0) return err;
  size_t start = (size_t) offset;
  if (start >= entry->data_len) {
    *out_n = 0;
    return 0;
  }
  size_t avail = entry->data_len - start;
  size_t to_read = buf_len < avail ? buf_len : avail;
  if (to_read > RAMFS_MAX_FILE_SIZE) to_read = RAMFS_MAX_FILE_SIZE;
  if (to_read > 0) memcpy(buf, entry->data + start, to_read);
  *out_n = (uint64_t) to_read;
  return 0;
}
int64_t ramfs_write(uint64_t handle, uint64_t offset, const uint8_t *data, size_t data_len, uint32_t caller_pd, uint64_t *out_n) {
  size_t idx = ramfs_find_active_by_handle(handle);
  if (idx == SIZE_MAX) return ERR_INVALID_HANDLE;
  RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  size_t start = (size_t) offset;
  uint8_t required = start >= entry->data_len ? CAP_RIGHT_APPEND : CAP_RIGHT_WRITE;
  int64_t err = ramfs_check_access(caller_pd, entry, required);
  if (err != 0) return err;
  if (start > RAMFS_MAX_FILE_SIZE || data_len > RAMFS_MAX_FILE_SIZE - start) return ERR_OVERFLOW;
  size_t end = start + data_len;
  if (end > entry->data_len) {
    memset(entry->data + entry->data_len, 0, end - entry->data_len);
    entry->data_len = end;
  }
  if (data_len > 0) memcpy(entry->data + start, data, data_len);
  *out_n = (uint64_t) data_len;
  return 0;
}
int64_t ramfs_close(uint64_t handle, uint32_t caller_pd) {
  size_t idx = ramfs_find_active_by_handle(handle);
  if (idx == SIZE_MAX) return ERR_INVALID_HANDLE;
  RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  int64_t err = ramfs_check_access(caller_pd, entry, CAP_RIGHT_READ);
  if (err != 0) return err;
  entry->active = false;
  return 0;
}
int64_t ramfs_stat(uint64_t handle, uint32_t caller_pd, uint64_t *out_size, uint32_t *out_name_len) {
  size_t idx = ramfs_find_active_by_handle(handle);
  if (idx == SIZE_MAX) return ERR_INVALID_HANDLE;
  const RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  int64_t err = ramfs_check_access(caller_pd, entry, CAP_RIGHT_READ);
  if (err != 0) return err;
  *out_size = (uint64_t) entry->data_len;
  *out_name_len = (uint32_t) entry->name_len;
  return 0;
}
bool ramfs_list_at(size_t index, uint32_t caller_pd, uint64_t *out_handle, uint32_t *out_name_len) {
  size_t count = 0;
  for (size_t i = 0; i < sexfiles_state.ramfs_files_len; ++i) {
    const RamfsFileEntry *entry = &sexfiles_state.ramfs_files[i];
    if (entry->in_use && entry->active) {
      bool allow = caller_pd == 0 || (caller_pd == entry->owner_pd || ramfs_check_access(caller_pd, entry, CAP_RIGHT_LIST) == 0);
      if (allow) {
        if (count == index) {
          *out_handle = entry->handle;
          *out_name_len = (uint32_t) entry->name_len;
          return true;
        }
        ++count;
      }
    }
  }
  return false;
}
size_t ramfs_len(uint32_t caller_pd) {
  size_t count = 0;
  for (size_t i = 0; i < sexfiles_state.ramfs_files_len; ++i) {
    const RamfsFileEntry *e = &sexfiles_state.ramfs_files[i];
    if (e->in_use && e->active) {
      if (caller_pd == 0) ++count;
      else if (e->owner_pd == caller_pd || ramfs_check_access(caller_pd, e, CAP_RIGHT_LIST) == 0) ++count;
    }
  }
  return count;
}
int64_t ramfs_create_with_owner(const uint8_t *name, size_t name_len, uint32_t owner_pd, uint32_t caller_pd, uint64_t *out_handle) {
  if (caller_pd != 0 && caller_pd != owner_pd) return ERR_PERM_DENIED;
  if (name_len > RAMFS_MAX_NAME) return ERR_NAME_TOO_LONG;
  if (name_len == 0) return ERR_NOT_FOUND;
  if (ramfs_find_any_by_name(name, name_len) != SIZE_MAX) return ERR_NOT_FOUND;
  if (sexfiles_state.ramfs_files_len >= RAMFS_SLOT_CAP) return ERR_FULL;
  uint64_t handle = sexfiles_state.ramfs_next_handle;
  sexfiles_state.ramfs_next_handle += 1;
  uint64_t object_id = sexfiles_state.ramfs_next_object_id;
  sexfiles_state.ramfs_next_object_id += 1;
  size_t idx = sexfiles_state.ramfs_files_len;
  sexfiles_state.ramfs_files_len += 1;
  RamfsFileEntry *entry = &sexfiles_state.ramfs_files[idx];
  memset(entry, 0, sizeof *entry);
  entry->handle = handle;
  entry->object_id = object_id;
  entry->active = true;
  entry->in_use = true;
  entry->owner_pd = owner_pd;
  entry->cap_generation = 1;
  memcpy(entry->name, name, name_len);
  entry->name_len = (uint8_t) name_len;
  entry->data_len = 0;
  *out_handle = handle;
  return 0;
}
/*VFS routing and DiskFS V4 dynamic variable-length object store (servers/sexfiles/src/vfs.rs).*/
static inline bool vfs_hot_log(void) {
  return sexfiles_take_budget(&sexfiles_state.vfs_hot_log_budget);
}
static inline uint64_t sat_add_u64(uint64_t a, uint64_t b) {
  uint64_t sum = a + b;
  return sum < a ? UINT64_MAX : sum;
}
static inline uint64_t read_le16(const uint8_t *p) {
  return (uint64_t) p[0] | (uint64_t) p[1] << 8;
}
static inline uint64_t read_le32(const uint8_t *p) {
  return (uint64_t) p[0] | (uint64_t) p[1] << 8 | ((uint64_t) p[2] << 16 | (uint64_t) p[3] << 24);
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
static uint64_t diskfs_selected_for(uint32_t caller_pd) {
  return sexfiles_state.diskfs_selected_path_id[(size_t) caller_pd % DISKFS_CLIENT_SLOTS];
}
uint64_t diskfs_bridge_get_buf_va(void) {
  uint64_t va = sexfiles_state.diskfs_bridge_buf_va;
  if (va != 0 && va != UINT64_MAX) {
    if (sexfiles_state.diskfs_bridge_reuse_printed == 0) {
      sexfiles_state.diskfs_bridge_reuse_printed = 1;
      sexos_line("[sexfiles.bridge.diskfs.buf.reuse] va=%#llx", (unsigned long long) va);
    }
    return va;
  }
  MemoryRegionReply info = sexos_memory_region_info(SEXOS_SLOT_buf_lend);
  if (info.status != 0 || (info.value0 < 4096 || info.value2 == 0)) return UINT64_MAX;
  MemoryRegionReply view = sexos_memory_region_view(SEXOS_SLOT_buf_lend, 0, 4096, REGION_QUERY | (REGION_READ | REGION_WRITE));
  uint64_t new_va = view.status == 0 ? view.value0 : UINT64_MAX;
  if (new_va != 0 && new_va != UINT64_MAX) {
    sexfiles_state.diskfs_bridge_buf_va = new_va;
    sexos_line("[sexfiles.bridge.diskfs.buf.ready] buf_va=%#llx", (unsigned long long) new_va);
  }
  return new_va;
}
static inline uint64_t v4_indirect_lba(size_t i) {
  return DISKFS_V4_INDIRECT_TOP_LBA - (uint64_t) i;
}
static size_t v4_name_len(const uint8_t *name) {
  size_t n = 0;
  while (n < DISKFS_V3_NAME_MAX && name[n] != 0) ++n;
  return n;
}
static bool v4_in_use(size_t i) {
  return sexfiles_state.v4_table[i].name[0] != 0;
}
static uint16_t v4_checksum_entry(const V4Entry *e) {
  uint32_t h = UINT32_C(0x811C9DC5);
  for (size_t i = 0; i < DISKFS_V3_NAME_MAX; ++i) {
    h ^= (uint32_t) e->name[i];
    h *= UINT32_C(0x01000193);
  }
  for (size_t i = 0; i < 4; ++i) {
    h ^= (uint32_t)(e->size_bytes >> i * 8 & 0xFFu);
    h *= UINT32_C(0x01000193);
  }
  for (size_t i = 0; i < 2; ++i) {
    h ^= (uint32_t)(e->gen >> i * 8 & 0xFFu);
    h *= UINT32_C(0x01000193);
  }
  return (uint16_t)(h ^ h >> 16);
}
static bool v4_bitmap_test(const uint64_t *bm, uint64_t block_idx) {
  size_t w = (size_t)(block_idx / 64);
  uint64_t b = block_idx % 64;
  return (bm[w] & UINT64_C(1) << b) != 0;
}
static void v4_bitmap_set(uint64_t *bm, uint64_t block_idx) {
  size_t w = (size_t)(block_idx / 64);
  uint64_t b = block_idx % 64;
  bm[w] |= UINT64_C(1) << b;
}
static void v4_bitmap_clear(uint64_t *bm, uint64_t block_idx) {
  size_t w = (size_t)(block_idx / 64);
  uint64_t b = block_idx % 64;
  bm[w] &= ~(UINT64_C(1) << b);
}
static void v4_cache_put(size_t i, const V4Extent *extents, size_t count) {
  sexfiles_state.v4_extent_cache.valid = true;
  sexfiles_state.v4_extent_cache.slot = i;
  memcpy(sexfiles_state.v4_extent_cache.extents, extents, sizeof sexfiles_state.v4_extent_cache.extents);
  sexfiles_state.v4_extent_cache.count = count;
}
static void v4_cache_invalidate(size_t i) {
  if (sexfiles_state.v4_extent_cache.valid && sexfiles_state.v4_extent_cache.slot == i) sexfiles_state.v4_extent_cache.valid = false;
}
static bool v4_indirect_read(size_t i, uint64_t buf_va, V4Extent *out_extents, size_t *out_count) {
  uint8_t sector[512];
  if (diskfs_block_read(v4_indirect_lba(i) * 512, 512, SEXOS_SLOT_buf_lend) != 0) return false;
  volatile const uint8_t *p = (volatile const uint8_t *)(uintptr_t) buf_va;
  for (size_t k = 0; k < 512; ++k) sector[k] = p[k];
  if (sector[0] != (uint8_t) 'S' || (sector[1] != (uint8_t) 'F' || (sector[2] != (uint8_t) 'E' || sector[3] != (uint8_t) 'X'))) return false;
  uint16_t version = (uint16_t) read_le16(sector + 4);
  if (version != 1) return false;
  size_t count = (size_t) read_le16(sector + 6);
  if (count > DISKFS_V4_MAX_EXTENTS) return false;
  uint32_t stored_cs = (uint32_t) read_le32(sector + 8);
  memset(out_extents, 0, sizeof sexfiles_state.v4_extent_cache.extents);
  uint32_t h = UINT32_C(0x811C9DC5);
  for (size_t k = 0; k < count; ++k) {
    size_t off = 12 + k * 4;
    uint16_t sb = (uint16_t) read_le16(sector + off);
    uint16_t sc = (uint16_t) read_le16(sector + (off + 2));
    out_extents[k] = (V4Extent) {
      .start_lba = sb,.sector_count = sc
    };
    for (size_t b = 0; b < 4; ++b) {
      h ^= (uint32_t) sector[off + b];
      h *= UINT32_C(0x01000193);
    }
  }
  if (h != stored_cs) return false;
  *out_count = count;
  return true;
}
static size_t v4_cache_get(size_t i, uint64_t buf_va, V4Extent *out_extents) {
  if (sexfiles_state.v4_extent_cache.valid && sexfiles_state.v4_extent_cache.slot == i) {
    memcpy(out_extents, sexfiles_state.v4_extent_cache.extents, sizeof sexfiles_state.v4_extent_cache.extents);
    return sexfiles_state.v4_extent_cache.count;
  }
  size_t count = 0;
  if (!v4_indirect_read(i, buf_va, out_extents, &count)) {
    memset(out_extents, 0, sizeof sexfiles_state.v4_extent_cache.extents);
    count = 0;
  }
  v4_cache_put(i, out_extents, count);
  return count;
}
static bool v4_indirect_write(size_t i, uint64_t buf_va, const V4Extent *extents, size_t count, uint64_t *out_err) {
  uint8_t sector[512];
  memset(sector, 0, 512);
  sector[0] = (uint8_t) 'S';
  sector[1] = (uint8_t) 'F';
  sector[2] = (uint8_t) 'E';
  sector[3] = (uint8_t) 'X';
  write_le16(sector + 4, 1);
  write_le16(sector + 6, (uint16_t) count);
  uint32_t h = UINT32_C(0x811C9DC5);
  for (size_t k = 0; k < count; ++k) {
    size_t off = 12 + k * 4;
    write_le16(sector + off, extents[k].start_lba);
    write_le16(sector + (off + 2), extents[k].sector_count);
    for (size_t b = 0; b < 4; ++b) {
      h ^= (uint32_t) sector[off + b];
      h *= UINT32_C(0x01000193);
    }
  }
  write_le32(sector + 8, h);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t k = 0; k < 512; ++k) p[k] = sector[k];
  uint64_t lba = v4_indirect_lba(i);
  uint64_t status = diskfs_block_write(lba * 512, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    *out_err = status;
    return false;
  }
  uint8_t check[512];
  if (diskfs_block_read(lba * 512, 512, SEXOS_SLOT_buf_lend) != 0) {
    *out_err = (uint64_t) ERR_BAD_CMD;
    return false;
  }
  for (size_t k = 0; k < 512; ++k) check[k] = p[k];
  if (memcmp(check, sector, 512) != 0) {
    *out_err = (uint64_t) ERR_BAD_CMD;
    return false;
  }
  return true;
}
static bool v4_zero_block(uint64_t start_lba, uint64_t buf_va, uint64_t *out_err) {
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  if (SEXOS_SEXFILES_CURRENT_DOCUMENT) {
    for (size_t k = 0; k < DISKFS_V4_BLOCK_BYTES; ++k) p[k] = 0;
    uint64_t status = diskfs_block_write(start_lba * 512, DISKFS_V4_BLOCK_BYTES, SEXOS_SLOT_buf_lend);
    if (status != 0) {
      *out_err = status;
      return false;
    }
  } else {
    for (size_t k = 0; k < 512; ++k) p[k] = 0;
    for (uint64_t s = 0; s < DISKFS_V4_BLOCK_SECTORS; ++s) {
      uint64_t status = diskfs_block_write((start_lba + s) * 512, 512, SEXOS_SLOT_buf_lend);
      if (status != 0) {
        *out_err = status;
        return false;
      }
    }
  }
  return true;
}
static bool v4_zero_tail(V4Extent e, uint64_t keep_len, uint64_t buf_va, uint64_t *out_err) {
  uint64_t cap = (uint64_t) e.sector_count * 512;
  if (keep_len >= cap) return true;
  uint64_t sector = keep_len / 512;
  size_t boundary_off = (size_t)(keep_len % 512);
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  if (boundary_off != 0) {
    uint64_t lba = (uint64_t) e.start_lba + sector;
    uint64_t rstatus = diskfs_block_read(lba * 512, 512, SEXOS_SLOT_buf_lend);
    if (rstatus != 0) {
      *out_err = rstatus;
      return false;
    }
    for (size_t k = boundary_off; k < 512; ++k) p[k] = 0;
    uint64_t status = diskfs_block_write(lba * 512, 512, SEXOS_SLOT_buf_lend);
    if (status != 0) {
      *out_err = status;
      return false;
    }
    sector += 1;
  }
  if (sector * 512 < cap) {
    for (size_t k = 0; k < 512; ++k) p[k] = 0;
    while (sector * 512 < cap) {
      uint64_t lba = (uint64_t) e.start_lba + sector;
      uint64_t status = diskfs_block_write(lba * 512, 512, SEXOS_SLOT_buf_lend);
      if (status != 0) {
        *out_err = status;
        return false;
      }
      sector += 1;
    }
  }
  return true;
}
static void v4_free_pool_only(uint64_t *bm, const V4Extent *freed, size_t freed_len) {
  uint64_t pool_end_lba = DISKFS_V4_POOL_BASE_LBA + DISKFS_V4_POOL_BLOCKS * DISKFS_V4_BLOCK_SECTORS;
  for (size_t k = 0; k < freed_len; ++k) {
    V4Extent e = freed[k];
    uint64_t start = (uint64_t) e.start_lba;
    uint64_t end = start + (uint64_t) e.sector_count;
    if (start >= DISKFS_V4_POOL_BASE_LBA && (end <= pool_end_lba && ((start - DISKFS_V4_POOL_BASE_LBA) % DISKFS_V4_BLOCK_SECTORS == 0 && (uint64_t) e.sector_count % DISKFS_V4_BLOCK_SECTORS == 0))) {
      uint64_t b0 = (start - DISKFS_V4_POOL_BASE_LBA) / DISKFS_V4_BLOCK_SECTORS;
      uint64_t bn = (uint64_t) e.sector_count / DISKFS_V4_BLOCK_SECTORS;
      for (uint64_t b = 0; b < bn; ++b) v4_bitmap_clear(bm, b0 + b);
    }
  }
}
static bool v4_allocate(uint64_t *bm, uint64_t need_blocks, V4Extent *out, size_t n, size_t *out_n, int64_t *out_err) {
  size_t start_n = n;
  uint64_t need = need_blocks;
  uint64_t block_idx = 0;
  while (need > 0) {
    if (block_idx >= DISKFS_V4_POOL_BLOCKS) {
      v4_free_pool_only(bm, out + start_n, n - start_n);
      *out_err = ERR_FULL;
      return false;
    }
    if (v4_bitmap_test(bm, block_idx)) {
      block_idx += 1;
      continue;
    }
    uint64_t run_start = block_idx;
    uint64_t run_len = 0;
    while (block_idx < DISKFS_V4_POOL_BLOCKS && (!v4_bitmap_test(bm, block_idx) && run_len < need)) {
      run_len += 1;
      block_idx += 1;
    }
    if (n >= DISKFS_V4_MAX_EXTENTS) {
      v4_free_pool_only(bm, out + start_n, n - start_n);
      *out_err = ERR_FULL;
      return false;
    }
    for (uint64_t b = 0; b < run_len; ++b) v4_bitmap_set(bm, run_start + b);
    out[n] = (V4Extent) {
      .start_lba = (uint16_t)(DISKFS_V4_POOL_BASE_LBA + run_start * DISKFS_V4_BLOCK_SECTORS),.sector_count = (uint16_t)(run_len * DISKFS_V4_BLOCK_SECTORS)
    };
    n += 1;
    need -= run_len;
  }
  *out_n = n;
  return true;
}
static bool v4_persist(uint64_t buf_va, uint64_t *out_err) {
  uint8_t sector[512];
  memset(sector, 0, 512);
  write_le64(sector + 0, DISKFS_MANIFEST_MAGIC);
  write_le16(sector + 8, DISKFS_V4_VERSION);
  write_le16(sector + 10, (uint16_t) DISKFS_SLOTS);
  uint64_t prev_gen = sexfiles_state.v4_generation;
  sexfiles_state.v4_generation = prev_gen + 1;
  uint32_t g = (uint32_t) prev_gen + 1;
  write_le32(sector + 12, g);
  for (size_t i = 0; i < DISKFS_SLOTS; ++i) {
    V4Entry e = sexfiles_state.v4_table[i];
    size_t off = 16 + i * 32;
    memcpy(sector + off, e.name, DISKFS_V3_NAME_MAX);
    write_le32(sector + (off + 24), e.size_bytes);
    write_le16(sector + (off + 28), v4_checksum_entry(&e));
    write_le16(sector + (off + 30), e.gen);
  }
  volatile uint8_t *p = (volatile uint8_t *)(uintptr_t) buf_va;
  for (size_t k = 0; k < 512; ++k) p[k] = sector[k];
  uint64_t status = diskfs_block_write(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend);
  if (status != 0) {
    *out_err = status;
    return false;
  }
  uint8_t check[512];
  if (diskfs_block_read(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend) != 0) {
    *out_err = (uint64_t) ERR_BAD_CMD;
    return false;
  }
  for (size_t k = 0; k < 512; ++k) check[k] = p[k];
  if (memcmp(check, sector, 512) != 0) {
    sexos_line("[sexfiles.diskfs.v4.persist.err] reason=verify_mismatch");
    *out_err = (uint64_t) ERR_BAD_CMD;
    return false;
  }
  sexos_line("[sexfiles.diskfs.v4.persist.ok] generation=%u", (unsigned) g);
  return true;
}
static void v4_bitmap_rebuild(uint64_t buf_va) {
  uint64_t bm[3] = {
    0, 0, 0
  };
  bool dropped = false;
  uint64_t pool_end_lba = DISKFS_V4_POOL_BASE_LBA + DISKFS_V4_POOL_BLOCKS * DISKFS_V4_BLOCK_SECTORS;
  for (size_t i = 0; i < DISKFS_SLOTS; ++i) {
    if (!v4_in_use(i)) continue;
    uint64_t size_bytes = (uint64_t) sexfiles_state.v4_table[i].size_bytes;
    if (size_bytes == 0) continue;
    V4Extent extents[DISKFS_V4_MAX_EXTENTS];
    size_t count = 0;
    if (!v4_indirect_read(i, buf_va, extents, &count)) {
      sexos_line("[sexfiles.diskfs.v4.rebuild.drop] slot=%zu reason=indirect_unreadable", i);
      memset(&sexfiles_state.v4_table[i], 0, sizeof sexfiles_state.v4_table[0]);
      dropped = true;
      continue;
    }
    bool ok = true;
    uint64_t total_bytes = 0;
    for (size_t k = 0; k < count; ++k) {
      V4Extent e = extents[k];
      if (e.sector_count == 0) {
        ok = false;
        break;
      }
      uint64_t start = (uint64_t) e.start_lba;
      uint64_t end = start + (uint64_t) e.sector_count;
      if (start >= DISKFS_V4_POOL_BASE_LBA && end <= pool_end_lba) {
        if ((start - DISKFS_V4_POOL_BASE_LBA) % DISKFS_V4_BLOCK_SECTORS != 0 || (uint64_t) e.sector_count % DISKFS_V4_BLOCK_SECTORS != 0) {
          ok = false;
          break;
        }
        uint64_t b0 = (start - DISKFS_V4_POOL_BASE_LBA) / DISKFS_V4_BLOCK_SECTORS;
        uint64_t bn = (uint64_t) e.sector_count / DISKFS_V4_BLOCK_SECTORS;
        bool bad = false;
        for (uint64_t b = 0; b < bn; ++b) if (v4_bitmap_test(bm, b0 + b)) {
          bad = true;
          break;
        }
        if (bad) {
          ok = false;
          break;
        }
        for (uint64_t b = 0; b < bn; ++b) v4_bitmap_set(bm, b0 + b);
      } else if (end > DISKFS_V4_POOL_BASE_LBA && start < pool_end_lba) {
        ok = false;
        break;
      }
      total_bytes += (uint64_t) e.sector_count * 512;
    }
    if (!ok || total_bytes < size_bytes) {
      sexos_line("[sexfiles.diskfs.v4.rebuild.drop] slot=%zu reason=extent_overlap_or_short", i);
      memset(&sexfiles_state.v4_table[i], 0, sizeof sexfiles_state.v4_table[0]);
      dropped = true;
    }
  }
  memcpy(sexfiles_state.v4_bitmap, bm, sizeof bm);
  if (dropped) {
    uint64_t dummy_err = 0;
    (void) v4_persist(buf_va, &dummy_err);
  }
}
static bool v4_locate(const V4Extent *extents, size_t count, uint64_t logical_offset, DiskManifestEntryV1 *out_piece, uint64_t *out_rel_off, uint64_t *out_err) {
  uint64_t cum = 0;
  for (size_t k = 0; k < count; ++k) {
    V4Extent e = extents[k];
    uint64_t cap = (uint64_t) e.sector_count * 512;
    if (logical_offset < cum + cap) {
      *out_piece = (DiskManifestEntryV1) {
        .name_hash = 0,.start_lba = (uint64_t) e.start_lba,.len_bytes = (uint32_t) cap,.flags = 0
      };
      *out_rel_off = logical_offset - cum;
      return true;
    }
    cum += cap;
  }
  *out_err = (uint64_t) ERR_OVERFLOW;
  return false;
}
static bool v4_resolve_offset(size_t i, uint64_t logical_offset, uint64_t buf_va, DiskManifestEntryV1 *out_piece, uint64_t *out_rel_off, uint64_t *out_err) {
  V4Extent extents[DISKFS_V4_MAX_EXTENTS];
  size_t count = v4_cache_get(i, buf_va, extents);
  return v4_locate(extents, count, logical_offset, out_piece, out_rel_off, out_err);
}
static bool v4_get(uint64_t path_id, V4Entry *out_entry, uint64_t *out_err) {
  if (path_id >= (uint64_t) DISKFS_SLOTS) {
    *out_err = (uint64_t) ERR_BAD_CMD;
    return false;
  }
  size_t i = (size_t) path_id;
  if (!v4_in_use(i)) {
    *out_err = (uint64_t) ERR_NOT_FOUND;
    return false;
  }
  *out_entry = sexfiles_state.v4_table[i];
  return true;
}
static inline bool ranges_overlap(uint64_t a_start, uint64_t a_len, uint64_t b_start, uint64_t b_len) {
  return a_start < b_start + b_len && b_start < a_start + a_len;
}
static void v4_layout_selfcheck(void) {
  uint64_t pool_end = DISKFS_V4_POOL_BASE_LBA + DISKFS_V4_POOL_BLOCKS * DISKFS_V4_BLOCK_SECTORS;
  typedef struct ReservedRegion {
    const char *name;
    uint64_t start;
    uint64_t len;
  }
  ReservedRegion;
  const ReservedRegion reserved[6] = {
    {
      "sexdrive_ap3", SEXDRIVE_AP3_WRITE_PROOF_LBA, SEXDRIVE_AP3_WRITE_PROOF_SECTORS
    }, {
      "sexdrive_ap4", SEXDRIVE_AP4_MULTI_BASE_LBA, SEXDRIVE_AP4_MULTI_SECTORS
    }, {
      "sexdrive_ap5a", SEXDRIVE_AP5A_PERSIST_BASE_LBA, SEXDRIVE_AP5A_PERSIST_SECTORS
    }, {
      "sexdrive_ap6", SEXDRIVE_AP6_NEG_MISMATCH_LBA, SEXDRIVE_AP6_NEG_MISMATCH_SECTORS
    }, {
      "diskfs_manifest", DISKFS_MANIFEST_LBA, DISKFS_MANIFEST_SECTORS
    }, {
      "diskfs_legacy_slots", DISKFS_LEGACY_SLOTS_START_LBA, DISKFS_LEGACY_SLOTS_SECTORS
    }
  };
  bool ok = true;
  for (size_t i = 0; i < 6; ++i) if (ranges_overlap(DISKFS_V4_POOL_BASE_LBA, DISKFS_V4_POOL_BLOCKS * DISKFS_V4_BLOCK_SECTORS, reserved[i].start, reserved[i].len)) {
    sexos_line("[sexfiles.diskfs.v4.layout.fail] pool=%llu..%llu collides_with=%s region=%llu..%llu", (unsigned long long) DISKFS_V4_POOL_BASE_LBA, (unsigned long long) pool_end, reserved[i].name, (unsigned long long) reserved[i].start, (unsigned long long)(reserved[i].start + reserved[i].len));
    ok = false;
  }
  if (ok) sexos_line("[sexfiles.diskfs.v4.layout.ok] pool=%llu..%llu indirect_top=%llu manifest=%llu", (unsigned long long) DISKFS_V4_POOL_BASE_LBA, (unsigned long long) pool_end, (unsigned long long) DISKFS_V4_INDIRECT_TOP_LBA, (unsigned long long) DISKFS_MANIFEST_LBA);
}
static bool v4_ensure(uint64_t buf_va, uint64_t *out_err) {
  if (sexfiles_state.v4_loaded != 0) return true;
  v4_layout_selfcheck();
  uint8_t sector[512];
  if (diskfs_block_read(DISKFS_MANIFEST_LBA * 512, 512, SEXOS_SLOT_buf_lend) != 0) {
    *out_err = (uint64_t) ERR_BAD_CMD;
    return false;
  }
  volatile const uint8_t *p = (volatile const uint8_t *)(uintptr_t) buf_va;
  for (size_t k = 0; k < 512; ++k) sector[k] = p[k];
  uint64_t magic = read_le64(sector + 0);
  uint16_t version = (uint16_t) read_le16(sector + 8);
  if (magic == DISKFS_MANIFEST_MAGIC && version == DISKFS_V4_VERSION) {
    uint32_t g = (uint32_t) read_le32(sector + 12);
    sexfiles_state.v4_generation = (uint64_t) g;
    uint32_t live = 0;
    for (size_t i = 0; i < DISKFS_SLOTS; ++i) {
      size_t off = 16 + i * 32;
      V4Entry e;
      memset(&e, 0, sizeof e);
      memcpy(e.name, sector + off, DISKFS_V3_NAME_MAX);
      e.size_bytes = (uint32_t) read_le32(sector + (off + 24));
      uint16_t stored_cs = (uint16_t) read_le16(sector + (off + 28));
      e.gen = (uint16_t) read_le16(sector + (off + 30));
      if (e.name[0] != 0 && v4_checksum_entry(&e) != stored_cs) {
        sexos_line("[sexfiles.diskfs.v4.load.drop] slot=%zu reason=checksum", i);
        memset(&e, 0, sizeof e);
      }
      if (e.name[0] != 0) ++live;
      sexfiles_state.v4_table[i] = e;
    }
    sexfiles_state.v4_loaded = 1;
    sexos_line("[sexfiles.diskfs.v4.load.ok] live=%u generation=%u", (unsigned) live, (unsigned) g);
    v4_bitmap_rebuild(buf_va);
    return true;
  }
  bool recognized_legacy = magic == DISKFS_MANIFEST_MAGIC && version < DISKFS_V4_VERSION;
  bool genuinely_blank = true;
  for (size_t k = 0; k < 512; ++k) if (sector[k] != 0) {
    genuinely_blank = false;
    break;
  }
  if (!recognized_legacy && !genuinely_blank) {
    sexos_line("[sexfiles.diskfs.v4.mount.err] reason=corrupt_manifest magic=%#llx version=%u", (unsigned long long) magic, (unsigned) version);
    *out_err = (uint64_t) ERR_CORRUPT;
    return false;
  }
  typedef struct LegacySeed {
    const char *name;
    size_t len;
    uint16_t lba;
  }
  LegacySeed;
  const LegacySeed legacy[3] = {
    {
      "sexfiles-proof-v1", 17, 2038
    }, {
      "linen-object-v1", 15, 2030
    }, {
      "quil-object-v1", 14, 2022
    }
  };
  for (size_t i = 0; i < DISKFS_SLOTS; ++i) if (i < 3) {
    V4Entry e;
    memset(&e, 0, sizeof e);
    memcpy(e.name, legacy[i].name, legacy[i].len);
    e.size_bytes = 4096;
    e.gen = 1;
    sexfiles_state.v4_table[i] = e;
    V4Extent extents[1] = {
      {
        legacy[i].lba, 8
      }
    };
    uint64_t er = 0;
    if (!v4_indirect_write(i, buf_va, extents, 1, &er)) {
      sexos_line("[sexfiles.diskfs.v4.migrate.err] slot=%zu err=%llu", i, (unsigned long long) er);
      *out_err = er;
      return false;
    }
  } else memset(&sexfiles_state.v4_table[i], 0, sizeof sexfiles_state.v4_table[0]);
  if (!v4_persist(buf_va, out_err)) return false;
  sexfiles_state.v4_loaded = 1;
  sexos_line("[sexfiles.diskfs.v4.migrate.ok] from_version=%u seeded=3", (unsigned) version);
  v4_bitmap_rebuild(buf_va);
  return true;
}
static size_t v4_unpack_name(uint64_t lo, uint64_t hi, uint8_t *out_name) {
  memset(out_name, 0, DISKFS_V3_NAME_MAX);
  write_le64(out_name + 0, lo);
  write_le64(out_name + 8, hi);
  size_t n = 0;
  while (n < 16 && out_name[n] != 0) {
    if (out_name[n] < 0x20 || out_name[n] > 0x7E) out_name[n] = (uint8_t) '_';
    ++n;
  }
  for (size_t k = n; k < DISKFS_V3_NAME_MAX; ++k) out_name[k] = 0;
  return n;
}
static uint64_t handle_diskfs_select(uint64_t path_id, uint32_t caller_pd) {
  if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.recv] op=0x3E select path_id=%llu", (unsigned long long) path_id);
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.bridge.diskfs.select.err] reason=grant_failed");
    return (uint64_t) ERR_NOT_FOUND;
  }
  uint64_t err = 0;
  if (!v4_ensure(buf_va, &err)) {
    sexos_line("[sexfiles.bridge.diskfs.select.err] reason=v4_ensure_failed code=%llu", (unsigned long long) err);
    return err;
  }
  V4Entry entry;
  if (v4_get(path_id, &entry, &err)) {
    sexfiles_state.diskfs_selected_path_id[(size_t) caller_pd % DISKFS_CLIENT_SLOTS] = path_id;
    if (sexfiles_state.diskfs_select_used == 0) {
      sexfiles_state.diskfs_select_used = 1;
      sexos_line("[sexfiles.bridge.diskfs.select.v1_single_client]");
    }
    sexos_line("[sexfiles.bridge.diskfs.select.ok] caller=%u path_id=%llu", (unsigned) caller_pd, (unsigned long long) path_id);
    return 0;
  }
  sexos_line("[sexfiles.bridge.diskfs.select.err] path_id=%llu code=%llu", (unsigned long long) path_id, (unsigned long long) err);
  return err;
}
static uint64_t handle_diskfs_write(uint64_t byte_offset, uint64_t data_lo, uint64_t data_hi, uint32_t caller_pd) {
  if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.recv] op=0x38 offset=%llu", (unsigned long long) byte_offset);
  if (byte_offset >= DISKFS_V4_MAX_OBJECT_BYTES || sat_add_u64(byte_offset, (uint64_t) DISKFS_MAX_WRITE) > DISKFS_V4_MAX_OBJECT_BYTES) {
    sexos_line("[sexfiles.bridge.diskfs.write.err] reason=cap_exceeded offset=%llu cap=%llu", (unsigned long long) byte_offset, (unsigned long long) DISKFS_V4_MAX_OBJECT_BYTES);
    return (uint64_t) ERR_OVERFLOW;
  }
  if (sat_add_u64(byte_offset % DISKFS_V4_BLOCK_BYTES, (uint64_t) DISKFS_MAX_WRITE) > DISKFS_V4_BLOCK_BYTES) {
    sexos_line("[sexfiles.bridge.diskfs.write.err] reason=block_straddle offset=%llu", (unsigned long long) byte_offset);
    return (uint64_t) ERR_OVERFLOW;
  }
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.bridge.diskfs.write.err] reason=grant_failed buf_va=%#llx", (unsigned long long) buf_va);
    return (uint64_t) ERR_NOT_FOUND;
  }
  uint64_t err = 0;
  if (!v4_ensure(buf_va, &err)) {
    sexos_line("[sexfiles.bridge.diskfs.write.err] reason=v4_ensure_failed code=%llu", (unsigned long long) err);
    return err;
  }
  uint64_t path_id = diskfs_selected_for(caller_pd);
  if (path_id >= (uint64_t) DISKFS_SLOTS) {
    sexos_line("[sexfiles.bridge.diskfs.write.err] reason=bad_path_id caller=%u path_id=%llu", (unsigned) caller_pd, (unsigned long long) path_id);
    return (uint64_t) ERR_BAD_CMD;
  }
  size_t i = (size_t) path_id;
  if (!v4_in_use(i)) {
    sexos_line("[sexfiles.bridge.diskfs.write.err] reason=slot_not_in_use caller=%u path_id=%llu", (unsigned) caller_pd, (unsigned long long) path_id);
    return (uint64_t) ERR_NOT_FOUND;
  }
  V4Entry entry = sexfiles_state.v4_table[i];
  uint64_t need_end = sat_add_u64(byte_offset, (uint64_t) DISKFS_MAX_WRITE);
  V4Extent extents[DISKFS_V4_MAX_EXTENTS];
  size_t orig_count = v4_cache_get(i, buf_va, extents);
  size_t count = orig_count;
  uint64_t cur_capacity = 0;
  for (size_t k = 0; k < count; ++k) cur_capacity += (uint64_t) extents[k].sector_count * 512;
  if (need_end > cur_capacity) {
    uint64_t more_bytes = need_end - cur_capacity;
    uint64_t more_blocks = (more_bytes + DISKFS_V4_BLOCK_BYTES - 1) / DISKFS_V4_BLOCK_BYTES;
    size_t new_count = 0;
    int64_t alloc_err = 0;
    if (!v4_allocate(sexfiles_state.v4_bitmap, more_blocks, extents, count, &new_count, &alloc_err)) {
      sexos_line("[sexfiles.bridge.diskfs.write.err] reason=alloc_failed need_blocks=%llu", (unsigned long long) more_blocks);
      return (uint64_t) alloc_err;
    }
    for (size_t k = count; k < new_count; ++k) {
      V4Extent e = extents[k];
      uint64_t s = 0;
      while (s < (uint64_t) e.sector_count) {
        uint64_t er = 0;
        if (!v4_zero_block((uint64_t) e.start_lba + s, buf_va, &er)) {
          sexos_line("[sexfiles.bridge.diskfs.write.err] reason=zero_fill_failed lba=%llu code=%llu", (unsigned long long)((uint64_t) e.start_lba + s), (unsigned long long) er);
          v4_free_pool_only(sexfiles_state.v4_bitmap, extents + count, new_count - count);
          return er;
        }
        s += DISKFS_V4_BLOCK_SECTORS;
      }
    }
    count = new_count;
  }
  DiskManifestEntryV1 piece;
  uint64_t rel_off = 0;
  if (!v4_locate(extents, count, byte_offset, &piece, &rel_off, &err)) {
    sexos_line("[sexfiles.bridge.diskfs.write.err] reason=locate_failed offset=%llu count=%zu code=%llu", (unsigned long long) byte_offset, count, (unsigned long long) err);
    if (count > orig_count) v4_free_pool_only(sexfiles_state.v4_bitmap, extents + orig_count, count - orig_count);
    return err;
  }
  uint8_t inline_data[16];
  write_le64(inline_data + 0, data_lo);
  write_le64(inline_data + 8, data_hi);
  uint64_t written = 0;
  if (!diskfs_write_object_entry(piece, rel_off, inline_data, 16, buf_va, &written, &err)) {
    sexos_line("[sexfiles.bridge.diskfs.write.err] offset=%llu code=%llu", (unsigned long long) byte_offset, (unsigned long long) err);
    if (count > orig_count) v4_free_pool_only(sexfiles_state.v4_bitmap, extents + orig_count, count - orig_count);
    return err;
  }
  if (count > orig_count) {
    if (!v4_indirect_write(i, buf_va, extents, count, &err)) {
      v4_free_pool_only(sexfiles_state.v4_bitmap, extents + orig_count, count - orig_count);
      return err;
    }
    v4_cache_put(i, extents, count);
    sexos_line("[sexfiles.diskfs.v4.crash_point.extent_committed] slot=%zu new_count=%zu", i, count);
  }
  if (need_end > (uint64_t) entry.size_bytes) {
    entry.size_bytes = (uint32_t) need_end;
    sexfiles_state.v4_table[i] = entry;
    if (!v4_persist(buf_va, &err)) return err;
    sexos_line("[sexfiles.diskfs.v4.crash_point.manifest_committed] slot=%zu size=%u", i, (unsigned) entry.size_bytes);
  }
  sexos_line("[sexfiles.bridge.diskfs.write.ok] offset=%llu written=%llu", (unsigned long long) byte_offset, (unsigned long long) written);
  return written;
}
static uint64_t handle_diskfs_read(uint64_t byte_offset, uint64_t max_len, uint32_t caller_pd) {
  if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.recv] op=0x39 offset=%llu", (unsigned long long) byte_offset);
  if (max_len == 0 || max_len > (uint64_t) DISKFS_MAX_READ) {
    sexos_line("[sexfiles.bridge.diskfs.read.err] reason=bad_max_len max_len=%llu", (unsigned long long) max_len);
    return (uint64_t) ERR_OVERFLOW;
  }
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) {
    sexos_line("[sexfiles.bridge.diskfs.read.err] reason=grant_failed buf_va=%#llx", (unsigned long long) buf_va);
    return (uint64_t) ERR_NOT_FOUND;
  }
  uint64_t err = 0;
  if (!v4_ensure(buf_va, &err)) {
    sexos_line("[sexfiles.bridge.diskfs.read.err] reason=v4_ensure_failed code=%llu", (unsigned long long) err);
    return err;
  }
  uint64_t path_id = diskfs_selected_for(caller_pd);
  V4Entry entry;
  if (!v4_get(path_id, &entry, &err)) return err;
  if (byte_offset >= (uint64_t) entry.size_bytes || sat_add_u64(byte_offset, max_len) > (uint64_t) entry.size_bytes) {
    sexos_line("[sexfiles.bridge.diskfs.read.err] reason=read_past_end offset=%llu max_len=%llu size=%u", (unsigned long long) byte_offset, (unsigned long long) max_len, (unsigned) entry.size_bytes);
    return (uint64_t) ERR_OVERFLOW;
  }
  DiskManifestEntryV1 piece;
  uint64_t rel_off = 0;
  if (!v4_resolve_offset((size_t) path_id, byte_offset, buf_va, &piece, &rel_off, &err)) return err;
  size_t rlen = (size_t) max_len;
  uint8_t rbuf[8] = {
    0, 0, 0, 0, 0, 0, 0, 0
  };
  uint64_t n = 0;
  if (diskfs_read_object_entry(piece, rel_off, rbuf, rlen, buf_va, &n, &err)) {
    uint64_t reply = 0;
    size_t i = 0;
    while (i < (size_t) n && i < 8) {
      reply |= (uint64_t) rbuf[i] << i * 8;
      ++i;
    }
    sexos_line("[sexfiles.bridge.diskfs.read.ok] offset=%llu read=%llu", (unsigned long long) byte_offset, (unsigned long long) n);
    if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.read.reply] caller=%u value=%#llx off=%llu read=%llu", (unsigned) caller_pd, (unsigned long long) reply, (unsigned long long) byte_offset, (unsigned long long) n);
    return reply;
  }
  sexos_line("[sexfiles.bridge.diskfs.read.err] caller=%u err=%llu off=%llu", (unsigned) caller_pd, (unsigned long long) err, (unsigned long long) byte_offset);
  return err;
}
static inline uint64_t diskfs_v2_pack_err(uint64_t e) {
  uint64_t mag = (int64_t) e < 0 ? UINT64_C(0) - e : e;
  return DISKFS_V2_STATUS_ERR << 56 | (mag & 0xFFu) << 48;
}
static inline uint64_t diskfs_v2_encode_eof(void) {
  return DISKFS_V2_STATUS_EOF << 56;
}
static inline uint64_t diskfs_v2_encode_ok(size_t n, const uint8_t *payload) {
  size_t len = n < DISKFS_V2_MAX_READ ? n : DISKFS_V2_MAX_READ;
  uint64_t word = 0;
  for (size_t i = 0; i < len; ++i) word |= (uint64_t) payload[i] << i * 8;
  return DISKFS_V2_STATUS_OK << 56 | ((uint64_t) len & 0xFFu) << 48 | (word & UINT64_C(0xFFFFFFFFFFFF));
}
static uint64_t handle_diskfs_read_v2(uint64_t byte_offset, uint64_t want_len, uint32_t caller_pd) {
  if (want_len == 0 || want_len > (uint64_t) DISKFS_V2_MAX_READ) {
    sexos_line("[sexfiles.bridge.diskfs.read_v2.err] reason=bad_want_len want_len=%llu", (unsigned long long) want_len);
    return diskfs_v2_pack_err((uint64_t) ERR_OVERFLOW);
  }
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return diskfs_v2_pack_err((uint64_t) ERR_NOT_FOUND);
  uint64_t err = 0;
  if (!v4_ensure(buf_va, &err)) return diskfs_v2_pack_err(err);
  uint64_t path_id = diskfs_selected_for(caller_pd);
  V4Entry entry;
  if (!v4_get(path_id, &entry, &err)) return diskfs_v2_pack_err(err);
  if (byte_offset == (uint64_t) entry.size_bytes) return diskfs_v2_encode_eof();
  if (byte_offset > (uint64_t) entry.size_bytes || sat_add_u64(byte_offset, want_len) > (uint64_t) entry.size_bytes) {
    sexos_line("[sexfiles.bridge.diskfs.read_v2.err] reason=read_past_end offset=%llu want_len=%llu size=%u", (unsigned long long) byte_offset, (unsigned long long) want_len, (unsigned) entry.size_bytes);
    return diskfs_v2_pack_err((uint64_t) ERR_OVERFLOW);
  }
  DiskManifestEntryV1 piece;
  uint64_t rel_off = 0;
  if (!v4_resolve_offset((size_t) path_id, byte_offset, buf_va, &piece, &rel_off, &err)) return diskfs_v2_pack_err(err);
  size_t rlen = (size_t) want_len;
  uint8_t rbuf[6] = {
    0, 0, 0, 0, 0, 0
  };
  uint64_t n = 0;
  if (diskfs_read_object_entry(piece, rel_off, rbuf, rlen, buf_va, &n, &err)) {
    size_t nr = (size_t) n;
    sexos_line("[sexfiles.bridge.diskfs.read_v2.ok] offset=%llu read=%zu", (unsigned long long) byte_offset, nr);
    return diskfs_v2_encode_ok(nr, rbuf);
  }
  sexos_line("[sexfiles.bridge.diskfs.read_v2.err] caller=%u err=%llu off=%llu", (unsigned) caller_pd, (unsigned long long) err, (unsigned long long) byte_offset);
  return diskfs_v2_pack_err(err);
}
static uint64_t handle_diskfs_truncate(uint64_t new_len, uint32_t caller_pd) {
  if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.recv] op=0x49 new_len=%llu", (unsigned long long) new_len);
  uint64_t buf_va = diskfs_bridge_get_buf_va();
  if (buf_va == 0 || buf_va == UINT64_MAX) return (uint64_t) ERR_NOT_FOUND;
  uint64_t err = 0;
  if (!v4_ensure(buf_va, &err)) return err;
  uint64_t path_id = diskfs_selected_for(caller_pd);
  if (path_id >= (uint64_t) DISKFS_SLOTS) return (uint64_t) ERR_BAD_CMD;
  size_t i = (size_t) path_id;
  if (!v4_in_use(i)) return (uint64_t) ERR_NOT_FOUND;
  V4Entry entry = sexfiles_state.v4_table[i];
  if (new_len > (uint64_t) entry.size_bytes) {
    sexos_line("[sexfiles.bridge.diskfs.truncate.err] reason=would_grow new_len=%llu size=%u", (unsigned long long) new_len, (unsigned) entry.size_bytes);
    return (uint64_t) ERR_OVERFLOW;
  }
  if (new_len == (uint64_t) entry.size_bytes) return new_len;
  entry.size_bytes = (uint32_t) new_len;
  sexfiles_state.v4_table[i] = entry;
  if (!v4_persist(buf_va, &err)) return err;
  sexos_line("[sexfiles.diskfs.v4.crash_point.manifest_committed] slot=%zu size=%u", i, (unsigned) entry.size_bytes);
  V4Extent extents[DISKFS_V4_MAX_EXTENTS];
  size_t count = v4_cache_get(i, buf_va, extents);
  uint64_t cum = 0;
  size_t keep = 0;
  V4Extent freed[DISKFS_V4_MAX_EXTENTS];
  size_t freed_n = 0;
  for (size_t k = 0; k < count; ++k) {
    V4Extent e = extents[k];
    uint64_t cap = (uint64_t) e.sector_count * 512;
    if (cum + cap <= new_len) {
      extents[keep] = e;
      ++keep;
    } else if (cum >= new_len) {
      freed[freed_n] = e;
      ++freed_n;
    } else {
      extents[keep] = e;
      ++keep;
      if (!v4_zero_tail(e, new_len - cum, buf_va, &err)) return err;
    }
    cum += cap;
  }
  if (freed_n > 0) {
    if (!v4_indirect_write(i, buf_va, extents, keep, &err)) return err;
    v4_cache_put(i, extents, keep);
    v4_free_pool_only(sexfiles_state.v4_bitmap, freed, freed_n);
  }
  sexos_line("[sexfiles.bridge.diskfs.truncate.ok] new_len=%llu freed_extents=%zu", (unsigned long long) new_len, freed_n);
  return new_len;
}
static uint64_t handle_diskfs_flush(void) {
  if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.recv] op=0x3A");
  uint64_t status = diskfs_fsync();
  if (status == 0) sexos_line("[sexfiles.bridge.diskfs.flush.ok]");
  else sexos_line("[sexfiles.bridge.diskfs.flush.err] status=%llu honest=flush_not_emulated_by_qemu_nvme", (unsigned long long) status);
  return status;
}
static uint64_t handle_diskfs_stat(uint32_t caller_pd) {
  if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.recv] op=0x3B");
  uint64_t path_id = diskfs_selected_for(caller_pd);
  V4Entry entry;
  uint64_t err = 0;
  if (!v4_get(path_id, &entry, &err)) return err;
  size_t nlen = v4_name_len(entry.name);
  char path_str[25];
  memcpy(path_str, entry.name, nlen);
  path_str[nlen] = 0;
  uint64_t flags = (uint64_t)(DISKFS_MANIFEST_FLAG_READ | DISKFS_MANIFEST_FLAG_WRITE);
  uint64_t size = (uint64_t) entry.size_bytes;
  uint64_t packed = flags << 32 | (size & UINT64_C(0xFFFFFFFF));
  if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.stat.ok] path_id=%llu path=%s size=%llu flags=%#llx", (unsigned long long) path_id, path_str, (unsigned long long) size, (unsigned long long) flags);
  return packed;
}
static uint64_t handle_diskfs_manifest_hash(uint32_t caller_pd) {
  if (vfs_hot_log()) sexos_line("[sexfiles.bridge.diskfs.recv] op=0x3C");
  uint64_t path_id = diskfs_selected_for(caller_pd);
  V4Entry entry;
  uint64_t err = 0;
  if (!v4_get(path_id, &entry, &err)) return err;
  size_t nlen = v4_name_len(entry.name);
  uint64_t hash = diskfs_manifest_name_hash(entry.name, nlen);
  sexos_line("[sexfiles.bridge.diskfs.manifest_hash.ok] path_id=%llu hash=%#llx", (unsigned long long) path_id, (unsigned long long) hash);
  return hash;
}
static uint64_t handle_diskfs_create(uint64_t name_lo, uint64_t name_hi, uint64_t buf_va) {
  uint8_t name[DISKFS_V3_NAME_MAX];
  size_t n = v4_unpack_name(name_lo, name_hi, name);
  if (n == 0) return (uint64_t) ERR_BAD_CMD;
  for (size_t i = 0; i < DISKFS_SLOTS; ++i) if (v4_in_use(i) && memcmp(sexfiles_state.v4_table[i].name, name, DISKFS_V3_NAME_MAX) == 0) {
    sexos_line("[sexfiles.diskfs.v4.create.err] reason=exists slot=%zu", i);
    return (uint64_t) ERR_EXISTS;
  }
  size_t slot = SIZE_MAX;
  for (size_t i = (size_t) DISKFS_V3_SYSTEM_SLOTS; i < DISKFS_SLOTS; ++i) if (!v4_in_use(i)) {
    slot = i;
    break;
  }
  if (slot == SIZE_MAX) {
    sexos_line("[sexfiles.diskfs.v4.create.err] reason=full");
    return (uint64_t) ERR_FULL;
  }
  uint16_t gen = (uint16_t)(sexfiles_state.v4_table[slot].gen + 1);
  if (gen == 0) gen = 1;
  V4Entry e;
  memset(&e, 0, sizeof e);
  memcpy(e.name, name, DISKFS_V3_NAME_MAX);
  e.size_bytes = 0;
  e.gen = gen;
  sexfiles_state.v4_table[slot] = e;
  sexos_line("[sexfiles.diskfs.v4.crash_point.create_pending] slot=%zu gen=%u", slot, (unsigned) gen);
  uint64_t er = 0;
  if (!v4_persist(buf_va, &er)) {
    memset(&sexfiles_state.v4_table[slot], 0, sizeof sexfiles_state.v4_table[0]);
    return er;
  }
  sexos_line("[sexfiles.diskfs.v4.create.ok] slot=%zu gen=%u", slot, (unsigned) gen);
  return (uint64_t) slot;
}
static uint64_t handle_diskfs_list(uint64_t path_id, uint64_t query) {
  if (query == 0xFEu) return sexfiles_state.v4_generation;
  if (query == 0xFFu) {
    if (path_id >= (uint64_t) DISKFS_SLOTS) return 0;
    size_t i = (size_t) path_id;
    uint64_t used = v4_in_use(i) ? 1u : 0u;
    uint64_t gen = (uint64_t) sexfiles_state.v4_table[i].gen;
    return used << 62 | gen << 32 | (uint64_t) DISKFS_SLOTS;
  }
  if (path_id >= (uint64_t) DISKFS_SLOTS || !v4_in_use((size_t) path_id)) return 0;
  const uint8_t *name = sexfiles_state.v4_table[(size_t) path_id].name;
  size_t chunk = (size_t) query < 2 ? (size_t) query : 2;
  uint64_t v = 0;
  for (size_t k = 0; k < 8; ++k) {
    size_t idx = chunk * 8 + k;
    if (idx < DISKFS_V3_NAME_MAX) v |= (uint64_t) name[idx] << k * 8;
  }
  return v;
}
static uint64_t handle_diskfs_delete(uint64_t path_id, uint64_t buf_va) {
  if (path_id < DISKFS_V3_SYSTEM_SLOTS) return (uint64_t) ERR_PERM_DENIED;
  if (path_id >= (uint64_t) DISKFS_SLOTS) return (uint64_t) ERR_BAD_CMD;
  size_t i = (size_t) path_id;
  if (!v4_in_use(i)) return (uint64_t) ERR_NOT_FOUND;
  V4Extent extents[DISKFS_V4_MAX_EXTENTS];
  size_t count = 0;
  if (!v4_indirect_read(i, buf_va, extents, &count)) {
    memset(extents, 0, sizeof sexfiles_state.v4_extent_cache.extents);
    count = 0;
  }
  memset(sexfiles_state.v4_table[i].name, 0, DISKFS_V3_NAME_MAX);
  sexfiles_state.v4_table[i].size_bytes = 0;
  sexfiles_state.v4_table[i].gen = (uint16_t)(sexfiles_state.v4_table[i].gen + 1);
  sexos_line("[sexfiles.diskfs.v4.crash_point.delete_pending] slot=%zu", i);
  uint64_t err = 0;
  if (!v4_persist(buf_va, &err)) return err;
  sexos_line("[sexfiles.diskfs.v4.crash_point.delete_committed] slot=%zu", i);
  v4_free_pool_only(sexfiles_state.v4_bitmap, extents, count);
  uint64_t dummy_err = 0;
  (void) v4_indirect_write(i, buf_va, extents, 0, &dummy_err);
  v4_cache_invalidate(i);
  sexos_line("[sexfiles.diskfs.v4.delete.ok] slot=%zu", i);
  return 0;
}
static uint64_t handle_diskfs_rename(uint64_t path_id, uint64_t name_lo, uint64_t name_hi, uint64_t buf_va) {
  if (path_id < DISKFS_V3_SYSTEM_SLOTS) return (uint64_t) ERR_PERM_DENIED;
  if (path_id >= (uint64_t) DISKFS_SLOTS) return (uint64_t) ERR_BAD_CMD;
  size_t i = (size_t) path_id;
  if (!v4_in_use(i)) return (uint64_t) ERR_NOT_FOUND;
  uint8_t name[DISKFS_V3_NAME_MAX];
  size_t n = v4_unpack_name(name_lo, name_hi, name);
  if (n == 0) return (uint64_t) ERR_BAD_CMD;
  for (size_t j = 0; j < DISKFS_SLOTS; ++j) if (j != i && (v4_in_use(j) && memcmp(sexfiles_state.v4_table[j].name, name, DISKFS_V3_NAME_MAX) == 0)) return (uint64_t) ERR_EXISTS;
  memcpy(sexfiles_state.v4_table[i].name, name, DISKFS_V3_NAME_MAX);
  sexos_line("[sexfiles.diskfs.v4.crash_point.rename_pending] slot=%zu", i);
  uint64_t err = 0;
  if (!v4_persist(buf_va, &err)) return err;
  sexos_line("[sexfiles.diskfs.v4.rename.ok] slot=%zu", i);
  return 0;
}
#if SEXOS_SEXFILES_CURRENT_DOCUMENT

static bool document_quil_target(uint32_t caller)
{
 static const uint32_t targets[16]={9,16,18,19,20,21,22,23,32,33,34,35,36,37,38,39};
 for(size_t i=0;i<16;i++)if(targets[i]==caller)return true;
 return false;
}
static bool document_v4_entry(uint8_t object,V4Entry *entry,uint64_t *buffer,uint32_t *error)
{
 if(object<2||object>=DISKFS_SLOTS){*error=SEXOS_DOCUMENT_DENIED;return false;}
 uint64_t va=diskfs_bridge_get_buf_va();if(!va||va==UINT64_MAX){*error=SEXOS_DOCUMENT_IO;return false;}uint64_t native_error=0;
 if(!v4_ensure(va,&native_error)){*error=SEXOS_DOCUMENT_IO;return false;}
 if(!v4_get(object,entry,&native_error)){*error=SEXOS_DOCUMENT_STALE;return false;}*buffer=va;return true;
}
bool document_backend_identity(uint8_t object,uint32_t *generation,uint32_t *error)
{
 V4Entry entry;uint64_t va=0;if(!document_v4_entry(object,&entry,&va,error))return false;(void)va;*generation=((uint32_t)object+1u)<<16|entry.gen;return true;
}
bool document_backend_name(uint8_t object,DocumentName *name,uint32_t *error)
{
 V4Entry entry;uint64_t va=0;if(!document_v4_entry(object,&entry,&va,error))return false;(void)va;for(size_t i=0;i<16;i++)name->bytes[i]=entry.name[i];return true;
}
bool document_backend_create(const DocumentName *name,bool fresh_only,uint8_t *object,uint32_t *generation,uint32_t *error)
{
 (void)fresh_only;uint64_t va=diskfs_bridge_get_buf_va();if(!va||va==UINT64_MAX){*error=SEXOS_DOCUMENT_IO;return false;}uint64_t native_error=0;if(!v4_ensure(va,&native_error)){*error=SEXOS_DOCUMENT_IO;return false;}
 uint64_t lo=0,hi=0;for(size_t i=0;i<8;i++){lo|=(uint64_t)name->bytes[i]<<(i*8);hi|=(uint64_t)name->bytes[i+8]<<(i*8);}uint64_t value=handle_diskfs_create(lo,hi,va);
 if(value==(uint64_t)ERR_EXISTS){*error=SEXOS_DOCUMENT_EXISTS;return false;}if(value<3||value>=15){*error=SEXOS_DOCUMENT_IO;return false;}*object=(uint8_t)value;return document_backend_identity(*object,generation,error);
}
bool document_backend_delete(uint8_t object,uint32_t *error)
{
 V4Entry entry;uint64_t va=0;if(!document_v4_entry(object,&entry,&va,error))return false;(void)entry;uint64_t value=handle_diskfs_delete(object,va);if(!value)return true;
 *error=value==(uint64_t)ERR_PERM_DENIED?SEXOS_DOCUMENT_DENIED:value==(uint64_t)ERR_NOT_FOUND?SEXOS_DOCUMENT_STALE:value==(uint64_t)ERR_FULL?SEXOS_DOCUMENT_BUSY:SEXOS_DOCUMENT_IO;return false;
}
static void document_growth_cancel(DocumentBackendJob *job)
{
 if(!job->growth.active)return;
 if(document_growth_cancel_releases(&job->growth)&&job->growth.count>job->growth.original)
  v4_free_pool_only(sexfiles_state.v4_bitmap,job->growth_extents+job->growth.original,job->growth.count-job->growth.original);
 v4_cache_invalidate(job->object);document_growth_abort(&job->growth);
}
static bool document_growth_begin(DocumentBackendJob *job,uint8_t object,uint32_t *error)
{
 V4Entry entry;uint64_t va=0;if(!document_v4_entry(object,&entry,&va,error))return false;
 if(entry.size_bytes==DOCUMENT_JOURNAL_CAPACITY){job->growth.active=false;return true;}
 if(entry.size_bytes>DOCUMENT_JOURNAL_CAPACITY){*error=SEXOS_DOCUMENT_INVALID;return false;}
 size_t count=v4_cache_get(object,va,job->growth_extents),original=count;uint64_t capacity=0;
 for(size_t i=0;i<count;i++)capacity+=(uint64_t)job->growth_extents[i].sector_count*512;
 if(capacity>DOCUMENT_JOURNAL_CAPACITY){*error=SEXOS_DOCUMENT_INVALID;return false;}
 if(capacity<DOCUMENT_JOURNAL_CAPACITY){
  uint64_t more=(DOCUMENT_JOURNAL_CAPACITY-capacity+DISKFS_V4_BLOCK_BYTES-1)/DISKFS_V4_BLOCK_BYTES;size_t next=0;int64_t allocation=0;
  if(!v4_allocate(sexfiles_state.v4_bitmap,more,job->growth_extents,count,&next,&allocation)){*error=SEXOS_DOCUMENT_BUSY;return false;}count=next;
 }
 uint16_t sector_count[DOCUMENT_GROWTH_MAX_EXTENTS]={0};for(size_t i=0;i<count;i++)sector_count[i]=job->growth_extents[i].sector_count;
 if(!document_growth_init(&job->growth,original,count)){if(count>original)v4_free_pool_only(sexfiles_state.v4_bitmap,job->growth_extents+original,count-original);*error=SEXOS_DOCUMENT_INVALID;return false;}
 for(size_t i=0;i<count;i++)if(!document_growth_set_extent(&job->growth,i,sector_count[i])){if(count>original)v4_free_pool_only(sexfiles_state.v4_bitmap,job->growth_extents+original,count-original);*error=SEXOS_DOCUMENT_INVALID;return false;}
 job->growth_buffer=va;job->growth_entry=entry;return true;
}
static uint8_t document_growth_step(DocumentBackendJob *job,uint32_t *error)
{
 uint64_t native_error=0;DocumentGrowthAction action=document_growth_current(&job->growth);
 if(!document_growth_begin_action(&job->growth,action)){*error=SEXOS_DOCUMENT_STALE;document_growth_cancel(job);return DOCUMENT_BACKEND_ERROR;}
 switch(action){
  case DOCUMENT_GROWTH_ZERO:{V4Extent extent=job->growth_extents[job->growth.extent];
   if(!v4_zero_block((uint64_t)extent.start_lba+job->growth.sector,job->growth_buffer,&native_error)){goto io_error;}break;}
  case DOCUMENT_GROWTH_ZERO_BARRIER:if(diskfs_block_sync()!=0)goto io_error;break;
  case DOCUMENT_GROWTH_DESCRIPTOR:
   if(!v4_indirect_write(job->object,job->growth_buffer,job->growth_extents,job->growth.count,&native_error)){goto io_error;}break;
  case DOCUMENT_GROWTH_DESCRIPTOR_BARRIER:if(diskfs_block_sync()!=0)goto io_error;break;
  case DOCUMENT_GROWTH_PUBLISH:
   v4_cache_put(job->object,job->growth_extents,job->growth.count);job->growth_entry.size_bytes=(uint32_t)DOCUMENT_JOURNAL_CAPACITY;
   sexfiles_state.v4_table[job->object]=job->growth_entry;if(!v4_persist(job->growth_buffer,&native_error))goto io_error;break;
  case DOCUMENT_GROWTH_PUBLISH_BARRIER:if(diskfs_block_sync()!=0)goto io_error;break;
  default:*error=SEXOS_DOCUMENT_STALE;document_growth_cancel(job);return DOCUMENT_BACKEND_ERROR;
 }
 if(!document_growth_finish_action(&job->growth,action)){*error=SEXOS_DOCUMENT_STALE;document_growth_cancel(job);return DOCUMENT_BACKEND_ERROR;}
 return job->growth.active?DOCUMENT_BACKEND_PENDING:DOCUMENT_BACKEND_DONE;
io_error:*error=SEXOS_DOCUMENT_IO;document_growth_cancel(job);return DOCUMENT_BACKEND_ERROR;
}
bool document_journal_io_read(uint8_t object,uint64_t offset,DocumentJournalBlock *block,uint32_t *error)
{
 V4Entry entry;uint64_t va=0;if(!document_v4_entry(object,&entry,&va,error))return false;if(block->len>DOCUMENT_JOURNAL_IO_CHUNK||offset>entry.size_bytes||block->len>(size_t)((uint64_t)entry.size_bytes-offset)){*error=SEXOS_DOCUMENT_INVALID;return false;}
 size_t done=0;while(done<block->len){DiskManifestEntryV1 piece;uint64_t relative=0,native_error=0;if(!v4_resolve_offset(object,offset+done,va,&piece,&relative,&native_error)){*error=SEXOS_DOCUMENT_IO;return false;}uint64_t available=(uint64_t)piece.len_bytes-relative;size_t count=block->len-done;if(count>available)count=(size_t)available;if(!count){*error=SEXOS_DOCUMENT_IO;return false;}uint64_t got=0;if(!diskfs_read_object_entry(piece,relative,block->bytes+done,count,va,&got,&native_error)||got!=count){*error=SEXOS_DOCUMENT_IO;return false;}done+=count;}return true;
}
bool document_journal_io_write(uint8_t object,uint64_t offset,const DocumentJournalBlock *block,uint32_t *error)
{
 V4Entry entry;uint64_t va=0;if(!document_v4_entry(object,&entry,&va,error))return false;if(block->len>DOCUMENT_JOURNAL_IO_CHUNK||offset>entry.size_bytes||block->len>(size_t)((uint64_t)entry.size_bytes-offset)){*error=SEXOS_DOCUMENT_INVALID;return false;}
 size_t done=0;while(done<block->len){DiskManifestEntryV1 piece;uint64_t relative=0,native_error=0;if(!v4_resolve_offset(object,offset+done,va,&piece,&relative,&native_error)){*error=SEXOS_DOCUMENT_IO;return false;}uint64_t available=(uint64_t)piece.len_bytes-relative;size_t count=block->len-done;if(count>available)count=(size_t)available;if(!count){*error=SEXOS_DOCUMENT_IO;return false;}uint64_t written=0;if(!diskfs_write_object_entry(piece,relative,block->bytes+done,count,va,&written,&native_error)||written!=count){*error=SEXOS_DOCUMENT_IO;return false;}done+=count;}return true;
}
bool document_journal_io_sync(uint8_t object,uint32_t *error)
{(void)object;if(diskfs_block_sync()==0)return true;*error=SEXOS_DOCUMENT_IO;return false;}
uint8_t document_backend_load_step(uint32_t caller,uint8_t object,SexosDocumentPayload *payload,DocumentName *name,uint32_t *error)
{
 uint32_t identity=0;if(!document_backend_identity(object,&identity,error))return DOCUMENT_BACKEND_ERROR;DocumentBackendJob *job=&sexfiles_state.document_job;
 if(!job->active){V4Entry entry;uint64_t va=0;if(!document_v4_entry(object,&entry,&va,error))return DOCUMENT_BACKEND_ERROR;job->active=true;job->loading=true;job->caller=caller;job->object=object;job->identity=identity;document_journal_load_init(&job->load,object,identity,entry.size_bytes==DOCUMENT_JOURNAL_CAPACITY);}
 if(!job->loading||job->caller!=caller||job->object!=object||job->identity!=identity){*error=SEXOS_DOCUMENT_BUSY;return DOCUMENT_BACKEND_ERROR;}DocumentJournalProgress progress=document_journal_load_step(&job->load,payload);if(progress.status==DOCUMENT_JOURNAL_PENDING)return DOCUMENT_BACKEND_PENDING;if(progress.status==DOCUMENT_JOURNAL_ERROR){*error=progress.error;return DOCUMENT_BACKEND_ERROR;}if(!document_backend_name(object,name,error))return DOCUMENT_BACKEND_ERROR;return DOCUMENT_BACKEND_DONE;
}
uint8_t document_backend_save_step(uint32_t caller,uint8_t object,const SexosDocumentPayload *payload,uint32_t *error)
{
 uint32_t identity=0;if(!document_backend_identity(object,&identity,error))return DOCUMENT_BACKEND_ERROR;DocumentBackendJob *job=&sexfiles_state.document_job;
 if(!job->active){job->active=true;job->loading=false;job->caller=caller;job->object=object;job->identity=identity;document_journal_save_init(&job->save,object,identity);if(!document_growth_begin(job,object,error)){memset(job,0,sizeof *job);return DOCUMENT_BACKEND_ERROR;}}
 if(job->loading||job->caller!=caller||job->object!=object||job->identity!=identity){*error=SEXOS_DOCUMENT_BUSY;return DOCUMENT_BACKEND_ERROR;}
 if(job->growth.active){uint8_t status=document_growth_step(job,error);return status==DOCUMENT_BACKEND_DONE?DOCUMENT_BACKEND_PENDING:status;}
 DocumentJournalProgress progress=document_journal_save_step(&job->save,payload);if(progress.status==DOCUMENT_JOURNAL_PENDING)return DOCUMENT_BACKEND_PENDING;if(progress.status==DOCUMENT_JOURNAL_ERROR){*error=progress.error;return DOCUMENT_BACKEND_ERROR;}return DOCUMENT_BACKEND_DONE;
}
void document_backend_cancel_work(uint32_t caller)
{DocumentBackendJob *job=&sexfiles_state.document_job;if(job->active&&job->caller==caller){document_growth_cancel(job);memset(job,0,sizeof *job);}}
bool document_backend_publish_load(uint32_t caller,SexosDocumentRequest request,uint32_t generation,const SexosDocumentPayload *payload,const DocumentName *name,uint32_t *error)
{DocumentSnapshotRegion region;if(!document_snapshot_resolve(caller,true,&region)){*error=SEXOS_DOCUMENT_DENIED;return false;}return document_snapshot_publish(&region,request,generation,name,payload,error);}
bool document_backend_clear_load(uint32_t caller,uint32_t *error)
{if(!document_quil_target(caller))return true;DocumentSnapshotRegion region;if(!document_snapshot_resolve(caller,true,&region)){*error=SEXOS_DOCUMENT_DENIED;return false;}return document_snapshot_clear(&region,error);}


#endif
static size_t vfs_unpack_name(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint8_t *out_name) {
  write_le64(out_name + 0, arg0);
  write_le64(out_name + 8, arg1);
  write_le64(out_name + 16, arg2);
  size_t len = 24;
  while (len > 0 && out_name[len - 1] == 0) --len;
  return len;
}
uint64_t handle_vfs_message(uint64_t type_id, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint32_t caller_pd) {
  sexos_line("[sexfiles.vfs.enter] type=%#llx caller=%u a0=%#llx a1=%#llx", (unsigned long long) type_id, (unsigned) caller_pd, (unsigned long long) arg0, (unsigned long long) arg1);
  sexfiles_state.ipc_ops_total += 1;
  switch (type_id) {
    case OP_RAMFS_OPEN: {
      uint8_t name_bytes[24];
      size_t name_len = vfs_unpack_name(arg0, arg1, arg2 & ~(UINT64_C(0xFF) << 24), name_bytes);
      uint32_t flags = (uint32_t)(arg2 >> 24);
      uint64_t handle = 0;
      int64_t err = ramfs_open(name_bytes, name_len, flags, 0, caller_pd, &handle);
      return err == 0 ? handle : (uint64_t) err;
    }
    case OP_RAMFS_READ: {
      size_t max_len = (size_t) arg2 < RAMFS_MAX_FILE_SIZE ? (size_t) arg2 : RAMFS_MAX_FILE_SIZE;
      size_t to_read = max_len < 8 ? max_len : 8;
      uint8_t buf[8] = {
        0, 0, 0, 0, 0, 0, 0, 0
      };
      uint64_t n = 0;
      int64_t err = ramfs_read(arg0, arg1, buf, to_read, caller_pd, &n);
      if (err != 0) return (uint64_t) err;
      uint64_t reply = 0;
      size_t limit = (size_t) n < 8 ? (size_t) n : 8;
      for (size_t i = 0; i < limit; ++i) reply |= (uint64_t) buf[i] << i * 8;
      return reply;
    }
    case OP_RAMFS_WRITE: {
      uint8_t data[8];
      write_le64(data, arg2);
      uint64_t n = 0;
      int64_t err = ramfs_write(arg0, arg1, data, 8, caller_pd, &n);
      return err == 0 ? n : (uint64_t) err;
    }
    case OP_RAMFS_CLOSE: {
      int64_t err = ramfs_close(arg0, caller_pd);
      return err == 0 ? 0 : (uint64_t) err;
    }
    case OP_RAMFS_LIST: {
      uint64_t handle = 0;
      uint32_t name_len = 0;
      if (ramfs_list_at((size_t) arg0, caller_pd, &handle, &name_len)) return handle << 32 | (uint64_t) name_len;
      return 0;
    }
    case OP_RAMFS_CREATE_OWNER: {
      uint8_t name_bytes[24];
      size_t name_len = vfs_unpack_name(arg0, arg1, arg2 & ~(UINT64_C(0xFFFFFFFF) << 32), name_bytes);
      uint32_t owner_pd = (uint32_t)(arg2 >> 32);
      uint64_t handle = 0;
      int64_t err = ramfs_create_with_owner(name_bytes, name_len, owner_pd, caller_pd, &handle);
      return err == 0 ? handle : (uint64_t) err;
    }
    case OP_RAMFS_OBJECT_ID: {
      uint64_t id = 0;
      int64_t err = ramfs_object_id_for_handle(arg0, caller_pd, &id);
      return err == 0 ? id : (uint64_t) err;
    }
    case OP_RAMFS_STAT: {
      uint64_t size = 0;
      uint32_t name_len = 0;
      int64_t err = ramfs_stat(arg0, caller_pd, &size, &name_len);
      return err == 0 ? size << 32 | (uint64_t) name_len : (uint64_t) err;
    }
    case OP_RAMFS_READNAME: {
      size_t max_len = (size_t) arg2 < 8 ? (size_t) arg2 : 8;
      uint8_t buf[8] = {
        0, 0, 0, 0, 0, 0, 0, 0
      };
      uint64_t n = 0;
      int64_t err = ramfs_read(arg0, arg1, buf, max_len, caller_pd, &n);
      if (err == 0) {
        uint64_t packed = 0;
        size_t limit = (size_t) n < 8 ? (size_t) n : 8;
        for (size_t i = 0; i < limit; ++i) packed |= (uint64_t) buf[i] << i * 8;
        sexos_line("[sexfiles.ramfs.readname.ok] handle=%llu off=%llu len=%zu", (unsigned long long) arg0, (unsigned long long) arg1, max_len);
        return packed;
      }
      sexos_line("[sexfiles.ramfs.readname.deny] handle=%llu err=%lld", (unsigned long long) arg0, (long long) err);
      return (uint64_t) err;
    }
    case OP_DISKFS_WRITE: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x38 name=write caller=%u", (unsigned) caller_pd);
      uint64_t reply = handle_diskfs_write(arg0, arg1, arg2, caller_pd);
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x38 caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_DISKFS_READ: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x39 name=read caller=%u", (unsigned) caller_pd);
      uint64_t reply = handle_diskfs_read(arg0, arg1, caller_pd);
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x39 caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_DISKFS_FLUSH: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x3A name=flush caller=%u", (unsigned) caller_pd);
      uint64_t reply = handle_diskfs_flush();
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x3A caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_DISKFS_STAT: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x3B name=stat caller=%u", (unsigned) caller_pd);
      uint64_t reply = handle_diskfs_stat(caller_pd);
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x3B caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_DISKFS_MANIFEST_HASH: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x3C name=hash caller=%u", (unsigned) caller_pd);
      uint64_t reply = handle_diskfs_manifest_hash(caller_pd);
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x3C caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_DISKFS_CREATE: {
      sexos_line("[sexfiles.route.dispatch] op=0x42 name=create caller=%u", (unsigned) caller_pd);
      uint64_t buf_va = diskfs_bridge_get_buf_va();
      if (buf_va == 0 || buf_va == UINT64_MAX) return (uint64_t) ERR_NOT_FOUND;
      uint64_t err = 0;
      if (!v4_ensure(buf_va, &err)) return err;
      uint64_t reply = handle_diskfs_create(arg1, arg2, buf_va);
      sexos_line("[sexfiles.route.reply] op=0x42 caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_DISKFS_LIST: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x43 name=list caller=%u", (unsigned) caller_pd);
      uint64_t buf_va = diskfs_bridge_get_buf_va();
      if (buf_va == 0 || buf_va == UINT64_MAX) return (uint64_t) ERR_NOT_FOUND;
      uint64_t err = 0;
      if (!v4_ensure(buf_va, &err)) return err;
      return handle_diskfs_list(arg0, arg1);
    }
    case OP_DISKFS_DELETE: {
      sexos_line("[sexfiles.route.dispatch] op=0x47 name=delete caller=%u", (unsigned) caller_pd);
      uint64_t buf_va = diskfs_bridge_get_buf_va();
      if (buf_va == 0 || buf_va == UINT64_MAX) return (uint64_t) ERR_NOT_FOUND;
      uint64_t err = 0;
      if (!v4_ensure(buf_va, &err)) return err;
      return handle_diskfs_delete(arg0, buf_va);
    }
    case OP_DISKFS_RENAME: {
      sexos_line("[sexfiles.route.dispatch] op=0x48 name=rename caller=%u", (unsigned) caller_pd);
      uint64_t buf_va = diskfs_bridge_get_buf_va();
      if (buf_va == 0 || buf_va == UINT64_MAX) return (uint64_t) ERR_NOT_FOUND;
      uint64_t err = 0;
      if (!v4_ensure(buf_va, &err)) return err;
      return handle_diskfs_rename(arg0, arg1, arg2, buf_va);
    }
    case OP_DISKFS_TRUNCATE: {
      sexos_line("[sexfiles.route.dispatch] op=0x49 name=truncate caller=%u", (unsigned) caller_pd);
      uint64_t reply = handle_diskfs_truncate(arg0, caller_pd);
      sexos_line("[sexfiles.route.reply] op=0x49 caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_DISKFS_READ_V2: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x4A name=read_v2 caller=%u", (unsigned) caller_pd);
      uint64_t reply = handle_diskfs_read_v2(arg0, arg1, caller_pd);
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x4A caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_DISKFS_SELECT: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x3E name=select caller=%u", (unsigned) caller_pd);
      uint64_t reply = handle_diskfs_select(arg0, caller_pd);
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x3E caller=%u value=%#llx", (unsigned) caller_pd, (unsigned long long) reply);
      return reply;
    }
    case OP_SEXOBJECT_NATIVE_PERSIST_PROOF: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x40 ok=1 caller=%u", (unsigned) caller_pd);
      uint64_t object_id = 0;
      int64_t err = 0;
      if (sexobject_native_persist_linen_proof(&object_id, &err)) {
        if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x40 status=S ok=1 caller=%u object_id=%llu", (unsigned) caller_pd, (unsigned long long) object_id);
        return object_id;
      }
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x40 status=E ok=0 caller=%u err=%lld", (unsigned) caller_pd, (long long) err);
      return (uint64_t) err;
    }
    case OP_SEXOBJECT_READ_BACK: {
      if (vfs_hot_log()) sexos_line("[sexfiles.route.dispatch] op=0x41 name=sexobject_read_back caller=%u", (unsigned) caller_pd);
      size_t len = 0;
      int64_t err = 0;
      if (sexobject_read_back_for_quil(arg0, &len, &err)) {
        if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x41 caller=%u len=%zu ok=1", (unsigned) caller_pd, len);
        return (uint64_t) len;
      }
      if (vfs_hot_log()) sexos_line("[sexfiles.route.reply] op=0x41 caller=%u err=%lld", (unsigned) caller_pd, (long long) err);
      return (uint64_t) err;
    }
    case OP_RAMFS_STATUS: {
      sexos_line("[sexfiles.status.query] object=%llu ok=1 reason=received", (unsigned long long) arg0);
      sexos_line("[sexfiles.status.result] object=%llu exists=0 size=0 generation=0 ok=1 reason=phaseb1_marker", (unsigned long long) arg0);
      return 0;
    }
    default: return (uint64_t) ERR_NOT_FOUND;
  }
}
/*Trampoline initialization and PDX message loop (servers/sexfiles/src/trampoline.rs & main.rs).*/
static inline bool tramp_hot_log(void) {
  return sexfiles_take_budget(&sexfiles_state.trampoline_hot_log_budget);
}
static void trampoline_main(void) {
  {
    sexos_line("[sexfiles.trampoline.enter] ok=1");
    sexos_line("[sexfiles.trampoline.before_register] slot=1 ok=1");
    if (RAMFS_PROOF_ENABLED) run_all_proofs();
    if (DISKFS_OBJECT_TABLE_PROOF_ENABLED) run_diskfs_object_table_proofs();
    if (SEXFILES_JOURNAL_PROOF_ENABLED) run_sexfiles_journal_proofs();
    if (SEXFILES_REPLAY_PROOF_ENABLED) run_sexfiles_replay_proofs();
    if (SEXFILES_CAP_RECORD_PROOF_ENABLED) run_sexfiles_cap_record_proofs();
    if (LINEN_SEXFILES_METADATA_PROOF_ENABLED) run_linen_sexfiles_metadata_proofs();
    if (SEXFILES_FAULT_INJECTION_PROOF_ENABLED) run_sexfiles_fault_injection_proofs();
    if (SEXFILES_REAL_BLOCK_PROOF_ENABLED) run_sexfiles_real_block_proofs();
    if (SEXFILES_REBOOT_PROOF_ENABLED) run_sexfiles_reboot_proofs();
    if (SEXFILES_EXTENT_PROOF_ENABLED) run_sexfiles_extent_proofs();
    if (SEXFILES_CHECKPOINT_PROOF_ENABLED) run_sexfiles_checkpoint_proofs();
    if (DISKFS_100_AP4_WRITE_PROOF_ENABLED) {
      run_diskfs100_ap4_write_proof();
      return;
    }
    if (DISKFS_100_AP4_READ_PROOF_ENABLED) {
      run_diskfs100_ap4_read_proof();
      if (DISKFS_100_AP5_NEG_READ_NO_WRITE_ENABLED) run_diskfs100_ap5_neg_read_no_write();
      return;
    }
    if (DISKFS_100_AP5_NEG_MISMATCH_ENABLED) {
      run_diskfs100_ap5_neg_mismatch();
      return;
    }
    if (DISKFS_100_AP5_NEG_MISSING_IMAGE_ENABLED) {
      run_diskfs100_ap5_neg_missing_image();
      return;
    }
    if (DISKFS_100_AP5_NEG_FLUSH_SKIP_ENABLED) {
      run_diskfs100_ap5_neg_flush_skip();
      return;
    }
    if (DISKFS_100_AP6_FLUSH_FSYNC_ENABLED) {
      run_diskfs100_ap6_flush_fsync();
      return;
    }
    if (DISKFS_NEGATIVE_BOUNDS_AUTH_PROOF_ENABLED) {
      run_diskfs_negative_bounds_auth_proof();
      return;
    }
    if (DISKFS_BRIDGE_STRICT_PROOF_ENABLED) {
      run_diskfs_bridge_strict_proof_v1();
      return;
    }
    if (DISKFS_100_AP2_PROOF_ENABLED) {
      run_diskfs100_ap2_proof();
      return;
    }
    if (DISKFS_100_AP3_PROOF_ENABLED) {
      run_diskfs_multi_object_proofs();
      return;
    }
    if (DISKFS_MULTI_OBJECT_PROOF_ENABLED) {
      if (SEXFILES_ROUTE_AUDIT_ONLY_ENABLED) sexos_line("[sexfiles.disk.multi.skip] reason=route_audit");
      else run_diskfs_multi_object_proofs();
    }
    if (SEXOBJECT_VIEW_PROOF_ENABLED) run_sexobject_view_proof();
    if (LINEN_DISK_OBJECT_PROOF_ENABLED) run_linen_disk_object_proof();
    if (SEXFS_V0_SUPERBLOCK_FORMAT_MOUNT_PROOF_ENABLED) run_sexfs_v0_superblock_format_mount_proofs();
    if (SEXOBJECT_TABLE_PERSIST_PROOF_ENABLED) run_sexobject_table_persist_proofs();
    if (SEXOBJECT_TABLE_EXTENT_ALLOC_PROOF_ENABLED) run_sexobject_table_extent_alloc_proofs();
    if (SEXOBJECT_EXTENT_WRITE_FULL_BLOCK_PROOF_ENABLED) run_sexobject_extent_write_full_block_proofs();
    if (SEXOBJECT_WRITE_READ_PERSIST_PROOF_ENABLED) run_sexobject_write_read_persist_proofs();
    if (SEXOBJECT_MULTI_OBJECT_PROOF_ENABLED) run_sexobject_multi_object_proofs();
    sexos_line("[sexfiles.ready]");
    sexos_line("[sexfiles.init.ready] slot=1 ok=1");
    sexos_line("[sexfiles.trampoline.loop.enter] ok=1");
#if SEXOS_SEXFILES_CURRENT_DOCUMENT
if (sexos_rpc_register() != 1) {
  sexos_line("[sexfiles.document.unavailable] reason=correlated_kernel_required");
  sexos_fail();
}
document_service_transport_init(&sexfiles_state.documents, true);

#endif
    while (1) {
#if SEXOS_SEXFILES_CURRENT_DOCUMENT
document_service_transport_progress(&sexfiles_state.documents);

(void) document_service_transport_poll(&sexfiles_state.documents);

#endif
      if (sexfiles_state.tramp_listen_budget > 0) {
        sexfiles_state.tramp_listen_budget -= 1;
        if (tramp_hot_log()) sexos_line("[sexfiles.trampoline.listen.enter]");
      }
      DeferEntry d;
      SexosRequest request = (SexosRequest) {
        .id = 0
      };
      if (defer_pop(&d)) request = d.request;
      else
#if SEXOS_SEXFILES_CURRENT_DOCUMENT
{
  request = sexos_rpc_listen_once(0);
  if (request.message.type_id == 0) {
    sexos_yield();
    continue;
  }
}
#else
request.message = sexos_listen(&sexfiles_state.listener, 0);

#endif
      uint64_t type_id = request.message.type_id;
      uint32_t caller = request.message.caller_pd;
      uint64_t arg0 = request.message.arg0;
      uint64_t arg1 = request.message.arg1;
      uint64_t arg2 = request.message.arg2;
      if (tramp_hot_log()) sexos_line("[sexfiles.trampoline.after_listen] type=%#llx caller=%u a0=%#llx a1=%#llx", (unsigned long long) type_id, (unsigned) caller, (unsigned long long) arg0, (unsigned long long) arg1);
      if (tramp_hot_log()) sexos_line("[sexfiles.route.recv] type=%#llx caller=%u a0=%#llx a1=%#llx", (unsigned long long) type_id, (unsigned) caller, (unsigned long long) arg0, (unsigned long long) arg1);
#if SEXOS_SEXFILES_CURRENT_DOCUMENT
if (document_service_transport_prepare(&sexfiles_state.documents, &request, true)) continue;

#endif
      uint64_t reply = handle_vfs_message(type_id, arg0, arg1, arg2, caller);
      if (type_id != 0) {
        if (type_id == OP_DISKFS_READ) sexos_line("[sexfiles.bridge.diskfs.read.reply.sent] caller=%u value=%#llx", (unsigned) caller, (unsigned long long) reply);
#if SEXOS_SEXFILES_CURRENT_DOCUMENT
(void) sexos_answer_request(&request, reply);

#else
(void) sexos_reply(caller, reply);

#endif
      }
    }
  }
}
void sexfiles_main(void) {
  sexfiles_state_init(&sexfiles_state);
  sexos_line("[sexfiles.init.start] ok=1");
  sexos_heap_init();
  sexos_line("[sexfiles.ready]");
  sexos_line("[sexfiles.init.before_trampoline] ok=1");
  trampoline_main();
  sexos_line("[sexfiles.trampoline.return.unexpected] ok=0");
  while (1) sexos_yield();
}
