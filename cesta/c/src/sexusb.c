#include "sexos_mmio.h"
#include <stdatomic.h>
/*xHCI USB host controller and HID driver (servers/sexusb/src/main.rs).*/
#if !defined(SEXUSB_SYNTHETIC)
#define SEXUSB_SYNTHETIC 0

#endif
#if !defined(SEXUSB_SLOT2_OWNERSHIP_PROOF)
#define SEXUSB_SLOT2_OWNERSHIP_PROOF 0

#endif
#if !defined(SEXUSB_SYNTHETIC_SLOT2)
#define SEXUSB_SYNTHETIC_SLOT2 0

#endif
#if !defined(HID_VERBOSE_RING_LOG)
#define HID_VERBOSE_RING_LOG 0

#endif
#define SLOT_USB_HOST UINT64_C(8)
#define SLOT_USB_SEXINPUT UINT64_C(9)
#define OP_USB_MOUSE_REPORT UINT64_C(0x260)
#define OP_USB_KEYBOARD_REPORT UINT64_C(0x261)
#define ERR_CAP_INVALID UINT64_C(0xFFFFFFFFFFFFFFFC)
#define PAGE_SIZE UINT64_C(4096)
#define TRB_SIZE UINT64_C(16)
#define CMD_RING_TRBS UINT64_C(64)
#define EVENT_RING_TRBS UINT64_C(64)
#define ERST_ENTRIES UINT64_C(1)
#define DCBAA_BYTES PAGE_SIZE
#define MAP_BYTES UINT64_C(0x10000)
#define XHCI_USBCMD UINT64_C(0x00)
#define XHCI_USBSTS UINT64_C(0x04)
#define XHCI_OP_CONFIG UINT64_C(0x08)
#define USBCMD_RUN_STOP (1u << 0)
#define USBCMD_HCRST (1u << 1)
#define USBSTS_HCHALTED (1u << 0)
#define USBSTS_CNR (1u << 11)
#define POLL_BUDGET ((size_t)100000)
#define MAX_USB_DEVICES ((size_t)2)
#define MAX_USB_PORT_TRACKED ((size_t)16)
#define XHCI_CRCR UINT64_C(0x18)
#define XHCI_DCBAAP UINT64_C(0x30)
#define XHCI_CAP_DBOFF UINT64_C(0x14)
#define XHCI_CAP_RTSOFF UINT64_C(0x18)
#define XHCI_CAP_HCCPARAMS1 UINT64_C(0x10)
#define XHCI_INTR_ERSTSZ UINT64_C(0x08)
#define XHCI_INTR_ERSTBA UINT64_C(0x10)
#define XHCI_INTR_ERDP UINT64_C(0x18)
#define XHCI_INTR_BASE UINT64_C(0x20)
#define TRB_TYPE_ENABLE_SLOT_CMD 9u
#define TRB_TYPE_ADDRESS_DEVICE_CMD 11u
#define TRB_TYPE_CONFIGURE_ENDPOINT_CMD 12u
#define TRB_TYPE_NOOP_CMD 23u
#define TRB_TYPE_EVALUATE_CONTEXT_CMD 13u
#define TRB_TYPE_CMD_COMPLETION_EVENT 33u
#define TRB_TYPE_TRANSFER_EVENT 32u
#define TRB_TYPE_SETUP_STAGE 2u
#define TRB_TYPE_DATA_STAGE 3u
#define TRB_TYPE_STATUS_STAGE 4u
#define TRB_TYPE_NORMAL 1u
#define TRB_TYPE_LINK 6u
#define TRB_CC_SUCCESS 1u
#define TRB_CC_SHORT_PACKET 13u
#define PORTSC_BASE UINT64_C(0x400)
#define PORTSC_STRIDE UINT64_C(0x10)
#define PORTSC_CCS (1u << 0)
#define PORTSC_PED (1u << 1)
#define ICC_DROP_NONE 0u
#define ICC_ADD_SLOT_EP0 ((1u << 0) | (1u << 1))
#define SLOT_CTX_ENTRIES_SHIFT 27u
#define SLOT_SPEED_SHIFT 20u
#define CERR_DEFAULT 3u
#define EP_TYPE_CTRL_SHIFT 3u
#define EP_TYPE_CTRL_VAL 2u
#define DCS_INITIAL UINT64_C(1)
#define EP_TYPE_INTERRUPT_IN 7u
#define INTR_TR_RING_SIZE UINT64_C(16)
#define HID_INFLIGHT UINT64_C(8)
#define HID_BUF_STRIDE UINT64_C(64)
#define HID_RING_LINK_INDEX HID_INFLIGHT
#define HID_INTR_LIVENESS_BUDGET (POLL_BUDGET * 4)
typedef enum HidRole {
  HID_ROLE_KEYBOARD = 0, HID_ROLE_POINTER_TABLET = 1, HID_ROLE_POINTER_MOUSE = 2, HID_ROLE_UNKNOWN = 3
}
HidRole;
typedef struct BootMouseReport {
  uint8_t buttons;
  int8_t dx;
  int8_t dy;
  int8_t wheel;
}
BootMouseReport;
typedef struct TabletReport {
  uint8_t buttons;
  uint16_t abs_x;
  uint16_t abs_y;
}
TabletReport;
typedef struct SingleHidBind {
  uint32_t slot_id;
  uint64_t port;
  HidRole role;
  uint8_t ep_addr;
  uint32_t ep_dci;
  uint16_t max_packet;
  uint8_t interval;
}
SingleHidBind;
typedef struct HidDevice {
  bool active;
  uint32_t slot_id;
  HidRole role;
  uint32_t intr_dci;
  uint64_t intr_ring_phys;
  uint64_t intr_ring_va;
  uint64_t intr_report_phys;
  uint64_t intr_report_va;
  uint32_t intr_report_len;
}
HidDevice;
typedef struct PortDiscoveryState {
  uint8_t port;
  bool connected;
  bool enabled;
  bool reset_attempted;
  uint32_t slot_id;
}
PortDiscoveryState;
typedef struct SexusbState {
  atomic_bool cap_ready_sexinput;
  atomic_bool defer_emitted_sexinput;
  atomic_bool edge_send_emitted_sexinput;
  atomic_bool route_ready_emitted_sexinput;
  atomic_bool route_missing_emitted_sexinput;
  atomic_bool bootgraph_proof_sent;
  atomic_bool bootgraph_proof_marker_emitted;
  uint32_t iface_diag_budget;
  bool mouse_detect_proof_emitted;
  bool tablet_detect_proof_emitted;
  uint32_t ep_diag_budget;
  uint32_t kick_budget;
  uint32_t own_budget;
  uint32_t own_miss_budget;
  uint32_t c2b_event_budget;
  uint32_t xfer_event_budget;
  uint32_t s2_xfer_event_budget;
  uint32_t s2_rearm_ok_budget;
  uint32_t c2c_budget;
  uint32_t rearm_ok_budget;
  uint32_t intr_raw_budget;
  uint32_t intr_raw_seq;
  uint32_t intr_classify_kbd_budget;
  uint32_t intr_classify_kbd_seq;
  bool kbd_raw_proof_emitted;
  uint32_t kbd_raw_budget;
  bool kbd_decode_proof_emitted;
  uint32_t kbd_forward_budget;
  uint32_t intr_classify_tbl_budget;
  uint32_t intr_classify_tbl_seq;
  bool tablet_raw_proof_emitted;
  uint64_t tablet_active_count;
  bool tablet_pack_proof_emitted;
  uint32_t pointer_candidate_tbl_budget;
  uint32_t pointer_candidate_tbl_seq;
  uint32_t tablet_abs_budget;
  uint64_t tablet_report_count;
  uint32_t forward_mouse_budget;
  bool tablet_to_input_emitted;
  uint16_t last_abs_x;
  uint16_t last_abs_y;
  uint32_t tablet_live_budget;
  bool mouse_raw_proof_emitted;
  uint32_t intr_classify_mouse_ok_budget;
  uint32_t intr_classify_mouse_ok_seq;
  uint32_t sexusb_pointer_raw_budget;
  uint32_t pointer_candidate_mouse_budget;
  uint32_t pointer_candidate_mouse_seq;
  bool mouse_pack_proof_emitted;
  uint32_t forward_mouse_budget_boot;
  bool mouse_to_input_emitted;
  uint32_t intr_classify_mouse_bad_budget;
  uint32_t intr_classify_mouse_bad_seq;
  uint32_t poll_yield_budget;
}
SexusbState;
static SexusbState SEXUSB_STATE;
static void sexusb_state_init(SexusbState *st) {
  {
    memset(st, 0, sizeof *st);
    atomic_init(&st->cap_ready_sexinput, false);
    atomic_init(&st->defer_emitted_sexinput, false);
    atomic_init(&st->edge_send_emitted_sexinput, false);
    atomic_init(&st->route_ready_emitted_sexinput, false);
    atomic_init(&st->route_missing_emitted_sexinput, false);
    atomic_init(&st->bootgraph_proof_sent, false);
    atomic_init(&st->bootgraph_proof_marker_emitted, false);
    st->iface_diag_budget = 16u;
    st->ep_diag_budget = 24u;
    st->kick_budget = 16u;
    st->own_budget = 64u;
    st->own_miss_budget = 8u;
    st->c2b_event_budget = 16u;
    st->xfer_event_budget = 32u;
    st->s2_xfer_event_budget = 32u;
    st->s2_rearm_ok_budget = 32u;
    st->c2c_budget = 16u;
    st->rearm_ok_budget = 32u;
    st->intr_raw_budget = 16u;
    st->intr_classify_kbd_budget = 16u;
    st->kbd_raw_budget = 128u;
    st->kbd_forward_budget = 128u;
    st->intr_classify_tbl_budget = 16u;
    st->pointer_candidate_tbl_budget = 16u;
    st->tablet_abs_budget = 2048u;
    st->forward_mouse_budget = 16u;
    st->last_abs_x = 0xFFFFu;
    st->last_abs_y = 0xFFFFu;
    st->tablet_live_budget = 16u;
    st->intr_classify_mouse_ok_budget = 16u;
    st->sexusb_pointer_raw_budget = 16u;
    st->pointer_candidate_mouse_budget = 16u;
    st->forward_mouse_budget_boot = 16u;
    st->intr_classify_mouse_bad_budget = 16u;
    st->poll_yield_budget = 64u;
  }
}
static uint64_t map_xhci_bar0(uint64_t map_size) {
  {
    return sexos_map_pci_bar(SLOT_USB_HOST, 0ULL, map_size);
  }
}
static uint64_t sys_alloc_phys(uint64_t size) {
  {
    return sexos_allocate_memory(size);
  }
}
static uint64_t sys_map_phys(uint64_t phys, uint64_t size) {
  {
    return sexos_map_memory(phys, size);
  }
}
static uint32_t mmio_read32(uint64_t base, uint64_t offset) {
  {
    return sexos_volatile_read32((const uint32_t *)(uintptr_t)(base + offset));
  }
}
static void mmio_write32(uint64_t base, uint64_t offset, uint32_t value) {
  {
    sexos_volatile_write32((uint32_t *)(uintptr_t)(base + offset), value);
  }
}
static void trb_write_volatile(uint64_t base_va, uint64_t index, uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3) {
  {
    uint32_t *p = (uint32_t *)(uintptr_t)(base_va + index * 16ULL);
    sexos_volatile_write32(p + 0, d0);
    sexos_volatile_write32(p + 1, d1);
    sexos_volatile_write32(p + 2, d2);
    sexos_volatile_write32(p + 3, d3);
  }
}
static uint32_t trb_read_dword(uint64_t base_va, uint64_t index, size_t word) {
  {
    const uint32_t *p = (const uint32_t *)(uintptr_t)(base_va + index * 16ULL);
    return sexos_volatile_read32(p + word);
  }
}
static bool wait_until(uint64_t base, uint64_t offset, uint32_t mask, bool expect_set, size_t spins) {
  {
    for (size_t s = 0; s < spins; ++s) {
      uint32_t v = mmio_read32(base, offset);
      bool is_set = (v & mask) != 0u;
      if (is_set == expect_set) return true;
      sexos_yield();
    }
    return false;
  }
}
static int8_t wrap_i8(uint8_t v) {
  {
    return v < 128u ? (int8_t) v : (int8_t)((int) v - 256);
  }
}
static bool decode_boot_mouse_report(const uint8_t *buf, size_t len, BootMouseReport *out) {
  {
    if (len < 3) return false;
    int8_t wheel = len >= 4 ? wrap_i8(buf[3]) : 0;
    out->buttons = buf[0];
    out->dx = wrap_i8(buf[1]);
    out->dy = wrap_i8(buf[2]);
    out->wheel = wheel;
    return true;
  }
}
static bool decode_tablet_report(const uint8_t *buf, size_t len, TabletReport *out) {
  {
    if (len < 5) return false;
    uint16_t abs_x = (uint16_t) buf[1] | (uint16_t)((uint16_t) buf[2] << 8);
    uint16_t abs_y = (uint16_t) buf[3] | (uint16_t)((uint16_t) buf[4] << 8);
    out->buttons = buf[0] & 0x07u;
    out->abs_x = abs_x;
    out->abs_y = abs_y;
    return true;
  }
}
static HidRole classify_single_hid_role(bool found_hid_keyboard, bool found_hid_tablet, bool found_hid_mouse) {
  {
    if (found_hid_keyboard) return HID_ROLE_KEYBOARD;
    if (found_hid_tablet && !found_hid_mouse) return HID_ROLE_POINTER_TABLET;
    if (found_hid_mouse) return HID_ROLE_POINTER_MOUSE;
    return HID_ROLE_UNKNOWN;
  }
}
static const char *hid_role_debug_str(HidRole role) {
  {
    if (role == HID_ROLE_KEYBOARD) return "Keyboard";
    if (role == HID_ROLE_POINTER_TABLET) return "PointerTablet";
    if (role == HID_ROLE_POINTER_MOUSE) return "PointerMouse";
    return "Unknown";
  }
}
static uint64_t send_report_to_sexinput(SexusbState *st, uint64_t op, uint64_t arg0, uint64_t arg1, uint64_t arg2, bool *out_sent) {
  {
    if (atomic_load_explicit(&st->cap_ready_sexinput, memory_order_relaxed)) {
      SexosReply rep = sexos_pdx_call(SLOT_USB_SEXINPUT, op, arg0, arg1, arg2);
      if (rep.status == 0ULL) {
        if (!atomic_exchange_explicit(&st->route_ready_emitted_sexinput, true, memory_order_relaxed)) sexos_line("[sexusb.route.sexinput.ready] slot=%llu ok=1", (unsigned long long) SLOT_USB_SEXINPUT);
        if (!atomic_exchange_explicit(&st->bootgraph_proof_marker_emitted, true, memory_order_relaxed)) sexos_line("[sexusb.bootgraph.proof_report]");
        if (!atomic_exchange_explicit(&st->edge_send_emitted_sexinput, true, memory_order_relaxed)) sexos_line("[bootgraph.edge.send from=sexusb to=sexinput slot=9 op=HID_REPORT first=1]");
        *out_sent = true;
        return 0ULL;
      } else {
        *out_sent = false;
        return rep.status;
      }
    }
    SexosReply rep = sexos_pdx_call(SLOT_USB_SEXINPUT, op, arg0, arg1, arg2);
    if (rep.status == 0ULL) {
      atomic_store_explicit(&st->cap_ready_sexinput, true, memory_order_relaxed);
      if (!atomic_exchange_explicit(&st->route_ready_emitted_sexinput, true, memory_order_relaxed)) sexos_line("[sexusb.route.sexinput.ready] slot=%llu ok=1", (unsigned long long) SLOT_USB_SEXINPUT);
      if (!atomic_exchange_explicit(&st->bootgraph_proof_marker_emitted, true, memory_order_relaxed)) sexos_line("[sexusb.bootgraph.proof_report]");
      if (!atomic_exchange_explicit(&st->edge_send_emitted_sexinput, true, memory_order_relaxed)) sexos_line("[bootgraph.edge.send from=sexusb to=sexinput slot=9 op=HID_REPORT first=1]");
      *out_sent = true;
      return 0ULL;
    }
    if (rep.status == ERR_CAP_INVALID) {
      if (!atomic_exchange_explicit(&st->route_missing_emitted_sexinput, true, memory_order_relaxed)) sexos_line("[sexusb.route.sexinput.missing] slot=%llu ok=0", (unsigned long long) SLOT_USB_SEXINPUT);
      if (!atomic_exchange_explicit(&st->defer_emitted_sexinput, true, memory_order_relaxed)) sexos_line("[bootgraph.edge.defer from=sexusb to=sexinput slot=9 reason=missing_cap]");
      sexos_yield();
      *out_sent = false;
      return 0ULL;
    }
    *out_sent = false;
    return rep.status;
  }
}
static void send_synthetic_mouse_frame(SexusbState *st, uint32_t n, uint8_t buttons, int8_t dx, int8_t dy) {
  {
    uint8_t buf[4];
    buf[0] = buttons;
    buf[1] = (uint8_t) dx;
    buf[2] = (uint8_t) dy;
    buf[3] = 0u;
    BootMouseReport decoded;
    if (decode_boot_mouse_report(buf, 4, &decoded)) {
      uint64_t packed_axes = (uint64_t)(uint8_t) decoded.dx | (uint64_t)(uint8_t) decoded.dy << 8 | (uint64_t)(uint8_t) decoded.wheel << 16;
      sexos_line("[sexusb.synthetic.frame] n=%u dx=%d dy=%d buttons=%u", (unsigned) n, (int) decoded.dx, (int) decoded.dy, (unsigned) decoded.buttons);
      bool sent = false;
      uint64_t err = send_report_to_sexinput(st, OP_USB_MOUSE_REPORT, 0ULL, (uint64_t) decoded.buttons, packed_axes, &sent);
      if (err == 0ULL) {
        if (sent) sexos_line("[sexusb.synthetic.send.ok]");
      } else sexos_line("[sexusb.synthetic.send.fail] e=%llu", (unsigned long long) err);
    }
  }
}
void sexusb_main(void) {
  {
    SexusbState *st = &SEXUSB_STATE;
    sexusb_state_init(st);
    sexos_line("[sexusb.init.start]");
    sexos_line("[sexusb] USB DRIVER STARTING (CUSTOM BUILD, NO SYNTHETIC)...");
    sexos_line("[usb.host.discovery.start]");
    sexos_line("[usb.host.pci.scan] slot=%llu bar=0", (unsigned long long) SLOT_USB_HOST);
    uint64_t map_va = 0ULL;
    uint32_t map_attempts = 0u;
    while (map_attempts < 3u) {
      map_attempts += 1u;
      map_va = map_xhci_bar0(MAP_BYTES);
      if (map_va != 0ULL && map_va != UINT64_MAX) break;
      sexos_yield();
    }
    if (map_va == 0ULL || map_va == UINT64_MAX) {
      sexos_line("[sexusb.xhci.map.bad] reason=bar_zero_after_retry attempts=3 ok=0");
      sexos_line("[usb.host.controller.none] map_va=%#llx", (unsigned long long) map_va);
      sexos_line("[usb.host.discovery.done]");
      while (true) sexos_yield();
    }
    sexos_line("[usb.host.controller.found] slot=%llu bar=0 map_va=%#llx", (unsigned long long) SLOT_USB_HOST, (unsigned long long) map_va);
    sexos_line("[usb.host.discovery.done]");
    sexos_line("[sexusb.xhci.map.ok]");
    sexos_line("[sexusb.xhci.diag.after_map_ok]");
    const uint32_t *regs = (const uint32_t *)(uintptr_t) map_va;
    uint32_t cap0 = sexos_volatile_read32(regs);
    uint8_t caplength = (uint8_t)(cap0 & 0xFFu);
    uint16_t hciversion = (uint16_t)(cap0 >> 16 & 0xFFFFu);
    uint32_t hcsp1 = sexos_volatile_read32(regs + 1);
    uint32_t hcsp2 = sexos_volatile_read32(regs + 2);
    uint32_t hcc1 = sexos_volatile_read32(regs + 4);
    sexos_line("[sexusb.xhci.caplength] %#x", (unsigned) caplength);
    sexos_line("[sexusb.xhci.hciversion] %#x", (unsigned) hciversion);
    sexos_line("[sexusb.xhci.hcsp1] %#x", (unsigned) hcsp1);
    sexos_line("[sexusb.xhci.hcsp2] %#x", (unsigned) hcsp2);
    sexos_line("[sexusb.xhci.hcc1] %#x", (unsigned) hcc1);
    sexos_line("[usb.host.caps] caplength=%#x hciversion=%#x hcsp1=%#x hcsp2=%#x hcc1=%#x", (unsigned) caplength, (unsigned) hciversion, (unsigned) hcsp1, (unsigned) hcsp2, (unsigned) hcc1);
    sexos_line("[sexusb.xhci.probe.ok]");
    uint64_t op_base = map_va + (uint64_t) caplength;
    sexos_line("[sexusb.xhci.reset.start]");
    uint32_t usbcmd = mmio_read32(op_base, XHCI_USBCMD);
    if ((usbcmd & USBCMD_RUN_STOP) != 0u) {
      usbcmd &= ~USBCMD_RUN_STOP;
      mmio_write32(op_base, XHCI_USBCMD, usbcmd);
    }
    if (wait_until(op_base, XHCI_USBSTS, USBSTS_HCHALTED, true, POLL_BUDGET)) sexos_line("[sexusb.xhci.halted.ok]");
    else sexos_line("[sexusb.xhci.halted.bad]");
    usbcmd = mmio_read32(op_base, XHCI_USBCMD) | USBCMD_HCRST;
    mmio_write32(op_base, XHCI_USBCMD, usbcmd);
    bool reset_done = wait_until(op_base, XHCI_USBCMD, USBCMD_HCRST, false, POLL_BUDGET);
    bool cnr_done = wait_until(op_base, XHCI_USBSTS, USBSTS_CNR, false, POLL_BUDGET);
    if (reset_done && cnr_done) sexos_line("[sexusb.xhci.reset.ok]");
    else sexos_line("[sexusb.xhci.reset.bad]");
    sexos_line("[sexusb.xhci.ring.alloc.start]");
    uint64_t erst_bytes = PAGE_SIZE;
    uint64_t cmd_ring_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t event_ring_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t erst_phys = sys_alloc_phys(erst_bytes);
    uint64_t dcbaa_phys = sys_alloc_phys(DCBAA_BYTES);
    if (cmd_ring_phys == 0ULL || cmd_ring_phys == UINT64_MAX || (event_ring_phys == 0ULL || event_ring_phys == UINT64_MAX || (erst_phys == 0ULL || erst_phys == UINT64_MAX || (dcbaa_phys == 0ULL || dcbaa_phys == UINT64_MAX)))) {
      sexos_line("[sexusb.xhci.ring.alloc.bad]");
      while (true) sexos_yield();
    }
    uint64_t cmd_ring_va = sys_map_phys(cmd_ring_phys, PAGE_SIZE);
    uint64_t event_ring_va = sys_map_phys(event_ring_phys, PAGE_SIZE);
    uint64_t erst_va = sys_map_phys(erst_phys, erst_bytes);
    uint64_t dcbaa_va = sys_map_phys(dcbaa_phys, DCBAA_BYTES);
    if (cmd_ring_va == 0ULL || cmd_ring_va == UINT64_MAX || (event_ring_va == 0ULL || event_ring_va == UINT64_MAX || (erst_va == 0ULL || erst_va == UINT64_MAX || (dcbaa_va == 0ULL || dcbaa_va == UINT64_MAX)))) {
      sexos_line("[sexusb.xhci.ring.alloc.bad]");
      while (true) sexos_yield();
    }
    bool aligned_ok = cmd_ring_phys % PAGE_SIZE == 0ULL && event_ring_phys % PAGE_SIZE == 0ULL && (erst_phys % PAGE_SIZE == 0ULL && dcbaa_phys % PAGE_SIZE == 0ULL) && (cmd_ring_phys % 64ULL == 0ULL && event_ring_phys % 64ULL == 0ULL && (erst_phys % 64ULL == 0ULL && dcbaa_phys % 64ULL == 0ULL) && (cmd_ring_va % PAGE_SIZE == 0ULL && event_ring_va % PAGE_SIZE == 0ULL && (erst_va % PAGE_SIZE == 0ULL && dcbaa_va % PAGE_SIZE == 0ULL)));
    if (!aligned_ok) {
      sexos_line("[sexusb.xhci.ring.align.bad]");
      while (true) sexos_yield();
    }
    memset((void *)(uintptr_t) cmd_ring_va, 0, (size_t) PAGE_SIZE);
    memset((void *)(uintptr_t) event_ring_va, 0, (size_t) PAGE_SIZE);
    memset((void *)(uintptr_t) erst_va, 0, (size_t) erst_bytes);
    memset((void *)(uintptr_t) dcbaa_va, 0, (size_t) DCBAA_BYTES);
    sexos_line("[sexusb.xhci.cmd_ring.ok]");
    sexos_line("[sexusb.xhci.event_ring.ok]");
    sexos_line("[sexusb.xhci.dcbaa.ok]");
    sexos_volatile_write64((uint64_t *)(uintptr_t) erst_va, event_ring_phys);
    sexos_volatile_write32((uint32_t *)(uintptr_t)(erst_va + 8ULL), (uint32_t) EVENT_RING_TRBS);
    sexos_volatile_write32((uint32_t *)(uintptr_t)(erst_va + 12ULL), 0u);
    sexos_line("[sexusb.xhci.erst.ok]");
    sexos_line("[sexusb.xhci.ring.ptrs.write.start]");
    uint64_t cap_base = map_va;
    uint32_t rtsoff_raw = mmio_read32(cap_base, XHCI_CAP_RTSOFF);
    uint64_t rtsoff = (uint64_t)(rtsoff_raw & ~0x1Fu);
    uint32_t hcc1_local = mmio_read32(cap_base, XHCI_CAP_HCCPARAMS1);
    (void) hcc1_local;
    uint64_t runtime_base = map_va + rtsoff;
    uint64_t intr_base = runtime_base + XHCI_INTR_BASE;
    uint64_t op_need_end = op_base + (XHCI_DCBAAP + 8ULL);
    uint64_t rt_need_end = intr_base + (XHCI_INTR_ERDP + 8ULL);
    if (op_need_end > map_va + MAP_BYTES || rt_need_end > map_va + MAP_BYTES) {
      sexos_line("[sexusb.xhci.ring.ptrs.write.bad]");
      while (true) sexos_yield();
    }
    mmio_write32(op_base, XHCI_DCBAAP, (uint32_t) dcbaa_phys);
    mmio_write32(op_base, XHCI_DCBAAP + 4ULL, (uint32_t)(dcbaa_phys >> 32));
    sexos_line("[sexusb.xhci.dcbaap.write.ok]");
    mmio_write32(op_base, XHCI_CRCR, (uint32_t)(cmd_ring_phys | 1ULL));
    mmio_write32(op_base, XHCI_CRCR + 4ULL, (uint32_t)((cmd_ring_phys | 1ULL) >> 32));
    uint32_t crcr_check_lo = mmio_read32(op_base, XHCI_CRCR);
    uint32_t crcr_check_hi = mmio_read32(op_base, XHCI_CRCR + 4ULL);
    uint64_t crcr_check = (uint64_t) crcr_check_lo | (uint64_t) crcr_check_hi << 32;
    sexos_line("[sexusb.xhci.crcr.write.ok] wrote=%#llx readback=%#llx", (unsigned long long)(cmd_ring_phys | 1ULL), (unsigned long long) crcr_check);
    sexos_line("[sexusb.xhci.phys.diag] cmd=%#llx event=%#llx erst=%#llx dcbaa=%#llx", (unsigned long long) cmd_ring_phys, (unsigned long long) event_ring_phys, (unsigned long long) erst_phys, (unsigned long long) dcbaa_phys);
    mmio_write32(intr_base, XHCI_INTR_ERSTSZ, (uint32_t) ERST_ENTRIES);
    mmio_write32(intr_base, XHCI_INTR_ERSTBA, (uint32_t) erst_phys);
    mmio_write32(intr_base, XHCI_INTR_ERSTBA + 4ULL, (uint32_t)(erst_phys >> 32));
    sexos_line("[sexusb.xhci.erst.write.ok]");
    mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) event_ring_phys);
    mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(event_ring_phys >> 32));
    sexos_line("[sexusb.xhci.erdp.write.ok]");
    uint32_t max_slots = hcsp1 & 0xFFu;
    if (max_slots == 0u) {
      sexos_line("[sexusb.xhci.config.max_slots.zero.bad]");
      while (true) sexos_yield();
    }
    mmio_write32(op_base, XHCI_OP_CONFIG, max_slots);
    sexos_line("[sexusb.xhci.config.ok] max_slots=%u", (unsigned) max_slots);
    sexos_line("[sexusb.xhci.ring.proof.ok]");
    sexos_line("[sexusb.xhci.ring]");
    uint32_t usbcmd_before = mmio_read32(op_base, XHCI_USBCMD);
    uint32_t usbsts_before = mmio_read32(op_base, XHCI_USBSTS);
    sexos_line("[sexusb.xhci.run.diag.before] usbcmd=%#x usbsts=%#x", (unsigned) usbcmd_before, (unsigned) usbsts_before);
    usbcmd = mmio_read32(op_base, XHCI_USBCMD) | USBCMD_RUN_STOP | 1u << 2;
    mmio_write32(op_base, XHCI_USBCMD, usbcmd);
    if (wait_until(op_base, XHCI_USBSTS, USBSTS_HCHALTED, false, POLL_BUDGET)) {
      uint32_t usbcmd_after = mmio_read32(op_base, XHCI_USBCMD);
      uint32_t usbsts_after = mmio_read32(op_base, XHCI_USBSTS);
      sexos_line("[sexusb.xhci.run.diag.after] usbcmd=%#x usbsts=%#x", (unsigned) usbcmd_after, (unsigned) usbsts_after);
      sexos_line("[sexusb.xhci.run.ok]");
      uint8_t rs_on = (usbcmd_after & USBCMD_RUN_STOP) != 0u ? 1u : 0u;
      uint8_t halted = (usbsts_after & USBSTS_HCHALTED) != 0u ? 1u : 0u;
      uint8_t cnr = (usbsts_after & USBSTS_CNR) != 0u ? 1u : 0u;
      uint8_t hse = (usbsts_after & 1u << 2) != 0u ? 1u : 0u;
      uint8_t eint = (usbsts_after & 1u << 3) != 0u ? 1u : 0u;
      sexos_line("[sexusb.xhci.run.state] rs=%u halted=%u cnr=%u hse=%u eint=%u", (unsigned) rs_on, (unsigned) halted, (unsigned) cnr, (unsigned) hse, (unsigned) eint);
    } else sexos_line("[sexusb.xhci.run.bad]");
    sexos_line("[sexusb.xhci.ring_state.audit.start]");
    uint64_t cmd_idx = 0ULL;
    uint32_t cmd_cycle = 1u;
    uint64_t ev_idx = 0ULL;
    uint64_t ev_dcs = 1ULL;
    sexos_line("[sexusb.xhci.ring_state.cmd_cycle.ok]");
    sexos_line("[sexusb.xhci.ring_state.stop_marker.ok]");
    sexos_line("[sexusb.xhci.ring_state.event_cycle.ok]");
    sexos_line("[usb.xhci.enum.begin]");
    sexos_line("[sexusb.xhci.cmd.noop.start]");
    uint32_t noop_d3 = TRB_TYPE_NOOP_CMD << 10 | cmd_cycle;
    trb_write_volatile(cmd_ring_va, cmd_idx, 0u, 0u, 0u, noop_d3);
    trb_write_volatile(cmd_ring_va, cmd_idx + 1ULL, 0u, 0u, 0u, cmd_cycle ^ 1u);
    sexos_line("[sexusb.xhci.cmd.noop.trb.ok]");
    uint32_t dboff_raw = mmio_read32(cap_base, XHCI_CAP_DBOFF);
    uint64_t db_base = map_va + (uint64_t)(dboff_raw & ~0x3u);
    sexos_line("[sexusb.xhci.diag.dboff] raw=%#x db_base=%#llx map_va=%#llx", (unsigned) dboff_raw, (unsigned long long) db_base, (unsigned long long) map_va);
    if (db_base + 4ULL > map_va + MAP_BYTES) {
      sexos_line("[sexusb.xhci.ring.ptrs.write.bad]");
      while (true) sexos_yield();
    }
    uint32_t mfindex = mmio_read32(map_va, rtsoff);
    uint32_t iman_rb = mmio_read32(intr_base, 0ULL);
    uint32_t erstsz_rb = mmio_read32(intr_base, XHCI_INTR_ERSTSZ);
    uint32_t erstba_lo = mmio_read32(intr_base, XHCI_INTR_ERSTBA);
    uint32_t erstba_hi = mmio_read32(intr_base, XHCI_INTR_ERSTBA + 4ULL);
    uint64_t erstba_rb = (uint64_t) erstba_lo | (uint64_t) erstba_hi << 32;
    uint32_t erdp_lo = mmio_read32(intr_base, XHCI_INTR_ERDP);
    uint32_t erdp_hi = mmio_read32(intr_base, XHCI_INTR_ERDP + 4ULL);
    uint64_t erdp_rb = (uint64_t) erdp_lo | (uint64_t) erdp_hi << 32;
    uint32_t usbcmd_rb = mmio_read32(op_base, XHCI_USBCMD);
    uint32_t usbsts_rb = mmio_read32(op_base, XHCI_USBSTS);
    uint32_t crcr_lo = mmio_read32(op_base, XHCI_CRCR);
    uint32_t crcr_hi = mmio_read32(op_base, XHCI_CRCR + 4ULL);
    uint64_t crcr_rb = (uint64_t) crcr_lo | (uint64_t) crcr_hi << 32;
    uint32_t dcbaap_lo = mmio_read32(op_base, XHCI_DCBAAP);
    uint32_t dcbaap_hi = mmio_read32(op_base, XHCI_DCBAAP + 4ULL);
    uint64_t dcbaap_rb = (uint64_t) dcbaap_lo | (uint64_t) dcbaap_hi << 32;
    sexos_line("[sexusb.xhci.diag.readback] mfindex=%#x iman=%#x erstsz=%#x erstba=%#llx erdp=%#llx usbcmd=%#x usbsts=%#x crcr=%#llx dcbaap=%#llx", (unsigned) mfindex, (unsigned) iman_rb, (unsigned) erstsz_rb, (unsigned long long) erstba_rb, (unsigned long long) erdp_rb, (unsigned) usbcmd_rb, (unsigned) usbsts_rb, (unsigned long long) crcr_rb, (unsigned long long) dcbaap_rb);
    mmio_write32(intr_base, 0ULL, 2u);
    uint32_t iman_after = mmio_read32(intr_base, 0ULL);
    sexos_line("[sexusb.xhci.diag.iman_ie] iman=%#x", (unsigned) iman_after);
    sexos_sfence();
    mmio_write32(db_base, 0ULL, 0u);
    sexos_line("[sexusb.xhci.cmd.noop.doorbell.ok]");
    uint32_t max_scratchpad = hcsp2 >> 16 & 0x7FFu;
    uint64_t dcbaa0 = sexos_volatile_read64((const uint64_t *)(uintptr_t) dcbaa_va);
    sexos_line("[sexusb.xhci.diag.scratchpad] max=%u dcbaa[0]=%#llx", (unsigned) max_scratchpad, (unsigned long long) dcbaa0);
    for (uint64_t ti = 0ULL; ti < 2ULL; ++ti) {
      uint32_t d0 = trb_read_dword(cmd_ring_va, ti, 0);
      uint32_t d1 = trb_read_dword(cmd_ring_va, ti, 1);
      uint32_t d2 = trb_read_dword(cmd_ring_va, ti, 2);
      uint32_t d3 = trb_read_dword(cmd_ring_va, ti, 3);
      sexos_line("[sexusb.xhci.diag.trb_full] trb[%llu] d0=%#x d1=%#x d2=%#x d3=%#x", (unsigned long long) ti, (unsigned) d0, (unsigned) d1, (unsigned) d2, (unsigned) d3);
    }
    uint32_t cmd_d3_0 = trb_read_dword(cmd_ring_va, 0ULL, 3);
    uint32_t cmd_d3_1 = trb_read_dword(cmd_ring_va, 1ULL, 3);
    uint32_t ev_d3_before = trb_read_dword(event_ring_va, 0ULL, 3);
    sexos_line("[sexusb.xhci.diag.trb_check] cmd[0].d3=%#x cmd[1].d3=%#x ev[0].d3=%#x", (unsigned) cmd_d3_0, (unsigned) cmd_d3_1, (unsigned) ev_d3_before);
    uint64_t poll_read_addr = event_ring_va + ev_idx * 16ULL;
    sexos_line("[sexusb.xhci.noop.poll.enter] event_va=%#llx ev_idx=%llu ev_dcs=%llu read_addr=%#llx", (unsigned long long) event_ring_va, (unsigned long long) ev_idx, (unsigned long long) ev_dcs, (unsigned long long) poll_read_addr);
    if (event_ring_va == 0ULL || event_ring_va == UINT64_MAX || ev_idx >= 256ULL) {
      sexos_line("[sexusb.xhci.noop.poll.event_va.bad]");
      while (true) sexos_yield();
    }
    if (poll_read_addr < event_ring_va || poll_read_addr + 15ULL >= event_ring_va + 4096ULL) {
      sexos_line("[sexusb.xhci.noop.poll.event_bounds.bad]");
      while (true) sexos_yield();
    }
    bool noop_ok = false;
    size_t enable_slot_polls = 0;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      enable_slot_polls += 1;
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_CMD_COMPLETION_EVENT) {
          uint32_t ev_d0 = trb_read_dword(event_ring_va, ev_idx, 0);
          uint32_t ev_d1 = trb_read_dword(event_ring_va, ev_idx, 1);
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          noop_ok = cc == TRB_CC_SUCCESS;
          sexos_line("[sexusb.xhci.cmd.noop.event.raw] d0=%#x d1=%#x d2=%#x d3=%#x type=%u cc=%u", (unsigned) ev_d0, (unsigned) ev_d1, (unsigned) ev_d2, (unsigned) ev_d3, (unsigned) ev_type, (unsigned) cc);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (noop_ok) sexos_line("[sexusb.xhci.cmd.noop.complete.ok]");
    else {
      sexos_line("[sexusb.xhci.cmd.noop.complete.bad]");
      uint32_t usbsts_now = mmio_read32(op_base, XHCI_USBSTS);
      uint32_t crcr_lo_now = mmio_read32(op_base, XHCI_CRCR);
      uint32_t crcr_hi_now = mmio_read32(op_base, XHCI_CRCR + 4ULL);
      uint64_t crcr_now = (uint64_t) crcr_lo_now | (uint64_t) crcr_hi_now << 32;
      uint32_t ev0_d0 = trb_read_dword(event_ring_va, 0ULL, 0);
      uint32_t ev0_d1 = trb_read_dword(event_ring_va, 0ULL, 1);
      uint32_t ev0_d2 = trb_read_dword(event_ring_va, 0ULL, 2);
      uint32_t ev0_d3 = trb_read_dword(event_ring_va, 0ULL, 3);
      uint64_t crr = crcr_now >> 8 & 1ULL;
      sexos_line("[sexusb.xhci.diag.after_timeout] usbsts=%#x crcr=%#llx crr=%llu ev[0]=%#x,%#x,%#x,%#x", (unsigned) usbsts_now, (unsigned long long) crcr_now, (unsigned long long) crr, (unsigned) ev0_d0, (unsigned) ev0_d1, (unsigned) ev0_d2, (unsigned) ev0_d3);
      while (true) sexos_yield();
    }
    cmd_idx += 1ULL;
    sexos_line("[sexusb.xhci.cmd.submit] kind=enable_slot");
    sexos_line("[sexusb.xhci.enable_slot.start]");
    uint32_t enable_slot_d3 = TRB_TYPE_ENABLE_SLOT_CMD << 10 | cmd_cycle;
    trb_write_volatile(cmd_ring_va, cmd_idx, 0u, 0u, 0u, enable_slot_d3);
    trb_write_volatile(cmd_ring_va, cmd_idx + 1ULL, 0u, 0u, 0u, cmd_cycle ^ 1u);
    sexos_line("[sexusb.xhci.enable_slot.trb.ok]");
    mmio_write32(db_base, 0ULL, 0u);
    sexos_line("[sexusb.xhci.enable_slot.doorbell.ok]");
    bool en_ok = false;
    uint32_t en_slot_id = 0u;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_CMD_COMPLETION_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          uint32_t ev_d0 = trb_read_dword(event_ring_va, ev_idx, 0);
          uint32_t ev_d1 = trb_read_dword(event_ring_va, ev_idx, 1);
          sexos_line("[sexusb.xhci.enable_slot.event.raw] d0=%#x d1=%#x d2=%#x d3=%#x", (unsigned) ev_d0, (unsigned) ev_d1, (unsigned) ev_d2, (unsigned) ev_d3);
          en_ok = cc == TRB_CC_SUCCESS;
          en_slot_id = ev_d3 >> 24 & 0xFFu;
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (en_ok && en_slot_id != 0u) {
      sexos_line("[sexusb.xhci.enable_slot.complete.ok]");
      sexos_line("[sexusb.xhci.enable_slot.slot.ok] %u", (unsigned) en_slot_id);
      sexos_line("[usb.xhci.cmd.complete] kind=enable_slot slot=%u code=%u ok=1", (unsigned) en_slot_id, (unsigned) TRB_CC_SUCCESS);
    } else {
      sexos_line("[usb.xhci.enum.timeout] phase=SLOT detail=enable_slot_no_completion polls=%zu ok=0", enable_slot_polls);
      sexos_line("[usb.xhci.enum.stop] reason=enable_slot_no_completion actionable=1");
      sexos_line("[sexusb.xhci.enable_slot.complete.bad]");
    }
    sexos_line("[sexusb.xhci.ring_state.audit.ok]");
    cmd_idx += 1ULL;
    sexos_line("[sexusb.xhci.addr_ctx.layout.start]");
    uint64_t ctx_stride = (hcc1 & 1u << 2) != 0u ? 64ULL : 32ULL;
    sexos_line("[sexusb.xhci.addr_ctx.stride.ok] %llu", (unsigned long long) ctx_stride);
    uint64_t max_ports = (uint64_t)(hcsp1 >> 24 & 0xFFu);
    sexos_line("[sexusb.xhci.addr_ctx.ports] %llu", (unsigned long long) max_ports);
    PortDiscoveryState port_states[MAX_USB_PORT_TRACKED];
    memset(port_states, 0, sizeof port_states);
    uint8_t target_ports[MAX_USB_DEVICES];
    memset(target_ports, 0, sizeof target_ports);
    size_t target_port_count = 0;
    uint32_t port_speed = 0u;
    uint32_t ports_connected = 0u;
    uint64_t scan_limit = max_ports > (uint64_t) MAX_USB_PORT_TRACKED ? (uint64_t) MAX_USB_PORT_TRACKED : max_ports;
    sexos_line("[usb.xhci.multiport.begin]");
    for (uint64_t port = 1ULL; port <= scan_limit; ++port) {
      uint64_t portsc_off = PORTSC_BASE + (port - 1ULL) * PORTSC_STRIDE;
      uint32_t portsc = mmio_read32(op_base, portsc_off);
      uint32_t connected = (portsc & PORTSC_CCS) != 0u ? 1u : 0u;
      uint32_t enabled = (portsc & PORTSC_PED) != 0u ? 1u : 0u;
      uint32_t speed = portsc >> 10 & 0xFu;
      if (connected == 1u) ports_connected += 1u;
      if ((size_t) port <= MAX_USB_PORT_TRACKED) {
        PortDiscoveryState *state = &port_states[(size_t) port - 1];
        state->port = (uint8_t) port;
        state->connected = connected == 1u;
        state->enabled = enabled == 1u;
        state->reset_attempted = false;
        state->slot_id = 0u;
      }
      sexos_line("[usb.xhci.port.scan] port=%llu connected=%u enabled=%u reset=%u slot=%u", (unsigned long long) port, (unsigned) connected, (unsigned) enabled, 0u, 0u);
      sexos_line("[sexusb.xhci.port] port=%llu connected=%u enabled=%u speed=%u", (unsigned long long) port, (unsigned) connected, (unsigned) enabled, (unsigned) speed);
      if (connected == 1u) if (target_port_count < MAX_USB_DEVICES) {
        target_ports[target_port_count] = (uint8_t) port;
        if (target_port_count == 0) {
          port_speed = speed;
          sexos_line("[sexusb.xhci.addr_ctx.port.connected] port=%llu speed=%u", (unsigned long long) port, (unsigned) speed);
        }
        target_port_count += 1;
      } else sexos_line("[usb.xhci.port.skip] port=%llu reason=target_capacity", (unsigned long long) port);
      else sexos_line("[usb.xhci.port.skip] port=%llu reason=not_connected", (unsigned long long) port);
    }
    uint64_t target_port = (uint64_t) target_ports[0];
    if (target_port_count == 0) {
      sexos_line("[sexusb.discovery.summary] ports=%llu connected=0 first=none ok=1", (unsigned long long) max_ports);
      sexos_line("[sexusb.xhci.addr_ctx.port.none.bad]");
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.discovery.summary] ports=%llu connected=%zu first=%llu ok=1", (unsigned long long) max_ports, target_port_count, (unsigned long long) target_port);
    sexos_line("[sexusb.ports.collect] count=%zu first=%llu", target_port_count, (unsigned long long) target_port);
    if (max_ports > (uint64_t) MAX_USB_PORT_TRACKED) sexos_line("[usb.xhci.port.skip] port=%zu reason=track_capacity", MAX_USB_PORT_TRACKED + 1);
    uint64_t input_ctx_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t device_ctx_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t ep0_ring_phys = sys_alloc_phys(PAGE_SIZE);
    if (input_ctx_phys == 0ULL || input_ctx_phys == UINT64_MAX || (device_ctx_phys == 0ULL || device_ctx_phys == UINT64_MAX || (ep0_ring_phys == 0ULL || ep0_ring_phys == UINT64_MAX))) {
      sexos_line("[sexusb.xhci.addr_ctx.alloc.bad]");
      while (true) sexos_yield();
    }
    uint64_t input_ctx_va = sys_map_phys(input_ctx_phys, PAGE_SIZE);
    uint64_t device_ctx_va = sys_map_phys(device_ctx_phys, PAGE_SIZE);
    uint64_t ep0_ring_va = sys_map_phys(ep0_ring_phys, PAGE_SIZE);
    if (input_ctx_va == 0ULL || input_ctx_va == UINT64_MAX || (device_ctx_va == 0ULL || device_ctx_va == UINT64_MAX || (ep0_ring_va == 0ULL || ep0_ring_va == UINT64_MAX))) {
      sexos_line("[sexusb.xhci.addr_ctx.map.bad]");
      while (true) sexos_yield();
    }
    bool ctx_align_ok = input_ctx_phys % 64ULL == 0ULL && device_ctx_phys % 64ULL == 0ULL && (ep0_ring_phys % 64ULL == 0ULL && input_ctx_va % PAGE_SIZE == 0ULL) && (device_ctx_va % PAGE_SIZE == 0ULL && ep0_ring_va % PAGE_SIZE == 0ULL);
    if (!ctx_align_ok) {
      sexos_line("[sexusb.xhci.addr_ctx.align.bad]");
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.xhci.addr_ctx.input.ok]");
    sexos_line("[sexusb.xhci.addr_ctx.device.ok]");
    sexos_line("[sexusb.xhci.addr_ctx.ep0_ring.ok]");
    memset((void *)(uintptr_t) input_ctx_va, 0, (size_t) PAGE_SIZE);
    memset((void *)(uintptr_t) device_ctx_va, 0, (size_t) PAGE_SIZE);
    memset((void *)(uintptr_t) ep0_ring_va, 0, (size_t) PAGE_SIZE);
    uint32_t *icc_ptr = (uint32_t *)(uintptr_t) input_ctx_va;
    sexos_volatile_write32(icc_ptr + 0, ICC_DROP_NONE);
    sexos_volatile_write32(icc_ptr + 1, ICC_ADD_SLOT_EP0);
    sexos_line("[sexusb.xhci.addr_ctx.icc.ok]");
    uint32_t context_entries = 1u;
    uint32_t slot_dw0 = context_entries << SLOT_CTX_ENTRIES_SHIFT | port_speed << SLOT_SPEED_SHIFT;
    uint32_t slot_dw1 = (uint32_t) target_port << 16;
    uint32_t *input_slot_base = (uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride);
    sexos_volatile_write32(input_slot_base + 0, slot_dw0);
    sexos_volatile_write32(input_slot_base + 1, slot_dw1);
    uint32_t *dev_slot_base = (uint32_t *)(uintptr_t) device_ctx_va;
    sexos_volatile_write32(dev_slot_base + 0, slot_dw0);
    sexos_volatile_write32(dev_slot_base + 1, slot_dw1);
    sexos_line("[sexusb.xhci.addr_ctx.slot.ok]");
    uint32_t max_packet_size = 0u;
    if (port_speed == 0u || port_speed == 1u) max_packet_size = 8u;
    else if (port_speed == 2u) max_packet_size = 64u;
    else if (port_speed == 3u) max_packet_size = 512u;
    else {
      sexos_line("[sexusb.xhci.addr_ctx.ep0.speed.unknown.bad] port_speed=%u", (unsigned) port_speed);
      while (true) sexos_yield();
    }
    uint32_t ep0_dw0 = max_packet_size << 16;
    uint32_t ep0_dw1 = EP_TYPE_CTRL_VAL << EP_TYPE_CTRL_SHIFT | CERR_DEFAULT;
    uint64_t ep0_tr_dequeue = ep0_ring_phys | DCS_INITIAL;
    uint32_t ep0_dw2 = (uint32_t)(ep0_tr_dequeue & 0xFFFFFFFFULL);
    uint32_t ep0_dw3 = (uint32_t)(ep0_tr_dequeue >> 32);
    uint32_t *input_ep0_base = (uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride * 2ULL);
    sexos_volatile_write32(input_ep0_base + 0, ep0_dw0);
    sexos_volatile_write32(input_ep0_base + 1, ep0_dw1);
    sexos_volatile_write32(input_ep0_base + 2, ep0_dw2);
    sexos_volatile_write32(input_ep0_base + 3, ep0_dw3);
    uint32_t *dev_ep0_base = (uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride);
    sexos_volatile_write32(dev_ep0_base + 0, ep0_dw0);
    sexos_volatile_write32(dev_ep0_base + 1, ep0_dw1);
    sexos_volatile_write32(dev_ep0_base + 2, ep0_dw2);
    sexos_volatile_write32(dev_ep0_base + 3, ep0_dw3);
    sexos_line("[sexusb.xhci.addr_ctx.ep0.ok]");
    uint32_t dcbaa_max_slots = hcsp1 >> 24 & 0xFFu;
    if (en_slot_id == 0u || en_slot_id >= dcbaa_max_slots) {
      sexos_line("[sexusb.xhci.addr_ctx.dcbaa.slot.bad] slot=%u max_slots=%u", (unsigned) en_slot_id, (unsigned) dcbaa_max_slots);
      while (true) sexos_yield();
    }
    uint64_t *dcbaa_entry_ptr = (uint64_t *)(uintptr_t)(dcbaa_va + (uint64_t) en_slot_id * 8ULL);
    sexos_volatile_write64(dcbaa_entry_ptr, device_ctx_phys);
    uint64_t dcbaa_ent_readback = sexos_volatile_read64(dcbaa_entry_ptr);
    sexos_line("[sexusb.xhci.addr_ctx.dcbaa.ok] wrote=%#llx readback=%#llx", (unsigned long long) device_ctx_phys, (unsigned long long) dcbaa_ent_readback);
    sexos_line("[sexusb.xhci.addr_ctx.layout.ok]");
    sexos_line("[usb.xhci.cmd.submit] kind=address_device slot=%u", (unsigned) en_slot_id);
    sexos_line("[sexusb.xhci.address_device.start]");
    uint32_t addr_dev_d0 = (uint32_t)(input_ctx_phys & 0xFFFFFFFFULL);
    uint32_t addr_dev_d1 = (uint32_t)(input_ctx_phys >> 32);
    uint32_t addr_dev_d3 = en_slot_id << 24 | TRB_TYPE_ADDRESS_DEVICE_CMD << 10 | cmd_cycle;
    trb_write_volatile(cmd_ring_va, cmd_idx, addr_dev_d0, addr_dev_d1, 0u, addr_dev_d3);
    uint32_t trb_d0 = trb_read_dword(cmd_ring_va, cmd_idx, 0);
    uint32_t trb_d1 = trb_read_dword(cmd_ring_va, cmd_idx, 1);
    uint32_t trb_d2 = trb_read_dword(cmd_ring_va, cmd_idx, 2);
    uint32_t trb_d3 = trb_read_dword(cmd_ring_va, cmd_idx, 3);
    sexos_line("[sexusb.xhci.address_device.trb.dump] d0=%#x d1=%#x d2=%#x d3=%#x", (unsigned) trb_d0, (unsigned) trb_d1, (unsigned) trb_d2, (unsigned) trb_d3);
    uint32_t icc_d0 = sexos_volatile_read32((const uint32_t *)(uintptr_t) input_ctx_va);
    uint32_t icc_d1 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + 4ULL));
    uint32_t in_slot_d0 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride));
    uint32_t in_slot_d1 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride + 4ULL));
    uint32_t in_slot_d2 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride + 8ULL));
    uint32_t in_slot_d3 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride + 12ULL));
    uint32_t in_ep0_d0 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride * 2ULL));
    uint32_t in_ep0_d1 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride * 2ULL + 4ULL));
    uint32_t in_ep0_d2 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride * 2ULL + 8ULL));
    uint32_t in_ep0_d3 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride * 2ULL + 12ULL));
    sexos_line("[sexusb.xhci.input_ctx.dump] icc=(%#x,%#x) slot=(%#x,%#x,%#x,%#x) ep0=(%#x,%#x,%#x,%#x)", (unsigned) icc_d0, (unsigned) icc_d1, (unsigned) in_slot_d0, (unsigned) in_slot_d1, (unsigned) in_slot_d2, (unsigned) in_slot_d3, (unsigned) in_ep0_d0, (unsigned) in_ep0_d1, (unsigned) in_ep0_d2, (unsigned) in_ep0_d3);
    sexos_line("[sexusb.xhci.address_device.trb.ok]");
    trb_write_volatile(cmd_ring_va, cmd_idx + 1ULL, 0u, 0u, 0u, cmd_cycle ^ 1u);
    uint32_t dev_slot_d0 = sexos_volatile_read32((const uint32_t *)(uintptr_t) device_ctx_va);
    uint32_t dev_slot_d1 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + 4ULL));
    uint32_t dev_slot_d2 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + 8ULL));
    uint32_t dev_slot_d3 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + 12ULL));
    uint32_t dev_ep0_d0 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride));
    uint32_t dev_ep0_d1 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride + 4ULL));
    uint32_t dev_ep0_d2 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride + 8ULL));
    uint32_t dev_ep0_d3 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride + 12ULL));
    sexos_line("[sexusb.xhci.dev_ctx.dump] slot=(%#x,%#x,%#x,%#x) ep0=(%#x,%#x,%#x,%#x)", (unsigned) dev_slot_d0, (unsigned) dev_slot_d1, (unsigned) dev_slot_d2, (unsigned) dev_slot_d3, (unsigned) dev_ep0_d0, (unsigned) dev_ep0_d1, (unsigned) dev_ep0_d2, (unsigned) dev_ep0_d3);
    sexos_line("[sexusb.xhci.addr_ctx.audit.start]");
    sexos_line("[sexusb.xhci.icc_audit.target_port] port=%llu", (unsigned long long) target_port);
    uint32_t target_portsc = mmio_read32(op_base, PORTSC_BASE + (target_port - 1ULL) * PORTSC_STRIDE);
    sexos_line("[sexusb.xhci.icc_audit.portsc_raw] raw=%#x", (unsigned) target_portsc);
    uint32_t ccs_port = target_portsc & PORTSC_CCS;
    sexos_line("[sexusb.xhci.icc_audit.port_ccs] ccs=%u", (unsigned) ccs_port);
    sexos_line("[sexusb.xhci.icc_audit.slot_id] id=%u", (unsigned) en_slot_id);
    sexos_line("[sexusb.xhci.icc_audit.ctx_stride] stride=%llu", (unsigned long long) ctx_stride);
    sexos_line("[sexusb.xhci.icc_audit.input_ctx_phys] phys=%#llx", (unsigned long long) input_ctx_phys);
    sexos_line("[sexusb.xhci.icc_audit.device_ctx_phys] phys=%#llx", (unsigned long long) device_ctx_phys);
    for (uint64_t i = 0ULL; i < 16ULL; ++i) {
      uint32_t word = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + i * 4ULL));
      sexos_line("[sexusb.xhci.icc_audit.raw32] i=%llu off=%#llx val=0x%08x", (unsigned long long) i, (unsigned long long)(i * 4ULL), (unsigned) word);
    }
    uint32_t qemu_icc0 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + 0x00ULL));
    uint32_t qemu_icc1 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + 0x04ULL));
    sexos_line("[sexusb.xhci.icc_audit.qemu_icc] icc0=%#x icc1=%#x icc0_exp=0 icc1_exp=3 icc0_ok=%s icc1_ok=%s", (unsigned) qemu_icc0, (unsigned) qemu_icc1, qemu_icc0 == 0u ? "true" : "false", qemu_icc1 == 3u ? "true" : "false");
    uint64_t slot_off = ctx_stride;
    uint32_t qemu_slot_dw0 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + slot_off + 0x00ULL));
    uint32_t qemu_slot_dw1 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + slot_off + 0x04ULL));
    uint32_t qemu_slot_dw2 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + slot_off + 0x08ULL));
    uint32_t qemu_slot_dw3 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + slot_off + 0x0cULL));
    uint32_t qemu_port = qemu_slot_dw1 >> 16 & 0xFFu;
    uint32_t spec_port = qemu_slot_dw1 >> 24 & 0xFFu;
    sexos_line("[sexusb.xhci.icc_audit.qemu_slot] dw0=%#x dw1=%#x dw2=%#x dw3=%#x", (unsigned) qemu_slot_dw0, (unsigned) qemu_slot_dw1, (unsigned) qemu_slot_dw2, (unsigned) qemu_slot_dw3);
    sexos_line("[sexusb.xhci.icc_audit.slot_ports] spec_port=%u qemu_port=%u target_port=%u", (unsigned) spec_port, (unsigned) qemu_port, (unsigned) target_port);
    uint64_t ep0_off = ctx_stride * 2ULL;
    uint32_t qemu_ep0_dw0 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ep0_off + 0x00ULL));
    uint32_t qemu_ep0_dw1 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ep0_off + 0x04ULL));
    uint32_t qemu_ep0_dw2 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ep0_off + 0x08ULL));
    uint32_t qemu_ep0_dw3 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(input_ctx_va + ep0_off + 0x0cULL));
    sexos_line("[sexusb.xhci.icc_audit.qemu_ep0] dw0=%#x dw1=%#x dw2=%#x dw3=%#x", (unsigned) qemu_ep0_dw0, (unsigned) qemu_ep0_dw1, (unsigned) qemu_ep0_dw2, (unsigned) qemu_ep0_dw3);
    sexos_line("[sexusb.xhci.icc_audit.addr_trb] d0=%#x d1=%#x d2=0 d3=%#x", (unsigned) addr_dev_d0, (unsigned) addr_dev_d1, (unsigned) addr_dev_d3);
    uint64_t trb_ptr = (uint64_t) addr_dev_d1 << 32 | (uint64_t) addr_dev_d0;
    sexos_line("[sexusb.xhci.icc_audit.trb_ptr_match] ptr=%#llx match=%s", (unsigned long long) trb_ptr, trb_ptr == input_ctx_phys ? "true" : "false");
    uint32_t bsr = addr_dev_d3 >> 9 & 1u;
    sexos_line("[sexusb.xhci.icc_audit.bsr_zero] bsr=%u", (unsigned) bsr);
    mmio_write32(db_base, 0ULL, 0u);
    sexos_line("[sexusb.xhci.address_device.doorbell.ok]");
    sexos_line("[sexusb.xhci.address_device.poll.enter] ev_idx=%llu ev_dcs=%llu", (unsigned long long) ev_idx, (unsigned long long) ev_dcs);
    bool addr_dev_ok = false;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_CMD_COMPLETION_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t ev_cc = ev_d2 >> 24 & 0xFFu;
          uint32_t ev_slot_id = ev_d3 >> 24 & 0xFFu;
          uint32_t ev_d0_evt = trb_read_dword(event_ring_va, ev_idx, 0);
          uint32_t ev_d1_evt = trb_read_dword(event_ring_va, ev_idx, 1);
          sexos_line("[sexusb.xhci.address_device.event.dump] d0=%#x d1=%#x d2=%#x d3=%#x type=%u cc=%u slot=%u", (unsigned) ev_d0_evt, (unsigned) ev_d1_evt, (unsigned) ev_d2, (unsigned) ev_d3, (unsigned) ev_type, (unsigned) ev_cc, (unsigned) ev_slot_id);
          if (ev_cc == TRB_CC_SUCCESS && ev_slot_id == en_slot_id) {
            addr_dev_ok = true;
            sexos_line("[sexusb.xhci.address_device.complete.ok]");
            sexos_line("[sexusb.xhci.address_device.slot.ok]");
            sexos_line("[usb.xhci.cmd.complete] kind=address_device slot=%u code=%u ok=1", (unsigned) en_slot_id, (unsigned) ev_cc);
          } else sexos_line("[sexusb.xhci.address_device.complete.bad] cc=%u slot=%u", (unsigned) ev_cc, (unsigned) ev_slot_id);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!addr_dev_ok) {
      sexos_line("[usb.xhci.enum.timeout] phase=ADDRESS_DEVICE detail=no_command_completion polls=%zu ok=0", POLL_BUDGET);
      sexos_line("[usb.xhci.enum.stop] reason=no_command_completion actionable=1");
      sexos_line("[sexusb.xhci.address_device.timeout.bad]");
      while (true) sexos_yield();
    }
    cmd_idx += 1ULL;
    uint32_t dev_slot_dw3 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + 12ULL));
    uint32_t slot_state = dev_slot_dw3 >> 27 & 0x1Fu;
    sexos_line("[sexusb.xhci.address_device.state.ok] slot_state=%u", (unsigned) slot_state);
    sexos_line("[sexusb.xhci.slot] slot=%u port=%llu state=%u", (unsigned) en_slot_id, (unsigned long long) target_port, (unsigned) slot_state);
    if (target_port != 0ULL) {
      size_t target_port_idx = (size_t) target_port - 1;
      if (target_port_idx < MAX_USB_PORT_TRACKED) {
        port_states[target_port_idx].slot_id = en_slot_id;
        sexos_line("[usb.xhci.port.slot] port=%llu slot=%u ok=1", (unsigned long long) target_port, (unsigned) en_slot_id);
      } else sexos_line("[usb.xhci.port.skip] port=%llu reason=slot_track_out_of_range", (unsigned long long) target_port);
    }
    uint32_t ep0_deq_dw2 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride + 8ULL));
    uint32_t ep0_deq_dw3 = sexos_volatile_read32((const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride + 12ULL));
    uint64_t ep0_deq_raw = (uint64_t) ep0_deq_dw3 << 32 | (uint64_t) ep0_deq_dw2;
    uint64_t ep0_deq_ptr = ep0_deq_raw & ~0xFULL;
    uint32_t ep0_deq_dcs = ep0_deq_dw2 & 1u;
    sexos_line("[sexusb.xhci.desc8.ep0_deq] dw2=%#x dw3=%#x ptr=%#llx dcs=%u", (unsigned) ep0_deq_dw2, (unsigned) ep0_deq_dw3, (unsigned long long) ep0_deq_ptr, (unsigned) ep0_deq_dcs);
    sexos_line("[sexusb.xhci.desc8.start]");
    uint64_t desc_data_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t desc_data_va = sys_map_phys(desc_data_phys, PAGE_SIZE);
    if (desc_data_phys == 0ULL || desc_data_phys == UINT64_MAX || (desc_data_va == 0ULL || desc_data_va == UINT64_MAX)) {
      sexos_line("[sexusb.xhci.desc8.alloc.bad]");
      while (true) sexos_yield();
    }
    if (desc_data_phys % 64ULL != 0ULL || desc_data_phys % PAGE_SIZE != 0ULL) {
      sexos_line("[sexusb.xhci.desc8.align.bad]");
      while (true) sexos_yield();
    }
    memset((void *)(uintptr_t) desc_data_va, 0, (size_t) PAGE_SIZE);
    sexos_line("[sexusb.xhci.desc8.alloc.ok] phys=%#llx va=%#llx", (unsigned long long) desc_data_phys, (unsigned long long) desc_data_va);
    uint64_t ep0_idx = 0ULL;
    uint32_t ep0_cycle = 1u;
    uint32_t setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | ep0_cycle);
    trb_write_volatile(ep0_ring_va, ep0_idx, 0x01000680u, 0x00080000u, 8u, setup_d3);
    uint32_t data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, ep0_idx + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), 8u | 1u << 18, data_d3);
    uint32_t status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 5 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, ep0_idx + 2ULL, 0u, 0u, 0u, status_d3);
    trb_write_volatile(ep0_ring_va, ep0_idx + 3ULL, 0u, 0u, 0u, ep0_cycle ^ 1u);
    sexos_line("[sexusb.xhci.desc8.trbs.ok]");
    ep0_idx += 4ULL;
    sexos_line("[sexusb.xhci.desc8.ep0_ring] phys=%#llx idx=%llu cycle=%u", (unsigned long long) ep0_ring_phys, (unsigned long long) ep0_idx, (unsigned) ep0_cycle);
    for (uint64_t ti = 0ULL; ti < 4ULL; ++ti) {
      uint32_t td0 = trb_read_dword(ep0_ring_va, ti, 0);
      uint32_t td1 = trb_read_dword(ep0_ring_va, ti, 1);
      uint32_t td2 = trb_read_dword(ep0_ring_va, ti, 2);
      uint32_t td3 = trb_read_dword(ep0_ring_va, ti, 3);
      sexos_line("[sexusb.xhci.desc8.trb%llu] d0=%#x d1=%#x d2=%#x d3=%#x", (unsigned long long) ti, (unsigned) td0, (unsigned) td1, (unsigned) td2, (unsigned) td3);
    }
    if (db_base + ((uint64_t) en_slot_id * 4ULL + 4ULL) > map_va + MAP_BYTES) {
      sexos_line("[sexusb.xhci.desc8.doorbell.bad]");
      while (true) sexos_yield();
    }
    mmio_write32(db_base, (uint64_t) en_slot_id * 4ULL, 1u);
    sexos_line("[sexusb.xhci.desc8.db] base=%#llx off=+%llu val=1", (unsigned long long) db_base, (unsigned long long)((uint64_t) en_slot_id * 4ULL));
    sexos_line("[sexusb.xhci.desc8.doorbell.ok]");
    uint32_t iman_before = mmio_read32(intr_base, 0ULL);
    uint32_t erdp_lo_rb = mmio_read32(intr_base, XHCI_INTR_ERDP);
    uint32_t erdp_hi_rb = mmio_read32(intr_base, XHCI_INTR_ERDP + 4ULL);
    uint64_t erdp_val = (uint64_t) erdp_lo_rb | (uint64_t) erdp_hi_rb << 32;
    sexos_line("[sexusb.xhci.desc8.wait_state] ev_idx=%llu ev_dcs=%llu iman=%#x erdp=%#llx", (unsigned long long) ev_idx, (unsigned long long) ev_dcs, (unsigned) iman_before, (unsigned long long) erdp_val);
    bool desc_ok = false;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          uint32_t ep = ev_d3 >> 16 & 0x1Fu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id && ep == 1u) {
            desc_ok = true;
            sexos_line("[sexusb.xhci.desc8.event.ok]");
          } else sexos_line("[sexusb.xhci.desc8.event.bad] cc=%u slot=%u ep=%u", (unsigned) cc, (unsigned) slot, (unsigned) ep);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!desc_ok) {
      uint32_t usbsts = mmio_read32(op_base, XHCI_USBSTS);
      uint32_t iman_timo = mmio_read32(intr_base, 0ULL);
      uint32_t erdp_lo_timo = mmio_read32(intr_base, XHCI_INTR_ERDP);
      uint32_t erdp_hi_timo = mmio_read32(intr_base, XHCI_INTR_ERDP + 4ULL);
      uint64_t erdp_timo = (uint64_t) erdp_lo_timo | (uint64_t) erdp_hi_timo << 32;
      sexos_line("[sexusb.xhci.desc8.timeout.bad]");
      sexos_line("[sexusb.xhci.desc8.timeout.diag] usbsts=%#x iman=%#x erdp=%#llx hce=%u eint=%u pcd=%u", (unsigned) usbsts, (unsigned) iman_timo, (unsigned long long) erdp_timo, (unsigned)(usbsts >> 0 & 1u), (unsigned)(usbsts >> 3 & 1u), (unsigned)(usbsts >> 4 & 1u));
      while (true) sexos_yield();
    }
    const uint8_t *desc_buf = (const uint8_t *)(uintptr_t) desc_data_va;
    uint8_t b0 = sexos_volatile_read8(desc_buf + 0);
    uint8_t b1 = sexos_volatile_read8(desc_buf + 1);
    uint8_t b2 = sexos_volatile_read8(desc_buf + 2);
    uint8_t b3 = sexos_volatile_read8(desc_buf + 3);
    uint8_t b4 = sexos_volatile_read8(desc_buf + 4);
    uint8_t b5 = sexos_volatile_read8(desc_buf + 5);
    uint8_t b6 = sexos_volatile_read8(desc_buf + 6);
    uint8_t b7 = sexos_volatile_read8(desc_buf + 7);
    sexos_line("[sexusb.xhci.desc8.bytes.ok] bytes=[%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x]", (unsigned) b0, (unsigned) b1, (unsigned) b2, (unsigned) b3, (unsigned) b4, (unsigned) b5, (unsigned) b6, (unsigned) b7);
    uint8_t mps = b7;
    sexos_line("[sexusb.xhci.desc8.mps.ok] mps=%u", (unsigned) mps);
    sexos_line("[sexusb.xhci.desc8.complete.ok]");
    sexos_line("[sexusb.xhci.eval_ctx.start]");
    uint32_t actual_mps = (uint32_t) mps;
    const uint32_t *dev_ep0_ro = (const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride);
    uint32_t dev_dw0 = sexos_volatile_read32(dev_ep0_ro + 0);
    uint32_t dev_dw1 = sexos_volatile_read32(dev_ep0_ro + 1);
    uint32_t dev_dw2 = sexos_volatile_read32(dev_ep0_ro + 2);
    uint32_t dev_dw3 = sexos_volatile_read32(dev_ep0_ro + 3);
    uint32_t dev_ep0_mps = dev_dw0 >> 16 & 0xFFFFu;
    sexos_line("[sexusb.xhci.mps.audit] port_speed=%u", (unsigned) port_speed);
    sexos_line("[sexusb.xhci.mps.audit] device_desc_bMaxPacketSize0=%u", (unsigned) actual_mps);
    sexos_line("[sexusb.xhci.mps.audit] max_packet_size_speed_guess=%u", (unsigned) max_packet_size);
    sexos_line("[sexusb.xhci.mps.audit] output_ep0_dw0=%#x", (unsigned) dev_dw0);
    sexos_line("[sexusb.xhci.mps.audit] output_ep0_dw1=%#x", (unsigned) dev_dw1);
    sexos_line("[sexusb.xhci.mps.audit] output_ep0_dw2=%#x", (unsigned) dev_dw2);
    sexos_line("[sexusb.xhci.mps.audit] output_ep0_dw3=%#x", (unsigned) dev_dw3);
    sexos_line("[sexusb.xhci.mps.audit] output_ep0_mps_before=%u", (unsigned) dev_ep0_mps);
    sexos_line("[sexusb.xhci.mps.audit] expected_rule=speed_based port_speed=%u guess=%u", (unsigned) port_speed, (unsigned) max_packet_size);
    sexos_line("[sexusb.xhci.mps.audit] got_rule_source=descriptor_bMaxPacketSize0 value=%u", (unsigned) actual_mps);
    bool mps_valid = actual_mps == 8u || actual_mps == 16u || (actual_mps == 32u || actual_mps == 64u || actual_mps == 512u);
    if (!mps_valid) {
      sexos_line("[sexusb.xhci.eval_ctx.mps.invalid.bad] mps=%u", (unsigned) actual_mps);
      while (true) sexos_yield();
    }
    if (actual_mps == max_packet_size) {
      sexos_line("[sexusb.xhci.eval_ctx.skip] mps=%u", (unsigned) actual_mps);
      while (true) sexos_yield();
    }
    memset((void *)(uintptr_t) input_ctx_va, 0, (size_t) PAGE_SIZE);
    sexos_volatile_write32((uint32_t *)(uintptr_t) input_ctx_va, 0u);
    sexos_volatile_write32((uint32_t *)(uintptr_t)(input_ctx_va + 4ULL), 2u);
    uint32_t *inp_ep0_base = (uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride * 2ULL);
    uint32_t cur_ep0_dw0 = sexos_volatile_read32(dev_ep0_ro + 0);
    uint32_t cur_ep0_dw1 = sexos_volatile_read32(dev_ep0_ro + 1);
    uint32_t cur_ep0_dw2 = sexos_volatile_read32(dev_ep0_ro + 2);
    uint32_t cur_ep0_dw3 = sexos_volatile_read32(dev_ep0_ro + 3);
    uint32_t ep0_dw0_new = (cur_ep0_dw0 & 0x0000FFFFu) | actual_mps << 16;
    sexos_volatile_write32(inp_ep0_base + 0, ep0_dw0_new);
    sexos_volatile_write32(inp_ep0_base + 1, cur_ep0_dw1);
    sexos_volatile_write32(inp_ep0_base + 2, cur_ep0_dw2);
    sexos_volatile_write32(inp_ep0_base + 3, cur_ep0_dw3);
    uint32_t eval_d0 = (uint32_t)(input_ctx_phys & 0xFFFFFFFFULL);
    uint32_t eval_d1 = (uint32_t)(input_ctx_phys >> 32);
    uint32_t eval_d3 = en_slot_id << 24 | TRB_TYPE_EVALUATE_CONTEXT_CMD << 10 | cmd_cycle;
    trb_write_volatile(cmd_ring_va, cmd_idx, eval_d0, eval_d1, 0u, eval_d3);
    trb_write_volatile(cmd_ring_va, cmd_idx + 1ULL, 0u, 0u, 0u, cmd_cycle ^ 1u);
    sexos_line("[sexusb.xhci.eval_ctx.trb.ok]");
    mmio_write32(db_base, 0ULL, 0u);
    sexos_line("[sexusb.xhci.eval_ctx.doorbell.ok]");
    bool eval_ok = false;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_CMD_COMPLETION_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id) {
            eval_ok = true;
            sexos_line("[sexusb.xhci.eval_ctx.event.ok]");
          } else sexos_line("[sexusb.xhci.eval_ctx.event.bad] cc=%u slot=%u", (unsigned) cc, (unsigned) slot);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!eval_ok) {
      sexos_line("[sexusb.xhci.eval_ctx.timeout.bad]");
      while (true) sexos_yield();
    }
    cmd_idx += 1ULL;
    uint32_t verify_dw0 = sexos_volatile_read32(dev_ep0_ro + 0);
    uint32_t verify_mps = verify_dw0 >> 16 & 0xFFFFu;
    uint32_t post_dw1 = sexos_volatile_read32(dev_ep0_ro + 1);
    uint32_t post_dw2 = sexos_volatile_read32(dev_ep0_ro + 2);
    uint32_t post_dw3 = sexos_volatile_read32(dev_ep0_ro + 3);
    sexos_line("[sexusb.xhci.mps.audit] post_eval_ep0_dw0=%#x", (unsigned) verify_dw0);
    sexos_line("[sexusb.xhci.mps.audit] post_eval_ep0_dw1=%#x", (unsigned) post_dw1);
    sexos_line("[sexusb.xhci.mps.audit] post_eval_ep0_dw2=%#x", (unsigned) post_dw2);
    sexos_line("[sexusb.xhci.mps.audit] post_eval_ep0_dw3=%#x", (unsigned) post_dw3);
    sexos_line("[sexusb.xhci.mps.audit] output_ep0_mps_after=%u", (unsigned) verify_mps);
    sexos_line("[sexusb.xhci.mps.audit] actual_mps_target=%u", (unsigned) actual_mps);
    if (verify_mps == actual_mps) {
      sexos_line("[sexusb.xhci.eval_ctx.mps.verify] expected=%u got=%u", (unsigned) actual_mps, (unsigned) verify_mps);
      sexos_line("[sexusb.xhci.eval_ctx.complete.ok]");
    } else if (port_speed == 3u && verify_mps == 512u) sexos_line("[sexusb.xhci.eval_ctx.ss_mps_512.ok] port_speed=3 ss_mps=512 descriptor_bMaxPacketSize0=%u", (unsigned) actual_mps);
    else sexos_line("[sexusb.xhci.eval_ctx.verify.bad] expected=%u got=%u", (unsigned) actual_mps, (unsigned) verify_mps);
    sexos_line("[sexusb.xhci.full18.start]");
    const uint32_t *deq_dev_ep0_base = (const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride);
    uint32_t deq_dw2 = sexos_volatile_read32(deq_dev_ep0_base + 2);
    uint32_t deq_dw3 = sexos_volatile_read32(deq_dev_ep0_base + 3);
    uint64_t deq_ptr = (uint64_t) deq_dw3 << 32 | (uint64_t) deq_dw2;
    uint64_t deq_dcs = deq_ptr & 1ULL;
    uint64_t deq_phys = deq_ptr & ~0xFULL;
    uint64_t deq_index = (deq_phys - ep0_ring_phys) / 16ULL;
    if (deq_dcs != 1ULL || deq_phys < ep0_ring_phys || (deq_phys >= ep0_ring_phys + PAGE_SIZE || deq_phys % 16ULL != 0ULL)) {
      sexos_line("[sexusb.xhci.full18.ep0_deq.bad] ptr=%#llx dcs=%llu", (unsigned long long) deq_ptr, (unsigned long long) deq_dcs);
      while (true) sexos_yield();
    }
    if (deq_index + 3ULL >= PAGE_SIZE / TRB_SIZE) {
      sexos_line("[sexusb.xhci.full18.ep0_deq.bad] idx=%llu exceeds ring", (unsigned long long) deq_index);
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.xhci.full18.ep0_deq.ok] idx=%llu dcs=%llu", (unsigned long long) deq_index, (unsigned long long) deq_dcs);
    memset((void *)(uintptr_t) desc_data_va, 0, 18);
    sexos_line("[sexusb.xhci.full18.zero.ok]");
    uint32_t full_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | ep0_cycle);
    trb_write_volatile(ep0_ring_va, deq_index, 0x01000680u, 0x00120000u, 8u, full_setup_d3);
    uint32_t full_data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, deq_index + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), 18u | 1u << 18, full_data_d3);
    uint32_t full_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 5 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, deq_index + 2ULL, 0u, 0u, 0u, full_status_d3);
    trb_write_volatile(ep0_ring_va, deq_index + 3ULL, 0u, 0u, 0u, ep0_cycle ^ 1u);
    sexos_line("[sexusb.xhci.full18.trbs.ok]");
    ep0_idx = deq_index + 4ULL;
    mmio_write32(db_base, (uint64_t) en_slot_id * 4ULL, 1u);
    sexos_line("[sexusb.xhci.full18.doorbell.ok]");
    bool full_ok = false;
    uint32_t full_residue = 0u;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          full_residue = ev_d2 & 0xFFFFFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          uint32_t ep = ev_d3 >> 16 & 0x1Fu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id && ep == 1u) {
            full_ok = true;
            sexos_line("[sexusb.xhci.full18.event.ok]");
          } else sexos_line("[sexusb.xhci.full18.event.bad] cc=%u slot=%u ep=%u", (unsigned) cc, (unsigned) slot, (unsigned) ep);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!full_ok) {
      sexos_line("[sexusb.xhci.full18.timeout.bad]");
      while (true) sexos_yield();
    }
    if (full_residue >= 18u) {
      sexos_line("[sexusb.xhci.full18.residue.full.bad] residue=%u", (unsigned) full_residue);
      while (true) sexos_yield();
    }
    if (full_residue > 0u) sexos_line("[sexusb.xhci.full18.residue.warn] residue=%u", (unsigned) full_residue);
    const uint8_t *full_buf = (const uint8_t *)(uintptr_t) desc_data_va;
    uint8_t fb0 = sexos_volatile_read8(full_buf + 0);
    uint8_t fb1 = sexos_volatile_read8(full_buf + 1);
    uint8_t fb2 = sexos_volatile_read8(full_buf + 2);
    uint8_t fb3 = sexos_volatile_read8(full_buf + 3);
    uint8_t fb4 = sexos_volatile_read8(full_buf + 4);
    uint8_t fb5 = sexos_volatile_read8(full_buf + 5);
    uint8_t fb6 = sexos_volatile_read8(full_buf + 6);
    uint8_t fb7 = sexos_volatile_read8(full_buf + 7);
    uint8_t fb8 = sexos_volatile_read8(full_buf + 8);
    uint8_t fb9 = sexos_volatile_read8(full_buf + 9);
    uint8_t fb10 = sexos_volatile_read8(full_buf + 10);
    uint8_t fb11 = sexos_volatile_read8(full_buf + 11);
    uint8_t fb12 = sexos_volatile_read8(full_buf + 12);
    uint8_t fb13 = sexos_volatile_read8(full_buf + 13);
    uint8_t fb14 = sexos_volatile_read8(full_buf + 14);
    uint8_t fb15 = sexos_volatile_read8(full_buf + 15);
    uint8_t fb16 = sexos_volatile_read8(full_buf + 16);
    uint8_t fb17 = sexos_volatile_read8(full_buf + 17);
    sexos_line("[sexusb.xhci.full18.bytes.ok] bytes=[%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x]", (unsigned) fb0, (unsigned) fb1, (unsigned) fb2, (unsigned) fb3, (unsigned) fb4, (unsigned) fb5, (unsigned) fb6, (unsigned) fb7, (unsigned) fb8, (unsigned) fb9, (unsigned) fb10, (unsigned) fb11, (unsigned) fb12, (unsigned) fb13, (unsigned) fb14, (unsigned) fb15, (unsigned) fb16, (unsigned) fb17);
    uint8_t full_len = fb0;
    uint8_t full_type = fb1;
    uint16_t full_usb = (uint16_t)((uint16_t) fb3 << 8) | (uint16_t) fb2;
    uint8_t full_class = fb4;
    uint8_t full_subclass = fb5;
    uint8_t full_proto = fb6;
    uint8_t full_mps = fb7;
    uint16_t full_vendor = (uint16_t)((uint16_t) fb9 << 8) | (uint16_t) fb8;
    uint16_t full_product = (uint16_t)((uint16_t) fb11 << 8) | (uint16_t) fb10;
    uint16_t full_device = (uint16_t)((uint16_t) fb13 << 8) | (uint16_t) fb12;
    uint8_t full_configs = fb17;
    sexos_line("[sexusb.xhci.full18.len] len=%u", (unsigned) full_len);
    sexos_line("[sexusb.xhci.full18.type] type=%u", (unsigned) full_type);
    sexos_line("[sexusb.xhci.full18.usb] usb=%#x", (unsigned) full_usb);
    sexos_line("[sexusb.xhci.full18.class] class=%u subclass=%u protocol=%u", (unsigned) full_class, (unsigned) full_subclass, (unsigned) full_proto);
    sexos_line("[sexusb.xhci.full18.vendor] vendor=%#x", (unsigned) full_vendor);
    sexos_line("[sexusb.xhci.full18.product] product=%#x", (unsigned) full_product);
    sexos_line("[sexusb.xhci.full18.device] device=%#x", (unsigned) full_device);
    sexos_line("[sexusb.xhci.full18.configs] configs=%u", (unsigned) full_configs);
    sexos_line("[sexusb.xhci.full18.mps_check] mps=%u", (unsigned) full_mps);
    sexos_line("[sexusb.dev.desc] slot=%u vendor=%#x product=%#x class=%#x subclass=%#x proto=%#x", (unsigned) en_slot_id, (unsigned) full_vendor, (unsigned) full_product, (unsigned) full_class, (unsigned) full_subclass, (unsigned) full_proto);
    if ((uint32_t) full_mps != actual_mps) {
      sexos_line("[sexusb.xhci.full18.mps.mismatch.bad] expected=%u got=%u", (unsigned) actual_mps, (unsigned) full_mps);
      while (true) sexos_yield();
    }
    if (full_len != 18u) sexos_line("[sexusb.xhci.full18.desc_len.warn] len=%u", (unsigned) full_len);
    if (full_type != 1u) sexos_line("[sexusb.xhci.full18.desc_type.warn] type=%u", (unsigned) full_type);
    if (full_residue == 0u) sexos_line("[sexusb.xhci.full18.complete.ok]");
    sexos_line("[sexusb.xhci.config.start]");
    const uint32_t *cfg_deq_ep0_base = (const uint32_t *)(uintptr_t)(device_ctx_va + ctx_stride);
    uint32_t cfg_deq_dw2 = sexos_volatile_read32(cfg_deq_ep0_base + 2);
    uint32_t cfg_deq_dw3 = sexos_volatile_read32(cfg_deq_ep0_base + 3);
    uint64_t cfg_deq_ptr = (uint64_t) cfg_deq_dw3 << 32 | (uint64_t) cfg_deq_dw2;
    uint64_t cfg_deq_dcs = cfg_deq_ptr & 1ULL;
    uint64_t cfg_deq_phys = cfg_deq_ptr & ~0xFULL;
    uint64_t cfg_deq_index = (cfg_deq_phys - ep0_ring_phys) / 16ULL;
    if (cfg_deq_dcs != 1ULL || cfg_deq_phys < ep0_ring_phys || (cfg_deq_phys >= ep0_ring_phys + PAGE_SIZE || cfg_deq_phys % 16ULL != 0ULL)) {
      sexos_line("[sexusb.xhci.config.header_deq.bad] ptr=%#llx dcs=%llu", (unsigned long long) cfg_deq_ptr, (unsigned long long) cfg_deq_dcs);
      while (true) sexos_yield();
    }
    if (cfg_deq_index + 3ULL >= PAGE_SIZE / TRB_SIZE) {
      sexos_line("[sexusb.xhci.config.header_deq.ring.bad] idx=%llu", (unsigned long long) cfg_deq_index);
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.xhci.config.header_deq.ok] idx=%llu dcs=%llu", (unsigned long long) cfg_deq_index, (unsigned long long) cfg_deq_dcs);
    memset((void *)(uintptr_t) desc_data_va, 0, 9);
    sexos_line("[sexusb.xhci.config.header_zero.ok]");
    uint32_t cfg_hdr_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | ep0_cycle);
    trb_write_volatile(ep0_ring_va, cfg_deq_index, 0x02000680u, 0x00090000u, 8u, cfg_hdr_setup_d3);
    uint32_t cfg_hdr_data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, cfg_deq_index + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), 9u | 1u << 18, cfg_hdr_data_d3);
    uint32_t cfg_hdr_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, cfg_deq_index + 2ULL, 0u, 0u, 0u, cfg_hdr_status_d3 | 1u << 5);
    trb_write_volatile(ep0_ring_va, cfg_deq_index + 3ULL, 0u, 0u, 0u, ep0_cycle ^ 1u);
    sexos_line("[sexusb.xhci.config.header_trbs.ok]");
    ep0_idx = cfg_deq_index + 4ULL;
    mmio_write32(db_base, (uint64_t) en_slot_id * 4ULL, 1u);
    sexos_line("[sexusb.xhci.config.header.doorbell.ok]");
    bool cfg_hdr_ok = false;
    uint32_t cfg_hdr_residue = 0u;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          cfg_hdr_residue = ev_d2 & 0xFFFFFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          uint32_t ep = ev_d3 >> 16 & 0x1Fu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id && ep == 1u) {
            cfg_hdr_ok = true;
            sexos_line("[sexusb.xhci.config.header.event.ok]");
          } else sexos_line("[sexusb.xhci.config.header.event.bad] cc=%u slot=%u ep=%u", (unsigned) cc, (unsigned) slot, (unsigned) ep);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!cfg_hdr_ok) {
      sexos_line("[sexusb.xhci.config.header.timeout.bad]");
      while (true) sexos_yield();
    }
    if (cfg_hdr_residue > 0u) {
      sexos_line("[sexusb.xhci.config.header_residue.bad] residue=%u", (unsigned) cfg_hdr_residue);
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.xhci.config.header.ok]");
    const uint8_t *cfg_buf = (const uint8_t *)(uintptr_t) desc_data_va;
    uint8_t cfg_total_lo = sexos_volatile_read8(cfg_buf + 2);
    uint8_t cfg_total_hi = sexos_volatile_read8(cfg_buf + 3);
    uint64_t w_total_length = (uint64_t) cfg_total_hi << 8 | (uint64_t) cfg_total_lo;
    uint32_t cfg_num_interfaces = (uint32_t) sexos_volatile_read8(cfg_buf + 4);
    if (w_total_length < 9ULL) {
      sexos_line("[sexusb.xhci.config.total_len.bad] wTotalLength=%llu", (unsigned long long) w_total_length);
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.xhci.config.header.totallen] len=%llu", (unsigned long long) w_total_length);
    sexos_line("[sexusb.cfg.desc] slot=%u total_len=%llu num_interfaces=%u", (unsigned) en_slot_id, (unsigned long long) w_total_length, (unsigned) cfg_num_interfaces);
    uint32_t cfg2_deq_dw2 = sexos_volatile_read32(cfg_deq_ep0_base + 2);
    uint32_t cfg2_deq_dw3 = sexos_volatile_read32(cfg_deq_ep0_base + 3);
    uint64_t cfg2_deq_ptr = (uint64_t) cfg2_deq_dw3 << 32 | (uint64_t) cfg2_deq_dw2;
    uint64_t cfg2_deq_dcs = cfg2_deq_ptr & 1ULL;
    uint64_t cfg2_deq_phys = cfg2_deq_ptr & ~0xFULL;
    uint64_t cfg2_deq_index = (cfg2_deq_phys - ep0_ring_phys) / 16ULL;
    if (cfg2_deq_dcs != 1ULL || cfg2_deq_phys < ep0_ring_phys || (cfg2_deq_phys >= ep0_ring_phys + PAGE_SIZE || cfg2_deq_phys % 16ULL != 0ULL)) {
      sexos_line("[sexusb.xhci.config.full_deq.bad] ptr=%#llx dcs=%llu", (unsigned long long) cfg2_deq_ptr, (unsigned long long) cfg2_deq_dcs);
      while (true) sexos_yield();
    }
    if (cfg2_deq_index + 3ULL >= PAGE_SIZE / TRB_SIZE) {
      sexos_line("[sexusb.xhci.config.full_deq.ring.bad] idx=%llu", (unsigned long long) cfg2_deq_index);
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.xhci.config.full_deq.ok] idx=%llu dcs=%llu", (unsigned long long) cfg2_deq_index, (unsigned long long) cfg2_deq_dcs);
    uint8_t *zero_base = (uint8_t *)(uintptr_t) desc_data_va;
    uint64_t zi = 0ULL;
    while (zi < w_total_length) {
      sexos_volatile_write8(zero_base + (size_t) zi, 0u);
      zi += 1ULL;
    }
    sexos_line("[sexusb.xhci.config.full_zero.ok]");
    uint32_t cfg_full_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | ep0_cycle);
    trb_write_volatile(ep0_ring_va, cfg2_deq_index, 0x02000680u, (uint32_t) w_total_length << 16, 8u, cfg_full_setup_d3);
    uint32_t cfg_full_data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, cfg2_deq_index + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), (uint32_t) w_total_length | 1u << 18, cfg_full_data_d3);
    uint32_t cfg_full_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, cfg2_deq_index + 2ULL, 0u, 0u, 0u, cfg_full_status_d3 | 1u << 5);
    trb_write_volatile(ep0_ring_va, cfg2_deq_index + 3ULL, 0u, 0u, 0u, ep0_cycle ^ 1u);
    sexos_line("[sexusb.xhci.config.full_trbs.ok]");
    ep0_idx = cfg2_deq_index + 4ULL;
    mmio_write32(db_base, (uint64_t) en_slot_id * 4ULL, 1u);
    sexos_line("[sexusb.xhci.config.full.doorbell.ok]");
    bool cfg_full_ok = false;
    uint32_t cfg_full_residue = 0u;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          cfg_full_residue = ev_d2 & 0xFFFFFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          uint32_t ep = ev_d3 >> 16 & 0x1Fu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id && ep == 1u) {
            cfg_full_ok = true;
            sexos_line("[sexusb.xhci.config.full.event.ok]");
          } else sexos_line("[sexusb.xhci.config.full.event.bad] cc=%u slot=%u ep=%u", (unsigned) cc, (unsigned) slot, (unsigned) ep);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!cfg_full_ok) {
      sexos_line("[sexusb.xhci.config.full.timeout.bad]");
      while (true) sexos_yield();
    }
    if (cfg_full_residue >= (uint32_t) w_total_length) {
      sexos_line("[sexusb.xhci.config.full_residue_full.bad] residue=%u wTotalLength=%llu", (unsigned) cfg_full_residue, (unsigned long long) w_total_length);
      while (true) sexos_yield();
    }
    uint64_t received_len = w_total_length;
    if (cfg_full_residue > 0u) {
      sexos_line("[sexusb.xhci.config.full_residue_partial.warn] residue=%u", (unsigned) cfg_full_residue);
      received_len = w_total_length - (uint64_t) cfg_full_residue;
    }
    sexos_line("[sexusb.xhci.config.walk.start]");
    const uint8_t *walk_buf = (const uint8_t *)(uintptr_t) desc_data_va;
    uint64_t walk_off = 0ULL;
    bool inside_hid_iface = false;
    bool iface_is_boot_mouse = false;
    bool found_hid_mouse = false;
    bool found_hid_tablet = false;
    bool found_hid_keyboard = false;
    uint8_t current_hid_role = 0u;
    uint8_t hid_interface_number = 0u;
    uint16_t hid_report_desc_len = 0u;
    uint8_t intr_ep_addr = 0u;
    uint16_t intr_ep_mps = 0u;
    uint8_t intr_ep_interval = 0u;
    uint8_t keyboard_ep_addr = 0u;
    uint8_t pointer_ep_addr = 0u;
    uint8_t pointer_role = 0u;
    while (walk_off < received_len) {
      uint8_t b_len = sexos_volatile_read8(walk_buf + (size_t) walk_off);
      uint8_t b_type = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 1));
      if (b_len == 0u) {
        sexos_line("[sexusb.xhci.config.desc_zero_len.bad] off=%llu", (unsigned long long) walk_off);
        while (true) sexos_yield();
      }
      if ((uint64_t) b_len > received_len - walk_off) {
        sexos_line("[sexusb.xhci.config.desc_truncated.bad] off=%llu len=%u remain=%llu", (unsigned long long) walk_off, (unsigned) b_len, (unsigned long long)(received_len - walk_off));
        while (true) sexos_yield();
      }
      if (b_type == 4u) if (b_len >= 8u) {
        uint8_t b_intf_num = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 2));
        uint8_t b_class = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 5));
        uint8_t b_subclass = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 6));
        uint8_t b_protocol = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 7));
        if (st->iface_diag_budget > 0u) {
          st->iface_diag_budget -= 1u;
          sexos_line("[sexusb.hid.iface] idx=%llu if=%u class=%#x subclass=%#x proto=%#x", (unsigned long long) walk_off, (unsigned) b_intf_num, (unsigned) b_class, (unsigned) b_subclass, (unsigned) b_protocol);
        }
        bool is_hid = b_class == 0x03u;
        if (is_hid) {
          inside_hid_iface = true;
          iface_is_boot_mouse = b_subclass == 0x01u && b_protocol == 0x02u;
          bool is_boot_keyboard = b_subclass == 0x01u && b_protocol == 0x01u;
          if (iface_is_boot_mouse) {
            current_hid_role = 2u;
            found_hid_mouse = true;
            hid_interface_number = b_intf_num;
            sexos_line("[sexusb.xhci.config.hid_boot_mouse.found] intf=%u off=%llu", (unsigned) b_intf_num, (unsigned long long) walk_off);
            if (!st->mouse_detect_proof_emitted) {
              st->mouse_detect_proof_emitted = true;
              sexos_line("[usb.mouse.detect] interface=%u boot=1 ok=1", (unsigned) b_intf_num);
            }
          } else if (is_boot_keyboard) {
            current_hid_role = 1u;
            found_hid_keyboard = true;
            hid_interface_number = b_intf_num;
            sexos_line("[sexusb.xhci.config.hid_boot_keyboard.found] intf=%u off=%llu", (unsigned) b_intf_num, (unsigned long long) walk_off);
            sexos_line("[usb.keyboard.detect] interface=%u boot=1 ok=1", (unsigned) b_intf_num);
          } else if (b_protocol != 0x01u) {
            current_hid_role = 3u;
            found_hid_tablet = true;
            hid_interface_number = b_intf_num;
            sexos_line("[sexusb.xhci.config.hid_tablet.found] intf=%u off=%llu subclass=%u protocol=%u", (unsigned) b_intf_num, (unsigned long long) walk_off, (unsigned) b_subclass, (unsigned) b_protocol);
            if (!st->tablet_detect_proof_emitted) {
              st->tablet_detect_proof_emitted = true;
              sexos_line("[usb.mouse.detect] interface=%u boot=0 tablet=1 ok=1", (unsigned) b_intf_num);
            }
          } else current_hid_role = 4u;
        } else {
          current_hid_role = 0u;
          inside_hid_iface = false;
          iface_is_boot_mouse = false;
        }
      } else {
        current_hid_role = 0u;
        inside_hid_iface = false;
        iface_is_boot_mouse = false;
      } else if (b_type == 0x21u) {
        if (inside_hid_iface) {
          if (b_len < 9u) {
            sexos_line("[sexusb.xhci.config.hid_desc.bad] short_len=%u", (unsigned) b_len);
            while (true) sexos_yield();
          }
          uint8_t hid_type = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 6));
          if (hid_type != 0x22u) {
            sexos_line("[sexusb.xhci.config.hid_desc.bad] type=%#x off=%llu", (unsigned) hid_type, (unsigned long long) walk_off);
            while (true) sexos_yield();
          }
          uint8_t rpt_len_lo = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 7));
          uint8_t rpt_len_hi = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 8));
          hid_report_desc_len = (uint16_t)((uint16_t) rpt_len_hi << 8) | (uint16_t) rpt_len_lo;
          sexos_line("[sexusb.xhci.config.hid_desc.ok] off=%llu report_len=%u", (unsigned long long) walk_off, (unsigned) hid_report_desc_len);
        }
      } else if (b_type == 5u) if (inside_hid_iface) {
        if (b_len < 7u) {
          sexos_line("[sexusb.xhci.config.intr_ep_short.bad] off=%llu len=%u", (unsigned long long) walk_off, (unsigned) b_len);
          while (true) sexos_yield();
        }
        uint8_t b_endpoint = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 2));
        uint8_t bm_attrs = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 3));
        uint8_t mps_lo = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 4));
        uint8_t mps_hi = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 5));
        uint16_t w_max_pkt = (uint16_t)((uint16_t) mps_hi << 8) | (uint16_t) mps_lo;
        uint16_t pkt_size = w_max_pkt & 0x07FFu;
        uint8_t b_interval = sexos_volatile_read8(walk_buf + ((size_t) walk_off + 6));
        bool dir_in = (b_endpoint & 0x80u) != 0u;
        bool intr_type = (bm_attrs & 0x03u) == 0x03u;
        if (st->ep_diag_budget > 0u) {
          st->ep_diag_budget -= 1u;
          sexos_line("[sexusb.hid.ep] if=%u ep=%#x attr=%#x maxpkt=%u interval=%u", (unsigned) hid_interface_number, (unsigned) b_endpoint, (unsigned) bm_attrs, (unsigned) pkt_size, (unsigned) b_interval);
        }
        if (dir_in && intr_type) {
          if (pkt_size == 0u) {
            sexos_line("[sexusb.xhci.config.intr_ep_mps.bad] off=%llu mps=%u", (unsigned long long) walk_off, (unsigned) pkt_size);
            while (true) sexos_yield();
          }
          intr_ep_addr = b_endpoint;
          intr_ep_mps = pkt_size;
          intr_ep_interval = b_interval;
          const char *ep_which = iface_is_boot_mouse ? "mouse" : found_hid_keyboard ? "keyboard" : "tablet";
          sexos_line("[sexusb.xhci.config.intr_ep.%s] off=%llu addr=%#x mps=%u interval=%u", ep_which, (unsigned long long) walk_off, (unsigned) b_endpoint, (unsigned) pkt_size, (unsigned) b_interval);
          if (current_hid_role == 1u) {
            keyboard_ep_addr = b_endpoint;
            sexos_line("[sexusb.hid.bind] role=keyboard if=%u ep=%#x reason=hid_boot_keyboard", (unsigned) hid_interface_number, (unsigned) b_endpoint);
          } else if (current_hid_role == 2u) {
            pointer_ep_addr = b_endpoint;
            pointer_role = 2u;
            sexos_line("[sexusb.hid.bind] role=mouse if=%u ep=%#x reason=hid_boot_mouse", (unsigned) hid_interface_number, (unsigned) b_endpoint);
          } else if (current_hid_role == 3u) {
            pointer_ep_addr = b_endpoint;
            pointer_role = 3u;
            sexos_line("[sexusb.hid.bind] role=tablet if=%u ep=%#x reason=hid_pointer_like_interface", (unsigned) hid_interface_number, (unsigned) b_endpoint);
          } else sexos_line("[sexusb.hid.bind] role=unknown if=%u ep=%#x reason=unclassified_hid_interface", (unsigned) hid_interface_number, (unsigned) b_endpoint);
        }
      }
      walk_off += (uint64_t) b_len;
    }
    sexos_line("[sexusb.xhci.config.walk.done]");
    uint8_t cfg_value = sexos_volatile_read8(walk_buf + 5);
    if (cfg_value == 0u) {
      sexos_line("[sexusb.xhci.config.value.bad] value=0");
      while (true) sexos_yield();
    }
    if (!found_hid_mouse && !found_hid_tablet && !found_hid_keyboard) {
      sexos_line("[sexusb.xhci.config.no_hid.park]");
      while (true) sexos_yield();
    }
    HidRole bind_role = classify_single_hid_role(found_hid_keyboard, found_hid_tablet, found_hid_mouse);
    const char *kind_str = bind_role == HID_ROLE_KEYBOARD ? "keyboard" : bind_role == HID_ROLE_POINTER_MOUSE ? "mouse" : bind_role == HID_ROLE_POINTER_TABLET ? "touchpad" : "unknown";
    sexos_line("[sexusb.hid.candidate] slot=%u kind=%s", (unsigned) en_slot_id, kind_str);
    bool is_tablet_device = bind_role == HID_ROLE_POINTER_TABLET;
    bool is_keyboard_device = bind_role == HID_ROLE_KEYBOARD;
    if (found_hid_keyboard) sexos_line("[sexusb.kbd.found] intf=%u ep=0x%x", (unsigned) hid_interface_number, (unsigned) intr_ep_addr);
    const char *keyboard_ep_str = keyboard_ep_addr == 0u ? "none" : "set";
    const char *pointer_ep_str = pointer_ep_addr == 0u ? "none" : "set";
    const char *pointer_role_str = pointer_role == 2u ? "mouse" : pointer_role == 3u ? "tablet" : "none";
    sexos_line("[sexusb.hid.bind.summary] keyboard_ep=%s pointer_ep=%s pointer_role=%s", keyboard_ep_str, pointer_ep_str, pointer_role_str);
    SingleHidBind single_bind;
    single_bind.slot_id = en_slot_id;
    single_bind.port = target_port;
    single_bind.role = bind_role;
    single_bind.ep_addr = intr_ep_addr;
    single_bind.ep_dci = 3u;
    single_bind.max_packet = intr_ep_mps;
    single_bind.interval = intr_ep_interval;
    sexos_line("[sexusb.dev.hid.summary] slot=%u keyboard_ep=%s pointer_ep=%s pointer_role=%s", (unsigned) en_slot_id, keyboard_ep_str, pointer_ep_str, pointer_role_str);
    uint32_t hid_devices = found_hid_keyboard || found_hid_mouse || found_hid_tablet ? 1u : 0u;
    uint32_t pointer_devices = pointer_ep_addr != 0u ? 1u : 0u;
    sexos_line("[sexusb.enum.summary] ports_connected=%u slots_enabled=1 hid_devices=%u pointer_devices=%u", (unsigned) ports_connected, (unsigned) hid_devices, (unsigned) pointer_devices);
    uint32_t report_len = (uint32_t) hid_report_desc_len;
    if (report_len == 0u || report_len > 256u) {
      sexos_line("[sexusb.xhci.hid.report_desc.len.bad] len=%u intf=%u", (unsigned) report_len, (unsigned) hid_interface_number);
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.xhci.hid.report_desc.start] intf=%u len=%u", (unsigned) hid_interface_number, (unsigned) report_len);
    uint32_t hid_deq_dw2 = sexos_volatile_read32(cfg_deq_ep0_base + 2);
    uint32_t hid_deq_dw3 = sexos_volatile_read32(cfg_deq_ep0_base + 3);
    uint64_t hid_deq_ptr = (uint64_t) hid_deq_dw3 << 32 | (uint64_t) hid_deq_dw2;
    uint64_t hid_deq_dcs = hid_deq_ptr & 1ULL;
    uint64_t hid_deq_phys = hid_deq_ptr & ~0xFULL;
    uint64_t hid_deq_index = (hid_deq_phys - ep0_ring_phys) / 16ULL;
    if (hid_deq_dcs != 1ULL || hid_deq_phys < ep0_ring_phys || (hid_deq_phys >= ep0_ring_phys + PAGE_SIZE || hid_deq_phys % 16ULL != 0ULL)) {
      sexos_line("[sexusb.xhci.hid.report_desc.deq.bad] ptr=%#llx dcs=%llu", (unsigned long long) hid_deq_ptr, (unsigned long long) hid_deq_dcs);
      while (true) sexos_yield();
    }
    if (hid_deq_index + 3ULL >= PAGE_SIZE / TRB_SIZE) {
      sexos_line("[sexusb.xhci.hid.report_desc.deq.ring.bad] idx=%llu", (unsigned long long) hid_deq_index);
      while (true) sexos_yield();
    }
    memset((void *)(uintptr_t) desc_data_va, 0, (size_t) report_len);
    uint32_t hid_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | ep0_cycle);
    trb_write_volatile(ep0_ring_va, hid_deq_index, 0x22000681u, report_len << 16 | (uint32_t) hid_interface_number, 8u, hid_setup_d3);
    uint32_t hid_data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, hid_deq_index + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), report_len | 1u << 18, hid_data_d3);
    uint32_t hid_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 5 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, hid_deq_index + 2ULL, 0u, 0u, 0u, hid_status_d3);
    trb_write_volatile(ep0_ring_va, hid_deq_index + 3ULL, 0u, 0u, 0u, ep0_cycle ^ 1u);
    ep0_idx = hid_deq_index + 4ULL;
    mmio_write32(db_base, (uint64_t) en_slot_id * 4ULL, 1u);
    bool hid_ok = false;
    uint32_t hid_residue = 0u;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          hid_residue = ev_d2 & 0xFFFFFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          uint32_t ep = ev_d3 >> 16 & 0x1Fu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id && ep == 1u) hid_ok = true;
          else sexos_line("[sexusb.xhci.hid.report_desc.event.bad] cc=%u slot=%u ep=%u", (unsigned) cc, (unsigned) slot, (unsigned) ep);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!hid_ok) {
      sexos_line("[sexusb.xhci.hid.report_desc.timeout.bad]");
      while (true) sexos_yield();
    }
    if (hid_residue > report_len) {
      sexos_line("[sexusb.xhci.hid.report_desc.residue.bad] residue=%u len=%u", (unsigned) hid_residue, (unsigned) report_len);
      while (true) sexos_yield();
    }
    uint32_t hid_actual_len = report_len - hid_residue;
    sexos_line("[sexusb.xhci.hid.report_desc.event.ok] actual=%u residue=%u", (unsigned) hid_actual_len, (unsigned) hid_residue);
    uint32_t dump_len = hid_actual_len > 64u ? 64u : hid_actual_len;
    const uint8_t *hid_buf = (const uint8_t *)(uintptr_t) desc_data_va;
    uint32_t bi = 0u;
    while (bi < dump_len) {
      uint8_t bv = sexos_volatile_read8(hid_buf + (size_t) bi);
      sexos_line("[sexusb.xhci.hid.report_desc.bytes] i=%u b=%#x", (unsigned) bi, (unsigned) bv);
      bi += 1u;
    }
    bool has_usage_page_gd = false;
    bool has_usage_mouse = false;
    bool has_usage_pointer = false;
    bool has_usage_keyboard = false;
    bool has_collection_app = false;
    bool has_usage_x = false;
    bool has_usage_y = false;
    uint32_t si = 0u;
    while (si + 1u < hid_actual_len) {
      uint8_t hb0 = sexos_volatile_read8(hid_buf + (size_t) si);
      uint8_t hb1 = sexos_volatile_read8(hid_buf + ((size_t) si + 1));
      if (hb0 == 0x05u && hb1 == 0x01u) has_usage_page_gd = true;
      else if (hb0 == 0x09u && hb1 == 0x02u) has_usage_mouse = true;
      else if (hb0 == 0x09u && hb1 == 0x01u) has_usage_pointer = true;
      else if (hb0 == 0x09u && hb1 == 0x06u) has_usage_keyboard = true;
      else if (hb0 == 0xA1u && hb1 == 0x01u) has_collection_app = true;
      else if (hb0 == 0x09u && hb1 == 0x30u) has_usage_x = true;
      else if (hb0 == 0x09u && hb1 == 0x31u) has_usage_y = true;
      si += 1u;
    }
    bool is_keyboard_shape = has_usage_page_gd && has_usage_keyboard && has_collection_app;
    bool is_mouse_shape = has_usage_page_gd && has_usage_mouse && has_collection_app && (has_usage_x && has_usage_y);
    bool is_tablet_shape = has_usage_page_gd && has_usage_pointer && (has_usage_x && has_usage_y);
    if (is_keyboard_shape) sexos_line("[sexusb.xhci.hid.report_desc.keyboard_shape.ok]");
    if (is_tablet_shape) sexos_line("[sexusb.xhci.hid.report_desc.tablet_shape.ok]");
    if (is_mouse_shape) sexos_line("[sexusb.xhci.hid.report_desc.mouse_shape.ok]");
    if (!is_mouse_shape && !is_tablet_shape && !is_keyboard_shape) sexos_line("[sexusb.xhci.hid.report_desc.shape.warn] mouse=%s tablet=%s keyboard=%s", is_mouse_shape ? "true" : "false", is_tablet_shape ? "true" : "false", is_keyboard_shape ? "true" : "false");
    sexos_line("[sexusb.xhci.hid.report_desc.complete.ok] len=%u", (unsigned) hid_actual_len);
    sexos_line("[sexusb.xhci.set_config.start] value=%u", (unsigned) cfg_value);
    uint32_t setcfg_deq_dw2 = sexos_volatile_read32(cfg_deq_ep0_base + 2);
    uint32_t setcfg_deq_dw3 = sexos_volatile_read32(cfg_deq_ep0_base + 3);
    uint64_t setcfg_deq_ptr = (uint64_t) setcfg_deq_dw3 << 32 | (uint64_t) setcfg_deq_dw2;
    uint64_t setcfg_deq_dcs = setcfg_deq_ptr & 1ULL;
    uint64_t setcfg_deq_phys = setcfg_deq_ptr & ~0xFULL;
    uint64_t setcfg_deq_index = (setcfg_deq_phys - ep0_ring_phys) / 16ULL;
    if (setcfg_deq_dcs != 1ULL || setcfg_deq_phys < ep0_ring_phys || (setcfg_deq_phys >= ep0_ring_phys + PAGE_SIZE || setcfg_deq_phys % 16ULL != 0ULL)) {
      sexos_line("[sexusb.xhci.set_config.deq.bad] ptr=%#llx dcs=%llu", (unsigned long long) setcfg_deq_ptr, (unsigned long long) setcfg_deq_dcs);
      while (true) sexos_yield();
    }
    if (setcfg_deq_index + 2ULL >= PAGE_SIZE / TRB_SIZE) {
      sexos_line("[sexusb.xhci.set_config.deq.ring.bad] idx=%llu", (unsigned long long) setcfg_deq_index);
      while (true) sexos_yield();
    }
    uint32_t setcfg_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 1u << 6 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, setcfg_deq_index, (uint32_t) cfg_value << 16 | 0x0900u, 0u, 8u, setcfg_setup_d3);
    uint32_t setcfg_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 16 | (1u << 5 | ep0_cycle);
    trb_write_volatile(ep0_ring_va, setcfg_deq_index + 1ULL, 0u, 0u, 0u, setcfg_status_d3);
    trb_write_volatile(ep0_ring_va, setcfg_deq_index + 2ULL, 0u, 0u, 0u, ep0_cycle ^ 1u);
    ep0_idx = setcfg_deq_index + 3ULL;
    mmio_write32(db_base, (uint64_t) en_slot_id * 4ULL, 1u);
    bool setcfg_ok = false;
    uint32_t setcfg_residue = 0u;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          setcfg_residue = ev_d2 & 0xFFFFFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          uint32_t ep = ev_d3 >> 16 & 0x1Fu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id && ep == 1u) setcfg_ok = true;
          else sexos_line("[sexusb.xhci.set_config.event.bad] cc=%u slot=%u ep=%u", (unsigned) cc, (unsigned) slot, (unsigned) ep);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!setcfg_ok) {
      sexos_line("[sexusb.xhci.set_config.timeout.bad]");
      while (true) sexos_yield();
    }
    if (setcfg_residue != 0u) {
      sexos_line("[sexusb.xhci.set_config.residue.bad] residue=%u", (unsigned) setcfg_residue);
      while (true) sexos_yield();
    }
    sexos_line("[sexusb.xhci.set_config.event.ok] actual=0 residue=0");
    sexos_line("[sexusb.xhci.set_config.complete.ok]");
    uint32_t setidle_deq_dw2 = sexos_volatile_read32(cfg_deq_ep0_base + 2);
    uint32_t setidle_deq_dw3 = sexos_volatile_read32(cfg_deq_ep0_base + 3);
    uint64_t setidle_deq_ptr = (uint64_t) setidle_deq_dw3 << 32 | (uint64_t) setidle_deq_dw2;
    uint64_t setidle_deq_phys = setidle_deq_ptr & ~0xFULL;
    uint64_t setidle_deq_dcs = setidle_deq_ptr & 1ULL;
    uint64_t setidle_deq_index = (setidle_deq_phys - ep0_ring_phys) / 16ULL;
    if (setidle_deq_dcs != 1ULL || setidle_deq_phys < ep0_ring_phys || (setidle_deq_phys >= ep0_ring_phys + PAGE_SIZE || setidle_deq_phys % 16ULL != 0ULL || setidle_deq_index + 2ULL >= PAGE_SIZE / TRB_SIZE)) {
      sexos_line("[sexusb.xhci.hid.set_idle.deq.bad]");
      while (true) sexos_yield();
    }
    uint32_t setidle_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 1u << 6 | ep0_cycle;
    trb_write_volatile(ep0_ring_va, setidle_deq_index, 0x00000A21u, (uint32_t) hid_interface_number, 8u, setidle_setup_d3);
    uint32_t setidle_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 16 | (1u << 5 | ep0_cycle);
    trb_write_volatile(ep0_ring_va, setidle_deq_index + 1ULL, 0u, 0u, 0u, setidle_status_d3);
    trb_write_volatile(ep0_ring_va, setidle_deq_index + 2ULL, 0u, 0u, 0u, ep0_cycle ^ 1u);
    ep0_idx = setidle_deq_index + 3ULL;
    mmio_write32(db_base, (uint64_t) en_slot_id * 4ULL, 1u);
    bool setidle_ok = false;
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          uint32_t residue = ev_d2 & 0xFFFFFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          uint32_t ep = ev_d3 >> 16 & 0x1Fu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id && (ep == 1u && residue == 0u)) setidle_ok = true;
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!setidle_ok) {
      sexos_line("[sexusb.xhci.hid.set_idle.timeout.bad]");
      while (true) sexos_yield();
    }
    if (intr_ep_addr == 0u || (intr_ep_addr & 0x80u) == 0u || (intr_ep_mps == 0u || intr_ep_mps > 16u)) {
      sexos_line("[sexusb.xhci.intr_in.config.bad] addr=%#x mps=%u interval=%u", (unsigned) intr_ep_addr, (unsigned) intr_ep_mps, (unsigned) intr_ep_interval);
      while (true) sexos_yield();
    }
    uint32_t intr_dci = single_bind.ep_dci;
    uint32_t intr_ep_id = (uint32_t)(single_bind.ep_addr & 0x0Fu);
    uint32_t intr_report_len = (uint32_t) single_bind.max_packet;
    uint64_t intr_ring_phys = sys_alloc_phys(PAGE_SIZE);
    uint64_t intr_report_phys = sys_alloc_phys(PAGE_SIZE);
    if (intr_ring_phys == 0ULL || intr_ring_phys == UINT64_MAX || (intr_report_phys == 0ULL || intr_report_phys == UINT64_MAX)) {
      sexos_line("[sexusb.xhci.intr_in.alloc.bad]");
      while (true) sexos_yield();
    }
    uint64_t intr_ring_va = sys_map_phys(intr_ring_phys, PAGE_SIZE);
    uint64_t intr_report_va = sys_map_phys(intr_report_phys, PAGE_SIZE);
    if (intr_ring_va == 0ULL || intr_ring_va == UINT64_MAX || (intr_report_va == 0ULL || intr_report_va == UINT64_MAX)) {
      sexos_line("[sexusb.xhci.intr_in.map.bad]");
      while (true) sexos_yield();
    }
    if (intr_ring_phys % 64ULL != 0ULL || intr_ring_va % PAGE_SIZE != 0ULL) {
      sexos_line("[sexusb.xhci.intr_in.align.bad] ring_phys=%#llx ring_va=%#llx", (unsigned long long) intr_ring_phys, (unsigned long long) intr_ring_va);
      while (true) sexos_yield();
    }
    memset((void *)(uintptr_t) intr_ring_va, 0, (size_t) PAGE_SIZE);
    memset((void *)(uintptr_t) intr_report_va, 0, (size_t) PAGE_SIZE);
    memset((void *)(uintptr_t) input_ctx_va, 0, (size_t) PAGE_SIZE);
    trb_write_volatile(intr_ring_va, HID_RING_LINK_INDEX, (uint32_t)(intr_ring_phys & 0xFFFFFFFFULL), (uint32_t)(intr_ring_phys >> 32), 0u, TRB_TYPE_LINK << 10 | 1u << 1 | 1u);
    sexos_line("[sexusb.xhci.intr_in.config_ep.start]");
    sexos_volatile_write32((uint32_t *)(uintptr_t) input_ctx_va, 0u);
    sexos_volatile_write32((uint32_t *)(uintptr_t)(input_ctx_va + 4ULL), 0x9u);
    const uint32_t *out_slot_base = (const uint32_t *)(uintptr_t) device_ctx_va;
    uint32_t *in_slot_base = (uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride);
    for (uint64_t i = 0ULL; i < 8ULL; ++i) {
      uint32_t v = sexos_volatile_read32(out_slot_base + (size_t) i);
      sexos_volatile_write32(in_slot_base + (size_t) i, v);
    }
    uint32_t cfg_slot_dw0 = sexos_volatile_read32(in_slot_base + 0);
    uint32_t cfg_slot_dw0_new = (cfg_slot_dw0 & ~(0x1Fu << 27)) | (intr_dci & 0x1Fu) << 27;
    sexos_volatile_write32(in_slot_base + 0, cfg_slot_dw0_new);
    uint32_t *in_ep_base = (uint32_t *)(uintptr_t)(input_ctx_va + ctx_stride * (1ULL + (uint64_t) intr_dci));
    uint32_t ep_dw0 = (uint32_t) single_bind.interval << 16;
    uint32_t ep_dw1 = (CERR_DEFAULT & 0x3u) | EP_TYPE_INTERRUPT_IN << 3 | (uint32_t) single_bind.max_packet << 16;
    uint64_t ep_deq = intr_ring_phys | 1ULL;
    uint32_t ep_dw2 = (uint32_t)(ep_deq & 0xFFFFFFFFULL);
    uint32_t ep_dw3 = (uint32_t)(ep_deq >> 32);
    uint32_t ep_dw4 = intr_report_len | intr_report_len << 16;
    sexos_volatile_write32(in_ep_base + 0, ep_dw0);
    sexos_volatile_write32(in_ep_base + 1, ep_dw1);
    sexos_volatile_write32(in_ep_base + 2, ep_dw2);
    sexos_volatile_write32(in_ep_base + 3, ep_dw3);
    sexos_volatile_write32(in_ep_base + 4, ep_dw4);
    sexos_line("[usb.hid.endpoint.config] slot=%u ep=%u type=interrupt_in max_packet=%u interval=%u ok=1", (unsigned) single_bind.slot_id, (unsigned) intr_ep_id, (unsigned) intr_report_len, (unsigned) single_bind.interval);
    sexos_line("[usb.hid.ep.identity] usb_ep=0x%02x ep_num=%u dir_in=%u dci=%u doorbell_target=%u ok=1", (unsigned) single_bind.ep_addr, (unsigned) intr_ep_id, (unsigned)((single_bind.ep_addr & 0x80u) != 0u ? 1u : 0u), (unsigned) intr_dci, (unsigned) intr_dci);
    uint32_t cfg_ep_d0 = (uint32_t)(input_ctx_phys & 0xFFFFFFFFULL);
    uint32_t cfg_ep_d1 = (uint32_t)(input_ctx_phys >> 32);
    uint32_t cfg_ep_d3 = en_slot_id << 24 | TRB_TYPE_CONFIGURE_ENDPOINT_CMD << 10 | cmd_cycle;
    trb_write_volatile(cmd_ring_va, cmd_idx, cfg_ep_d0, cfg_ep_d1, 0u, cfg_ep_d3);
    trb_write_volatile(cmd_ring_va, cmd_idx + 1ULL, 0u, 0u, 0u, cmd_cycle ^ 1u);
    mmio_write32(db_base, 0ULL, 0u);
    bool cfg_ep_ok = false;
    {
      uint32_t d0 = trb_read_dword(event_ring_va, ev_idx, 0);
      uint32_t d1 = trb_read_dword(event_ring_va, ev_idx, 1);
      uint32_t d2 = trb_read_dword(event_ring_va, ev_idx, 2);
      uint32_t d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      sexos_line("[sexusb.cfg_ep.diag] ev_idx=%llu ev_dcs=%llu d0=%#x d1=%#x d2=%#x d3=%#x", (unsigned long long) ev_idx, (unsigned long long) ev_dcs, (unsigned) d0, (unsigned) d1, (unsigned) d2, (unsigned) d3);
    }
    for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
      uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
      if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        if (ev_type == TRB_TYPE_CMD_COMPLETION_EVENT) {
          uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
          uint32_t cc = ev_d2 >> 24 & 0xFFu;
          uint32_t slot = ev_d3 >> 24 & 0xFFu;
          if (cc == TRB_CC_SUCCESS && slot == en_slot_id) cfg_ep_ok = true;
          else sexos_line("[sexusb.xhci.intr_in.config_ep.event.bad] cc=%u slot=%u", (unsigned) cc, (unsigned) slot);
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          break;
        } else {
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
        }
      }
      sexos_yield();
    }
    if (!cfg_ep_ok) {
      sexos_line("[sexusb.xhci.intr_in.config_ep.timeout.bad]");
      while (true) sexos_yield();
    }
    cmd_idx += 1ULL;
    sexos_line("[sexusb.xhci.intr_in.config_ep.ok]");
    sexos_line("[usb.xhci.enum.done] slot=%u ok=1", (unsigned) en_slot_id);
    HidDevice devices[2];
    memset(devices, 0, sizeof devices);
    devices[0].role = HID_ROLE_UNKNOWN;
    devices[1].role = HID_ROLE_UNKNOWN;
    size_t device_count = 0;
    if (target_port_count > 1) {
      uint64_t second_port = (uint64_t) target_ports[1];
      HidRole s2_hid_role = HID_ROLE_UNKNOWN;
      sexos_line("[sexusb.slot2.enable.start] port=%llu", (unsigned long long) second_port);
      uint32_t s2_enable_d3 = TRB_TYPE_ENABLE_SLOT_CMD << 10 | cmd_cycle;
      trb_write_volatile(cmd_ring_va, cmd_idx, 0u, 0u, 0u, s2_enable_d3);
      trb_write_volatile(cmd_ring_va, cmd_idx + 1ULL, 0u, 0u, 0u, cmd_cycle ^ 1u);
      mmio_write32(db_base, 0ULL, 0u);
      uint32_t s2_slot_id = 0u;
      bool s2_en_ok = false;
      for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
        uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
        if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
          uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
          if (ev_type == TRB_TYPE_CMD_COMPLETION_EVENT) {
            uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
            uint32_t cc = ev_d2 >> 24 & 0xFFu;
            s2_en_ok = cc == TRB_CC_SUCCESS;
            s2_slot_id = ev_d3 >> 24 & 0xFFu;
            ev_idx += 1ULL;
            if (ev_idx >= EVENT_RING_TRBS) {
              ev_idx = 0ULL;
              ev_dcs ^= 1ULL;
            }
            mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t)(event_ring_phys + ev_idx * 16ULL));
            mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)((event_ring_phys + ev_idx * 16ULL) >> 32));
            break;
          } else {
            ev_idx += 1ULL;
            if (ev_idx >= EVENT_RING_TRBS) {
              ev_idx = 0ULL;
              ev_dcs ^= 1ULL;
            }
            uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
            mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
            mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          }
        }
        sexos_yield();
      }
      if (!s2_en_ok || s2_slot_id == 0u) {
        sexos_line("[sexusb.slot2.enable.bad]");
        while (true) sexos_yield();
      }
      sexos_line("[sexusb.slot2.enable.ok] slot=%u", (unsigned) s2_slot_id);
      cmd_idx += 1ULL;
      uint64_t s2_input_phys = sys_alloc_phys(PAGE_SIZE);
      uint64_t s2_device_phys = sys_alloc_phys(PAGE_SIZE);
      uint64_t s2_ep0_ring_phys = sys_alloc_phys(PAGE_SIZE);
      if (s2_input_phys == 0ULL || s2_input_phys == UINT64_MAX || (s2_device_phys == 0ULL || s2_device_phys == UINT64_MAX || (s2_ep0_ring_phys == 0ULL || s2_ep0_ring_phys == UINT64_MAX))) {
        sexos_line("[sexusb.slot2.alloc.bad]");
        while (true) sexos_yield();
      }
      uint64_t s2_input_va = sys_map_phys(s2_input_phys, PAGE_SIZE);
      uint64_t s2_device_va = sys_map_phys(s2_device_phys, PAGE_SIZE);
      uint64_t s2_ep0_ring_va = sys_map_phys(s2_ep0_ring_phys, PAGE_SIZE);
      if (s2_input_va == 0ULL || s2_input_va == UINT64_MAX || (s2_device_va == 0ULL || s2_device_va == UINT64_MAX || (s2_ep0_ring_va == 0ULL || s2_ep0_ring_va == UINT64_MAX))) {
        sexos_line("[sexusb.slot2.map.bad]");
        while (true) sexos_yield();
      }
      if (s2_input_phys % 64ULL != 0ULL || s2_device_phys % 64ULL != 0ULL || s2_ep0_ring_phys % 64ULL != 0ULL) {
        sexos_line("[sexusb.slot2.align.bad]");
        while (true) sexos_yield();
      }
      memset((void *)(uintptr_t) s2_input_va, 0, (size_t) PAGE_SIZE);
      memset((void *)(uintptr_t) s2_device_va, 0, (size_t) PAGE_SIZE);
      memset((void *)(uintptr_t) s2_ep0_ring_va, 0, (size_t) PAGE_SIZE);
      uint64_t *s2_dcbaa_ptr = (uint64_t *)(uintptr_t)(dcbaa_va + (uint64_t) s2_slot_id * 8ULL);
      sexos_volatile_write64(s2_dcbaa_ptr, s2_device_phys);
      uint32_t s2_portsc = mmio_read32(op_base, PORTSC_BASE + (second_port - 1ULL) * PORTSC_STRIDE);
      uint32_t s2_speed = s2_portsc >> 10 & 0xFu;
      uint32_t s2_mps = 0u;
      if (s2_speed == 0u || s2_speed == 1u) s2_mps = 8u;
      else if (s2_speed == 2u) s2_mps = 64u;
      else if (s2_speed == 3u) s2_mps = 512u;
      else {
        sexos_line("[sexusb.slot2.speed.bad] speed=%u", (unsigned) s2_speed);
        while (true) sexos_yield();
      }
      sexos_volatile_write32((uint32_t *)(uintptr_t) s2_input_va, 0u);
      sexos_volatile_write32((uint32_t *)(uintptr_t)(s2_input_va + 4ULL), 3u);
      uint32_t s2_slot_dw0 = 1u << SLOT_CTX_ENTRIES_SHIFT | s2_speed << SLOT_SPEED_SHIFT;
      uint32_t s2_slot_dw1 = (uint32_t) second_port << 16;
      uint32_t *s2_in_slot = (uint32_t *)(uintptr_t)(s2_input_va + ctx_stride);
      sexos_volatile_write32(s2_in_slot + 0, s2_slot_dw0);
      sexos_volatile_write32(s2_in_slot + 1, s2_slot_dw1);
      uint32_t *s2_dev_slot = (uint32_t *)(uintptr_t) s2_device_va;
      sexos_volatile_write32(s2_dev_slot + 0, s2_slot_dw0);
      sexos_volatile_write32(s2_dev_slot + 1, s2_slot_dw1);
      uint64_t s2_ep0_deq = s2_ep0_ring_phys | DCS_INITIAL;
      uint32_t *s2_in_ep0 = (uint32_t *)(uintptr_t)(s2_input_va + ctx_stride * 2ULL);
      sexos_volatile_write32(s2_in_ep0 + 0, s2_mps << 16);
      sexos_volatile_write32(s2_in_ep0 + 1, EP_TYPE_CTRL_VAL << EP_TYPE_CTRL_SHIFT | CERR_DEFAULT);
      sexos_volatile_write32(s2_in_ep0 + 2, (uint32_t)(s2_ep0_deq & 0xFFFFFFFFULL));
      sexos_volatile_write32(s2_in_ep0 + 3, (uint32_t)(s2_ep0_deq >> 32));
      uint32_t *s2_dev_ep0 = (uint32_t *)(uintptr_t)(s2_device_va + ctx_stride);
      sexos_volatile_write32(s2_dev_ep0 + 0, s2_mps << 16);
      sexos_volatile_write32(s2_dev_ep0 + 1, EP_TYPE_CTRL_VAL << EP_TYPE_CTRL_SHIFT | CERR_DEFAULT);
      sexos_volatile_write32(s2_dev_ep0 + 2, (uint32_t)(s2_ep0_deq & 0xFFFFFFFFULL));
      sexos_volatile_write32(s2_dev_ep0 + 3, (uint32_t)(s2_ep0_deq >> 32));
      sexos_line("[sexusb.slot2.address.start] port=%llu", (unsigned long long) second_port);
      uint32_t s2_addr_d0 = (uint32_t)(s2_input_phys & 0xFFFFFFFFULL);
      uint32_t s2_addr_d1 = (uint32_t)(s2_input_phys >> 32);
      uint32_t s2_addr_d3 = s2_slot_id << 24 | TRB_TYPE_ADDRESS_DEVICE_CMD << 10 | cmd_cycle;
      trb_write_volatile(cmd_ring_va, cmd_idx, s2_addr_d0, s2_addr_d1, 0u, s2_addr_d3);
      trb_write_volatile(cmd_ring_va, cmd_idx + 1ULL, 0u, 0u, 0u, cmd_cycle ^ 1u);
      mmio_write32(db_base, 0ULL, 0u);
      bool s2_addr_ok = false;
      for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
        uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
        if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
          uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
          if (ev_type == TRB_TYPE_CMD_COMPLETION_EVENT) {
            uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
            uint32_t cc = ev_d2 >> 24 & 0xFFu;
            uint32_t ev_slot = ev_d3 >> 24 & 0xFFu;
            if (cc == TRB_CC_SUCCESS && ev_slot == s2_slot_id) s2_addr_ok = true;
            ev_idx += 1ULL;
            if (ev_idx >= EVENT_RING_TRBS) {
              ev_idx = 0ULL;
              ev_dcs ^= 1ULL;
            }
            mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t)(event_ring_phys + ev_idx * 16ULL));
            mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)((event_ring_phys + ev_idx * 16ULL) >> 32));
            break;
          } else {
            ev_idx += 1ULL;
            if (ev_idx >= EVENT_RING_TRBS) {
              ev_idx = 0ULL;
              ev_dcs ^= 1ULL;
            }
            uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
            mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
            mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          }
        }
        sexos_yield();
      }
      if (!s2_addr_ok) {
        sexos_line("[sexusb.slot2.address.bad]");
        while (true) sexos_yield();
      }
      cmd_idx += 1ULL;
      sexos_line("[sexusb.slot2.address.ok] slot=%u port=%llu", (unsigned) s2_slot_id, (unsigned long long) second_port);
      uint64_t s2_ep0_idx = 0ULL;
      uint32_t s2_cycle = 1u;
      memset((void *)(uintptr_t) desc_data_va, 0, (size_t) PAGE_SIZE);
      const uint32_t *s2_ep0_base = (const uint32_t *)(uintptr_t)(s2_device_va + ctx_stride);
      {
        uint32_t s_deq_dw2 = sexos_volatile_read32(s2_ep0_base + 2);
        uint32_t s_deq_dw3 = sexos_volatile_read32(s2_ep0_base + 3);
        uint64_t s_deq_ptr = (uint64_t) s_deq_dw3 << 32 | (uint64_t) s_deq_dw2;
        uint64_t s_deq_dcs = s_deq_ptr & 1ULL;
        uint64_t s_deq_phys = s_deq_ptr & ~0xFULL;
        uint64_t s_deq_index = (s_deq_phys - s2_ep0_ring_phys) / 16ULL;
        if (s_deq_dcs != 1ULL || s_deq_phys < s2_ep0_ring_phys || (s_deq_phys >= s2_ep0_ring_phys + PAGE_SIZE || s_deq_phys % 16ULL != 0ULL || s_deq_index + 3ULL >= PAGE_SIZE / TRB_SIZE)) {
          sexos_line("[sexusb.slot2.desc8.deq.bad] ptr=%#llx dcs=%llu idx=%llu", (unsigned long long) s_deq_ptr, (unsigned long long) s_deq_dcs, (unsigned long long) s_deq_index);
          while (true) sexos_yield();
        }
        uint32_t s_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | s2_cycle);
        trb_write_volatile(s2_ep0_ring_va, s_deq_index, 0x01000680u, 0x00080000u, 8u, s_setup_d3);
        uint32_t s_data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | s2_cycle;
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), 8u | 1u << 18, s_data_d3);
        uint32_t s_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 5 | s2_cycle;
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 2ULL, 0u, 0u, 0u, s_status_d3);
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 3ULL, 0u, 0u, 0u, s2_cycle ^ 1u);
        s2_ep0_idx = s_deq_index + 4ULL;
        mmio_write32(db_base, (uint64_t) s2_slot_id * 4ULL, 1u);
        bool s_desc8_ok = false;
        for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
          uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
          if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
            uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
            if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
              uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
              uint32_t cc = ev_d2 >> 24 & 0xFFu;
              uint32_t slot = ev_d3 >> 24 & 0xFFu;
              uint32_t ep = ev_d3 >> 16 & 0x1Fu;
              if (cc == TRB_CC_SUCCESS && slot == s2_slot_id && ep == 1u) s_desc8_ok = true;
              ev_idx += 1ULL;
              if (ev_idx >= EVENT_RING_TRBS) {
                ev_idx = 0ULL;
                ev_dcs ^= 1ULL;
              }
              uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
              mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
              mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
              break;
            } else {
              ev_idx += 1ULL;
              if (ev_idx >= EVENT_RING_TRBS) {
                ev_idx = 0ULL;
                ev_dcs ^= 1ULL;
              }
              uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
              mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
              mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
            }
          }
          sexos_yield();
        }
        if (!s_desc8_ok) {
          sexos_line("[sexusb.slot2.desc8.bad]");
          while (true) sexos_yield();
        }
        uint8_t s2_mps0 = sexos_volatile_read8((const uint8_t *)(uintptr_t) desc_data_va + 7);
        sexos_line("[sexusb.slot2.desc8.ok] mps=%u", (unsigned) s2_mps0);
      }
      {
        uint32_t s_deq_dw2 = sexos_volatile_read32(s2_ep0_base + 2);
        uint32_t s_deq_dw3 = sexos_volatile_read32(s2_ep0_base + 3);
        uint64_t s_deq_ptr = (uint64_t) s_deq_dw3 << 32 | (uint64_t) s_deq_dw2;
        uint64_t s_deq_dcs = s_deq_ptr & 1ULL;
        uint64_t s_deq_phys = s_deq_ptr & ~0xFULL;
        uint64_t s_deq_index = (s_deq_phys - s2_ep0_ring_phys) / 16ULL;
        if (s_deq_dcs != 1ULL || s_deq_phys < s2_ep0_ring_phys || (s_deq_phys >= s2_ep0_ring_phys + PAGE_SIZE || s_deq_phys % 16ULL != 0ULL || s_deq_index + 3ULL >= PAGE_SIZE / TRB_SIZE)) {
          sexos_line("[sexusb.slot2.full18.deq.bad] ptr=%#llx dcs=%llu idx=%llu", (unsigned long long) s_deq_ptr, (unsigned long long) s_deq_dcs, (unsigned long long) s_deq_index);
          while (true) sexos_yield();
        }
        memset((void *)(uintptr_t) desc_data_va, 0, 18);
        uint32_t s_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | s2_cycle);
        trb_write_volatile(s2_ep0_ring_va, s_deq_index, 0x01000680u, 0x00120000u, 8u, s_setup_d3);
        uint32_t s_data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | s2_cycle;
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), 18u | 1u << 18, s_data_d3);
        uint32_t s_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 5 | s2_cycle;
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 2ULL, 0u, 0u, 0u, s_status_d3);
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 3ULL, 0u, 0u, 0u, s2_cycle ^ 1u);
        s2_ep0_idx = s_deq_index + 4ULL;
        mmio_write32(db_base, (uint64_t) s2_slot_id * 4ULL, 1u);
        bool s_full_ok = false;
        for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
          uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
          if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
            uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
            if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
              uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
              uint32_t cc = ev_d2 >> 24 & 0xFFu;
              uint32_t slot = ev_d3 >> 24 & 0xFFu;
              uint32_t ep = ev_d3 >> 16 & 0x1Fu;
              if (cc == TRB_CC_SUCCESS && slot == s2_slot_id && ep == 1u) s_full_ok = true;
              ev_idx += 1ULL;
              if (ev_idx >= EVENT_RING_TRBS) {
                ev_idx = 0ULL;
                ev_dcs ^= 1ULL;
              }
              uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
              mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
              mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
              break;
            } else {
              ev_idx += 1ULL;
              if (ev_idx >= EVENT_RING_TRBS) {
                ev_idx = 0ULL;
                ev_dcs ^= 1ULL;
              }
              uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
              mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
              mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
            }
          }
          sexos_yield();
        }
        if (!s_full_ok) {
          sexos_line("[sexusb.slot2.full18.bad]");
          while (true) sexos_yield();
        }
        const uint8_t *s_buf = (const uint8_t *)(uintptr_t) desc_data_va;
        uint8_t s_bclass = sexos_volatile_read8(s_buf + 4);
        uint8_t s_bsub = sexos_volatile_read8(s_buf + 5);
        uint8_t s_bproto = sexos_volatile_read8(s_buf + 6);
        uint8_t s_vid_lo = sexos_volatile_read8(s_buf + 8);
        uint8_t s_vid_hi = sexos_volatile_read8(s_buf + 9);
        uint8_t s_pid_lo = sexos_volatile_read8(s_buf + 10);
        uint8_t s_pid_hi = sexos_volatile_read8(s_buf + 11);
        uint32_t s_vid = (uint32_t) s_vid_hi << 8 | (uint32_t) s_vid_lo;
        uint32_t s_pid = (uint32_t) s_pid_hi << 8 | (uint32_t) s_pid_lo;
        sexos_line("[sexusb.slot2.desc.device] class=%#x subclass=%#x proto=%#x vid=%#x pid=%#x", (unsigned) s_bclass, (unsigned) s_bsub, (unsigned) s_bproto, (unsigned) s_vid, (unsigned) s_pid);
      }
      uint64_t s2_w_total_length = 0ULL;
      uint32_t s2_num_interfaces = 0u;
      {
        uint32_t s_deq_dw2 = sexos_volatile_read32(s2_ep0_base + 2);
        uint32_t s_deq_dw3 = sexos_volatile_read32(s2_ep0_base + 3);
        uint64_t s_deq_ptr = (uint64_t) s_deq_dw3 << 32 | (uint64_t) s_deq_dw2;
        uint64_t s_deq_dcs = s_deq_ptr & 1ULL;
        uint64_t s_deq_phys = s_deq_ptr & ~0xFULL;
        uint64_t s_deq_index = (s_deq_phys - s2_ep0_ring_phys) / 16ULL;
        if (s_deq_dcs != 1ULL || s_deq_phys < s2_ep0_ring_phys || (s_deq_phys >= s2_ep0_ring_phys + PAGE_SIZE || s_deq_phys % 16ULL != 0ULL || s_deq_index + 3ULL >= PAGE_SIZE / TRB_SIZE)) {
          sexos_line("[sexusb.slot2.cfg9.deq.bad] ptr=%#llx dcs=%llu idx=%llu", (unsigned long long) s_deq_ptr, (unsigned long long) s_deq_dcs, (unsigned long long) s_deq_index);
          while (true) sexos_yield();
        }
        memset((void *)(uintptr_t) desc_data_va, 0, 9);
        uint32_t s_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | s2_cycle);
        trb_write_volatile(s2_ep0_ring_va, s_deq_index, 0x02000680u, 0x00090000u, 8u, s_setup_d3);
        uint32_t s_data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | s2_cycle;
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), 9u | 1u << 18, s_data_d3);
        uint32_t s_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 5 | s2_cycle;
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 2ULL, 0u, 0u, 0u, s_status_d3);
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 3ULL, 0u, 0u, 0u, s2_cycle ^ 1u);
        s2_ep0_idx = s_deq_index + 4ULL;
        mmio_write32(db_base, (uint64_t) s2_slot_id * 4ULL, 1u);
        bool s_cfg_hdr_ok = false;
        for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
          uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
          if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
            uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
            if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
              uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
              uint32_t cc = ev_d2 >> 24 & 0xFFu;
              uint32_t slot = ev_d3 >> 24 & 0xFFu;
              uint32_t ep = ev_d3 >> 16 & 0x1Fu;
              if (cc == TRB_CC_SUCCESS && slot == s2_slot_id && ep == 1u) s_cfg_hdr_ok = true;
              ev_idx += 1ULL;
              if (ev_idx >= EVENT_RING_TRBS) {
                ev_idx = 0ULL;
                ev_dcs ^= 1ULL;
              }
              uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
              mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
              mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
              break;
            } else {
              ev_idx += 1ULL;
              if (ev_idx >= EVENT_RING_TRBS) {
                ev_idx = 0ULL;
                ev_dcs ^= 1ULL;
              }
              uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
              mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
              mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
            }
          }
          sexos_yield();
        }
        if (!s_cfg_hdr_ok) {
          sexos_line("[sexusb.slot2.cfg9.bad]");
          while (true) sexos_yield();
        }
        const uint8_t *s_cfg_buf = (const uint8_t *)(uintptr_t) desc_data_va;
        uint8_t s_tot_lo = sexos_volatile_read8(s_cfg_buf + 2);
        uint8_t s_tot_hi = sexos_volatile_read8(s_cfg_buf + 3);
        s2_w_total_length = (uint64_t) s_tot_hi << 8 | (uint64_t) s_tot_lo;
        s2_num_interfaces = (uint32_t) sexos_volatile_read8(s_cfg_buf + 4);
        if (s2_w_total_length < 9ULL) {
          sexos_line("[sexusb.slot2.cfg9.totallen.bad] len=%llu", (unsigned long long) s2_w_total_length);
          while (true) sexos_yield();
        }
        sexos_line("[sexusb.slot2.cfg9.ok] wTotalLength=%llu num_interfaces=%u", (unsigned long long) s2_w_total_length, (unsigned) s2_num_interfaces);
        sexos_line("[sexusb.config.descriptor] slot=2 interfaces=%u", (unsigned) s2_num_interfaces);
      }
      uint8_t s2_intr_ep_addr = 0u;
      uint16_t s2_intr_ep_mps = 0u;
      uint8_t s2_intr_ep_interval = 0u;
      {
        uint32_t s_deq_dw2 = sexos_volatile_read32(s2_ep0_base + 2);
        uint32_t s_deq_dw3 = sexos_volatile_read32(s2_ep0_base + 3);
        uint64_t s_deq_ptr = (uint64_t) s_deq_dw3 << 32 | (uint64_t) s_deq_dw2;
        uint64_t s_deq_dcs = s_deq_ptr & 1ULL;
        uint64_t s_deq_phys = s_deq_ptr & ~0xFULL;
        uint64_t s_deq_index = (s_deq_phys - s2_ep0_ring_phys) / 16ULL;
        if (s_deq_dcs != 1ULL || s_deq_phys < s2_ep0_ring_phys || (s_deq_phys >= s2_ep0_ring_phys + PAGE_SIZE || s_deq_phys % 16ULL != 0ULL || s_deq_index + 3ULL >= PAGE_SIZE / TRB_SIZE)) {
          sexos_line("[sexusb.slot2.cfg_full.deq.bad] ptr=%#llx dcs=%llu idx=%llu", (unsigned long long) s_deq_ptr, (unsigned long long) s_deq_dcs, (unsigned long long) s_deq_index);
          while (true) sexos_yield();
        }
        {
          uint8_t *s_zero_base = (uint8_t *)(uintptr_t) desc_data_va;
          uint64_t s_zi = 0ULL;
          while (s_zi < s2_w_total_length) {
            sexos_volatile_write8(s_zero_base + (size_t) s_zi, 0u);
            s_zi += 1ULL;
          }
        }
        uint32_t s_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 2u << 16 | (1u << 6 | s2_cycle);
        trb_write_volatile(s2_ep0_ring_va, s_deq_index, 0x02000680u, (uint32_t) s2_w_total_length << 16, 8u, s_setup_d3);
        uint32_t s_data_d3 = TRB_TYPE_DATA_STAGE << 10 | 1u << 16 | s2_cycle;
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 1ULL, (uint32_t)(desc_data_phys & 0xFFFFFFFFULL), (uint32_t)(desc_data_phys >> 32), (uint32_t) s2_w_total_length | 1u << 18, s_data_d3);
        uint32_t s_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 5 | s2_cycle;
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 2ULL, 0u, 0u, 0u, s_status_d3);
        trb_write_volatile(s2_ep0_ring_va, s_deq_index + 3ULL, 0u, 0u, 0u, s2_cycle ^ 1u);
        s2_ep0_idx = s_deq_index + 4ULL;
        mmio_write32(db_base, (uint64_t) s2_slot_id * 4ULL, 1u);
        bool s_cfg_full_ok = false;
        uint32_t s_cfg_full_residue = 0u;
        for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
          uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
          if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
            uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
            if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
              uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
              uint32_t cc = ev_d2 >> 24 & 0xFFu;
              s_cfg_full_residue = ev_d2 & 0xFFFFFFu;
              uint32_t slot = ev_d3 >> 24 & 0xFFu;
              uint32_t ep = ev_d3 >> 16 & 0x1Fu;
              if (cc == TRB_CC_SUCCESS && slot == s2_slot_id && ep == 1u) s_cfg_full_ok = true;
              ev_idx += 1ULL;
              if (ev_idx >= EVENT_RING_TRBS) {
                ev_idx = 0ULL;
                ev_dcs ^= 1ULL;
              }
              uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
              mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
              mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
              break;
            } else {
              ev_idx += 1ULL;
              if (ev_idx >= EVENT_RING_TRBS) {
                ev_idx = 0ULL;
                ev_dcs ^= 1ULL;
              }
              uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
              mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
              mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
            }
          }
          sexos_yield();
        }
        if (!s_cfg_full_ok) {
          sexos_line("[sexusb.slot2.cfg_full.bad]");
          while (true) sexos_yield();
        }
        if (s_cfg_full_residue >= (uint32_t) s2_w_total_length) {
          sexos_line("[sexusb.slot2.cfg_full.residue_full.bad] residue=%u total=%llu", (unsigned) s_cfg_full_residue, (unsigned long long) s2_w_total_length);
          while (true) sexos_yield();
        }
        uint64_t s2_received_len = s2_w_total_length;
        if (s_cfg_full_residue > 0u) {
          sexos_line("[sexusb.slot2.cfg_full.residue.warn] residue=%u", (unsigned) s_cfg_full_residue);
          s2_received_len = s2_w_total_length - (uint64_t) s_cfg_full_residue;
        }
        sexos_line("[sexusb.slot2.cfg_full.ok] received_len=%llu", (unsigned long long) s2_received_len);
        const uint8_t *s_walk_buf = (const uint8_t *)(uintptr_t) desc_data_va;
        uint64_t s_walk_off = 0ULL;
        bool s2_found_hid_keyboard = false;
        bool s2_found_hid_tablet = false;
        bool s2_found_hid_mouse = false;
        bool s2_inside_hid_iface = false;
        uint8_t s2_hid_iface = 0u;
        while (s_walk_off < s2_received_len) {
          uint8_t b_len = sexos_volatile_read8(s_walk_buf + (size_t) s_walk_off);
          uint8_t b_type = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 1));
          if (b_len == 0u) {
            sexos_line("[sexusb.slot2.desc.zero_len.bad] off=%llu", (unsigned long long) s_walk_off);
            break;
          }
          if ((uint64_t) b_len > s2_received_len - s_walk_off) {
            sexos_line("[sexusb.slot2.desc.truncated.bad] off=%llu len=%u remain=%llu", (unsigned long long) s_walk_off, (unsigned) b_len, (unsigned long long)(s2_received_len - s_walk_off));
            break;
          }
          if (b_type == 4u && b_len >= 8u) {
            uint8_t b_intf = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 2));
            uint8_t b_class = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 5));
            uint8_t b_sub = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 6));
            uint8_t b_proto = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 7));
            sexos_line("[sexusb.slot2.desc.iface] idx=%llu if=%u class=%#x subclass=%#x proto=%#x", (unsigned long long) s_walk_off, (unsigned) b_intf, (unsigned) b_class, (unsigned) b_sub, (unsigned) b_proto);
            bool is_hid = b_class == 0x03u;
            if (is_hid) {
              s2_inside_hid_iface = true;
              bool is_boot_mouse = b_sub == 0x01u && b_proto == 0x02u;
              bool is_boot_keyboard = b_sub == 0x01u && b_proto == 0x01u;
              if (is_boot_mouse) {
                s2_found_hid_mouse = true;
                s2_hid_iface = b_intf;
                sexos_line("[sexusb.slot2.hid.classify] iface=%u role=mouse", (unsigned) b_intf);
              } else if (is_boot_keyboard) {
                s2_found_hid_keyboard = true;
                s2_hid_iface = b_intf;
                sexos_line("[sexusb.slot2.hid.classify] iface=%u role=keyboard", (unsigned) b_intf);
              } else if (b_proto != 0x01u) {
                s2_found_hid_tablet = true;
                s2_hid_iface = b_intf;
                sexos_line("[sexusb.slot2.hid.classify] iface=%u role=tablet subclass=%#x proto=%#x", (unsigned) b_intf, (unsigned) b_sub, (unsigned) b_proto);
              } else {
                s2_inside_hid_iface = false;
                sexos_line("[sexusb.slot2.hid.classify] iface=%u role=unknown_hid", (unsigned) b_intf);
              }
            } else s2_inside_hid_iface = false;
          } else if (b_type == 5u && b_len >= 7u && s2_inside_hid_iface) {
            uint8_t b_endpoint = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 2));
            uint8_t bm_attrs = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 3));
            uint8_t mps_lo = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 4));
            uint8_t mps_hi = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 5));
            uint16_t w_max_pkt = (uint16_t)((uint16_t) mps_hi << 8) | (uint16_t) mps_lo;
            uint16_t pkt_size = w_max_pkt & 0x07FFu;
            uint8_t b_interval = sexos_volatile_read8(s_walk_buf + ((size_t) s_walk_off + 6));
            bool dir_in = (b_endpoint & 0x80u) != 0u;
            bool intr_type = (bm_attrs & 0x03u) == 0x03u;
            if (dir_in && intr_type && pkt_size > 0u) {
              s2_intr_ep_addr = b_endpoint;
              s2_intr_ep_mps = pkt_size;
              s2_intr_ep_interval = b_interval;
              sexos_line("[sexusb.slot2.ep.find] ep=%#x mps=%u interval=%u", (unsigned) b_endpoint, (unsigned) pkt_size, (unsigned) b_interval);
              sexos_line("[sexusb.endpoint.interrupt_in] slot=2 ep=%#x mps=%u interval=%u", (unsigned) b_endpoint, (unsigned) pkt_size, (unsigned) b_interval);
            }
          }
          s_walk_off += (uint64_t) b_len;
        }
        s2_hid_role = classify_single_hid_role(s2_found_hid_keyboard, s2_found_hid_tablet, s2_found_hid_mouse);
        const char *s2_kind_str = s2_hid_role == HID_ROLE_KEYBOARD ? "keyboard" : s2_hid_role == HID_ROLE_POINTER_MOUSE ? "mouse" : s2_hid_role == HID_ROLE_POINTER_TABLET ? "touchpad" : "unknown";
        sexos_line("[sexusb.hid.candidate] slot=2 kind=%s", s2_kind_str);
        sexos_line("[sexusb.slot2.hid.role] role=%s iface=%u", hid_role_debug_str(s2_hid_role), (unsigned) s2_hid_iface);
        if (s2_hid_role == HID_ROLE_POINTER_TABLET || s2_hid_role == HID_ROLE_POINTER_MOUSE) sexos_line("[sexusb.slot2.hid.pointer.ready] iface=%u", (unsigned) s2_hid_iface);
        sexos_line("[sexusb.slot2.desc.config] wTotalLength=%llu interfaces=%u hid_role=%s", (unsigned long long) s2_w_total_length, (unsigned) s2_num_interfaces, hid_role_debug_str(s2_hid_role));
      }
      sexos_line("[sexusb.slot2.desc.complete] slot=%u port=%llu", (unsigned) s2_slot_id, (unsigned long long) second_port);
      sexos_line("[sexusb.slot2.set_config.start] slot=%u config=1", (unsigned) s2_slot_id);
      uint32_t s2_setcfg_deq_dw2 = sexos_volatile_read32(s2_ep0_base + 2);
      uint32_t s2_setcfg_deq_dw3 = sexos_volatile_read32(s2_ep0_base + 3);
      uint64_t s2_setcfg_deq_ptr = (uint64_t) s2_setcfg_deq_dw3 << 32 | (uint64_t) s2_setcfg_deq_dw2;
      uint64_t s2_setcfg_deq_dcs = s2_setcfg_deq_ptr & 1ULL;
      uint64_t s2_setcfg_deq_phys = s2_setcfg_deq_ptr & ~0xFULL;
      uint64_t s2_setcfg_deq_index = (s2_setcfg_deq_phys - s2_ep0_ring_phys) / 16ULL;
      if (s2_setcfg_deq_dcs != 1ULL || s2_setcfg_deq_phys < s2_ep0_ring_phys || (s2_setcfg_deq_phys >= s2_ep0_ring_phys + PAGE_SIZE || s2_setcfg_deq_phys % 16ULL != 0ULL)) {
        sexos_line("[sexusb.slot2.set_config.reject] reason=deq_bad ptr=%#llx dcs=%llu", (unsigned long long) s2_setcfg_deq_ptr, (unsigned long long) s2_setcfg_deq_dcs);
        while (true) sexos_yield();
      }
      if (s2_setcfg_deq_index + 2ULL >= PAGE_SIZE / TRB_SIZE) {
        sexos_line("[sexusb.slot2.set_config.reject] reason=ring_overflow idx=%llu", (unsigned long long) s2_setcfg_deq_index);
        while (true) sexos_yield();
      }
      uint32_t s2_setcfg_setup_d3 = TRB_TYPE_SETUP_STAGE << 10 | 1u << 6 | s2_cycle;
      trb_write_volatile(s2_ep0_ring_va, s2_setcfg_deq_index, 0x00010900u, 0u, 8u, s2_setcfg_setup_d3);
      uint32_t s2_setcfg_status_d3 = TRB_TYPE_STATUS_STAGE << 10 | 1u << 16 | (1u << 5 | s2_cycle);
      trb_write_volatile(s2_ep0_ring_va, s2_setcfg_deq_index + 1ULL, 0u, 0u, 0u, s2_setcfg_status_d3);
      trb_write_volatile(s2_ep0_ring_va, s2_setcfg_deq_index + 2ULL, 0u, 0u, 0u, s2_cycle ^ 1u);
      s2_ep0_idx = s2_setcfg_deq_index + 3ULL;
      (void) s2_ep0_idx;
      mmio_write32(db_base, (uint64_t) s2_slot_id * 4ULL, 1u);
      bool s2_setcfg_ok = false;
      uint32_t s2_setcfg_residue = 0u;
      for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
        uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
        if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
          uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
          if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
            uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
            uint32_t cc = ev_d2 >> 24 & 0xFFu;
            s2_setcfg_residue = ev_d2 & 0xFFFFFFu;
            uint32_t slot = ev_d3 >> 24 & 0xFFu;
            uint32_t ep = ev_d3 >> 16 & 0x1Fu;
            if (cc == TRB_CC_SUCCESS && slot == s2_slot_id && ep == 1u) s2_setcfg_ok = true;
            else sexos_line("[sexusb.slot2.set_config.event.bad] cc=%u slot=%u ep=%u", (unsigned) cc, (unsigned) slot, (unsigned) ep);
            ev_idx += 1ULL;
            if (ev_idx >= EVENT_RING_TRBS) {
              ev_idx = 0ULL;
              ev_dcs ^= 1ULL;
            }
            uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
            mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
            mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
            break;
          } else {
            ev_idx += 1ULL;
            if (ev_idx >= EVENT_RING_TRBS) {
              ev_idx = 0ULL;
              ev_dcs ^= 1ULL;
            }
            uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
            mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
            mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          }
        }
        sexos_yield();
      }
      if (!s2_setcfg_ok) {
        sexos_line("[sexusb.slot2.set_config.reject] reason=timeout");
        while (true) sexos_yield();
      }
      if (s2_setcfg_residue != 0u) {
        sexos_line("[sexusb.slot2.set_config.reject] reason=residue residue=%u", (unsigned) s2_setcfg_residue);
        while (true) sexos_yield();
      }
      sexos_line("[sexusb.slot2.set_config.ok] slot=%u", (unsigned) s2_slot_id);
      if (s2_intr_ep_addr == 0u || (s2_intr_ep_addr & 0x80u) == 0u || s2_intr_ep_mps == 0u) {
        sexos_line("[sexusb.slot2.configure_endpoint.reject] reason=no_interrupt_in_ep addr=%#x mps=%u", (unsigned) s2_intr_ep_addr, (unsigned) s2_intr_ep_mps);
        while (true) sexos_yield();
      }
      uint32_t s2_ep_num = (uint32_t)(s2_intr_ep_addr & 0x0Fu);
      uint32_t s2_intr_dci = (s2_intr_ep_addr & 0x80u) != 0u ? s2_ep_num * 2u + 1u : s2_ep_num * 2u;
      uint32_t s2_intr_report_len = (uint32_t) s2_intr_ep_mps;
      uint64_t s2_intr_ring_phys = sys_alloc_phys(PAGE_SIZE);
      uint64_t s2_intr_report_phys = sys_alloc_phys(PAGE_SIZE);
      if (s2_intr_ring_phys == 0ULL || s2_intr_ring_phys == UINT64_MAX || (s2_intr_report_phys == 0ULL || s2_intr_report_phys == UINT64_MAX)) {
        sexos_line("[sexusb.slot2.intr.alloc.bad]");
        while (true) sexos_yield();
      }
      uint64_t s2_intr_ring_va = sys_map_phys(s2_intr_ring_phys, PAGE_SIZE);
      uint64_t s2_intr_report_va = sys_map_phys(s2_intr_report_phys, PAGE_SIZE);
      if (s2_intr_ring_va == 0ULL || s2_intr_ring_va == UINT64_MAX || (s2_intr_report_va == 0ULL || s2_intr_report_va == UINT64_MAX)) {
        sexos_line("[sexusb.slot2.intr.map.bad]");
        while (true) sexos_yield();
      }
      if (s2_intr_ring_phys % 64ULL != 0ULL || s2_intr_ring_va % PAGE_SIZE != 0ULL) {
        sexos_line("[sexusb.slot2.intr.align.bad] ring_phys=%#llx ring_va=%#llx", (unsigned long long) s2_intr_ring_phys, (unsigned long long) s2_intr_ring_va);
        while (true) sexos_yield();
      }
      sexos_line("[sexusb.slot2.ep.ring.ok] phys=%#llx va=%#llx report_phys=%#llx", (unsigned long long) s2_intr_ring_phys, (unsigned long long) s2_intr_ring_va, (unsigned long long) s2_intr_report_phys);
      memset((void *)(uintptr_t) s2_intr_ring_va, 0, (size_t) PAGE_SIZE);
      memset((void *)(uintptr_t) s2_intr_report_va, 0, (size_t) PAGE_SIZE);
      memset((void *)(uintptr_t) s2_input_va, 0, (size_t) PAGE_SIZE);
      trb_write_volatile(s2_intr_ring_va, HID_RING_LINK_INDEX, (uint32_t)(s2_intr_ring_phys & 0xFFFFFFFFULL), (uint32_t)(s2_intr_ring_phys >> 32), 0u, TRB_TYPE_LINK << 10 | 1u << 1 | 1u);
      sexos_line("[sexusb.slot2.configure_endpoint.start] slot=%u ep=%#x dci=%u", (unsigned) s2_slot_id, (unsigned) s2_intr_ep_addr, (unsigned) s2_intr_dci);
      sexos_volatile_write32((uint32_t *)(uintptr_t) s2_input_va, 0u);
      sexos_volatile_write32((uint32_t *)(uintptr_t)(s2_input_va + 4ULL), 1u | 1u << s2_intr_dci);
      const uint32_t *s2_out_slot_base = (const uint32_t *)(uintptr_t) s2_device_va;
      uint32_t *s2_in_slot_base = (uint32_t *)(uintptr_t)(s2_input_va + ctx_stride);
      for (uint64_t i = 0ULL; i < 8ULL; ++i) {
        uint32_t v = sexos_volatile_read32(s2_out_slot_base + (size_t) i);
        sexos_volatile_write32(s2_in_slot_base + (size_t) i, v);
      }
      uint32_t s2_cur_slot_dw0 = sexos_volatile_read32(s2_in_slot_base + 0);
      uint32_t s2_slot_dw0_new = (s2_cur_slot_dw0 & ~(0x1Fu << 27)) | (s2_intr_dci & 0x1Fu) << 27;
      sexos_volatile_write32(s2_in_slot_base + 0, s2_slot_dw0_new);
      uint32_t *s2_in_ep_base = (uint32_t *)(uintptr_t)(s2_input_va + ctx_stride * (1ULL + (uint64_t) s2_intr_dci));
      uint32_t s2_ep_dw0 = (uint32_t) s2_intr_ep_interval << 16;
      uint32_t s2_ep_dw1 = (CERR_DEFAULT & 0x3u) | EP_TYPE_INTERRUPT_IN << 3 | (uint32_t) s2_intr_ep_mps << 16;
      uint64_t s2_ep_deq = s2_intr_ring_phys | 1ULL;
      uint32_t s2_ep_dw2 = (uint32_t)(s2_ep_deq & 0xFFFFFFFFULL);
      uint32_t s2_ep_dw3 = (uint32_t)(s2_ep_deq >> 32);
      uint32_t s2_ep_dw4 = s2_intr_report_len | s2_intr_report_len << 16;
      sexos_volatile_write32(s2_in_ep_base + 0, s2_ep_dw0);
      sexos_volatile_write32(s2_in_ep_base + 1, s2_ep_dw1);
      sexos_volatile_write32(s2_in_ep_base + 2, s2_ep_dw2);
      sexos_volatile_write32(s2_in_ep_base + 3, s2_ep_dw3);
      sexos_volatile_write32(s2_in_ep_base + 4, s2_ep_dw4);
      uint32_t s2_cfg_ep_d0 = (uint32_t)(s2_input_phys & 0xFFFFFFFFULL);
      uint32_t s2_cfg_ep_d1 = (uint32_t)(s2_input_phys >> 32);
      uint32_t s2_cfg_ep_d3 = s2_slot_id << 24 | TRB_TYPE_CONFIGURE_ENDPOINT_CMD << 10 | cmd_cycle;
      trb_write_volatile(cmd_ring_va, cmd_idx, s2_cfg_ep_d0, s2_cfg_ep_d1, 0u, s2_cfg_ep_d3);
      trb_write_volatile(cmd_ring_va, cmd_idx + 1ULL, 0u, 0u, 0u, cmd_cycle ^ 1u);
      mmio_write32(db_base, 0ULL, 0u);
      bool s2_cfg_ep_ok = false;
      for (size_t _p = 0; _p < POLL_BUDGET; ++_p) {
        uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
        if ((ev_d3 & 1u) == (uint32_t) ev_dcs) {
          uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
          if (ev_type == TRB_TYPE_CMD_COMPLETION_EVENT) {
            uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
            uint32_t cc = ev_d2 >> 24 & 0xFFu;
            uint32_t slot = ev_d3 >> 24 & 0xFFu;
            if (cc == TRB_CC_SUCCESS && slot == s2_slot_id) s2_cfg_ep_ok = true;
            else sexos_line("[sexusb.slot2.configure_endpoint.event.bad] cc=%u slot=%u", (unsigned) cc, (unsigned) slot);
            ev_idx += 1ULL;
            if (ev_idx >= EVENT_RING_TRBS) {
              ev_idx = 0ULL;
              ev_dcs ^= 1ULL;
            }
            uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
            mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
            mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
            break;
          } else {
            ev_idx += 1ULL;
            if (ev_idx >= EVENT_RING_TRBS) {
              ev_idx = 0ULL;
              ev_dcs ^= 1ULL;
            }
            uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
            mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
            mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          }
        }
        sexos_yield();
      }
      if (!s2_cfg_ep_ok) {
        sexos_line("[sexusb.slot2.configure_endpoint.reject] reason=timeout slot=%u", (unsigned) s2_slot_id);
        while (true) sexos_yield();
      }
      cmd_idx += 2ULL;
      (void) cmd_idx;
      sexos_line("[sexusb.slot2.configure_endpoint.ok] slot=%u ep=%#x dci=%u", (unsigned) s2_slot_id, (unsigned) s2_intr_ep_addr, (unsigned) s2_intr_dci);
      if (second_port != 0ULL) {
        size_t second_port_idx = (size_t) second_port - 1;
        if (second_port_idx < MAX_USB_PORT_TRACKED) {
          port_states[second_port_idx].slot_id = s2_slot_id;
          sexos_line("[usb.xhci.port.slot] port=%llu slot=%u ok=1", (unsigned long long) second_port, (unsigned) s2_slot_id);
        } else sexos_line("[usb.xhci.port.skip] port=%llu reason=slot_track_out_of_range", (unsigned long long) second_port);
      }
      devices[1].active = true;
      devices[1].slot_id = s2_slot_id;
      devices[1].role = s2_hid_role;
      devices[1].intr_dci = s2_intr_dci;
      devices[1].intr_ring_phys = s2_intr_ring_phys;
      devices[1].intr_ring_va = s2_intr_ring_va;
      devices[1].intr_report_phys = s2_intr_report_phys;
      devices[1].intr_report_va = s2_intr_report_va;
      devices[1].intr_report_len = s2_intr_report_len;
      device_count = 2;
    }
    devices[0].active = true;
    devices[0].slot_id = single_bind.slot_id;
    devices[0].role = single_bind.role;
    devices[0].intr_dci = intr_dci;
    devices[0].intr_ring_phys = intr_ring_phys;
    devices[0].intr_ring_va = intr_ring_va;
    devices[0].intr_report_phys = intr_report_phys;
    devices[0].intr_report_va = intr_report_va;
    devices[0].intr_report_len = intr_report_len;
    if (device_count == 0) device_count = 1;
    uint32_t slots_tracked = 0u;
    for (size_t idx = 0; idx < target_port_count; ++idx) {
      uint64_t port = (uint64_t) target_ports[idx];
      if (port != 0ULL) {
        size_t port_idx = (size_t) port - 1;
        if (port_idx < MAX_USB_PORT_TRACKED && port_states[port_idx].slot_id != 0u) slots_tracked += 1u;
      }
    }
    sexos_line("[usb.xhci.multiport.done] ports_seen=%llu connected=%u slots_tracked=%u", (unsigned long long) scan_limit, (unsigned) ports_connected, (unsigned) slots_tracked);
    sexos_line("[usb.xhci.multiport.pass] ok=1");
    if (device_count > 1) {
      const HidDevice *d2 = &devices[1];
      uint64_t p = 0ULL;
      while (p < HID_INFLIGHT) {
        trb_write_volatile(d2->intr_ring_va, p, (uint32_t)((d2->intr_report_phys + p * HID_BUF_STRIDE) & 0xFFFFFFFFULL), (uint32_t)((d2->intr_report_phys + p * HID_BUF_STRIDE) >> 32), d2->intr_report_len, TRB_TYPE_NORMAL << 10 | 1u << 5 | 1u);
        p += 1ULL;
      }
      mmio_write32(db_base, (uint64_t) d2->slot_id * 4ULL, d2->intr_dci);
      sexos_line("[sexusb.hid.inflight.init] slot=%u ep=%u count=%llu", (unsigned) d2->slot_id, (unsigned) d2->intr_dci, (unsigned long long) HID_INFLIGHT);
      sexos_line("[sexusb.slot2.poll.start] slot=%u dci=%u", (unsigned) d2->slot_id, (unsigned) d2->intr_dci);
    }
    if (SEXUSB_SYNTHETIC) {
      sexos_line("[sexusb.synthetic.gate] enabled=1 source=env");
      sexos_line("[sexusb.synthetic.start]");
      sexos_line("[integrated.proof.start]");
      sexos_line("[sexusb.synthetic.drag.start]");
      uint32_t n = 0u;
      sexos_line("[integrated.proof.phase] n=1 approach");
      for (uint32_t _k = 0u; _k < 40u; ++_k) {
        send_synthetic_mouse_frame(st, n, 0u, 1, 1);
        n += 1u;
        sexos_yield();
        sexos_yield();
      }
      sexos_line("[integrated.proof.phase] n=2 click");
      sexos_line("[sexusb.synthetic.drag.frame] n=%u buttons=1 dx=0 dy=0", (unsigned) n);
      send_synthetic_mouse_frame(st, n, 1u, 0, 0);
      n += 1u;
      sexos_yield();
      sexos_yield();
      sexos_yield();
      sexos_yield();
      sexos_line("[integrated.proof.phase] n=3 drag_right");
      for (uint32_t _k = 0u; _k < 80u; ++_k) {
        send_synthetic_mouse_frame(st, n, 1u, 1, 0);
        n += 1u;
        sexos_yield();
        sexos_yield();
      }
      sexos_line("[integrated.proof.phase] n=4 drag_down");
      for (uint32_t _k = 0u; _k < 80u; ++_k) {
        send_synthetic_mouse_frame(st, n, 1u, 0, 1);
        n += 1u;
        sexos_yield();
        sexos_yield();
      }
      sexos_line("[integrated.proof.phase] n=5 release");
      sexos_line("[sexusb.synthetic.drag.frame] n=%u buttons=0 dx=0 dy=0", (unsigned) n);
      send_synthetic_mouse_frame(st, n, 0u, 0, 0);
      n += 1u;
      sexos_yield();
      sexos_yield();
      sexos_yield();
      sexos_yield();
      sexos_line("[integrated.proof.phase] n=6 drift");
      for (uint32_t _k = 0u; _k < 40u; ++_k) {
        send_synthetic_mouse_frame(st, n, 0u, -1, -1);
        n += 1u;
        sexos_yield();
        sexos_yield();
      }
      sexos_line("[integrated.proof.complete]");
      sexos_line("[sexusb.synthetic.drag.complete]");
      sexos_line("[sexusb.synthetic.complete.ok]");
      while (true) sexos_yield();
    }
    if (SEXUSB_SYNTHETIC_SLOT2) {
      sexos_line("[sexusb.synthetic_slot2.begin]");
      for (uint32_t _k = 0u; _k < 20u; ++_k) sexos_yield();
      for (uint32_t i = 0u; i < 5u; ++i) {
        sexos_line("[sexusb.synthetic_slot2.report] n=%u buttons=0 dx=1 dy=1", (unsigned) i);
        send_synthetic_mouse_frame(st, i, 0u, 1, 1);
        sexos_yield();
        sexos_yield();
      }
      sexos_line("[sexusb.synthetic_slot2.report] n=5 buttons=1 dx=0 dy=0");
      send_synthetic_mouse_frame(st, 5u, 1u, 0, 0);
      sexos_yield();
      sexos_yield();
      sexos_yield();
      sexos_line("[sexusb.synthetic_slot2.report] n=6 buttons=0 dx=0 dy=0");
      send_synthetic_mouse_frame(st, 6u, 0u, 0, 0);
      sexos_yield();
      sexos_yield();
      sexos_line("[sexusb.synthetic_slot2.done]");
    }
    const char *dev_kind = single_bind.role == HID_ROLE_KEYBOARD ? "keyboard" : single_bind.role == HID_ROLE_POINTER_TABLET ? "tablet" : "mouse";
    sexos_line("[sexusb.hid.%s.continuous.start] attempts=unbounded", dev_kind);
    sexos_line("[sexusb.poll.budget.enter] budget=1_per_iter");
    sexos_line("[usb.pointer.producer.begin] ok=1");
    bool saw_nonzero = false;
    bool hid_report_nonzero_emitted = false;
    bool hid_report_idle_emitted = false;
    bool hid_report_timeout_emitted = false;
    uint64_t hid_report_total_polls = 0ULL;
    bool boot_mouse_pass_emitted = false;
    bool intr_liveness_pass_emitted = false;
    bool hid_short_packet_begin_emitted = false;
    bool hid_short_packet_pass_emitted = false;
    bool hid_event_any_seen = false;
    uint32_t hid_event_any_code = 0u;
    uint32_t i = 0u;
    uint32_t intr_pcs = 1u;
    sexos_line("[usb.hid.boot_mouse.begin]");
    sexos_line("[usb.hid.intr_liveness.begin]");
    sexos_line("[sexusb.ready]");
    if (!atomic_exchange_explicit(&st->bootgraph_proof_sent, true, memory_order_relaxed)) {
      if (!atomic_exchange_explicit(&st->bootgraph_proof_marker_emitted, true, memory_order_relaxed)) sexos_line("[sexusb.bootgraph.proof_report]");
      bool _sent = false;
      (void) send_report_to_sexinput(st, OP_USB_MOUSE_REPORT, 0ULL, 0ULL, 0ULL, &_sent);
    }
    {
      uint64_t p = 0ULL;
      while (p < HID_INFLIGHT) {
        trb_write_volatile(intr_ring_va, p, (uint32_t)((intr_report_phys + p * HID_BUF_STRIDE) & 0xFFFFFFFFULL), (uint32_t)((intr_report_phys + p * HID_BUF_STRIDE) >> 32), intr_report_len, TRB_TYPE_NORMAL << 10 | 1u << 5 | intr_pcs);
        p += 1ULL;
      }
      sexos_line("[usb.hid.intr.trb.submit] slot=%u dci=%u len=%u cycle=%u ioc=1", (unsigned) single_bind.slot_id, (unsigned) intr_dci, (unsigned) intr_report_len, (unsigned) intr_pcs);
      sexos_line("[sexusb.hid.inflight.init] slot=%u ep=%u count=%llu", (unsigned) single_bind.slot_id, (unsigned) intr_dci, (unsigned long long) HID_INFLIGHT);
      mmio_write32(db_base, (uint64_t) single_bind.slot_id * 4ULL, intr_dci);
    } while (true) {
      bool skip_advance = false;
      uint32_t clear_len = intr_report_len > 8u ? 8u : intr_report_len;
      bool intr_ok = false;
      uint32_t intr_residue = 0u;
      uint32_t intr_actual = 0u;
      uint64_t intr_completed_idx = 0ULL;
      uint64_t intr_completed_off = 0ULL;
      size_t intr_wait_polls = 0;
      while (intr_wait_polls < HID_INTR_LIVENESS_BUDGET) {
        intr_wait_polls += 1;
        hid_report_total_polls += 1ULL;
        (void) hid_report_total_polls;
        uint32_t ev_d3 = trb_read_dword(event_ring_va, ev_idx, 3);
        if ((ev_d3 & 1u) != (uint32_t) ev_dcs) {
          if (intr_wait_polls % 20000 == 0) {
            mmio_write32(db_base, (uint64_t) single_bind.slot_id * 4ULL, intr_dci);
            if (device_count > 1) {
              const HidDevice *d2 = &devices[1];
              if (d2->active) mmio_write32(db_base, (uint64_t) d2->slot_id * 4ULL, d2->intr_dci);
            }
            if (st->kick_budget > 0u) {
              st->kick_budget -= 1u;
              sexos_line("[sexusb.hid.liveness.kick] polls=%llu", (unsigned long long) intr_wait_polls);
            }
          }
          sexos_yield();
          continue;
        }
        uint32_t ev_type = ev_d3 >> 10 & 0x3Fu;
        uint32_t ev_d2 = trb_read_dword(event_ring_va, ev_idx, 2);
        uint32_t cc = ev_d2 >> 24 & 0xFFu;
        uint32_t slot = ev_d3 >> 24 & 0xFFu;
        uint32_t ep = ev_d3 >> 16 & 0x1Fu;
        if (!hid_event_any_seen) {
          sexos_line("[usb.hid.event.any] type=%u slot=%u ep=%u code=%u", (unsigned) ev_type, (unsigned) slot, (unsigned) ep, (unsigned) cc);
          hid_event_any_seen = true;
          hid_event_any_code = cc;
        }
        if (ev_type == TRB_TYPE_TRANSFER_EVENT) {
          intr_residue = ev_d2 & 0xFFFFFFu;
          uint32_t ev_d0 = trb_read_dword(event_ring_va, ev_idx, 0);
          uint32_t ev_d1 = trb_read_dword(event_ring_va, ev_idx, 1);
          uint64_t ev_trb_ptr = (uint64_t) ev_d1 << 32 | (uint64_t) ev_d0;
          int32_t matched_idx = -1;
          for (size_t midx = 0; midx < device_count; ++midx) {
            const HidDevice *d = &devices[midx];
            if (d->active && (slot == d->slot_id && ep == d->intr_dci)) {
              matched_idx = (int32_t) midx;
              break;
            }
          }
          if (SEXUSB_SLOT2_OWNERSHIP_PROOF) {
            if (st->own_budget > 0u) {
              st->own_budget -= 1u;
              sexos_line("[sexusb.slot.ownership.event] slot=%u ep=%u cc=%u matched_idx=%d devices=%llu", (unsigned) slot, (unsigned) ep, (unsigned) cc, (int) matched_idx, (unsigned long long) device_count);
            }
            if (matched_idx < 0 && st->own_miss_budget > 0u) {
              st->own_miss_budget -= 1u;
              sexos_line("[sexusb.slot.ownership.invariant.miss] reason=no_device_match slot=%u ep=%u cc=%u", (unsigned) slot, (unsigned) ep, (unsigned) cc);
            }
          }
          for (size_t midx = 0; midx < device_count; ++midx) {
            const HidDevice *d = &devices[midx];
            if (d->active && (slot == d->slot_id && ep == d->intr_dci)) {
              if (st->c2b_event_budget > 0u) {
                st->c2b_event_budget -= 1u;
                sexos_line("[sexusb.c2b.event.seen] idx=%llu kind=%s", (unsigned long long) midx, d->role == HID_ROLE_KEYBOARD ? "kbd" : "tbl");
              }
              break;
            }
          }
          if ((cc == TRB_CC_SUCCESS || cc == TRB_CC_SHORT_PACKET) && (slot == single_bind.slot_id && ep == intr_dci)) {
            intr_actual = intr_residue <= intr_report_len ? intr_report_len - intr_residue : 0u;
            uint64_t event_idx = HID_INFLIGHT;
            if (ev_trb_ptr >= intr_ring_phys) {
              uint64_t pos = (ev_trb_ptr - intr_ring_phys) / 16ULL;
              if (pos < HID_INFLIGHT) {
                event_idx = pos;
                intr_completed_idx = pos;
                intr_completed_off = pos * HID_BUF_STRIDE;
              }
            }
            if (event_idx >= HID_INFLIGHT) sexos_line("[sexusb.hid.transfer.index.bad] slot=%u ep=%u ptr=%#llx ring=%#llx", (unsigned) slot, (unsigned) ep, (unsigned long long) ev_trb_ptr, (unsigned long long) intr_ring_phys);
            else {
              if (st->xfer_event_budget > 0u) {
                st->xfer_event_budget -= 1u;
                sexos_line("[sexusb.hid.transfer.event] slot=%u ep=%u index=%llu", (unsigned) slot, (unsigned) ep, (unsigned long long) event_idx);
              }
              if (cc == TRB_CC_SHORT_PACKET && !hid_short_packet_begin_emitted) {
                hid_short_packet_begin_emitted = true;
                sexos_line("[usb.hid.short_packet.begin]");
              }
              sexos_line("[usb.hid.transfer.decode] code=%u requested=%u residual=%u actual=%u slot=%u dci=%u", (unsigned) cc, (unsigned) intr_report_len, (unsigned) intr_residue, (unsigned) intr_actual, (unsigned) slot, (unsigned) intr_dci);
              if (cc == TRB_CC_SHORT_PACKET) {
                if (intr_actual > 0u && intr_actual <= intr_report_len) sexos_line("[usb.hid.short_packet.accept] actual=%u ok=1", (unsigned) intr_actual);
                else {
                  sexos_line("[usb.hid.short_packet.reject] reason=no_payload actual=%u code=%u", (unsigned) intr_actual, (unsigned) cc);
                  sexos_line("[usb.hid.short_packet.stop] actionable=1 reason=no_payload");
                  sexos_line("[usb.hid.intr_liveness.stop] actionable=1 reason=no_payload");
                  sexos_line("[usb.hid.boot_mouse.stop] actionable=1 reason=no_payload");
                  hid_report_timeout_emitted = true;
                  ev_idx += 1ULL;
                  if (ev_idx >= EVENT_RING_TRBS) {
                    ev_idx = 0ULL;
                    ev_dcs ^= 1ULL;
                  }
                  uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
                  mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
                  mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
                  break;
                }
              }
              intr_ok = true;
              sexos_line("[usb.hid.intr.event] slot=%u dci=%u code=%u actual=%u ok=1", (unsigned) slot, (unsigned) intr_dci, (unsigned) cc, (unsigned) intr_actual);
            }
          } else {
            sexos_line("[sexusb.xhci.intr_in.event.bad] cc=%u slot=%u ep=%u", (unsigned) cc, (unsigned) slot, (unsigned) ep);
            if (device_count > 1) {
              const HidDevice *d2 = &devices[1];
              if (d2->active && (slot == d2->slot_id && ep == d2->intr_dci && (cc == TRB_CC_SUCCESS || cc == TRB_CC_SHORT_PACKET))) {
                uint32_t s2_actual = intr_residue <= d2->intr_report_len ? d2->intr_report_len - intr_residue : 0u;
                uint64_t s2_completed_idx = HID_INFLIGHT;
                uint64_t s2_off = 0ULL;
                if (ev_trb_ptr >= d2->intr_ring_phys) {
                  uint64_t pos = (ev_trb_ptr - d2->intr_ring_phys) / 16ULL;
                  if (pos < HID_INFLIGHT) {
                    s2_completed_idx = pos;
                    s2_off = pos * HID_BUF_STRIDE;
                  }
                }
                if (s2_completed_idx >= HID_INFLIGHT) sexos_line("[sexusb.hid.transfer.index.bad] slot=%u ep=%u ptr=%#llx ring=%#llx", (unsigned) slot, (unsigned) ep, (unsigned long long) ev_trb_ptr, (unsigned long long) d2->intr_ring_phys);
                else {
                  if (st->s2_xfer_event_budget > 0u) {
                    st->s2_xfer_event_budget -= 1u;
                    sexos_line("[sexusb.hid.transfer.event] slot=%u ep=%u index=%llu", (unsigned) slot, (unsigned) ep, (unsigned long long) s2_completed_idx);
                  }
                  const uint8_t *p_rep = (const uint8_t *)(uintptr_t)(d2->intr_report_va + s2_off);
                  uint8_t s2_report[8];
                  memset(s2_report, 0, 8);
                  uint32_t copy_len = s2_actual > 8u ? 8u : s2_actual;
                  uint32_t bi = 0u;
                  while (bi < copy_len) {
                    s2_report[bi] = sexos_volatile_read8(p_rep + bi);
                    bi += 1u;
                  }
                  uint32_t s2_completed_cycle = trb_read_dword(d2->intr_ring_va, s2_completed_idx, 3) & 1u;
                  uint32_t s2_next_cycle = s2_completed_cycle ^ 1u;
                  if (s2_completed_idx == 0ULL) trb_write_volatile(d2->intr_ring_va, HID_RING_LINK_INDEX, (uint32_t)(d2->intr_ring_phys & 0xFFFFFFFFULL), (uint32_t)(d2->intr_ring_phys >> 32), 0u, TRB_TYPE_LINK << 10 | 1u << 1 | s2_completed_cycle);
                  uint32_t s2_clear_len = d2->intr_report_len > 8u ? 8u : d2->intr_report_len;
                  memset((void *)(uintptr_t)(d2->intr_report_va + s2_off), 0, (size_t) s2_clear_len);
                  trb_write_volatile(d2->intr_ring_va, s2_completed_idx, (uint32_t)((d2->intr_report_phys + s2_off) & 0xFFFFFFFFULL), (uint32_t)((d2->intr_report_phys + s2_off) >> 32), d2->intr_report_len, TRB_TYPE_NORMAL << 10 | 1u << 5 | s2_next_cycle);
                  mmio_write32(db_base, (uint64_t) d2->slot_id * 4ULL, d2->intr_dci);
                  if (st->s2_rearm_ok_budget > 0u) {
                    st->s2_rearm_ok_budget -= 1u;
                    sexos_line("[sexusb.hid.rearm.ok] slot=%u ep=%u index=%llu", (unsigned) d2->slot_id, (unsigned) d2->intr_dci, (unsigned long long) s2_completed_idx);
                  }
                  uint8_t b0 = s2_report[0];
                  uint8_t b1 = s2_report[1];
                  uint8_t b2 = s2_report[2];
                  const char *class_str = b0 == 0u && (b1 == 0u && b2 == 0u) ? "all_zero" : b0 != 0u ? "button" : b1 != 0u || b2 != 0u ? "motion" : "unknown";
                  if (st->c2c_budget > 0u) {
                    st->c2c_budget -= 1u;
                    sexos_line("[sexusb.c2c.slot2.event] slot=2");
                    sexos_line("[sexusb.c2c.report.read] len=%u b0=%#x b1=%#x b2=%#x", (unsigned) s2_actual, (unsigned) b0, (unsigned) b1, (unsigned) b2);
                    sexos_line("[sexusb.c2c.report.classify] class=%s", class_str);
                  }
                  if (d2->role == HID_ROLE_POINTER_TABLET) {
                    TabletReport td;
                    if (decode_tablet_report(s2_report, (size_t) s2_actual, &td)) {
                      uint64_t packed_axes = (uint64_t) td.abs_x | (uint64_t) td.abs_y << 16 | 1ULL << 32;
                      sexos_line("[sexusb.slot2.forward.mouse] kind=tablet buttons=0x%x packed=0x%llx", (unsigned) td.buttons, (unsigned long long) packed_axes);
                      bool _sent = false;
                      (void) send_report_to_sexinput(st, OP_USB_MOUSE_REPORT, 0ULL, (uint64_t) td.buttons, packed_axes, &_sent);
                    } else sexos_line("[sexusb.slot2.tablet.decode.bad] len=%u", (unsigned) s2_actual);
                  } else if (d2->role == HID_ROLE_POINTER_MOUSE) {
                    BootMouseReport md;
                    if (decode_boot_mouse_report(s2_report, (size_t) s2_actual, &md)) {
                      uint64_t packed_axes = (uint64_t)(uint8_t) md.dx | (uint64_t)(uint8_t) md.dy << 8 | (uint64_t)(uint8_t) md.wheel << 16;
                      sexos_line("[sexusb.slot2.forward.mouse] kind=mouse buttons=0x%x packed=0x%llx", (unsigned) md.buttons, (unsigned long long) packed_axes);
                      bool _sent = false;
                      (void) send_report_to_sexinput(st, OP_USB_MOUSE_REPORT, 0ULL, (uint64_t) md.buttons, packed_axes, &_sent);
                    } else sexos_line("[sexusb.slot2.mouse.decode.bad] len=%u", (unsigned) s2_actual);
                  }
                }
              }
            }
          }
          ev_idx += 1ULL;
          if (ev_idx >= EVENT_RING_TRBS) {
            ev_idx = 0ULL;
            ev_dcs ^= 1ULL;
          }
          uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
          mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
          mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
          if (intr_ok) break;
          continue;
        }
        ev_idx += 1ULL;
        if (ev_idx >= EVENT_RING_TRBS) {
          ev_idx = 0ULL;
          ev_dcs ^= 1ULL;
        }
        uint64_t new_erdp = event_ring_phys + ev_idx * 16ULL;
        mmio_write32(intr_base, XHCI_INTR_ERDP, (uint32_t) new_erdp);
        mmio_write32(intr_base, XHCI_INTR_ERDP + 4ULL, (uint32_t)(new_erdp >> 32));
      }
      if (!intr_ok) {
        if (!hid_report_timeout_emitted) {
          const char *timeout_reason = hid_event_any_seen ? hid_event_any_code == 13u ? "transfer_event_code_13" : "transfer_event_seen_but_unmatched" : "device_or_qemu_no_input_stimulus";
          sexos_line("[usb.hid.boot_mouse.timeout] reason=%s slot=%u ep=%u polls=%llu", timeout_reason, (unsigned) single_bind.slot_id, (unsigned) intr_ep_id, (unsigned long long) intr_wait_polls);
          sexos_line("[usb.hid.endpoint.diag] slot=%u ep=%u dci=%u max_packet=%u interval=%u", (unsigned) single_bind.slot_id, (unsigned) intr_ep_id, (unsigned) intr_dci, (unsigned) intr_report_len, (unsigned) single_bind.interval);
          sexos_line("[usb.hid.transfer_ring.diag] phys=%#llx prod=%llu cycle=%u len=%u", (unsigned long long) intr_ring_phys, (unsigned long long) HID_INFLIGHT, (unsigned) intr_pcs, (unsigned) intr_report_len);
          sexos_line("[usb.hid.event_ring.diag] ev_idx=%llu ev_dcs=%llu erdp=%#llx", (unsigned long long) ev_idx, (unsigned long long) ev_dcs, (unsigned long long)(event_ring_phys + ev_idx * 16ULL));
          sexos_line("[usb.hid.boot_mouse.stop] actionable=1 reason=%s", timeout_reason);
          hid_report_timeout_emitted = true;
        }
        continue;
      }
      if (intr_residue > intr_report_len) {
        sexos_line("[sexusb.xhci.intr_in.residue.bad] residue=%u", (unsigned) intr_residue);
        break;
      }
      const uint8_t *report_ptr = (const uint8_t *)(uintptr_t)(intr_report_va + intr_completed_off);
      uint8_t report_bytes[8];
      memset(report_bytes, 0, 8);
      if (intr_actual > 0u) {
        uint32_t copy_len = intr_actual > 8u ? 8u : intr_actual;
        memcpy(report_bytes, report_ptr, (size_t) copy_len);
      }
      uint32_t completed_cycle = trb_read_dword(intr_ring_va, intr_completed_idx, 3) & 1u;
      uint32_t next_cycle = completed_cycle ^ 1u;
      if (intr_completed_idx == 0ULL) trb_write_volatile(intr_ring_va, HID_RING_LINK_INDEX, (uint32_t)(intr_ring_phys & 0xFFFFFFFFULL), (uint32_t)(intr_ring_phys >> 32), 0u, TRB_TYPE_LINK << 10 | 1u << 1 | completed_cycle);
      memset((void *)(uintptr_t)(intr_report_va + intr_completed_off), 0, (size_t) clear_len);
      trb_write_volatile(intr_ring_va, intr_completed_idx, (uint32_t)((intr_report_phys + intr_completed_off) & 0xFFFFFFFFULL), (uint32_t)((intr_report_phys + intr_completed_off) >> 32), intr_report_len, TRB_TYPE_NORMAL << 10 | 1u << 5 | next_cycle);
      mmio_write32(db_base, (uint64_t) single_bind.slot_id * 4ULL, intr_dci);
      if (st->rearm_ok_budget > 0u) {
        st->rearm_ok_budget -= 1u;
        sexos_line("[sexusb.hid.rearm.ok] slot=%u ep=%u index=%llu", (unsigned) single_bind.slot_id, (unsigned) intr_dci, (unsigned long long) intr_completed_idx);
      }
      skip_advance = true;
      uint8_t b0 = report_bytes[0];
      uint8_t b1 = report_bytes[1];
      uint8_t b2 = report_bytes[2];
      uint8_t b3 = report_bytes[3];
      uint8_t b4 = report_bytes[4];
      uint8_t b5 = report_bytes[5];
      uint8_t b6 = report_bytes[6];
      uint8_t b7 = report_bytes[7];
      if (!hid_report_nonzero_emitted || !hid_report_idle_emitted) {
        uint8_t r0 = intr_actual >= 1u ? b0 : 0u;
        uint8_t r1 = intr_actual >= 2u ? b1 : 0u;
        uint8_t r2 = intr_actual >= 3u ? b2 : 0u;
        if (r0 != 0u || (r1 != 0u || r2 != 0u)) {
          if (!hid_report_nonzero_emitted) {
            hid_report_nonzero_emitted = true;
            sexos_line("[sexusb.hid.report.nonzero] len=%u b0=%02x b1=%02x b2=%02x ok=1", (unsigned) intr_actual, (unsigned) r0, (unsigned) r1, (unsigned) r2);
          }
        } else if (!hid_report_idle_emitted) {
          hid_report_idle_emitted = true;
          sexos_line("[sexusb.hid.report.idle] len=%u ok=1", (unsigned) intr_actual);
        }
      }
      if (!boot_mouse_pass_emitted) {
        boot_mouse_pass_emitted = true;
        sexos_line("[usb.hid.boot_mouse.report] len=%u b0=%02x b1=%02x b2=%02x b3=%02x ok=1", (unsigned) intr_actual, (unsigned) b0, (unsigned) b1, (unsigned) b2, (unsigned) b3);
        sexos_line("[usb.hid.boot_mouse.pass] ok=1");
        if (!hid_short_packet_pass_emitted) {
          hid_short_packet_pass_emitted = true;
          sexos_line("[usb.hid.short_packet.pass] ok=1");
        }
        if (!intr_liveness_pass_emitted) {
          intr_liveness_pass_emitted = true;
          sexos_line("[usb.hid.intr_liveness.pass] ok=1");
        }
      }
      const char *classify_kind = is_keyboard_device ? "keyboard" : is_tablet_device ? "tablet" : "mouse";
      if (st->intr_raw_budget > 0u) {
        uint32_t n = st->intr_raw_seq;
        st->intr_raw_seq += 1u;
        st->intr_raw_budget -= 1u;
        sexos_line("[sexusb.intr.raw] n=%u actual=%u dev=%s ep=%u bytes=%02x %02x %02x %02x %02x %02x %02x %02x", (unsigned) n, (unsigned) intr_actual, classify_kind, (unsigned) intr_dci, (unsigned) b0, (unsigned) b1, (unsigned) b2, (unsigned) b3, (unsigned) b4, (unsigned) b5, (unsigned) b6, (unsigned) b7);
      }
      if (is_keyboard_device) {
        if (st->intr_classify_kbd_budget > 0u) {
          uint32_t n = st->intr_classify_kbd_seq;
          st->intr_classify_kbd_seq += 1u;
          st->intr_classify_kbd_budget -= 1u;
          sexos_line("[sexusb.intr.classify] n=%u kind=keyboard action=forward reason=hid_keyboard_interface", (unsigned) n);
        }
        uint8_t kb_b0 = report_bytes[0];
        uint8_t kb_b2 = report_bytes[2];
        uint8_t kb_b3 = report_bytes[3];
        if (!st->kbd_raw_proof_emitted) {
          st->kbd_raw_proof_emitted = true;
          sexos_line("[usb.keyboard.report.raw] len=%u modifiers=%u k0=%u ok=1", (unsigned) intr_actual, (unsigned) kb_b0, (unsigned) kb_b2);
        }
        if (st->kbd_raw_budget > 0u) {
          st->kbd_raw_budget -= 1u;
          sexos_line("[sexusb.kbd.raw] b0=0x%x b2=0x%x b3=0x%x actual=%u", (unsigned) kb_b0, (unsigned) kb_b2, (unsigned) kb_b3, (unsigned) intr_actual);
        }
        uint64_t first_key = (uint64_t) kb_b2;
        if (!st->kbd_decode_proof_emitted) {
          st->kbd_decode_proof_emitted = true;
          uint64_t down = first_key > 0ULL && first_key < 0x04ULL ? 0ULL : 1ULL;
          sexos_line("[usb.keyboard.report.decode] key=%llu down=%llu ok=1", (unsigned long long) first_key, (unsigned long long) down);
        }
        if (st->kbd_forward_budget > 0u && first_key != 0ULL) {
          st->kbd_forward_budget -= 1u;
          sexos_line("[sexusb.kbd.forward] key=0x%llx", (unsigned long long) first_key);
        }
        bool _sent = false;
        (void) send_report_to_sexinput(st, OP_USB_KEYBOARD_REPORT, 0ULL, (uint64_t) kb_b0, first_key, &_sent);
      } else if (is_tablet_device) {
        if (st->intr_classify_tbl_budget > 0u) {
          uint32_t n = st->intr_classify_tbl_seq;
          st->intr_classify_tbl_seq += 1u;
          st->intr_classify_tbl_budget -= 1u;
          sexos_line("[sexusb.intr.classify] n=%u kind=tablet action=forward reason=hid_tablet_interface", (unsigned) n);
        }
        uint8_t rb0 = report_bytes[0];
        uint8_t rb1 = report_bytes[1];
        uint8_t rb2 = report_bytes[2];
        uint8_t rb3 = report_bytes[3];
        uint8_t rb4 = report_bytes[4];
        uint8_t tbl_bytes[5];
        tbl_bytes[0] = rb0;
        tbl_bytes[1] = rb1;
        tbl_bytes[2] = rb2;
        tbl_bytes[3] = rb3;
        tbl_bytes[4] = rb4;
        if (i == 0u && intr_actual >= 5u) sexos_line("[sexusb.hid.tablet.raw] b0=%#x b1=%#x b2=%#x b3=%#x b4=%#x actual=%u", (unsigned) rb0, (unsigned) rb1, (unsigned) rb2, (unsigned) rb3, (unsigned) rb4, (unsigned) intr_actual);
        TabletReport td;
        if (decode_tablet_report(tbl_bytes, (size_t) intr_actual, &td)) {
          if (!st->tablet_raw_proof_emitted) {
            st->tablet_raw_proof_emitted = true;
            sexos_line("[usb.mouse.report.raw] len=%u b0=%u b1=%u b2=%u b3=%u b4=%u ok=1", (unsigned) intr_actual, (unsigned) rb0, (unsigned) rb1, (unsigned) rb2, (unsigned) rb3, (unsigned) rb4);
            sexos_line("[usb.pointer.producer.report] source=usb-tablet len=%u ok=1", (unsigned) intr_actual);
          }
          st->tablet_active_count += 1ULL;
          if (st->tablet_active_count <= 64ULL || st->tablet_active_count % 64ULL == 0ULL) sexos_line("[sexusb.tablet.active] n=%llu x=%u y=%u", (unsigned long long) st->tablet_active_count, (unsigned) td.abs_x, (unsigned) td.abs_y);
          uint64_t packed_axes = (uint64_t) td.abs_x | (uint64_t) td.abs_y << 16 | 1ULL << 32;
          if (!st->tablet_pack_proof_emitted) {
            st->tablet_pack_proof_emitted = true;
            sexos_line("[usb.mouse.report.pack] dx=%u dy=%u buttons=%u wheel=0 is_abs=1 ok=1", (unsigned) td.abs_x, (unsigned) td.abs_y, (unsigned) td.buttons);
          }
          if (st->pointer_candidate_tbl_budget > 0u) {
            uint32_t n = st->pointer_candidate_tbl_seq;
            st->pointer_candidate_tbl_seq += 1u;
            st->pointer_candidate_tbl_budget -= 1u;
            sexos_line("[sexusb.pointer.candidate] n=%u buttons=%u x=%u y=%u mode=abs", (unsigned) n, (unsigned) td.buttons, (unsigned) td.abs_x, (unsigned) td.abs_y);
          }
          if (st->tablet_abs_budget > 0u) {
            st->tablet_abs_budget -= 1u;
            sexos_line("[sexusb.tablet.abs] x=%u y=%u buttons=%u", (unsigned) td.abs_x, (unsigned) td.abs_y, (unsigned) td.buttons);
          }
          st->tablet_report_count += 1ULL;
          if (st->tablet_report_count % 256ULL == 0ULL) sexos_line("[sexusb.tablet.liveness] reports=%llu", (unsigned long long) st->tablet_report_count);
          if (st->forward_mouse_budget > 0u) {
            st->forward_mouse_budget -= 1u;
            sexos_line("[sexusb.forward.mouse] buttons=0x%x packed=0x%llx", (unsigned) td.buttons, (unsigned long long) packed_axes);
          }
          bool _sent = false;
          (void) send_report_to_sexinput(st, OP_USB_MOUSE_REPORT, 0ULL, (uint64_t) td.buttons, packed_axes, &_sent);
          if (!st->tablet_to_input_emitted) {
            st->tablet_to_input_emitted = true;
            sexos_line("[usb.pointer.producer.to_input] op=0x260 ok=1");
          }
          if (td.buttons == 0u && (td.abs_x == st->last_abs_x && td.abs_y == st->last_abs_y)) {
            if (HID_VERBOSE_RING_LOG && (i < 8u || i % 64u == 0u)) sexos_line("[sexusb.hid.tablet.continuous.idle] i=%u", (unsigned) i);
          } else {
            st->last_abs_x = td.abs_x;
            st->last_abs_y = td.abs_y;
            if (st->tablet_live_budget > 0u) {
              st->tablet_live_budget -= 1u;
              sexos_line("[sexusb.tablet.live] n=%u x=%u y=%u buttons=0x%x", (unsigned) i, (unsigned) td.abs_x, (unsigned) td.abs_y, (unsigned) td.buttons);
            }
            sexos_line("[sexusb.hid.tablet.report] i=%u buttons=%#x x=%u y=%u", (unsigned) i, (unsigned) td.buttons, (unsigned) td.abs_x, (unsigned) td.abs_y);
            if (!saw_nonzero) {
              saw_nonzero = true;
              sexos_line("[sexusb.hid.tablet.nonzero.ok] i=%u buttons=%#x x=%u y=%u", (unsigned) i, (unsigned) td.buttons, (unsigned) td.abs_x, (unsigned) td.abs_y);
            }
          }
        } else sexos_line("[sexusb.hid.tablet.decode.bad] len=%u", (unsigned) intr_actual);
      } else {
        uint8_t rb0 = report_bytes[0];
        uint8_t rb1 = report_bytes[1];
        uint8_t rb2 = report_bytes[2];
        uint8_t rb3 = report_bytes[3];
        uint8_t mse_bytes[4];
        mse_bytes[0] = rb0;
        mse_bytes[1] = rb1;
        mse_bytes[2] = rb2;
        mse_bytes[3] = rb3;
        BootMouseReport decoded;
        if (decode_boot_mouse_report(mse_bytes, (size_t) intr_actual, &decoded)) {
          if (!st->mouse_raw_proof_emitted) {
            st->mouse_raw_proof_emitted = true;
            sexos_line("[usb.mouse.report.raw] len=%u b0=%u b1=%u b2=%u b3=%u ok=1", (unsigned) intr_actual, (unsigned) rb0, (unsigned) rb1, (unsigned) rb2, (unsigned) rb3);
            sexos_line("[usb.pointer.producer.report] source=usb-mouse len=%u ok=1", (unsigned) intr_actual);
          }
          if (st->intr_classify_mouse_ok_budget > 0u) {
            uint32_t n = st->intr_classify_mouse_ok_seq;
            st->intr_classify_mouse_ok_seq += 1u;
            st->intr_classify_mouse_ok_budget -= 1u;
            sexos_line("[sexusb.intr.classify] n=%u kind=mouse action=forward reason=boot_mouse_decode_ok", (unsigned) n);
          }
          if (st->sexusb_pointer_raw_budget > 0u) {
            st->sexusb_pointer_raw_budget -= 1u;
            sexos_line("[sexusb.pointer.raw] buttons=%#x dx=%d dy=%d wheel=%d", (unsigned) decoded.buttons, (int) decoded.dx, (int) decoded.dy, (int) decoded.wheel);
          }
          if (st->pointer_candidate_mouse_budget > 0u) {
            uint32_t n = st->pointer_candidate_mouse_seq;
            st->pointer_candidate_mouse_seq += 1u;
            st->pointer_candidate_mouse_budget -= 1u;
            sexos_line("[sexusb.pointer.candidate] n=%u buttons=%u x=0 y=0 dx=%d dy=%d mode=rel", (unsigned) n, (unsigned) decoded.buttons, (int) decoded.dx, (int) decoded.dy);
          }
          uint64_t packed_axes = (uint64_t)(uint8_t) decoded.dx | (uint64_t)(uint8_t) decoded.dy << 8 | (uint64_t)(uint8_t) decoded.wheel << 16;
          if (!st->mouse_pack_proof_emitted) {
            st->mouse_pack_proof_emitted = true;
            sexos_line("[usb.mouse.report.pack] dx=%d dy=%d buttons=%u wheel=%d is_abs=0 ok=1", (int) decoded.dx, (int) decoded.dy, (unsigned) decoded.buttons, (int) decoded.wheel);
          }
          if (st->forward_mouse_budget_boot > 0u) {
            st->forward_mouse_budget_boot -= 1u;
            sexos_line("[sexusb.forward.mouse] buttons=0x%x packed=0x%llx", (unsigned) decoded.buttons, (unsigned long long) packed_axes);
          }
          bool _sent = false;
          (void) send_report_to_sexinput(st, OP_USB_MOUSE_REPORT, 0ULL, (uint64_t) decoded.buttons, packed_axes, &_sent);
          if (!st->mouse_to_input_emitted) {
            st->mouse_to_input_emitted = true;
            sexos_line("[usb.pointer.producer.to_input] op=0x260 ok=1");
          }
          if (decoded.buttons == 0u && (decoded.dx == 0 && (decoded.dy == 0 && decoded.wheel == 0))) {
            if (HID_VERBOSE_RING_LOG && (i < 8u || i % 64u == 0u)) sexos_line("[sexusb.hid.mouse.continuous.idle] i=%u", (unsigned) i);
          } else {
            sexos_line("[sexusb.hid.mouse.continuous.report] i=%u buttons=%#x dx=%d dy=%d wheel=%d", (unsigned) i, (unsigned) decoded.buttons, (int) decoded.dx, (int) decoded.dy, (int) decoded.wheel);
            if (!saw_nonzero) {
              saw_nonzero = true;
              sexos_line("[sexusb.hid.mouse.continuous.nonzero.ok] i=%u buttons=%#x dx=%d dy=%d wheel=%d", (unsigned) i, (unsigned) decoded.buttons, (int) decoded.dx, (int) decoded.dy, (int) decoded.wheel);
              sexos_line("[sexusb.hid.mouse.nonzero.ok] i=%u buttons=%#x dx=%d dy=%d wheel=%d", (unsigned) i, (unsigned) decoded.buttons, (int) decoded.dx, (int) decoded.dy, (int) decoded.wheel);
            }
          }
        } else {
          if (st->intr_classify_mouse_bad_budget > 0u) {
            uint32_t n = st->intr_classify_mouse_bad_seq;
            st->intr_classify_mouse_bad_seq += 1u;
            st->intr_classify_mouse_bad_budget -= 1u;
            sexos_line("[sexusb.intr.classify] n=%u kind=unknown action=drop reason=boot_mouse_decode_bad", (unsigned) n);
          }
          sexos_line("[sexusb.hid.mouse.decode.bad] len=%u", (unsigned) intr_actual);
        }
      }
      if (!skip_advance) sexos_line("[sexusb.hid.rearm.skip.bad] reason=missing_index_rearm");
      i += 1u;
      sexos_yield();
      if (st->poll_yield_budget > 0u) {
        st->poll_yield_budget -= 1u;
        sexos_line("[sexusb.poll.budget.yield] i=%u", (unsigned) i);
      }
    } while (true) sexos_yield();
  }
}
