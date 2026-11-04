#ifndef SEXOS_QUIL_AUTOSAVE_H
#define SEXOS_QUIL_AUTOSAVE_H
#include "sexos_client.h"
#define QUIL_AUTOSAVE_IDLE_MS UINT64_C(30000)
#define QUIL_AUTOSAVE_EXISTS UINT32_C(10)
typedef enum QuilAutosaveDoneKind {QUIL_AUTOSAVE_SAVED,QUIL_AUTOSAVE_NAME_TAKEN,QUIL_AUTOSAVE_FAILED,QUIL_AUTOSAVE_GAVE_UP} QuilAutosaveDoneKind;
typedef struct QuilAutosaveDone {QuilAutosaveDoneKind kind;uint8_t draft,next;uint64_t revision;uint32_t error;} QuilAutosaveDone;
typedef struct QuilAutosaveTitle {uint8_t bytes[16];} QuilAutosaveTitle;
typedef struct QuilAutosaveName {uint8_t bytes[16];} QuilAutosaveName;
typedef struct QuilAutosave {
 bool draft_present,drafted_present,seen_present,pending;
 uint8_t draft,suffix;
 uint64_t drafted_revision,seen_revision,seen_at;
 int8_t queued_save;
} QuilAutosave;
typedef struct QuilTextCounts {size_t lines,words,characters;} QuilTextCounts;
void quil_autosave_init(QuilAutosave *state);
void quil_autosave_reset(QuilAutosave *state);
bool quil_autosave_pending(const QuilAutosave *state);
bool quil_autosave_armed(const QuilAutosave *state);
bool quil_autosave_due(QuilAutosave *state,uint64_t now,bool dirty,uint64_t revision,bool ready);
QuilAutosaveName quil_autosave_name(const QuilAutosave *state,const QuilAutosaveTitle *title);
bool quil_autosave_target(const QuilAutosave *state,uint8_t *draft);
bool quil_autosave_kept(const QuilAutosave *state,uint64_t revision);
QuilAutosaveDone quil_autosave_finish(QuilAutosave *state,bool saved,uint8_t object,uint64_t revision,uint32_t error);
#endif
