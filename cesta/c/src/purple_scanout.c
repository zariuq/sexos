#include "sexos_server.h"
#define OP_WINDOW_CREATE UINT64_C(0xDE)
#define OP_WINDOW_SUBMIT UINT64_C(0xDD)
typedef struct SexWindowCreateParams {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint64_t pfn_base;
}
SexWindowCreateParams;
_Static_assert(sizeof(SexWindowCreateParams) == 24 && offsetof(SexWindowCreateParams, pfn_base) == 16, "repr(C) SexWindowCreateParams");

typedef struct PurpleScanout {
  uint32_t fb_page[1024];
  uint64_t pfn_list[1];
}
PurpleScanout;
static PurpleScanout _Alignas(4096) purple_state;
static void debug_syscall_probe(void) {
  (void) sexos_pdx_call(SEXOS_SLOT_kernel, 0x5151u, 0, 0, 0);
}
static bool pdx_call_checked(uint64_t slot, uint64_t opcode, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t *ok_out, uint64_t *err_out) {
  SexosReply r = sexos_pdx_call(slot, opcode, arg0, arg1, arg2);
  if (r.status == 0u) {
    *ok_out = r.value;
    return true;
  } else {
    *err_out = r.status;
    return false;
  }
}
void purple_scanout_main(void) {
  PurpleScanout *p = &purple_state;
  debug_syscall_probe();
  sexos_line("[purple] single-pixel scanout test start");
  uint64_t fb_ptr = (uint64_t)(uintptr_t) p->fb_page;
  SexWindowCreateParams params = (SexWindowCreateParams) {
    .x = 0u,.y = 0u,.width = 1u,.height = 1u,.pfn_base = fb_ptr
  };
  uint64_t window_id = 0;
  uint64_t code = 0;
  if (!pdx_call_checked(SEXOS_SLOT_display, OP_WINDOW_CREATE, (uint64_t)(uintptr_t) &params, 0, 0, &window_id, &code)) {
    sexos_line("[purple] create failed status=%#llx", (unsigned long long) code);
    while (1) sexos_yield();
  }
  *(volatile uint32_t *) p->fb_page = 0x00FF00FFu;
  p->pfn_list[0] = fb_ptr;
  uint64_t unused_ok = 0;
  if (!pdx_call_checked(SEXOS_SLOT_display, OP_WINDOW_SUBMIT, window_id, (uint64_t)(uintptr_t) p->pfn_list, 1, &unused_ok, &code)) {
    sexos_line("[purple] submit failed status=%#llx", (unsigned long long) code);
    while (1) sexos_yield();
  }
  sexos_line("[purple] submitted single magenta pixel");
  while (1) sexos_yield();
}
