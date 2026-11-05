#if !defined(SEXOS_CESTA_CLIENT_H)
#define SEXOS_CESTA_CLIENT_H 1

#include <stdint.h>

#include <stddef.h>

#include <stdbool.h>

#include "client_constants.h"

#include "packed_words.h"

#include "format_bytes.h"

typedef struct SexosReply {
  uint64_t status;
  uint64_t value;
}
SexosReply;

typedef struct SexosListener {
  unsigned wrapper_budget;
}
SexosListener;

typedef struct SexosMessage {
  uint64_t type_id;
  uint64_t arg0;
  uint64_t arg1;
  uint64_t arg2;
  uint32_t caller_pd;
  uint32_t caller_task;
}
SexosMessage;

SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2);

SexosMessage sexos_listen(SexosListener *listener, uint64_t slot);

SexosMessage sexos_listen_once(uint64_t slot);

void sexos_yield(void);

void sexos_wait_self(void);

void sexos_raw_print(const char *text, size_t length);

void sexos_line(const char *format, ...);


void sexos_line(const char *format, ...) __attribute__((format(printf, 1, 2)));


void sexos_fail(void);

void kaleidoscope_main(void);

size_t sexos_strlen(const char *text);


_Static_assert(sizeof(SexosMessage) == 40 && offsetof(SexosMessage, caller_pd) == 32 &&
                   offsetof(SexosMessage, caller_task) == 36,
               "PDX message ABI");



_Static_assert(sizeof(SexosReply) == 16 && offsetof(SexosReply, value) == 8, "PDX call ABI");


#endif
