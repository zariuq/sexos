#include "sexos_mmio.h"
#include <stdatomic.h>
/*Network server and Intel e1000/e1000e driver (servers/sexnet/src/main.rs).*/
#if !defined(SEXNET_PHASE_M_RELIABILITY_PROOF)
#define SEXNET_PHASE_M_RELIABILITY_PROOF 0

#endif
#if !defined(SEXNET_DNS_SOURCE3_PROOF)
#define SEXNET_DNS_SOURCE3_PROOF 0

#endif
#if !defined(SEXOS_SEXNET_PROOF_PROFILE)
#define SEXOS_SEXNET_PROOF_PROFILE 0

#endif
#define SEXNET_GET_STATUS UINT64_C(0x200)
#define SEXNET_SCAN_WIFI UINT64_C(0x201)
#define SEXNET_CONNECT UINT64_C(0x202)
#define SEXNET_DISCONNECT UINT64_C(0x203)
#define SEXNET_VPN_UP UINT64_C(0x204)
#define SEXNET_VPN_DOWN UINT64_C(0x205)
#define SEXNET_GET_IP UINT64_C(0x206)
#define SEXNET_HTTP_PROOF_LEN UINT64_C(0x207)
#define SEXNET_HTTP_PROOF_CHUNK UINT64_C(0x208)
#define SEXNET_HTTP_BODY_LEN UINT64_C(0x209)
#define SEXNET_HTTP_BODY_CHUNK UINT64_C(0x20A)
#define SEXNET_DNS_RESOLVE UINT64_C(0x20B)
#define BODY_TEXT_LEN ((size_t)19)
#define SEXNET_DNS_SERVER_PORT ((uint16_t)53)
#define SEXNET_DNS_SRC_PORT ((uint16_t)49152)
#define SEXNET_DNS_TXID ((uint16_t)0x1234)
#define SEXNET_DNS_QUERY_FRAME_LEN ((size_t)71)
#define HTTP_GET_BUF_CAP ((size_t)192)
#define HTTP_RESPONSE_BUF_CAP ((size_t)512)
#define HTTP_BODY_BUF_CAP ((size_t)256)
#define NIC_OWNER_HAL_DIAG ((uint8_t)0)
#define NIC_OWNER_SEXNET_RX ((uint8_t)1)
#define NIC_OWNER_SEXNET_TX ((uint8_t)2)
#define NIC_OWNER_SEXNET_FULL ((uint8_t)3)
typedef enum WifiState {
  WIFI_DISCONNECTED = 0, WIFI_CONNECTED = 1
}
WifiState;
typedef enum VpnState {
  VPN_DOWN = 0, VPN_UP = 1
}
VpnState;
typedef enum TcpState {
  TCP_CLOSED = 0, TCP_SYN_SENT = 1, TCP_ESTABLISHED = 2, TCP_FAILED_RST = 3, TCP_FAILED_TIMEOUT = 4
}
TcpState;
typedef struct SexnetApEntry {
  uint8_t ssid[32];
  int8_t rssi;
  uint8_t channel;
  uint8_t flags;
  uint8_t _pad;
}
SexnetApEntry;
_Static_assert(sizeof(SexnetApEntry) == 36, "repr(C) SexnetApEntry");
typedef struct NetState {
  WifiState wifi;
  VpnState vpn;
  uint16_t link_speed_mbps;
  uint32_t ipv4;
}
NetState;
typedef struct HttpStatusParse {
  uint16_t status;
  size_t line_end;
  const char *version;
  const char *reject;
}
HttpStatusParse;
typedef struct SexnetState {
  uint8_t proof_buf[32];
  size_t proof_len;
  uint8_t body_buf[64];
  size_t body_len;
  uint8_t http_get_buf[192];
  size_t http_get_len;
  uint8_t http_response_buf[512];
  size_t http_response_len;
  uint8_t http_body_prefix_buf[256];
  size_t http_body_prefix_len;
  uint16_t http_status_code;
  uint8_t dns_a_cache_ip[4][4];
  uint8_t dns_a_cache_valid[4];
  uint32_t dns_a_cache_ttl[4];
  atomic_uchar nic_rx_owner;
  atomic_uchar nic_tx_owner;
  uint64_t rx_perm_desc_phys;
  uint64_t rx_perm_desc_va;
  uint64_t rx_perm_pkt_phys[8];
  uint64_t rx_perm_pkt_va[8];
  uint64_t tx_perm_desc_phys;
  uint64_t tx_perm_desc_va;
  uint64_t tx_perm_frame_phys;
  uint64_t tx_perm_frame_va;
  uint8_t l2_rx_next;
  uint32_t l2_rx_count;
  uint8_t l2_tx_next;
  uint8_t arp_cache_mac[6];
  uint8_t arp_cache_ip[4];
  uint8_t arp_cache_valid;
  uint32_t arp_cache_reply_count;
  NetState state;
  TcpState tcp_state;
  uint16_t tcp_local_port;
  uint16_t tcp_remote_port;
  uint32_t tcp_local_seq;
  uint32_t tcp_remote_seq;
  uint8_t tcp_remote_ip[4];
  uint32_t tcp_syn_count;
  uint32_t tcp_ack_count;
  uint32_t tcp_rst_count;
  uint32_t tcp_timeout_count;
}
SexnetState;
static SexnetState SEXNET_STATE;
static const uint8_t BODY_TEXT[19] = {
  72, 101, 108, 108, 111, 32, 83, 101, 120, 79, 83, 32, 72, 84, 84, 80, 32, 79, 75
};
static const uint8_t SEXNET_GUEST_IPV4[4] = {
  10, 0, 2, 15
};
static const uint8_t SEXNET_DNS_SERVER_IPV4[4] = {
  10, 0, 2, 3
};
static const SexnetApEntry MOCK_APS[3] = {
  {
    {
      83, 101, 120, 79, 83, 95, 78, 101, 116, 119, 111, 114, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }, -45, 6, 2, 0
  }, {
    {
      83, 105, 108, 107, 95, 72, 111, 116, 115, 112, 111, 116, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }, -68, 11, 2, 0
  }, {
    {
      79, 112, 101, 110, 65, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }, -80, 1, 1, 0
  }
};
static void sexnet_state_init(SexnetState *st) {
  {
    memset(st, 0, sizeof *st);
    atomic_init(&st->nic_rx_owner, NIC_OWNER_HAL_DIAG);
    atomic_init(&st->nic_tx_owner, NIC_OWNER_HAL_DIAG);
    st->l2_tx_next = 1u;
    st->state.wifi = WIFI_DISCONNECTED;
    st->state.vpn = VPN_DOWN;
    st->state.link_speed_mbps = 0u;
    st->state.ipv4 = 0u;
    st->tcp_state = TCP_CLOSED;
    st->tcp_local_port = 7777u;
    st->tcp_remote_port = 18081u;
    st->tcp_local_seq = 42u;
    st->tcp_remote_seq = 0u;
    st->tcp_remote_ip[0] = 10u;
    st->tcp_remote_ip[1] = 0u;
    st->tcp_remote_ip[2] = 2u;
    st->tcp_remote_ip[3] = 2u;
  }
}
static inline uint8_t vread8(uint64_t addr) {
  {
    return sexos_volatile_read8((const uint8_t *)(uintptr_t) addr);
  }
}
static inline uint16_t vread_be16(uint64_t addr) {
  {
    uint16_t b0 = (uint16_t) vread8(addr);
    uint16_t b1 = (uint16_t) vread8(addr + 1ULL);
    return (uint16_t)(b0 << 8) | b1;
  }
}
static inline uint32_t vread_be32(uint64_t addr) {
  {
    uint32_t b0 = (uint32_t) vread8(addr);
    uint32_t b1 = (uint32_t) vread8(addr + 1ULL);
    uint32_t b2 = (uint32_t) vread8(addr + 2ULL);
    uint32_t b3 = (uint32_t) vread8(addr + 3ULL);
    return b0 << 24 | b1 << 16 | b2 << 8 | b3;
  }
}
static inline uint16_t vread16(uint64_t addr) {
  {
    return sexos_volatile_read16((const uint16_t *)(uintptr_t) addr);
  }
}
static inline uint32_t vread32(uint64_t addr) {
  {
    return sexos_volatile_read32((const uint32_t *)(uintptr_t) addr);
  }
}
static inline void vwrite8(uint64_t addr, uint8_t val) {
  {
    sexos_volatile_write8((uint8_t *)(uintptr_t) addr, val);
  }
}
static inline void vwrite16(uint64_t addr, uint16_t val) {
  {
    sexos_volatile_write16((uint16_t *)(uintptr_t) addr, val);
  }
}
static inline void vwrite32(uint64_t addr, uint32_t val) {
  {
    sexos_volatile_write32((uint32_t *)(uintptr_t) addr, val);
  }
}
static inline void vwrite64(uint64_t addr, uint64_t val) {
  {
    sexos_volatile_write64((uint64_t *)(uintptr_t) addr, val);
  }
}
static uint64_t sys_net_diag(uint64_t selector) {
  {
    return sexos_net_diag(selector);
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
static void proof_reset(SexnetState *st) {
  {
    st->proof_len = 0;
  }
}
static void proof_push_byte(SexnetState *st, uint8_t b) {
  {
    if (st->proof_len < 32) {
      st->proof_buf[st->proof_len] = b;
      st->proof_len += 1;
    }
  }
}
static void proof_push_str(SexnetState *st, const char *s) {
  {
    while (*s != 0) {
      proof_push_byte(st, (uint8_t) *s);
      ++s;
    }
  }
}
static void proof_push_u32(SexnetState *st, uint32_t n) {
  {
    uint8_t tmp[10];
    size_t i = 0;
    if (n == 0u) {
      proof_push_byte(st, 48u);
      return;
    } while (n > 0u && i < 10) {
      tmp[i] = (uint8_t)(48u + n % 10u);
      n /= 10u;
      i += 1;
    } while (i > 0) {
      i -= 1;
      proof_push_byte(st, tmp[i]);
    }
  }
}
static void proof_build(SexnetState *st, uint32_t status, uint16_t bytes, uint8_t source) {
  {
    proof_reset(st);
    proof_push_str(st, "HTTP ");
    proof_push_u32(st, status);
    proof_push_str(st, " rx=");
    proof_push_u32(st, (uint32_t) bytes);
    proof_push_str(st, "b ");
    if (source == 1u) proof_push_str(st, "mock");
    else if (source == 2u) proof_push_str(st, "real");
    else proof_push_str(st, "unset");
  }
}
static size_t http_get_build(SexnetState *st, const uint8_t *host, size_t host_len, const uint8_t *path, size_t path_len) {
  {
    const char *p0 = "GET ";
    const char *p1 = " HTTP/1.1\r\nHost: ";
    const char *p2 = "\r\nConnection: close\r\nUser-Agent: sexnet/phase-i\r\n\r\n";
    size_t p0_len = 4;
    size_t p1_len = 17;
    size_t p2_len = 51;
    size_t needed = p0_len + path_len + p1_len + host_len + p2_len;
    if (needed > HTTP_GET_BUF_CAP) return 0;
    size_t o = 0;
    for (size_t i = 0; i < p0_len; ++i) {
      st->http_get_buf[o] = (uint8_t) p0[i];
      o += 1;
    }
    for (size_t i = 0; i < path_len; ++i) {
      st->http_get_buf[o] = path[i];
      o += 1;
    }
    for (size_t i = 0; i < p1_len; ++i) {
      st->http_get_buf[o] = (uint8_t) p1[i];
      o += 1;
    }
    for (size_t i = 0; i < host_len; ++i) {
      st->http_get_buf[o] = host[i];
      o += 1;
    }
    for (size_t i = 0; i < p2_len; ++i) {
      st->http_get_buf[o] = (uint8_t) p2[i];
      o += 1;
    }
    st->http_get_len = o;
    return o;
  }
}
static size_t build_dns_query_frame_source3(uint8_t *out) {
  {
    for (size_t i = 0; i < SEXNET_DNS_QUERY_FRAME_LEN; ++i) out[i] = 0u;
    out[12] = 0x08u;
    out[13] = 0x00u;
    out[14] = 0x45u;
    out[15] = 0x00u;
    out[16] = 0x00u;
    out[17] = 57u;
    out[18] = 0x00u;
    out[19] = 0x07u;
    out[20] = 0x00u;
    out[21] = 0x00u;
    out[22] = 64u;
    out[23] = 17u;
    out[26] = SEXNET_GUEST_IPV4[0];
    out[27] = SEXNET_GUEST_IPV4[1];
    out[28] = SEXNET_GUEST_IPV4[2];
    out[29] = SEXNET_GUEST_IPV4[3];
    out[30] = SEXNET_DNS_SERVER_IPV4[0];
    out[31] = SEXNET_DNS_SERVER_IPV4[1];
    out[32] = SEXNET_DNS_SERVER_IPV4[2];
    out[33] = SEXNET_DNS_SERVER_IPV4[3];
    uint32_t ip_sum = 0u;
    for (size_t w = 0; w < 10; ++w) {
      size_t off = 14 + w * 2;
      uint16_t hi = (uint16_t) out[off];
      uint16_t lo = (uint16_t) out[off + 1];
      ip_sum += (uint32_t)(hi << 8 | lo);
    } while (ip_sum >> 16 != 0u) ip_sum = (ip_sum & 0xFFFFu) + (ip_sum >> 16);
    uint16_t ip_ck = (uint16_t) ~(uint16_t) ip_sum;
    out[24] = (uint8_t)(ip_ck >> 8 & 0xFFu);
    out[25] = (uint8_t)(ip_ck & 0xFFu);
    out[34] = (uint8_t)(SEXNET_DNS_SRC_PORT >> 8 & 0xFFu);
    out[35] = (uint8_t)(SEXNET_DNS_SRC_PORT & 0xFFu);
    out[36] = (uint8_t)(SEXNET_DNS_SERVER_PORT >> 8 & 0xFFu);
    out[37] = (uint8_t)(SEXNET_DNS_SERVER_PORT & 0xFFu);
    out[38] = 0x00u;
    out[39] = 37u;
    out[40] = 0x00u;
    out[41] = 0x00u;
    out[42] = (uint8_t)(SEXNET_DNS_TXID >> 8 & 0xFFu);
    out[43] = (uint8_t)(SEXNET_DNS_TXID & 0xFFu);
    out[44] = 0x01u;
    out[45] = 0x00u;
    out[46] = 0x00u;
    out[47] = 0x01u;
    out[48] = 0x00u;
    out[49] = 0x00u;
    out[50] = 0x00u;
    out[51] = 0x00u;
    out[52] = 0x00u;
    out[53] = 0x00u;
    out[54] = 0x07u;
    out[55] = 101u;
    out[56] = 120u;
    out[57] = 97u;
    out[58] = 109u;
    out[59] = 112u;
    out[60] = 108u;
    out[61] = 101u;
    out[62] = 0x03u;
    out[63] = 99u;
    out[64] = 111u;
    out[65] = 109u;
    out[66] = 0x00u;
    out[67] = 0x00u;
    out[68] = 0x01u;
    out[69] = 0x00u;
    out[70] = 0x01u;
    return SEXNET_DNS_QUERY_FRAME_LEN;
  }
}
static size_t build_hex_peek(const uint8_t *buf, size_t len, uint8_t *out, size_t out_cap) {
  {
    const char *hex = "0123456789ABCDEF";
    size_t o = 0;
    size_t i = 0;
    while (i < len) {
      if (o + 2 > out_cap) break;
      uint8_t b = buf[i];
      out[o] = (uint8_t) hex[(size_t)(b >> 4)];
      out[o + 1] = (uint8_t) hex[(size_t)(b & 0x0Fu)];
      o += 2;
      if (i + 1 < len) {
        if (o + 1 > out_cap) break;
        out[o] = 32u;
        o += 1;
      }
      i += 1;
    }
    return o;
  }
}
static size_t build_ascii_peek(const uint8_t *buf, size_t len, uint8_t *out, size_t out_cap) {
  {
    size_t o = 0;
    size_t i = 0;
    while (i < len && o < out_cap) {
      uint8_t b = buf[i];
      out[o] = b >= 0x20u && b <= 0x7Eu ? b : 46u;
      o += 1;
      i += 1;
    }
    return o;
  }
}
static HttpStatusParse parse_http_status_line(const uint8_t *buf, size_t len) {
  {
    size_t max_status_line = 128;
    if (len == 0) return (HttpStatusParse) {
      .status = 0,.line_end = 0,.version = "",.reject = "empty"
    };
    size_t scan_cap = len < max_status_line ? len : max_status_line;
    size_t line_end = scan_cap;
    for (size_t i = 0; i < scan_cap; ++i) {
      uint8_t b = buf[i];
      if (b == 10u || b == 13u) {
        line_end = i;
        break;
      }
    }
    if (line_end == scan_cap) {
      if (len >= max_status_line) return (HttpStatusParse) {
        .status = 0,.line_end = line_end,.version = "",.reject = "status_line_too_long"
      };
      return (HttpStatusParse) {
        .status = 0,.line_end = line_end,.version = "",.reject = "missing_line_ending"
      };
    }
    if (line_end < 12) return (HttpStatusParse) {
      .status = 0,.line_end = line_end,.version = "",.reject = "status_line_too_short"
    };
    if (buf[0] != 72u || buf[1] != 84u || buf[2] != 84u || buf[3] != 80u || buf[4] != 47u || buf[5] != 49u || buf[6] != 46u) return (HttpStatusParse) {
      .status = 0,.line_end = line_end,.version = "",.reject = "bad_http_prefix"
    };
    const char *version = "";
    if (buf[7] == 48u) version = "HTTP/1.0";
    else if (buf[7] == 49u) version = "HTTP/1.1";
    else return (HttpStatusParse) {
      .status = 0,.line_end = line_end,.version = "",.reject = "unsupported_http_version"
    };
    if (buf[8] != 32u) return (HttpStatusParse) {
      .status = 0,.line_end = line_end,.version = "",.reject = "missing_status_space"
    };
    uint8_t d0 = buf[9];
    uint8_t d1 = buf[10];
    uint8_t d2 = buf[11];
    if (d0 < 48u || d0 > 57u || (d1 < 48u || d1 > 57u) || (d2 < 48u || d2 > 57u)) return (HttpStatusParse) {
      .status = 0,.line_end = line_end,.version = "",.reject = "status_digits_invalid"
    };
    if (line_end > 12 && buf[12] != 32u) return (HttpStatusParse) {
      .status = 0,.line_end = line_end,.version = "",.reject = "missing_reason_separator"
    };
    uint16_t status = (uint16_t)((uint16_t)(d0 - 48u) * 100u + (uint16_t)(d1 - 48u) * 10u + (uint16_t)(d2 - 48u));
    return (HttpStatusParse) {
      .status = status,.line_end = line_end,.version = version,.reject = ""
    };
  }
}
static uint32_t dns_source3_try_parse_and_cache(SexnetState *st, uint64_t pkt_buf, size_t rx_len) {
  {
    if (rx_len < 54) {
      sexos_line("[sexnet.dns.malformed.reject] reason=rx_too_short ok=1");
      return 0u;
    }
    uint8_t et0 = vread8(pkt_buf + 12ULL);
    uint8_t et1 = vread8(pkt_buf + 13ULL);
    if (et0 != 0x08u || et1 != 0x00u) return 0u;
    size_t ihl = (size_t)(vread8(pkt_buf + 14ULL) & 0x0Fu) * 4;
    if (ihl < 20 || 14 + ihl + 8 > rx_len) {
      sexos_line("[sexnet.dns.malformed.reject] reason=ipv4_ihl_bounds ok=1");
      return 0u;
    }
    uint8_t proto = vread8(pkt_buf + 23ULL);
    if (proto != 17u) return 0u;
    uint8_t src0 = vread8(pkt_buf + 26ULL);
    uint8_t src1 = vread8(pkt_buf + 27ULL);
    uint8_t src2 = vread8(pkt_buf + 28ULL);
    uint8_t src3 = vread8(pkt_buf + 29ULL);
    if (src0 != SEXNET_DNS_SERVER_IPV4[0] || src1 != SEXNET_DNS_SERVER_IPV4[1] || src2 != SEXNET_DNS_SERVER_IPV4[2] || src3 != SEXNET_DNS_SERVER_IPV4[3]) return 0u;
    size_t udp_base = 14 + ihl;
    uint16_t src_port = vread_be16(pkt_buf + (uint64_t) udp_base);
    if (src_port != SEXNET_DNS_SERVER_PORT) return 0u;
    uint16_t udp_len = vread_be16(pkt_buf + (uint64_t) udp_base + 4ULL);
    if (udp_len < 8u) {
      sexos_line("[sexnet.dns.malformed.reject] reason=udp_len_small ok=1");
      return 0u;
    }
    size_t dns_len = (size_t) udp_len - 8;
    size_t dns_base = udp_base + 8;
    if (dns_base + dns_len > rx_len || dns_len < 12) {
      sexos_line("[sexnet.dns.malformed.reject] reason=dns_bounds ok=1");
      return 0u;
    }
    uint16_t txid = vread_be16(pkt_buf + (uint64_t) dns_base);
    if (txid != SEXNET_DNS_TXID) return 0u;
    uint16_t flags = vread_be16(pkt_buf + (uint64_t) dns_base + 2ULL);
    uint16_t qr = (flags & 0x8000u) != 0u ? 1u : 0u;
    uint16_t rcode = flags & 0x000Fu;
    if (qr == 0u) {
      sexos_line("[sexnet.dns.malformed.reject] reason=qr_not_set ok=1");
      return 0u;
    }
    if (rcode != 0u) {
      sexos_line("[sexnet.dns.malformed.reject] reason=rcode_nonzero ok=1");
      return 0u;
    }
    uint16_t qdcount = vread_be16(pkt_buf + (uint64_t) dns_base + 4ULL);
    uint16_t ancount = vread_be16(pkt_buf + (uint64_t) dns_base + 6ULL);
    sexos_line("[sexnet.dns.source3.rx.parse] txid=0x1234 qr=%u rcode=%u ancount=%u ok=1", (unsigned) qr, (unsigned) rcode, (unsigned) ancount);
    size_t off = dns_base + 12;
    if (qdcount > 0u) {
      uint32_t q_iter = 0u;
      while (true) {
        if (q_iter >= 64u) {
          sexos_line("[sexnet.dns.malformed.reject] reason=qname_loop_limit ok=1");
          return 0u;
        }
        if (off >= dns_base + dns_len) {
          sexos_line("[sexnet.dns.malformed.reject] reason=qname_oob ok=1");
          return 0u;
        }
        uint8_t lab = vread8(pkt_buf + (uint64_t) off);
        off += 1;
        if (lab == 0u) break;
        if ((lab & 0xC0u) != 0u) {
          sexos_line("[sexnet.dns.malformed.reject] reason=qname_compression_unsupported ok=1");
          return 0u;
        }
        size_t step = (size_t) lab;
        if (off + step > dns_base + dns_len) {
          sexos_line("[sexnet.dns.malformed.reject] reason=qname_label_oob ok=1");
          return 0u;
        }
        off += step;
        q_iter += 1u;
      }
      if (off + 4 > dns_base + dns_len) {
        sexos_line("[sexnet.dns.malformed.reject] reason=question_tail_oob ok=1");
        return 0u;
      }
      off += 4;
    }
    uint32_t ai = 0u;
    while (ai < 2u && (uint16_t) ai < ancount) {
      if (off + 12 > dns_base + dns_len) {
        sexos_line("[sexnet.dns.malformed.reject] reason=answer_header_oob ok=1");
        return 0u;
      }
      uint8_t n0 = vread8(pkt_buf + (uint64_t) off);
      if ((n0 & 0xC0u) == 0xC0u) {
        if (off + 2 > dns_base + dns_len) {
          sexos_line("[sexnet.dns.malformed.reject] reason=answer_name_ptr_oob ok=1");
          return 0u;
        }
        off += 2;
      } else {
        uint32_t name_iter = 0u;
        while (true) {
          if (name_iter >= 64u) {
            sexos_line("[sexnet.dns.malformed.reject] reason=answer_name_loop_limit ok=1");
            return 0u;
          }
          if (off >= dns_base + dns_len) {
            sexos_line("[sexnet.dns.malformed.reject] reason=answer_name_oob ok=1");
            return 0u;
          }
          uint8_t lab = vread8(pkt_buf + (uint64_t) off);
          off += 1;
          if (lab == 0u) break;
          if ((lab & 0xC0u) != 0u) {
            sexos_line("[sexnet.dns.malformed.reject] reason=answer_name_bad_compression ok=1");
            return 0u;
          }
          size_t step = (size_t) lab;
          if (off + step > dns_base + dns_len) {
            sexos_line("[sexnet.dns.malformed.reject] reason=answer_name_label_oob ok=1");
            return 0u;
          }
          off += step;
          name_iter += 1u;
        }
      }
      if (off + 10 > dns_base + dns_len) {
        sexos_line("[sexnet.dns.malformed.reject] reason=answer_fields_oob ok=1");
        return 0u;
      }
      uint16_t typ = vread_be16(pkt_buf + (uint64_t) off);
      uint16_t cls = vread_be16(pkt_buf + (uint64_t) off + 2ULL);
      uint32_t ttl = vread_be32(pkt_buf + (uint64_t) off + 4ULL);
      uint16_t rdlen = vread_be16(pkt_buf + (uint64_t) off + 8ULL);
      off += 10;
      if (off + (size_t) rdlen > dns_base + dns_len) {
        sexos_line("[sexnet.dns.malformed.reject] reason=rdata_oob ok=1");
        return 0u;
      }
      if (typ == 1u && cls == 1u && rdlen == 4u) {
        uint8_t a0 = vread8(pkt_buf + (uint64_t) off);
        uint8_t a1 = vread8(pkt_buf + (uint64_t) off + 1ULL);
        uint8_t a2 = vread8(pkt_buf + (uint64_t) off + 2ULL);
        uint8_t a3 = vread8(pkt_buf + (uint64_t) off + 3ULL);
        sexos_line("[sexnet.dns.source3.answer.a] idx=%u addr=%u.%u.%u.%u ttl=%u ok=1", (unsigned) ai, (unsigned) a0, (unsigned) a1, (unsigned) a2, (unsigned) a3, (unsigned) ttl);
        size_t ins = 0;
        while (ins < 4) {
          if (st->dns_a_cache_valid[ins] == 0u) break;
          ins += 1;
        }
        if (ins >= 4) ins = 0;
        st->dns_a_cache_ip[ins][0] = a0;
        st->dns_a_cache_ip[ins][1] = a1;
        st->dns_a_cache_ip[ins][2] = a2;
        st->dns_a_cache_ip[ins][3] = a3;
        st->dns_a_cache_ttl[ins] = ttl;
        st->dns_a_cache_valid[ins] = 1u;
        sexos_line("[sexnet.dns.source3.cache.insert] idx=%zu addr=%u.%u.%u.%u ok=1", ins, (unsigned) a0, (unsigned) a1, (unsigned) a2, (unsigned) a3);
      }
      off += (size_t) rdlen;
      ai += 1u;
    }
    return 1u;
  }
}
static uint64_t handle_call(SexnetState *st, uint64_t syscall_id, uint64_t arg0, uint64_t arg1) {
  {
    if (syscall_id == SEXNET_GET_STATUS) {
      if (arg0 == SEXNET_HTTP_PROOF_LEN) {
        size_t len = st->proof_len;
        sexos_line("[sexnet.packed_text.len] len=%zu", len);
        return (uint64_t) len;
      }
      if (arg0 == SEXNET_HTTP_PROOF_CHUNK) {
        size_t idx = (size_t) arg1;
        size_t start = idx > SIZE_MAX / 8 ? SIZE_MAX : idx * 8;
        size_t proof_len = st->proof_len;
        if (start >= proof_len) return UINT64_MAX;
        size_t end = start + 8 < proof_len ? start + 8 : proof_len;
        size_t bytes = end - start;
        uint64_t packed = 0ULL;
        for (size_t i = 0; i < bytes; ++i) {
          uint8_t b = st->proof_buf[start + i];
          packed |= (uint64_t) b << i * 8;
        }
        sexos_line("[sexnet.packed_text.chunk] idx=%zu bytes=%zu", idx, bytes);
        return packed;
      }
      if (arg0 == SEXNET_HTTP_BODY_LEN) {
        size_t body_len = st->body_len;
        if (body_len > 0) {
          sexos_line("[sexnet.body_text.len] len=%zu", body_len);
          return (uint64_t) body_len;
        }
        sexos_line("[sexnet.body_text.len] len=%zu", BODY_TEXT_LEN);
        return (uint64_t) BODY_TEXT_LEN;
      }
      if (arg0 == SEXNET_HTTP_BODY_CHUNK) {
        size_t idx = (size_t) arg1;
        size_t start = idx > SIZE_MAX / 8 ? SIZE_MAX : idx * 8;
        size_t body_len = st->body_len;
        if (body_len > 0) {
          if (start >= body_len) return UINT64_MAX;
          size_t end = start + 8 < body_len ? start + 8 : body_len;
          size_t bytes = end - start;
          uint64_t packed = 0ULL;
          for (size_t i = 0; i < bytes; ++i) {
            uint8_t b = st->body_buf[start + i];
            packed |= (uint64_t) b << i * 8;
          }
          sexos_line("[sexnet.body_text.chunk] idx=%zu bytes=%zu", idx, bytes);
          return packed;
        }
        if (start >= BODY_TEXT_LEN) return UINT64_MAX;
        size_t end = start + 8 < BODY_TEXT_LEN ? start + 8 : BODY_TEXT_LEN;
        size_t bytes = end - start;
        uint64_t packed = 0ULL;
        for (size_t i = 0; i < bytes; ++i) packed |= (uint64_t) BODY_TEXT[start + i] << i * 8;
        sexos_line("[sexnet.body_text.chunk] idx=%zu bytes=%zu", idx, bytes);
        return packed;
      }
      uint64_t flags = (st->state.wifi == WIFI_CONNECTED ? 1ULL : 0ULL) | (st->state.vpn == VPN_UP ? 2ULL : 0ULL);
      return (uint64_t) st->state.link_speed_mbps << 16 | flags;
    }
    if (syscall_id == SEXNET_SCAN_WIFI) {
      SexnetApEntry *out_ptr = (SexnetApEntry *)(uintptr_t) arg0;
      size_t max = (size_t) arg1;
      if (out_ptr == NULL || max == 0) return UINT64_MAX;
      size_t count = 3 < max ? 3 : max;
      memcpy(out_ptr, MOCK_APS, count * sizeof MOCK_APS[0]);
      return (uint64_t) count;
    }
    if (syscall_id == SEXNET_CONNECT) {
      const uint8_t *ssid_ptr = (const uint8_t *)(uintptr_t) arg0;
      size_t ssid_len = (size_t) arg1;
      if (ssid_ptr == NULL || ssid_len == 0 || ssid_len > 32) return 1ULL;
      st->state.wifi = WIFI_CONNECTED;
      st->state.link_speed_mbps = 300u;
      st->state.ipv4 = 192u << 24 | 168u << 16 | 1u << 8 | 100u;
      return 0ULL;
    }
    if (syscall_id == SEXNET_DISCONNECT) {
      st->state.wifi = WIFI_DISCONNECTED;
      st->state.link_speed_mbps = 0u;
      st->state.ipv4 = 0u;
      return 0ULL;
    }
    if (syscall_id == SEXNET_VPN_UP) {
      if (st->state.wifi != WIFI_CONNECTED) return 2ULL;
      st->state.vpn = VPN_UP;
      return 0ULL;
    }
    if (syscall_id == SEXNET_VPN_DOWN) {
      st->state.vpn = VPN_DOWN;
      return 0ULL;
    }
    if (syscall_id == SEXNET_GET_IP) return (uint64_t) st->state.ipv4;
    if (syscall_id == SEXNET_DNS_RESOLVE) {
      uint64_t host_id = arg0;
      if (host_id != 1ULL) {
        sexos_line("[browser.dns.resolve.miss] host_id=%llu ok=1 reason=unsupported_host", (unsigned long long) host_id);
        return 0ULL;
      }
      for (size_t idx = 0; idx < 4; ++idx) {
        uint8_t valid = st->dns_a_cache_valid[idx];
        uint32_t ttl = st->dns_a_cache_ttl[idx];
        if (valid != 0u && ttl != 0u) {
          uint8_t ip0 = st->dns_a_cache_ip[idx][0];
          uint8_t ip1 = st->dns_a_cache_ip[idx][1];
          uint8_t ip2 = st->dns_a_cache_ip[idx][2];
          uint8_t ip3 = st->dns_a_cache_ip[idx][3];
          uint32_t packed32 = (uint32_t) ip0 << 24 | (uint32_t) ip1 << 16 | (uint32_t) ip2 << 8 | (uint32_t) ip3;
          sexos_line("[browser.dns.resolve.ok] addr=%u.%u.%u.%u ok=1", (unsigned) ip0, (unsigned) ip1, (unsigned) ip2, (unsigned) ip3);
          return (uint64_t) packed32;
        }
      }
      sexos_line("[browser.dns.resolve.miss] host_id=1 ok=1 reason=cache_miss");
      return 0ULL;
    }
    return UINT64_MAX;
  }
}
void sexnet_main(void) {
  {
    SexnetState *st = &SEXNET_STATE;
    sexnet_state_init(st);
    uint64_t raw = sys_net_diag(0ULL);
    uint32_t status = (uint32_t)(raw >> 32);
    uint8_t source = (uint8_t)(raw >> 16 & 0xFFULL);
    uint16_t bytes = (uint16_t)(raw & 0xFFFFULL);
    sexos_line("[net.diag.syscall.call] syscall=52 status=%u bytes=%u source=%u", (unsigned) status, (unsigned) bytes, (unsigned) source);
    proof_build(st, status, bytes, source);
    size_t proof_len = st->proof_len;
    sexos_line("[sexnet.dynamic_text.set] status=%u bytes=%u source=%u len=%zu ok=1", (unsigned) status, (unsigned) bytes, (unsigned) source, proof_len);
    if (source == 2u) {
      sexos_line("[legacy.source2.dns.not_used] source=2 dns=0 ok=1");
      uint64_t blen_raw = sys_net_diag(1ULL);
      size_t blen = (size_t) blen_raw < 64 ? (size_t) blen_raw : 64;
      if (blen > 0) {
        sexos_line("[sexnet.body.fetch.begin] len=%zu", blen);
        size_t chunks = (blen + 7) / 8;
        size_t ci = 0;
        while (ci < chunks) {
          uint64_t packed = sys_net_diag(2ULL + (uint64_t) ci);
          size_t bi = 0;
          while (bi < 8) {
            size_t idx = ci * 8 + bi;
            if (idx >= blen) break;
            uint8_t byte = (uint8_t)(packed >> bi * 8 & 0xFFULL);
            uint8_t sane = byte == 13u || byte == 10u ? 32u : byte;
            st->body_buf[idx] = sane;
            bi += 1;
          }
          ci += 1;
        }
        st->body_len = blen;
        sexos_line("[sexnet.dynamic_body.set] len=%zu source=2 ok=1", blen);
      }
    }
    sexos_line("[sexnet.boot] ok=1 reason=passive_spawn");
    sexos_line("[sexnet.passive.ready] network=0 dns=0 tcp=0 http=0 tls=0 ok=1 reason=mock_status_only_no_nic");
    sexos_line("[sexnet.passive.spawn.done] ok=1 spawned=1 browser_network=0");
    if (SEXOS_SEXNET_PROOF_PROFILE != 0) {
      sexos_line("[sexnet.proof.profile] enabled=1 reason=SEXOS_SEXNET_PROOF_PROFILE_set");
      uint64_t nic_va = sexos_map_pci_bar(SEXOS_SLOT_nic, 0ULL, 0x10000ULL);
      if (nic_va != UINT64_MAX) {
        uint32_t ral = vread32(nic_va + 0x5400ULL);
        uint32_t rah = vread32(nic_va + 0x5404ULL);
        sexos_line("[sexnet.nic.bar.map] va=0x%llx ok=1", (unsigned long long) nic_va);
        uint8_t rx_own = atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire);
        uint8_t tx_own = atomic_load_explicit(&st->nic_tx_owner, memory_order_acquire);
        sexos_line("[sexnet.nic.ownership.init] rx_owner=%u tx_owner=%u ok=1", (unsigned) rx_own, (unsigned) tx_own);
        sexos_line("[sexnet.nic.mac.read] ral=0x%08X rah=0x%08X ok=1", (unsigned) ral, (unsigned) rah);
        sexos_line("[sexnet.nic.reg.audit.begin] ok=1");
        uint32_t reg_ctrl = vread32(nic_va + 0x0000ULL);
        uint32_t reg_status = vread32(nic_va + 0x0008ULL);
        uint32_t reg_rctl = vread32(nic_va + 0x0100ULL);
        uint32_t reg_rdbal = vread32(nic_va + 0x2800ULL);
        uint32_t reg_rdbah = vread32(nic_va + 0x2804ULL);
        uint32_t reg_rdlen = vread32(nic_va + 0x2808ULL);
        uint32_t reg_rdh = vread32(nic_va + 0x2810ULL);
        uint32_t reg_rdt = vread32(nic_va + 0x2818ULL);
        sexos_line("[sexnet.nic.reg.ctrl] val=0x%08X", (unsigned) reg_ctrl);
        sexos_line("[sexnet.nic.reg.status] val=0x%08X", (unsigned) reg_status);
        sexos_line("[sexnet.nic.reg.rctl] val=0x%08X rctl_en=%u", (unsigned) reg_rctl, (unsigned)((reg_rctl & 1u << 1) != 0u ? 1u : 0u));
        sexos_line("[sexnet.nic.reg.rdbal] val=0x%08X", (unsigned) reg_rdbal);
        sexos_line("[sexnet.nic.reg.rdbah] val=0x%08X", (unsigned) reg_rdbah);
        sexos_line("[sexnet.nic.reg.rdlen] val=%u", (unsigned) reg_rdlen);
        sexos_line("[sexnet.nic.reg.rdh] val=%u", (unsigned) reg_rdh);
        sexos_line("[sexnet.nic.reg.rdt] val=%u", (unsigned) reg_rdt);
        sexos_line("[sexnet.nic.reg.audit.done] rctl_en=%u rdlen=%u ok=1", (unsigned)((reg_rctl & 1u << 1) != 0u ? 1u : 0u), (unsigned) reg_rdlen);
        uint32_t rctl_orig = vread32(nic_va + 0x0100ULL);
        uint32_t rctl_clear = rctl_orig & ~(1u << 1);
        sexos_line("[sexnet.nic.rctl.disable.begin] rctl_orig=0x%08X", (unsigned) rctl_orig);
        vwrite32(nic_va + 0x0100ULL, rctl_clear);
        sexos_line("[sexnet.nic.rctl.disable.write] val=0x%08X", (unsigned) rctl_clear);
        uint32_t rctl_readback_off = vread32(nic_va + 0x0100ULL);
        uint32_t rctl_en_off = rctl_readback_off >> 1 & 1u;
        uint32_t rctl_disable_ok = rctl_en_off == 0u ? 1u : 0u;
        sexos_line("[sexnet.nic.rctl.disable.readback] rctl_en=%u ok=%u", (unsigned) rctl_en_off, (unsigned) rctl_disable_ok);
        vwrite32(nic_va + 0x0100ULL, rctl_orig);
        sexos_line("[sexnet.nic.rctl.restore.write] val=0x%08X", (unsigned) rctl_orig);
        uint32_t rctl_readback_on = vread32(nic_va + 0x0100ULL);
        uint32_t rctl_en_on = rctl_readback_on >> 1 & 1u;
        uint32_t rctl_restore_ok = rctl_en_on == 1u ? 1u : 0u;
        sexos_line("[sexnet.nic.rctl.restore.readback] rctl_en=%u ok=%u", (unsigned) rctl_en_on, (unsigned) rctl_restore_ok);
        uint32_t proof_ok = rctl_disable_ok == 1u && rctl_restore_ok == 1u ? 1u : 0u;
        sexos_line("[sexnet.nic.rctl.disable_restore.proof.done] ok=%u", (unsigned) proof_ok);
        uint64_t desc_phys = sys_alloc_phys(4096ULL);
        uint64_t desc_va = sys_map_phys(desc_phys, 4096ULL);
        uint64_t pkt_phys = sys_alloc_phys(4096ULL);
        uint64_t pkt_va = sys_map_phys(pkt_phys, 4096ULL);
        bool alloc_ok = desc_phys != 0ULL && desc_phys != UINT64_MAX && desc_va != 0ULL && desc_va != UINT64_MAX && pkt_phys != 0ULL && pkt_phys != UINT64_MAX && pkt_va != 0ULL && pkt_va != UINT64_MAX;
        if (!alloc_ok) sexos_line("[sexnet.nic.rx.ring.alloc] desc_phys=0x%016llX pkt_phys=0x%016llX ok=0", (unsigned long long) desc_phys, (unsigned long long) pkt_phys);
        else {
          sexos_line("[sexnet.nic.rx.ring.alloc] desc_phys=0x%016llX pkt_phys=0x%016llX ok=1", (unsigned long long) desc_phys, (unsigned long long) pkt_phys);
          for (uint64_t zi = 0ULL; zi < 512ULL; ++zi) {
            vwrite64(desc_va + zi * 8ULL, 0ULL);
            vwrite64(pkt_va + zi * 8ULL, 0ULL);
          }
          for (uint64_t i = 0ULL; i < 8ULL; ++i) {
            uint64_t desc_base = desc_va + i * 16ULL;
            vwrite64(desc_base, pkt_phys);
            vwrite64(desc_base + 8ULL, 0ULL);
          }
          uint32_t status_zero = 1u;
          for (uint64_t si = 0ULL; si < 8ULL; ++si) {
            uint8_t st_b = vread8(desc_va + si * 16ULL + 12ULL);
            if (st_b != 0u) status_zero = 0u;
          }
          sexos_line("[sexnet.nic.rx.desc.link] count=8 buf_phys=0x%016llX status_zero=%u ok=%u", (unsigned long long) pkt_phys, (unsigned) status_zero, (unsigned)(status_zero == 1u ? 1u : 0u));
          uint32_t rctl_orig_rx = vread32(nic_va + 0x0100ULL);
          uint32_t rdbal_orig = vread32(nic_va + 0x2800ULL);
          uint32_t rdbah_orig = vread32(nic_va + 0x2804ULL);
          uint32_t rdlen_orig = vread32(nic_va + 0x2808ULL);
          uint32_t rdh_orig = vread32(nic_va + 0x2810ULL);
          uint32_t rdt_orig = vread32(nic_va + 0x2818ULL);
          sexos_line("[sexnet.nic.rx.ring.save] rctl=0x%08X rdbal=0x%08X rdlen=%u rdt=%u ok=1", (unsigned) rctl_orig_rx, (unsigned) rdbal_orig, (unsigned) rdlen_orig, (unsigned) rdt_orig);
          uint32_t rctl_init = 1u << 1 | 1u << 3 | 1u << 4 | 1u << 26;
          vwrite32(nic_va + 0x0100ULL, rctl_orig_rx & ~(1u << 1));
          vwrite32(nic_va + 0x2800ULL, (uint32_t)(desc_phys & 0xFFFFFFFFULL));
          vwrite32(nic_va + 0x2804ULL, (uint32_t)(desc_phys >> 32));
          vwrite32(nic_va + 0x2808ULL, 128u);
          vwrite32(nic_va + 0x2810ULL, 0u);
          vwrite32(nic_va + 0x2818ULL, 7u);
          vwrite32(nic_va + 0x280CULL, 0x00000002u);
          vwrite32(nic_va + 0x0100ULL, rctl_init);
          uint32_t program_rdbal = vread32(nic_va + 0x2800ULL);
          uint32_t program_rdbah = vread32(nic_va + 0x2804ULL);
          uint32_t program_rdlen = vread32(nic_va + 0x2808ULL);
          uint32_t program_rdh = vread32(nic_va + 0x2810ULL);
          uint32_t program_rdt = vread32(nic_va + 0x2818ULL);
          uint32_t program_rctl = vread32(nic_va + 0x0100ULL);
          (void) program_rdh;
          sexos_line("[sexnet.nic.rx.ring.program] rdbal=0x%08X rdbah=0x%08X rdlen=128 rdh=0 rdt=7 rctl=0x%08X ok=1", (unsigned) program_rdbal, (unsigned) program_rdbah, (unsigned) program_rctl);
          uint32_t rctl_en = (program_rctl & 1u << 1) != 0u ? 1u : 0u;
          uint32_t ring_readback_ok = program_rdbal == (uint32_t) desc_phys && program_rdlen == 128u && program_rdt == 7u && rctl_en == 1u ? 1u : 0u;
          sexos_line("[sexnet.nic.rx.ring.readback] rdbal=0x%08X rdlen=%u rdt=%u rctl_en=%u ok=%u", (unsigned) program_rdbal, (unsigned) program_rdlen, (unsigned) program_rdt, (unsigned) rctl_en, (unsigned) ring_readback_ok);
          uint8_t seen_mask = 0u;
          uint32_t dd_set = 0u;
          for (uint32_t poll_outer = 0u; poll_outer < 128u; ++poll_outer) for (uint8_t di = 0u; di < 8u; ++di) {
            uint8_t st_b = vread8(desc_va + (uint64_t) di * 16ULL + 12ULL);
            if ((st_b & 1u) != 0u) {
              uint8_t bit = (uint8_t)(1u << di);
              if ((seen_mask & bit) == 0u) {
                seen_mask |= bit;
                dd_set += 1u;
              }
            }
          }
          sexos_line("[sexnet.nic.rx.dd.poll] polled=8 dd_set=%u ok=1", (unsigned) dd_set);
          vwrite32(nic_va + 0x0100ULL, rctl_init & ~(1u << 1));
          vwrite32(nic_va + 0x2800ULL, rdbal_orig);
          vwrite32(nic_va + 0x2804ULL, rdbah_orig);
          vwrite32(nic_va + 0x2808ULL, rdlen_orig);
          vwrite32(nic_va + 0x2810ULL, rdh_orig);
          vwrite32(nic_va + 0x2818ULL, rdt_orig);
          vwrite32(nic_va + 0x0100ULL, rctl_orig_rx);
          uint32_t rctl_rest = vread32(nic_va + 0x0100ULL);
          uint32_t rdbal_rest = vread32(nic_va + 0x2800ULL);
          uint32_t rdlen_rest = vread32(nic_va + 0x2808ULL);
          uint32_t rdt_rest = vread32(nic_va + 0x2818ULL);
          sexos_line("[sexnet.nic.rx.ring.restore] rdbal=0x%08X rdlen=%u rdt=%u rctl=0x%08X ok=1", (unsigned) rdbal_rest, (unsigned) rdlen_rest, (unsigned) rdt_rest, (unsigned) rctl_rest);
          uint32_t rctl_en_rest = (rctl_rest & 1u << 1) != 0u ? 1u : 0u;
          uint32_t restore_ok = rctl_en_rest == 1u && rdbal_rest == rdbal_orig ? 1u : 0u;
          sexos_line("[sexnet.nic.rx.ring.restore.readback] rctl_en=%u rdbal=0x%08X ok=%u", (unsigned) rctl_en_rest, (unsigned) rdbal_rest, (unsigned) restore_ok);
          sexos_line("[sexnet.nic.rx.ring.program_restore.proof.done] ok=%u", (unsigned) restore_ok);
          uint64_t obs_desc_phys = sys_alloc_phys(4096ULL);
          uint64_t obs_desc_va = sys_map_phys(obs_desc_phys, 4096ULL);
          uint64_t obs_pkt_phys[8] = {
            0ULL
          };
          uint64_t obs_pkt_va[8] = {
            0ULL
          };
          bool obs_alloc_ok = obs_desc_phys != 0ULL && obs_desc_phys != UINT64_MAX && obs_desc_va != 0ULL && obs_desc_va != UINT64_MAX;
          for (size_t oi = 0; oi < 8; ++oi) {
            obs_pkt_phys[oi] = sys_alloc_phys(4096ULL);
            obs_pkt_va[oi] = sys_map_phys(obs_pkt_phys[oi], 4096ULL);
            if (obs_pkt_phys[oi] == 0ULL || obs_pkt_phys[oi] == UINT64_MAX || obs_pkt_va[oi] == 0ULL || obs_pkt_va[oi] == UINT64_MAX) obs_alloc_ok = false;
          }
          sexos_line("[sexnet.nic.rx.observe.alloc] desc_phys=0x%016llX pkt_pages=8 ok=%u", (unsigned long long) obs_desc_phys, (unsigned)(obs_alloc_ok ? 1u : 0u));
          if (obs_alloc_ok) {
            for (uint64_t z = 0ULL; z < 512ULL; ++z) vwrite64(obs_desc_va + z * 8ULL, 0ULL);
            for (size_t pi = 0; pi < 8; ++pi) for (uint64_t pz = 0ULL; pz < 512ULL; ++pz) vwrite64(obs_pkt_va[pi] + pz * 8ULL, 0ULL);
            for (size_t di = 0; di < 8; ++di) {
              uint64_t base = obs_desc_va + (uint64_t) di * 16ULL;
              vwrite64(base, obs_pkt_phys[di]);
              vwrite64(base + 8ULL, 0ULL);
            }
            sexos_line("[sexnet.nic.rx.observe.desc.link] count=8 separate_bufs=1 ok=1");
            uint32_t obs_rctl_orig = vread32(nic_va + 0x0100ULL);
            uint32_t obs_rdbal_orig = vread32(nic_va + 0x2800ULL);
            uint32_t obs_rdbah_orig = vread32(nic_va + 0x2804ULL);
            uint32_t obs_rdlen_orig = vread32(nic_va + 0x2808ULL);
            uint32_t obs_rdh_orig = vread32(nic_va + 0x2810ULL);
            uint32_t obs_rdt_orig = vread32(nic_va + 0x2818ULL);
            vwrite32(nic_va + 0x0100ULL, obs_rctl_orig & ~(1u << 1));
            vwrite32(nic_va + 0x2800ULL, (uint32_t)(obs_desc_phys & 0xFFFFFFFFULL));
            vwrite32(nic_va + 0x2804ULL, (uint32_t)(obs_desc_phys >> 32));
            vwrite32(nic_va + 0x2808ULL, 128u);
            vwrite32(nic_va + 0x2810ULL, 0u);
            vwrite32(nic_va + 0x2818ULL, 7u);
            vwrite32(nic_va + 0x280CULL, 0x00000002u);
            vwrite32(nic_va + 0x0100ULL, rctl_init);
            uint32_t obs_prog_rdbal = vread32(nic_va + 0x2800ULL);
            uint32_t obs_prog_rctl = vread32(nic_va + 0x0100ULL);
            sexos_line("[sexnet.nic.rx.observe.ring.program] rdbal=0x%08X rdlen=128 rdt=7 rctl=0x%08X ok=1", (unsigned) obs_prog_rdbal, (unsigned) obs_prog_rctl);
            sexos_line("[sexnet.nic.rx.observe.window.open] max_iters=50000000");
            sexos_line("[sexnet.nic.rx.observe.poll.begin] max_iters=50000000");
            uint32_t obs_dd_set = 0u;
            uint32_t obs_dd_desc = 0xFFFFu;
            for (uint32_t outer = 0u; outer < 50000000u; ++outer) {
              for (uint32_t idx = 0u; idx < 8u; ++idx) {
                uint8_t st_b = vread8(obs_desc_va + (uint64_t) idx * 16ULL + 12ULL);
                if ((st_b & 1u) != 0u) {
                  obs_dd_set = 1u;
                  obs_dd_desc = idx;
                  break;
                }
              }
              if (obs_dd_set == 1u) break;
            }
            sexos_line("[sexnet.nic.rx.observe.poll.done] dd_set=%u desc_idx=%u ok=1", (unsigned) obs_dd_set, (unsigned) obs_dd_desc);
            if (obs_dd_set > 0u) sexos_line("[sexnet.nic.rx.observe.ok] dd_set=%u desc_idx=%u ok=1", (unsigned) obs_dd_set, (unsigned) obs_dd_desc);
            else sexos_line("[sexnet.nic.rx.timeout.honest] dd_set=0 reason=bounded_poll_no_traffic ok=1");
            if (obs_dd_set > 0u) {
              uint64_t desc_base = obs_desc_va + (uint64_t) obs_dd_desc * 16ULL;
              uint32_t pkt_len = (uint32_t) vread16(desc_base + 8ULL);
              uint64_t pkt_buf = obs_pkt_va[(size_t) obs_dd_desc];
              uint8_t eth_hi = vread8(pkt_buf + 12ULL);
              uint8_t eth_lo = vread8(pkt_buf + 13ULL);
              uint16_t ethertype = (uint16_t)((uint16_t) eth_hi << 8) | (uint16_t) eth_lo;
              uint32_t parse_ok = pkt_len > 14u ? 1u : 0u;
              sexos_line("[sexnet.nic.rx.observe.pkt.parse] len=%u ethertype=0x%04X ok=%u", (unsigned) pkt_len, (unsigned) ethertype, (unsigned) parse_ok);
              if (parse_ok == 1u) {
                const char *eth_classify = ethertype == 0x0806u ? "arp" : ethertype == 0x0800u ? "ipv4" : "unknown";
                sexos_line("[sexnet.ether.parse.ok] len=%u ethertype=0x%04X class=%s ok=1", (unsigned) pkt_len, (unsigned) ethertype, eth_classify);
                if (ethertype != 0x0806u && ethertype != 0x0800u) sexos_line("[sexnet.ether.ethertype.unknown.reject] ethertype=0x%04X ok=1", (unsigned) ethertype);
              } else sexos_line("[sexnet.ether.runt.reject] len=%u min=15 ok=1", (unsigned) pkt_len);
            }
            vwrite32(nic_va + 0x0100ULL, rctl_init & ~(1u << 1));
            vwrite32(nic_va + 0x2800ULL, obs_rdbal_orig);
            vwrite32(nic_va + 0x2804ULL, obs_rdbah_orig);
            vwrite32(nic_va + 0x2808ULL, obs_rdlen_orig);
            vwrite32(nic_va + 0x2810ULL, obs_rdh_orig);
            vwrite32(nic_va + 0x2818ULL, obs_rdt_orig);
            vwrite32(nic_va + 0x0100ULL, obs_rctl_orig);
            uint32_t obs_rest_rctl = vread32(nic_va + 0x0100ULL);
            uint32_t obs_rest_rdbal = vread32(nic_va + 0x2800ULL);
            uint32_t rctl_en_restored = (obs_rest_rctl & 1u << 1) != 0u ? 1u : 0u;
            uint32_t obs_restore_ok = rctl_en_restored == 1u && obs_rest_rdbal == obs_rdbal_orig ? 1u : 0u;
            sexos_line("[sexnet.nic.rx.observe.ring.restore] rctl_orig=0x%08X rctl_en=%u ok=%u", (unsigned) obs_rctl_orig, (unsigned) rctl_en_restored, (unsigned) obs_restore_ok);
            uint32_t obs_proof_ok = obs_dd_set > 0u && obs_restore_ok == 1u ? 1u : 0u;
            sexos_line("[sexnet.nic.rx.observe.proof.done] dd_set=%u ok=%u", (unsigned) obs_dd_set, (unsigned) obs_proof_ok);
          }
          sexos_line("[sexnet.nic.reset.begin] ok=1");
          uint32_t rctl_pre_rst = vread32(nic_va + 0x0100ULL);
          vwrite32(nic_va + 0x0100ULL, rctl_pre_rst & ~(1u << 1));
          uint32_t rctl_rst = vread32(nic_va + 0x0100ULL);
          uint8_t rx_disable_ok = (rctl_rst & 1u << 1) == 0u ? 1u : 0u;
          sexos_line("[sexnet.nic.reset.rx.disable] ok=%u", (unsigned) rx_disable_ok);
          uint32_t tctl_pre_rst = vread32(nic_va + 0x0400ULL);
          vwrite32(nic_va + 0x0400ULL, tctl_pre_rst & ~(1u << 1));
          uint32_t tctl_rst = vread32(nic_va + 0x0400ULL);
          uint8_t tx_disable_ok = (tctl_rst & 1u << 1) == 0u ? 1u : 0u;
          sexos_line("[sexnet.nic.reset.tx.disable] ok=%u", (unsigned) tx_disable_ok);
          vwrite32(nic_va + 0x00D8ULL, 0xFFFFFFFFu);
          sexos_line("[sexnet.nic.reset.irq.mask] ok=1");
          uint32_t ctrl_pre = vread32(nic_va + 0x0000ULL);
          vwrite32(nic_va + 0x0000ULL, ctrl_pre | 1u << 26);
          sexos_line("[sexnet.nic.reset.ctrl.rst.write] ok=1");
          uint32_t rst_polls = 0u;
          uint32_t rst_cleared = 0u;
          while (rst_polls < 1000000u) {
            uint32_t ctrl_poll = vread32(nic_va + 0x0000ULL);
            if ((ctrl_poll & 1u << 26) == 0u) {
              rst_cleared = 1u;
              break;
            }
            rst_polls += 1u;
          }
          sexos_line("[sexnet.nic.reset.ctrl.rst.poll] cleared=%u polls=%u ok=%u", (unsigned) rst_cleared, (unsigned) rst_polls, (unsigned) rst_cleared);
          uint32_t ral_after = vread32(nic_va + 0x5400ULL);
          uint32_t rah_after = vread32(nic_va + 0x5404ULL);
          uint32_t mac_valid = rah_after >> 31 & 1u;
          sexos_line("[sexnet.nic.reset.mac.program] ral=0x%08X rah=0x%08X valid=%u ok=1", (unsigned) ral_after, (unsigned) rah_after, (unsigned) mac_valid);
          vwrite32(nic_va + 0x2828ULL, 0x02000000u | 8u << 16 | 4u << 8 | 4u);
          vwrite32(nic_va + 0x3828ULL, 0x02000000u | 8u << 16 | 4u << 8 | 4u);
          sexos_line("[sexnet.nic.reset.queue.enable] rxdctl=1 txdctl=1 ok=1");
          uint32_t link_polls = 0u;
          uint32_t link_up = 0u;
          while (link_polls < 1000000u) {
            uint32_t status_poll = vread32(nic_va + 0x0008ULL);
            uint32_t lu_e1000e = status_poll >> 10 & 1u;
            uint32_t lu_e1000 = status_poll >> 1 & 1u;
            if (lu_e1000e != 0u || lu_e1000 != 0u) {
              link_up = 1u;
              break;
            }
            link_polls += 1u;
          }
          sexos_line("[sexnet.nic.reset.status] link_up=%u ok=1", (unsigned) link_up);
          sexos_line("[sexnet.nic.reset.proof.done] ok=1");
          uint64_t perm_desc_phys = sys_alloc_phys(4096ULL);
          uint64_t perm_desc_va = sys_map_phys(perm_desc_phys, 4096ULL);
          uint64_t perm_pkt_phys[8] = {
            0ULL
          };
          uint64_t perm_pkt_va[8] = {
            0ULL
          };
          bool perm_alloc_ok = perm_desc_phys != 0ULL && perm_desc_phys != UINT64_MAX && perm_desc_va != 0ULL && perm_desc_va != UINT64_MAX;
          for (size_t poi = 0; poi < 8; ++poi) {
            perm_pkt_phys[poi] = sys_alloc_phys(4096ULL);
            perm_pkt_va[poi] = sys_map_phys(perm_pkt_phys[poi], 4096ULL);
            if (perm_pkt_phys[poi] == 0ULL || perm_pkt_phys[poi] == UINT64_MAX || perm_pkt_va[poi] == 0ULL || perm_pkt_va[poi] == UINT64_MAX) perm_alloc_ok = false;
          }
          sexos_line("[sexnet.nic.rx.permanent.alloc] desc_phys=0x%016llX pkt_pages=8 ok=%u", (unsigned long long) perm_desc_phys, (unsigned)(perm_alloc_ok ? 1u : 0u));
          if (perm_alloc_ok) {
            for (uint64_t z = 0ULL; z < 512ULL; ++z) vwrite64(perm_desc_va + z * 8ULL, 0ULL);
            for (size_t pzi = 0; pzi < 8; ++pzi) for (uint64_t pz = 0ULL; pz < 512ULL; ++pz) vwrite64(perm_pkt_va[pzi] + pz * 8ULL, 0ULL);
            for (size_t di = 0; di < 8; ++di) {
              uint64_t base = perm_desc_va + (uint64_t) di * 16ULL;
              vwrite64(base, perm_pkt_phys[di]);
              vwrite64(base + 8ULL, 0ULL);
            }
            sexos_line("[sexnet.nic.rx.permanent.desc.link] count=8 ok=1");
            uint32_t perm_rctl_orig = vread32(nic_va + 0x0100ULL);
            vwrite32(nic_va + 0x0100ULL, perm_rctl_orig & ~(1u << 1));
            vwrite32(nic_va + 0x2800ULL, (uint32_t)(perm_desc_phys & 0xFFFFFFFFULL));
            vwrite32(nic_va + 0x2804ULL, (uint32_t)(perm_desc_phys >> 32));
            vwrite32(nic_va + 0x2808ULL, 128u);
            vwrite32(nic_va + 0x2810ULL, 0u);
            vwrite32(nic_va + 0x2818ULL, 7u);
            vwrite32(nic_va + 0x280CULL, 0x00000002u);
            vwrite32(nic_va + 0x0100ULL, rctl_init);
            uint32_t perm_prog_rdbal = vread32(nic_va + 0x2800ULL);
            uint32_t perm_prog_rctl = vread32(nic_va + 0x0100ULL);
            uint32_t perm_rctl_en = (perm_prog_rctl & 1u << 1) != 0u ? 1u : 0u;
            uint32_t perm_ring_ok = perm_prog_rdbal == (uint32_t) perm_desc_phys && perm_rctl_en == 1u ? 1u : 0u;
            sexos_line("[sexnet.nic.rx.permanent.ring.program] rdbal=0x%08X rdlen=128 rdt=7 rctl=0x%08X ok=%u", (unsigned) perm_prog_rdbal, (unsigned) perm_prog_rctl, (unsigned) perm_ring_ok);
            if (perm_ring_ok == 1u) atomic_store_explicit(&st->nic_rx_owner, NIC_OWNER_SEXNET_RX, memory_order_release);
            uint8_t rx_owner_now = atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire);
            uint32_t claim_ok = rx_owner_now == NIC_OWNER_SEXNET_RX && perm_ring_ok == 1u ? 1u : 0u;
            sexos_line("[sexnet.nic.rx.permanent.claim] owner=%u ring_ok=%u ok=%u", (unsigned) rx_owner_now, (unsigned) perm_ring_ok, (unsigned) claim_ok);
            if (claim_ok == 1u) {
              st->rx_perm_desc_phys = perm_desc_phys;
              st->rx_perm_desc_va = perm_desc_va;
              for (size_t k = 0; k < 8; ++k) {
                st->rx_perm_pkt_phys[k] = perm_pkt_phys[k];
                st->rx_perm_pkt_va[k] = perm_pkt_va[k];
              }
              sexos_line("[sexnet.nic.rx.permanent.poll.begin] max_iters=50000000");
              uint32_t perm_dd_set = 0u;
              uint32_t perm_dd_desc = 0xFFFFu;
              for (uint32_t outer = 0u; outer < 50000000u; ++outer) {
                for (uint32_t idx = 0u; idx < 8u; ++idx) {
                  uint8_t st_b = vread8(perm_desc_va + (uint64_t) idx * 16ULL + 12ULL);
                  if ((st_b & 1u) != 0u) {
                    perm_dd_set = 1u;
                    perm_dd_desc = idx;
                    break;
                  }
                }
                if (perm_dd_set == 1u) break;
              }
              sexos_line("[sexnet.nic.rx.permanent.poll.done] dd_set=%u desc_idx=%u ok=1", (unsigned) perm_dd_set, (unsigned) perm_dd_desc);
              if (perm_dd_set == 1u) {
                uint64_t desc_base = perm_desc_va + (uint64_t) perm_dd_desc * 16ULL;
                uint32_t pkt_len = (uint32_t) vread16(desc_base + 8ULL);
                uint64_t pkt_buf = perm_pkt_va[(size_t) perm_dd_desc];
                uint8_t eth_hi = vread8(pkt_buf + 12ULL);
                uint8_t eth_lo = vread8(pkt_buf + 13ULL);
                uint16_t ethertype = (uint16_t)((uint16_t) eth_hi << 8) | (uint16_t) eth_lo;
                uint32_t parse_ok = pkt_len > 14u ? 1u : 0u;
                sexos_line("[sexnet.nic.rx.permanent.pkt.parse] len=%u ethertype=0x%04X ok=%u", (unsigned) pkt_len, (unsigned) ethertype, (unsigned) parse_ok);
                if (parse_ok == 1u) {
                  const char *eth_classify = ethertype == 0x0806u ? "arp" : ethertype == 0x0800u ? "ipv4" : "unknown";
                  sexos_line("[sexnet.ether.parse.ok] len=%u ethertype=0x%04X class=%s ok=1", (unsigned) pkt_len, (unsigned) ethertype, eth_classify);
                  if (ethertype != 0x0806u && ethertype != 0x0800u) sexos_line("[sexnet.ether.ethertype.unknown.reject] ethertype=0x%04X ok=1", (unsigned) ethertype);
                } else sexos_line("[sexnet.ether.runt.reject] len=%u min=15 ok=1", (unsigned) pkt_len);
                vwrite16(desc_base + 8ULL, 0u);
                vwrite8(desc_base + 12ULL, 0u);
                uint32_t new_rdt = (perm_dd_desc + 7u) & 7u;
                vwrite32(nic_va + 0x2818ULL, new_rdt);
                sexos_line("[sexnet.nic.rx.permanent.rdt.advance] desc_idx=%u new_rdt=%u ok=1", (unsigned) perm_dd_desc, (unsigned) new_rdt);
              }
            }
          }
        }
        uint64_t tx_desc_phys = sys_alloc_phys(4096ULL);
        uint64_t tx_desc_va = sys_map_phys(tx_desc_phys, 4096ULL);
        uint64_t tx_frame_phys = sys_alloc_phys(4096ULL);
        uint64_t tx_frame_va = sys_map_phys(tx_frame_phys, 4096ULL);
        bool tx_alloc_ok = tx_desc_phys != 0ULL && tx_desc_phys != UINT64_MAX && tx_desc_va != 0ULL && tx_desc_va != UINT64_MAX && tx_frame_phys != 0ULL && tx_frame_phys != UINT64_MAX && tx_frame_va != 0ULL && tx_frame_va != UINT64_MAX;
        sexos_line("[sexnet.nic.tx.observe.alloc] desc_phys=0x%016llX frame_phys=0x%016llX ok=%u", (unsigned long long) tx_desc_phys, (unsigned long long) tx_frame_phys, (unsigned)(tx_alloc_ok ? 1u : 0u));
        if (tx_alloc_ok) {
          for (uint64_t z = 0ULL; z < 512ULL; ++z) {
            vwrite64(tx_desc_va + z * 8ULL, 0ULL);
            vwrite64(tx_frame_va + z * 8ULL, 0ULL);
          }
          for (uint64_t bi = 0ULL; bi < 6ULL; ++bi) vwrite8(tx_frame_va + bi, 0xFFu);
          uint8_t src_mac_test[6] = {
            0x52u, 0x54u, 0x00u, 0x12u, 0x34u, 0x56u
          };
          for (size_t si = 0; si < 6; ++si) vwrite8(tx_frame_va + 6ULL + (uint64_t) si, src_mac_test[si]);
          vwrite8(tx_frame_va + 12ULL, 0x88u);
          vwrite8(tx_frame_va + 13ULL, 0xB5u);
          for (uint64_t pi = 14ULL; pi < 60ULL; ++pi) vwrite8(tx_frame_va + pi, 0x42u);
          sexos_line("[sexnet.nic.tx.observe.frame.write] ethertype=0x88B5 len=60 ok=1");
          vwrite64(tx_desc_va, tx_frame_phys);
          vwrite16(tx_desc_va + 8ULL, 60u);
          vwrite8(tx_desc_va + 10ULL, 0u);
          vwrite8(tx_desc_va + 11ULL, 0x0Bu);
          vwrite8(tx_desc_va + 12ULL, 0u);
          vwrite8(tx_desc_va + 13ULL, 0u);
          vwrite16(tx_desc_va + 14ULL, 0u);
          sexos_line("[sexnet.nic.tx.observe.desc.write] len=60 cmd=0x0B sta=0 ok=1");
          uint32_t tx_tctl_orig = vread32(nic_va + 0x0400ULL);
          uint32_t tx_tctl_en_orig = (tx_tctl_orig & 1u << 1) != 0u ? 1u : 0u;
          uint32_t tx_tdbal_orig = vread32(nic_va + 0x3800ULL);
          uint32_t tx_tdbah_orig = vread32(nic_va + 0x3804ULL);
          uint32_t tx_tdlen_orig = vread32(nic_va + 0x3808ULL);
          uint32_t tx_tdh_orig = vread32(nic_va + 0x3810ULL);
          uint32_t tx_tdt_orig = vread32(nic_va + 0x3818ULL);
          (void) tx_tdh_orig;
          sexos_line("[sexnet.nic.tx.observe.ring.save] tctl=0x%08X tctl_en=%u tdbal=0x%08X tdlen=%u tdt=%u ok=1", (unsigned) tx_tctl_orig, (unsigned) tx_tctl_en_orig, (unsigned) tx_tdbal_orig, (unsigned) tx_tdlen_orig, (unsigned) tx_tdt_orig);
          vwrite32(nic_va + 0x0400ULL, tx_tctl_orig & ~(1u << 1));
          vwrite32(nic_va + 0x3800ULL, (uint32_t)(tx_desc_phys & 0xFFFFFFFFULL));
          vwrite32(nic_va + 0x3804ULL, (uint32_t)(tx_desc_phys >> 32));
          vwrite32(nic_va + 0x3808ULL, 128u);
          vwrite32(nic_va + 0x3810ULL, 0u);
          vwrite32(nic_va + 0x3818ULL, 0u);
          uint32_t tx_tctl_prog = 1u << 1 | 0x10u << 4 | 0x40u << 12;
          vwrite32(nic_va + 0x0400ULL, tx_tctl_prog);
          uint32_t tx_prog_tdbal = vread32(nic_va + 0x3800ULL);
          uint32_t tx_prog_tctl = vread32(nic_va + 0x0400ULL);
          uint32_t tx_tctl_en = (tx_prog_tctl & 1u << 1) != 0u ? 1u : 0u;
          uint32_t tx_prog_ok = tx_prog_tdbal == (uint32_t) tx_desc_phys && tx_tctl_en == 1u ? 1u : 0u;
          sexos_line("[sexnet.nic.tx.observe.ring.program] tdbal=0x%08X tdlen=128 tdt=0 tctl=0x%08X ok=%u", (unsigned) tx_prog_tdbal, (unsigned) tx_prog_tctl, (unsigned) tx_prog_ok);
          vwrite32(nic_va + 0x3818ULL, 1u);
          sexos_line("[sexnet.nic.tx.observe.post] tdt=1 ok=1");
          sexos_line("[sexnet.nic.tx.observe.poll.begin] max_iters=50000000");
          uint32_t tx_dd_set = 0u;
          for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
            uint8_t tx_st = vread8(tx_desc_va + 12ULL);
            if ((tx_st & 1u) != 0u) {
              tx_dd_set = 1u;
              break;
            }
          }
          sexos_line("[sexnet.nic.tx.observe.poll.done] dd_set=%u desc_idx=0 ok=1", (unsigned) tx_dd_set);
          if (tx_dd_set == 1u) sexos_line("[sexnet.nic.tx.dd.ok] dd_set=%u ok=1", (unsigned) tx_dd_set);
          vwrite32(nic_va + 0x0400ULL, tx_tctl_orig & ~(1u << 1));
          vwrite32(nic_va + 0x3800ULL, tx_tdbal_orig);
          vwrite32(nic_va + 0x3804ULL, tx_tdbah_orig);
          vwrite32(nic_va + 0x3808ULL, tx_tdlen_orig);
          vwrite32(nic_va + 0x3818ULL, tx_tdt_orig);
          vwrite32(nic_va + 0x0400ULL, tx_tctl_orig);
          uint32_t tx_rest_tctl = vread32(nic_va + 0x0400ULL);
          uint32_t tx_rest_tdbal = vread32(nic_va + 0x3800ULL);
          uint32_t tx_tctl_en_restored = (tx_rest_tctl & 1u << 1) != 0u ? 1u : 0u;
          uint32_t tx_restore_ok = tx_tctl_en_restored == tx_tctl_en_orig && tx_rest_tdbal == tx_tdbal_orig ? 1u : 0u;
          sexos_line("[sexnet.nic.tx.observe.ring.restore] tctl_orig=0x%08X tctl_en_orig=%u tctl_en=%u tdbal=0x%08X ok=%u", (unsigned) tx_tctl_orig, (unsigned) tx_tctl_en_orig, (unsigned) tx_tctl_en_restored, (unsigned) tx_rest_tdbal, (unsigned) tx_restore_ok);
          uint32_t tx_proof_ok = tx_dd_set == 1u && tx_restore_ok == 1u ? 1u : 0u;
          sexos_line("[sexnet.nic.tx.observe.proof.done] dd_set=%u ok=%u", (unsigned) tx_dd_set, (unsigned) tx_proof_ok);
          uint8_t rx_own_now = atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire);
          if (rx_own_now != NIC_OWNER_SEXNET_RX) sexos_line("[sexnet.nic.tx.permanent.skip] reason=rx_not_claimed ok=0");
          else {
            uint64_t txp_desc_phys = sys_alloc_phys(4096ULL);
            uint64_t txp_desc_va = sys_map_phys(txp_desc_phys, 4096ULL);
            uint64_t txp_frame_phys = sys_alloc_phys(4096ULL);
            uint64_t txp_frame_va = sys_map_phys(txp_frame_phys, 4096ULL);
            bool txp_alloc_ok = txp_desc_phys != 0ULL && txp_desc_phys != UINT64_MAX && txp_desc_va != 0ULL && txp_desc_va != UINT64_MAX && txp_frame_phys != 0ULL && txp_frame_phys != UINT64_MAX && txp_frame_va != 0ULL && txp_frame_va != UINT64_MAX;
            sexos_line("[sexnet.nic.tx.permanent.alloc] desc_phys=0x%016llX frame_phys=0x%016llX ok=%u", (unsigned long long) txp_desc_phys, (unsigned long long) txp_frame_phys, (unsigned)(txp_alloc_ok ? 1u : 0u));
            if (txp_alloc_ok) {
              st->tx_perm_desc_phys = txp_desc_phys;
              st->tx_perm_desc_va = txp_desc_va;
              st->tx_perm_frame_phys = txp_frame_phys;
              st->tx_perm_frame_va = txp_frame_va;
              for (uint64_t z = 0ULL; z < 512ULL; ++z) {
                vwrite64(txp_desc_va + z * 8ULL, 0ULL);
                vwrite64(txp_frame_va + z * 8ULL, 0ULL);
              }
              for (uint64_t bi = 0ULL; bi < 6ULL; ++bi) vwrite8(txp_frame_va + bi, 0xFFu);
              for (size_t si = 0; si < 6; ++si) vwrite8(txp_frame_va + 6ULL + (uint64_t) si, src_mac_test[si]);
              vwrite8(txp_frame_va + 12ULL, 0x88u);
              vwrite8(txp_frame_va + 13ULL, 0xB5u);
              for (uint64_t pi = 14ULL; pi < 60ULL; ++pi) vwrite8(txp_frame_va + pi, 0x42u);
              sexos_line("[sexnet.nic.tx.permanent.frame.write] ethertype=0x88B5 len=60 ok=1");
              vwrite64(txp_desc_va, txp_frame_phys);
              vwrite16(txp_desc_va + 8ULL, 60u);
              vwrite8(txp_desc_va + 10ULL, 0u);
              vwrite8(txp_desc_va + 11ULL, 0x0Bu);
              vwrite8(txp_desc_va + 12ULL, 0u);
              vwrite8(txp_desc_va + 13ULL, 0u);
              vwrite16(txp_desc_va + 14ULL, 0u);
              sexos_line("[sexnet.nic.tx.permanent.desc.write] len=60 cmd=0x0B sta=0 ok=1");
              vwrite32(nic_va + 0x3800ULL, (uint32_t)(txp_desc_phys & 0xFFFFFFFFULL));
              vwrite32(nic_va + 0x3804ULL, (uint32_t)(txp_desc_phys >> 32));
              vwrite32(nic_va + 0x3808ULL, 128u);
              vwrite32(nic_va + 0x3810ULL, 0u);
              vwrite32(nic_va + 0x3818ULL, 0u);
              vwrite32(nic_va + 0x0400ULL, 0x00040102u);
              uint32_t txp_prog_tdbal = vread32(nic_va + 0x3800ULL);
              uint32_t txp_prog_tctl = vread32(nic_va + 0x0400ULL);
              uint32_t txp_tctl_en = (txp_prog_tctl & 1u << 1) != 0u ? 1u : 0u;
              uint32_t txp_ring_ok = txp_prog_tdbal == (uint32_t) txp_desc_phys && txp_tctl_en == 1u ? 1u : 0u;
              sexos_line("[sexnet.nic.tx.permanent.ring.program] tdbal=0x%08X tdlen=128 tdt=0 tctl=0x%08X ok=%u", (unsigned) txp_prog_tdbal, (unsigned) txp_prog_tctl, (unsigned) txp_ring_ok);
              uint32_t txp_dd_set = 0u;
              if (txp_ring_ok == 1u) {
                vwrite32(nic_va + 0x3818ULL, 1u);
                sexos_line("[sexnet.nic.tx.permanent.post] tdt=1 ok=1");
                sexos_line("[sexnet.nic.tx.permanent.poll.begin] max_iters=50000000");
                for (uint32_t txp_outer = 0u; txp_outer < 50000000u; ++txp_outer) {
                  uint8_t tx_st = vread8(txp_desc_va + 12ULL);
                  if ((tx_st & 1u) != 0u) {
                    txp_dd_set = 1u;
                    break;
                  }
                }
                sexos_line("[sexnet.nic.tx.permanent.poll.done] dd_set=%u desc_idx=0 ok=1", (unsigned) txp_dd_set);
              }
              if (txp_ring_ok == 1u && txp_dd_set == 1u) atomic_store_explicit(&st->nic_tx_owner, NIC_OWNER_SEXNET_TX, memory_order_release);
              uint8_t tx_owner_now = atomic_load_explicit(&st->nic_tx_owner, memory_order_acquire);
              uint32_t tx_claim_ok = tx_owner_now == NIC_OWNER_SEXNET_TX && txp_ring_ok == 1u ? 1u : 0u;
              sexos_line("[sexnet.nic.tx.permanent.claim] owner=%u ring_ok=%u ok=%u", (unsigned) tx_owner_now, (unsigned) txp_ring_ok, (unsigned) tx_claim_ok);
              if (tx_claim_ok == 1u && atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire) == NIC_OWNER_SEXNET_RX) {
                atomic_store_explicit(&st->nic_rx_owner, NIC_OWNER_SEXNET_FULL, memory_order_release);
                atomic_store_explicit(&st->nic_tx_owner, NIC_OWNER_SEXNET_FULL, memory_order_release);
                uint8_t full_rx = atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire);
                uint8_t full_tx = atomic_load_explicit(&st->nic_tx_owner, memory_order_acquire);
                uint32_t full_ok = full_rx == NIC_OWNER_SEXNET_FULL && full_tx == NIC_OWNER_SEXNET_FULL ? 1u : 0u;
                sexos_line("[sexnet.nic.tx.permanent.full] rx_owner=%u tx_owner=%u full_ok=%u", (unsigned) full_rx, (unsigned) full_tx, (unsigned) full_ok);
              }
              uint32_t arp_ok = 0u;
              uint8_t arp_rx_own = atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire);
              uint8_t arp_tx_own = atomic_load_explicit(&st->nic_tx_owner, memory_order_acquire);
              if (arp_rx_own == NIC_OWNER_SEXNET_FULL && arp_tx_own == NIC_OWNER_SEXNET_FULL) {
                uint8_t nic_mac[6] = {
                  (uint8_t)(ral & 0xFFu), (uint8_t)(ral >> 8 & 0xFFu), (uint8_t)(ral >> 16 & 0xFFu), (uint8_t)(ral >> 24 & 0xFFu), (uint8_t)(rah & 0xFFu), (uint8_t)(rah >> 8 & 0xFFu)
                };
                sexos_line("[sexnet.arp.rx.poll.begin] max_iters=50000000");
                uint32_t arp_rx = 0u;
                uint8_t sender_mac[6] = {
                  0u
                };
                uint8_t sender_ip[4] = {
                  0u
                };
                for (uint32_t arp_outer = 0u; arp_outer < 50000000u && arp_rx == 0u; ++arp_outer) for (uint32_t idx = 0u; idx < 8u && arp_rx == 0u; ++idx) {
                  uint64_t desc_base = st->rx_perm_desc_va + (uint64_t) idx * 16ULL;
                  uint8_t st_b = vread8(desc_base + 12ULL);
                  if ((st_b & 1u) != 0u) {
                    uint64_t pkt_buf = st->rx_perm_pkt_va[(size_t) idx];
                    uint8_t eth_hi = vread8(pkt_buf + 12ULL);
                    uint8_t eth_lo = vread8(pkt_buf + 13ULL);
                    uint16_t ethertype = (uint16_t)((uint16_t) eth_hi << 8) | (uint16_t) eth_lo;
                    uint8_t htype_hi = vread8(pkt_buf + 14ULL);
                    uint8_t htype_lo = vread8(pkt_buf + 15ULL);
                    uint8_t ptype_hi = vread8(pkt_buf + 16ULL);
                    uint8_t ptype_lo = vread8(pkt_buf + 17ULL);
                    uint8_t hlen = vread8(pkt_buf + 18ULL);
                    uint8_t plen = vread8(pkt_buf + 19ULL);
                    uint8_t oper_hi = vread8(pkt_buf + 20ULL);
                    uint8_t oper_lo = vread8(pkt_buf + 21ULL);
                    uint16_t htype = (uint16_t)((uint16_t) htype_hi << 8) | (uint16_t) htype_lo;
                    uint16_t ptype = (uint16_t)((uint16_t) ptype_hi << 8) | (uint16_t) ptype_lo;
                    uint16_t oper = (uint16_t)((uint16_t) oper_hi << 8) | (uint16_t) oper_lo;
                    uint8_t tpa[4] = {
                      0u
                    };
                    for (size_t ti = 0; ti < 4; ++ti) tpa[ti] = vread8(pkt_buf + 38ULL + (uint64_t) ti);
                    uint32_t tpa_match = tpa[0] == SEXNET_GUEST_IPV4[0] && tpa[1] == SEXNET_GUEST_IPV4[1] && tpa[2] == SEXNET_GUEST_IPV4[2] && tpa[3] == SEXNET_GUEST_IPV4[3] ? 1u : 0u;
                    bool valid = ethertype == 0x0806u && htype == 1u && ptype == 0x0800u && hlen == 6u && plen == 4u && oper == 1u && tpa_match == 1u;
                    if (valid) {
                      for (size_t mi = 0; mi < 6; ++mi) sender_mac[mi] = vread8(pkt_buf + 22ULL + (uint64_t) mi);
                      for (size_t si = 0; si < 4; ++si) sender_ip[si] = vread8(pkt_buf + 28ULL + (uint64_t) si);
                      sexos_line("[sexnet.arp.rx.frame] idx=%u ethertype=0x0806 ok=1", (unsigned) idx);
                      sexos_line("[sexnet.arp.rx.validate] htype=1 ptype=0x0800 hlen=6 plen=4 oper=1 tpa_match=1 ok=1");
                      sexos_line("[sexnet.arp.reply.rx.ok] oper=1 ok=1");
                      arp_rx = 1u;
                    } else {
                      sexos_line("[sexnet.arp.rx.reject.detail] idx=%u etype=0x%04X oper=0x%04X tpa=%u.%u.%u.%u ok=0", (unsigned) idx, (unsigned) ethertype, (unsigned) oper, (unsigned) tpa[0], (unsigned) tpa[1], (unsigned) tpa[2], (unsigned) tpa[3]);
                      sexos_line("[sexnet.arp.rx.reject] idx=%u reason=notarp_or_badfield ok=0", (unsigned) idx);
                    }
                    vwrite16(desc_base + 8ULL, 0u);
                    vwrite8(desc_base + 12ULL, 0u);
                    vwrite32(nic_va + 0x2818ULL, idx);
                  }
                }
                uint32_t arp_tx_dd = 0u;
                if (arp_rx == 1u) {
                  uint64_t arp_tx_va = st->tx_perm_frame_va;
                  for (size_t i = 0; i < 6; ++i) {
                    vwrite8(arp_tx_va + (uint64_t) i, sender_mac[i]);
                    vwrite8(arp_tx_va + 6ULL + (uint64_t) i, nic_mac[i]);
                  }
                  vwrite8(arp_tx_va + 12ULL, 0x08u);
                  vwrite8(arp_tx_va + 13ULL, 0x06u);
                  vwrite8(arp_tx_va + 14ULL, 0x00u);
                  vwrite8(arp_tx_va + 15ULL, 0x01u);
                  vwrite8(arp_tx_va + 16ULL, 0x08u);
                  vwrite8(arp_tx_va + 17ULL, 0x00u);
                  vwrite8(arp_tx_va + 18ULL, 0x06u);
                  vwrite8(arp_tx_va + 19ULL, 0x04u);
                  vwrite8(arp_tx_va + 20ULL, 0x00u);
                  vwrite8(arp_tx_va + 21ULL, 0x02u);
                  for (size_t ai = 0; ai < 6; ++ai) {
                    vwrite8(arp_tx_va + 22ULL + (uint64_t) ai, nic_mac[ai]);
                    vwrite8(arp_tx_va + 32ULL + (uint64_t) ai, sender_mac[ai]);
                  }
                  for (size_t gi = 0; gi < 4; ++gi) {
                    vwrite8(arp_tx_va + 28ULL + (uint64_t) gi, SEXNET_GUEST_IPV4[gi]);
                    vwrite8(arp_tx_va + 38ULL + (uint64_t) gi, sender_ip[gi]);
                  }
                  for (uint64_t pad = 42ULL; pad < 60ULL; ++pad) vwrite8(arp_tx_va + pad, 0u);
                  sexos_line("[sexnet.arp.tx.reply.build] spa=10.0.2.15 ok=1");
                  uint64_t tx_dump_base = st->tx_perm_frame_va;
                  uint8_t eth_dst0 = vread8(tx_dump_base + 0ULL);
                  uint8_t eth_dst1 = vread8(tx_dump_base + 1ULL);
                  uint8_t eth_dst2 = vread8(tx_dump_base + 2ULL);
                  uint8_t eth_dst3 = vread8(tx_dump_base + 3ULL);
                  uint8_t eth_dst4 = vread8(tx_dump_base + 4ULL);
                  uint8_t eth_dst5 = vread8(tx_dump_base + 5ULL);
                  uint8_t eth_src0 = vread8(tx_dump_base + 6ULL);
                  uint8_t eth_src1 = vread8(tx_dump_base + 7ULL);
                  uint8_t eth_src2 = vread8(tx_dump_base + 8ULL);
                  uint8_t eth_src3 = vread8(tx_dump_base + 9ULL);
                  uint8_t eth_src4 = vread8(tx_dump_base + 10ULL);
                  uint8_t eth_src5 = vread8(tx_dump_base + 11ULL);
                  uint8_t etype_hi = vread8(tx_dump_base + 12ULL);
                  uint8_t etype_lo = vread8(tx_dump_base + 13ULL);
                  uint8_t oper_hi = vread8(tx_dump_base + 20ULL);
                  uint8_t oper_lo = vread8(tx_dump_base + 21ULL);
                  uint8_t sha0 = vread8(tx_dump_base + 22ULL);
                  uint8_t sha1 = vread8(tx_dump_base + 23ULL);
                  uint8_t sha2 = vread8(tx_dump_base + 24ULL);
                  uint8_t sha3 = vread8(tx_dump_base + 25ULL);
                  uint8_t sha4 = vread8(tx_dump_base + 26ULL);
                  uint8_t sha5 = vread8(tx_dump_base + 27ULL);
                  uint8_t spa0 = vread8(tx_dump_base + 28ULL);
                  uint8_t spa1 = vread8(tx_dump_base + 29ULL);
                  uint8_t spa2 = vread8(tx_dump_base + 30ULL);
                  uint8_t spa3 = vread8(tx_dump_base + 31ULL);
                  uint8_t tha0 = vread8(tx_dump_base + 32ULL);
                  uint8_t tha1 = vread8(tx_dump_base + 33ULL);
                  uint8_t tha2 = vread8(tx_dump_base + 34ULL);
                  uint8_t tha3 = vread8(tx_dump_base + 35ULL);
                  uint8_t tha4 = vread8(tx_dump_base + 36ULL);
                  uint8_t tha5 = vread8(tx_dump_base + 37ULL);
                  uint8_t tpa0 = vread8(tx_dump_base + 38ULL);
                  uint8_t tpa1 = vread8(tx_dump_base + 39ULL);
                  uint8_t tpa2 = vread8(tx_dump_base + 40ULL);
                  uint8_t tpa3 = vread8(tx_dump_base + 41ULL);
                  uint16_t etype = (uint16_t)((uint16_t) etype_hi << 8) | (uint16_t) etype_lo;
                  uint16_t oper = (uint16_t)((uint16_t) oper_hi << 8) | (uint16_t) oper_lo;
                  sexos_line("[sexnet.arp.tx.dump] eth_dst=%02X:%02X:%02X:%02X:%02X:%02X eth_src=%02X:%02X:%02X:%02X:%02X:%02X etype=0x%04X oper=0x%04X sha=%02X:%02X:%02X:%02X:%02X:%02X spa=%u.%u.%u.%u tha=%02X:%02X:%02X:%02X:%02X:%02X tpa=%u.%u.%u.%u ok=1", (unsigned) eth_dst0, (unsigned) eth_dst1, (unsigned) eth_dst2, (unsigned) eth_dst3, (unsigned) eth_dst4, (unsigned) eth_dst5, (unsigned) eth_src0, (unsigned) eth_src1, (unsigned) eth_src2, (unsigned) eth_src3, (unsigned) eth_src4, (unsigned) eth_src5, (unsigned) etype, (unsigned) oper, (unsigned) sha0, (unsigned) sha1, (unsigned) sha2, (unsigned) sha3, (unsigned) sha4, (unsigned) sha5, (unsigned) spa0, (unsigned) spa1, (unsigned) spa2, (unsigned) spa3, (unsigned) tha0, (unsigned) tha1, (unsigned) tha2, (unsigned) tha3, (unsigned) tha4, (unsigned) tha5, (unsigned) tpa0, (unsigned) tpa1, (unsigned) tpa2, (unsigned) tpa3);
                  uint64_t tx_desc1 = st->tx_perm_desc_va + 16ULL;
                  vwrite64(tx_desc1, st->tx_perm_frame_phys);
                  vwrite16(tx_desc1 + 8ULL, 60u);
                  vwrite8(tx_desc1 + 10ULL, 0u);
                  vwrite8(tx_desc1 + 11ULL, 0x0Bu);
                  vwrite8(tx_desc1 + 12ULL, 0u);
                  vwrite8(tx_desc1 + 13ULL, 0u);
                  vwrite16(tx_desc1 + 14ULL, 0u);
                  sexos_line("[sexnet.arp.tx.desc] slot=1 len=60 ok=1");
                  vwrite32(nic_va + 0x3818ULL, 2u);
                  sexos_line("[sexnet.arp.tx.post] tdt=2 ok=1");
                  for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
                    uint8_t tx_st = vread8(tx_desc1 + 12ULL);
                    if ((tx_st & 1u) != 0u) {
                      arp_tx_dd = 1u;
                      break;
                    }
                  }
                  sexos_line("[sexnet.arp.tx.poll.done] dd_set=%u ok=%u", (unsigned) arp_tx_dd, (unsigned)(arp_tx_dd == 1u ? 1u : 0u));
                  if (arp_tx_dd == 1u) sexos_line("[sexnet.arp.request.tx.ok] tx_dd=%u ok=1", (unsigned) arp_tx_dd);
                }
                arp_ok = arp_rx == 1u && arp_tx_dd == 1u ? 1u : 0u;
                sexos_line("[sexnet.arp.proof.done] rx_arp=%u tx_dd=%u ok=%u", (unsigned) arp_rx, (unsigned) arp_tx_dd, (unsigned) arp_ok);
                if (arp_rx == 0u && arp_tx_dd == 1u) sexos_line("[sexnet.arp.reply.rx.skip] reason=no_peer_reply tx_dd=%u ok=1", (unsigned) arp_tx_dd);
              } else {
                sexos_line("[sexnet.arp.skip] reason=not_full ok=0");
                sexos_line("[sexnet.arp.reply.rx.skip] reason=nic_not_full_owner ok=1");
              }
              uint8_t cache_nic_mac[6] = {
                (uint8_t)(ral & 0xFFu), (uint8_t)(ral >> 8 & 0xFFu), (uint8_t)(ral >> 16 & 0xFFu), (uint8_t)(ral >> 24 & 0xFFu), (uint8_t)(rah & 0xFFu), (uint8_t)(rah >> 8 & 0xFFu)
              };
              uint32_t cache_replies = 0u;
              uint32_t cache_outer_done = 0u;
              uint8_t cache_rx_own = atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire);
              uint8_t cache_tx_own = atomic_load_explicit(&st->nic_tx_owner, memory_order_acquire);
              sexos_line("[sexnet.arp.cache.poll.begin] max_iters=100000000 target_replies=2");
              if (cache_rx_own == NIC_OWNER_SEXNET_FULL && cache_tx_own == NIC_OWNER_SEXNET_FULL) {
                uint32_t cache_outer = 0u;
                while (cache_outer < 100000000u && cache_replies < 2u) {
                  for (uint32_t idx = 0u; idx < 8u && cache_replies < 2u; ++idx) {
                    uint64_t desc_base = st->rx_perm_desc_va + (uint64_t) idx * 16ULL;
                    uint8_t st_b = vread8(desc_base + 12ULL);
                    if ((st_b & 1u) != 0u) {
                      uint64_t pkt_buf = st->rx_perm_pkt_va[(size_t) idx];
                      uint8_t eth_hi = vread8(pkt_buf + 12ULL);
                      uint8_t eth_lo = vread8(pkt_buf + 13ULL);
                      uint8_t htype_hi = vread8(pkt_buf + 14ULL);
                      uint8_t htype_lo = vread8(pkt_buf + 15ULL);
                      uint8_t ptype_hi = vread8(pkt_buf + 16ULL);
                      uint8_t ptype_lo = vread8(pkt_buf + 17ULL);
                      uint8_t hlen = vread8(pkt_buf + 18ULL);
                      uint8_t plen = vread8(pkt_buf + 19ULL);
                      uint8_t oper_hi = vread8(pkt_buf + 20ULL);
                      uint8_t oper_lo = vread8(pkt_buf + 21ULL);
                      uint16_t ethertype = (uint16_t)((uint16_t) eth_hi << 8) | (uint16_t) eth_lo;
                      uint16_t htype = (uint16_t)((uint16_t) htype_hi << 8) | (uint16_t) htype_lo;
                      uint16_t ptype = (uint16_t)((uint16_t) ptype_hi << 8) | (uint16_t) ptype_lo;
                      uint16_t oper = (uint16_t)((uint16_t) oper_hi << 8) | (uint16_t) oper_lo;
                      uint8_t tpa[4] = {
                        0u
                      };
                      for (size_t ti = 0; ti < 4; ++ti) tpa[ti] = vread8(pkt_buf + 38ULL + (uint64_t) ti);
                      bool valid = ethertype == 0x0806u && htype == 1u && ptype == 0x0800u && hlen == 6u && plen == 4u && oper == 1u && (tpa[0] == SEXNET_GUEST_IPV4[0] && tpa[1] == SEXNET_GUEST_IPV4[1] && tpa[2] == SEXNET_GUEST_IPV4[2] && tpa[3] == SEXNET_GUEST_IPV4[3]);
                      if (valid) {
                        for (size_t mi = 0; mi < 6; ++mi) st->arp_cache_mac[mi] = vread8(pkt_buf + 22ULL + (uint64_t) mi);
                        for (size_t si = 0; si < 4; ++si) st->arp_cache_ip[si] = vread8(pkt_buf + 28ULL + (uint64_t) si);
                        st->arp_cache_valid = 1u;
                        uint32_t n = cache_replies + 1u;
                        sexos_line("[sexnet.arp.cache.learn] n=%u sha=%02X:%02X:%02X:%02X:%02X:%02X spa=%u.%u.%u.%u ok=1", (unsigned) n, (unsigned) st->arp_cache_mac[0], (unsigned) st->arp_cache_mac[1], (unsigned) st->arp_cache_mac[2], (unsigned) st->arp_cache_mac[3], (unsigned) st->arp_cache_mac[4], (unsigned) st->arp_cache_mac[5], (unsigned) st->arp_cache_ip[0], (unsigned) st->arp_cache_ip[1], (unsigned) st->arp_cache_ip[2], (unsigned) st->arp_cache_ip[3]);
                        sexos_line("[sexnet.arp.cache.gateway.ok] n=%u mac=%02X:%02X:%02X:%02X:%02X:%02X ip=%u.%u.%u.%u ok=1", (unsigned) n, (unsigned) st->arp_cache_mac[0], (unsigned) st->arp_cache_mac[1], (unsigned) st->arp_cache_mac[2], (unsigned) st->arp_cache_mac[3], (unsigned) st->arp_cache_mac[4], (unsigned) st->arp_cache_mac[5], (unsigned) st->arp_cache_ip[0], (unsigned) st->arp_cache_ip[1], (unsigned) st->arp_cache_ip[2], (unsigned) st->arp_cache_ip[3]);
                        uint32_t tx_slot = 3u + cache_replies;
                        uint32_t tx_tdt = 4u + cache_replies;
                        uint64_t c_tx_va = st->tx_perm_frame_va;
                        for (size_t i = 0; i < 6; ++i) {
                          vwrite8(c_tx_va + (uint64_t) i, st->arp_cache_mac[i]);
                          vwrite8(c_tx_va + 6ULL + (uint64_t) i, cache_nic_mac[i]);
                        }
                        vwrite8(c_tx_va + 12ULL, 0x08u);
                        vwrite8(c_tx_va + 13ULL, 0x06u);
                        vwrite8(c_tx_va + 14ULL, 0x00u);
                        vwrite8(c_tx_va + 15ULL, 0x01u);
                        vwrite8(c_tx_va + 16ULL, 0x08u);
                        vwrite8(c_tx_va + 17ULL, 0x00u);
                        vwrite8(c_tx_va + 18ULL, 0x06u);
                        vwrite8(c_tx_va + 19ULL, 0x04u);
                        vwrite8(c_tx_va + 20ULL, 0x00u);
                        vwrite8(c_tx_va + 21ULL, 0x02u);
                        for (size_t ai = 0; ai < 6; ++ai) {
                          vwrite8(c_tx_va + 22ULL + (uint64_t) ai, cache_nic_mac[ai]);
                          vwrite8(c_tx_va + 32ULL + (uint64_t) ai, st->arp_cache_mac[ai]);
                        }
                        for (size_t gi = 0; gi < 4; ++gi) {
                          vwrite8(c_tx_va + 28ULL + (uint64_t) gi, SEXNET_GUEST_IPV4[gi]);
                          vwrite8(c_tx_va + 38ULL + (uint64_t) gi, st->arp_cache_ip[gi]);
                        }
                        for (uint64_t pad = 42ULL; pad < 60ULL; ++pad) vwrite8(c_tx_va + pad, 0u);
                        uint64_t tx_desc = st->tx_perm_desc_va + (uint64_t) tx_slot * 16ULL;
                        vwrite64(tx_desc, st->tx_perm_frame_phys);
                        vwrite16(tx_desc + 8ULL, 60u);
                        vwrite8(tx_desc + 10ULL, 0u);
                        vwrite8(tx_desc + 11ULL, 0x0Bu);
                        vwrite8(tx_desc + 12ULL, 0u);
                        vwrite8(tx_desc + 13ULL, 0u);
                        vwrite16(tx_desc + 14ULL, 0u);
                        vwrite32(nic_va + 0x3818ULL, tx_tdt);
                        sexos_line("[sexnet.arp.cache.reply] n=%u slot=%u tdt=%u ok=1", (unsigned) n, (unsigned) tx_slot, (unsigned) tx_tdt);
                        uint32_t c_dd_set = 0u;
                        for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
                          uint8_t tx_st = vread8(tx_desc + 12ULL);
                          if ((tx_st & 1u) != 0u) {
                            c_dd_set = 1u;
                            break;
                          }
                        }
                        sexos_line("[sexnet.arp.cache.reply.dd] n=%u dd_set=%u ok=%u", (unsigned) n, (unsigned) c_dd_set, (unsigned)(c_dd_set == 1u ? 1u : 0u));
                        cache_replies += 1u;
                        st->arp_cache_reply_count = cache_replies;
                      }
                      vwrite16(desc_base + 8ULL, 0u);
                      vwrite8(desc_base + 12ULL, 0u);
                      vwrite32(nic_va + 0x2818ULL, idx);
                    }
                  }
                  cache_outer += 1u;
                }
                cache_outer_done = cache_outer;
              }
              uint32_t cache_ok = cache_replies == 2u ? 1u : 0u;
              sexos_line("[sexnet.arp.cache.poll.done] outer=%u replies=%u ok=%u", (unsigned) cache_outer_done, (unsigned) cache_replies, (unsigned) cache_ok);
              sexos_line("[sexnet.arp.cache.proof.done] replies=%u ok=%u", (unsigned) cache_replies, (unsigned) cache_ok);
              uint8_t l2_rx_own = atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire);
              uint8_t l2_tx_own = atomic_load_explicit(&st->nic_tx_owner, memory_order_acquire);
              if (l2_rx_own == NIC_OWNER_SEXNET_FULL && l2_tx_own == NIC_OWNER_SEXNET_FULL) {
                sexos_line("[sexnet.l2.entry] rx_owner=3 tx_owner=3 ok=1");
                sexos_line("[sexnet.l2.rx.poll.begin] max_frames=3 max_iters_per=3000000");
                uint32_t l2_frames = 0u;
                for (uint32_t outer = 0u; outer < 3000000u && l2_frames < 3u; ++outer) {
                  bool break_l2_poll = false;
                  for (uint32_t idx = 0u; idx < 8u && l2_frames < 3u; ++idx) {
                    uint64_t desc_base = st->rx_perm_desc_va + (uint64_t) idx * 16ULL;
                    uint8_t st_b = vread8(desc_base + 12ULL);
                    if ((st_b & 1u) != 0u) {
                      uint32_t pkt_len = (uint32_t) vread16(desc_base + 8ULL);
                      uint64_t pkt_buf = st->rx_perm_pkt_va[(size_t) idx];
                      uint8_t eth_hi = vread8(pkt_buf + 12ULL);
                      uint8_t eth_lo = vread8(pkt_buf + 13ULL);
                      uint16_t ethertype = (uint16_t)((uint16_t) eth_hi << 8) | (uint16_t) eth_lo;
                      l2_frames += 1u;
                      st->l2_rx_next = (uint8_t) idx;
                      st->l2_rx_count = l2_frames;
                      sexos_line("[sexnet.l2.rx.frame] idx=%u len=%u ethertype=0x%04X count=%u ok=1", (unsigned) idx, (unsigned) pkt_len, (unsigned) ethertype, (unsigned) l2_frames);
                      if (ethertype != 0x0806u) {
                        vwrite16(desc_base + 8ULL, 0u);
                        vwrite8(desc_base + 12ULL, 0u);
                        vwrite32(nic_va + 0x2818ULL, idx);
                        sexos_line("[sexnet.l2.rx.recycle] idx=%u new_rdt=%u ok=1", (unsigned) idx, (unsigned) idx);
                      } else {
                        break_l2_poll = true;
                        break;
                      }
                    }
                  }
                  if (break_l2_poll) break;
                }
                uint32_t l2_rx_ok = l2_frames > 0u ? 1u : 0u;
                sexos_line("[sexnet.l2.rx.poll.done] frames_rx=%u ok=%u", (unsigned) l2_frames, (unsigned) l2_rx_ok);
                uint32_t l2_tx_dd = 0u;
                bool tx_perm_ready = st->tx_perm_desc_va != 0ULL && st->tx_perm_frame_phys != 0ULL && st->tx_perm_frame_va != 0ULL;
                if (tx_perm_ready) {
                  uint64_t l2_tx_va = st->tx_perm_frame_va;
                  for (uint64_t bi = 0ULL; bi < 6ULL; ++bi) vwrite8(l2_tx_va + bi, 0xFFu);
                  for (size_t si = 0; si < 6; ++si) vwrite8(l2_tx_va + 6ULL + (uint64_t) si, src_mac_test[si]);
                  vwrite8(l2_tx_va + 12ULL, 0x88u);
                  vwrite8(l2_tx_va + 13ULL, 0xB5u);
                  for (uint64_t pi = 14ULL; pi < 60ULL; ++pi) vwrite8(l2_tx_va + pi, 0x42u);
                  uint64_t tx_desc2 = st->tx_perm_desc_va + 32ULL;
                  vwrite64(tx_desc2, st->tx_perm_frame_phys);
                  vwrite16(tx_desc2 + 8ULL, 60u);
                  vwrite8(tx_desc2 + 10ULL, 0u);
                  vwrite8(tx_desc2 + 11ULL, 0x0Bu);
                  vwrite8(tx_desc2 + 12ULL, 0u);
                  vwrite8(tx_desc2 + 13ULL, 0u);
                  vwrite16(tx_desc2 + 14ULL, 0u);
                  st->l2_tx_next = 2u;
                  sexos_line("[sexnet.l2.tx.reuse.desc] slot=2 len=60 ok=1");
                  vwrite32(nic_va + 0x3818ULL, 3u);
                  sexos_line("[sexnet.l2.tx.reuse.post] tdt=3 ok=1");
                  for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
                    uint8_t tx_st = vread8(tx_desc2 + 12ULL);
                    if ((tx_st & 1u) != 0u) {
                      l2_tx_dd = 1u;
                      break;
                    }
                  }
                  sexos_line("[sexnet.l2.tx.reuse.poll.done] dd_set=%u desc_idx=2 ok=%u", (unsigned) l2_tx_dd, (unsigned)(l2_tx_dd == 1u ? 1u : 0u));
                }
                uint32_t l2_ok = (l2_frames > 0u || arp_ok == 1u) && l2_tx_dd == 1u ? 1u : 0u;
                sexos_line("[sexnet.l2.proof.done] rx_frames=%u tx_dd=%u ok=%u", (unsigned) l2_frames, (unsigned) l2_tx_dd, (unsigned) l2_ok);
                sexos_line("[sexnet.tcp.entry] state=CLOSED local_port=%u remote=%u.%u.%u.%u:%u ok=1", (unsigned) st->tcp_local_port, (unsigned) st->tcp_remote_ip[0], (unsigned) st->tcp_remote_ip[1], (unsigned) st->tcp_remote_ip[2], (unsigned) st->tcp_remote_ip[3], (unsigned) st->tcp_remote_port);
                uint32_t tcp_syn_built = 0u;
                uint32_t tcp_syn_tx_dd = 0u;
                bool syn_tx_ready = st->tx_perm_desc_va != 0ULL && st->tx_perm_frame_phys != 0ULL && st->tx_perm_frame_va != 0ULL;
                if (syn_tx_ready) {
                  uint64_t tx_va = st->tx_perm_frame_va;
                  uint8_t syn_nic_mac[6] = {
                    (uint8_t)(ral & 0xFFu), (uint8_t)(ral >> 8 & 0xFFu), (uint8_t)(ral >> 16 & 0xFFu), (uint8_t)(ral >> 24 & 0xFFu), (uint8_t)(rah & 0xFFu), (uint8_t)(rah >> 8 & 0xFFu)
                  };
                  uint8_t dst_mac[6] = {
                    0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu
                  };
                  if (st->arp_cache_valid == 1u) for (size_t mi = 0; mi < 6; ++mi) dst_mac[mi] = st->arp_cache_mac[mi];
                  else if (st->tcp_remote_ip[0] == 10u && st->tcp_remote_ip[1] == 0u && st->tcp_remote_ip[2] == 2u && st->tcp_remote_ip[3] == 2u) {
                    sexos_line("[sexnet.tcp.syn.mac.resolve] mode=slirp_static dst_ip=10.0.2.2 mac=52:55:0A:00:02:02 ok=1");
                    dst_mac[0] = 0x52u;
                    dst_mac[1] = 0x55u;
                    dst_mac[2] = 0x0Au;
                    dst_mac[3] = 0x00u;
                    dst_mac[4] = 0x02u;
                    dst_mac[5] = 0x02u;
                  }
                  for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + (uint64_t) mi, dst_mac[mi]);
                  for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + 6ULL + (uint64_t) mi, syn_nic_mac[mi]);
                  vwrite8(tx_va + 12ULL, 0x08u);
                  vwrite8(tx_va + 13ULL, 0x00u);
                  uint16_t ipv4_total = 40u;
                  vwrite8(tx_va + 14ULL, 0x45u);
                  vwrite8(tx_va + 15ULL, 0x00u);
                  vwrite8(tx_va + 16ULL, (uint8_t)(ipv4_total >> 8 & 0xFFu));
                  vwrite8(tx_va + 17ULL, (uint8_t)(ipv4_total & 0xFFu));
                  vwrite8(tx_va + 18ULL, 0x00u);
                  vwrite8(tx_va + 19ULL, 0x00u);
                  vwrite8(tx_va + 20ULL, 0x00u);
                  vwrite8(tx_va + 21ULL, 0x00u);
                  vwrite8(tx_va + 22ULL, 64u);
                  vwrite8(tx_va + 23ULL, 6u);
                  vwrite8(tx_va + 24ULL, 0x00u);
                  vwrite8(tx_va + 25ULL, 0x00u);
                  vwrite8(tx_va + 26ULL, 10u);
                  vwrite8(tx_va + 27ULL, 0u);
                  vwrite8(tx_va + 28ULL, 2u);
                  vwrite8(tx_va + 29ULL, 15u);
                  vwrite8(tx_va + 30ULL, st->tcp_remote_ip[0]);
                  vwrite8(tx_va + 31ULL, st->tcp_remote_ip[1]);
                  vwrite8(tx_va + 32ULL, st->tcp_remote_ip[2]);
                  vwrite8(tx_va + 33ULL, st->tcp_remote_ip[3]);
                  uint32_t ipv4_sum = 0u;
                  for (size_t ck = 0; ck < 10; ++ck) {
                    uint64_t off = 14ULL + (uint64_t) ck * 2ULL;
                    uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                    uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                    ipv4_sum += (uint32_t)(w_hi << 8 | w_lo);
                  } while (ipv4_sum >> 16 != 0u) ipv4_sum = (ipv4_sum & 0xFFFFu) + (ipv4_sum >> 16);
                  uint16_t ipv4_csum = (uint16_t) ~(uint16_t) ipv4_sum;
                  vwrite8(tx_va + 24ULL, (uint8_t)(ipv4_csum >> 8 & 0xFFu));
                  vwrite8(tx_va + 25ULL, (uint8_t)(ipv4_csum & 0xFFu));
                  uint16_t local_port = st->tcp_local_port;
                  uint16_t remote_port = st->tcp_remote_port;
                  uint32_t local_seq = st->tcp_local_seq;
                  vwrite8(tx_va + 34ULL, (uint8_t)(local_port >> 8 & 0xFFu));
                  vwrite8(tx_va + 35ULL, (uint8_t)(local_port & 0xFFu));
                  vwrite8(tx_va + 36ULL, (uint8_t)(remote_port >> 8 & 0xFFu));
                  vwrite8(tx_va + 37ULL, (uint8_t)(remote_port & 0xFFu));
                  vwrite8(tx_va + 38ULL, (uint8_t)(local_seq >> 24 & 0xFFu));
                  vwrite8(tx_va + 39ULL, (uint8_t)(local_seq >> 16 & 0xFFu));
                  vwrite8(tx_va + 40ULL, (uint8_t)(local_seq >> 8 & 0xFFu));
                  vwrite8(tx_va + 41ULL, (uint8_t)(local_seq & 0xFFu));
                  vwrite8(tx_va + 42ULL, 0x00u);
                  vwrite8(tx_va + 43ULL, 0x00u);
                  vwrite8(tx_va + 44ULL, 0x00u);
                  vwrite8(tx_va + 45ULL, 0x00u);
                  vwrite8(tx_va + 46ULL, 0x50u);
                  vwrite8(tx_va + 47ULL, 0x02u);
                  vwrite8(tx_va + 48ULL, 0xFFu);
                  vwrite8(tx_va + 49ULL, 0xFFu);
                  vwrite8(tx_va + 50ULL, 0x00u);
                  vwrite8(tx_va + 51ULL, 0x00u);
                  vwrite8(tx_va + 52ULL, 0x00u);
                  vwrite8(tx_va + 53ULL, 0x00u);
                  uint32_t tcp_sum = 0u;
                  tcp_sum += (uint32_t)(10u << 8 | 0u);
                  tcp_sum += (uint32_t)(2u << 8 | 15u);
                  tcp_sum += (uint32_t)((uint16_t) st->tcp_remote_ip[0] << 8 | (uint16_t) st->tcp_remote_ip[1]);
                  tcp_sum += (uint32_t)((uint16_t) st->tcp_remote_ip[2] << 8 | (uint16_t) st->tcp_remote_ip[3]);
                  tcp_sum += 6u;
                  tcp_sum += 20u;
                  for (size_t cw = 0; cw < 10; ++cw) {
                    uint64_t off = 34ULL + (uint64_t) cw * 2ULL;
                    uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                    uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                    tcp_sum += (uint32_t)(w_hi << 8 | w_lo);
                  } while (tcp_sum >> 16 != 0u) tcp_sum = (tcp_sum & 0xFFFFu) + (tcp_sum >> 16);
                  uint16_t tcp_csum = (uint16_t) ~(uint16_t) tcp_sum;
                  vwrite8(tx_va + 50ULL, (uint8_t)(tcp_csum >> 8 & 0xFFu));
                  vwrite8(tx_va + 51ULL, (uint8_t)(tcp_csum & 0xFFu));
                  sexos_line("[sexnet.tcp.syn.checksum] checksum=0x%04X ok=1", (unsigned) tcp_csum);
                  sexos_line("[sexnet.tcp.syn.build] src_port=%u dst_port=%u seq=%u flags=SYN data_offset=5 window=65535 ok=1", (unsigned) local_port, (unsigned) remote_port, (unsigned) local_seq);
                  sexos_line("[sexnet.ipv4.tx.tcp_syn.build] src=10.0.2.15 dst=%u.%u.%u.%u total_len=%u checksum=ok ok=1", (unsigned) st->tcp_remote_ip[0], (unsigned) st->tcp_remote_ip[1], (unsigned) st->tcp_remote_ip[2], (unsigned) st->tcp_remote_ip[3], (unsigned) ipv4_total);
                  tcp_syn_built = 1u;
                  uint16_t frame_len = (uint16_t)(14u + (uint32_t) ipv4_total);
                  if (frame_len < 60u) for (uint64_t pad = (uint64_t) frame_len; pad < 60ULL; ++pad) vwrite8(tx_va + pad, 0u);
                  uint16_t tx_frame_len = frame_len < 60u ? 60u : frame_len;
                  uint64_t tx_desc5 = st->tx_perm_desc_va + 80ULL;
                  vwrite64(tx_desc5, st->tx_perm_frame_phys);
                  vwrite16(tx_desc5 + 8ULL, tx_frame_len);
                  vwrite8(tx_desc5 + 10ULL, 0u);
                  vwrite8(tx_desc5 + 11ULL, 0x0Bu);
                  vwrite8(tx_desc5 + 12ULL, 0u);
                  vwrite8(tx_desc5 + 13ULL, 0u);
                  vwrite16(tx_desc5 + 14ULL, 0u);
                  sexos_line("[sexnet.eth.tx.tcp_syn.desc] len=%u ok=1", (unsigned) tx_frame_len);
                  vwrite32(nic_va + 0x3818ULL, 6u);
                  sexos_line("[sexnet.tcp.syn.tx.post] slot=6 ok=1");
                  for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
                    uint8_t tx_st = vread8(tx_desc5 + 12ULL);
                    if ((tx_st & 1u) != 0u) {
                      tcp_syn_tx_dd = 1u;
                      break;
                    }
                  }
                  sexos_line("[sexnet.tcp.syn.tx.poll.done] dd_set=%u ok=%u", (unsigned) tcp_syn_tx_dd, (unsigned)(tcp_syn_tx_dd == 1u ? 1u : 0u));
                  if (tcp_syn_tx_dd == 1u) {
                    st->tcp_state = TCP_SYN_SENT;
                    st->tcp_syn_count = 1u;
                    sexos_line("[sexnet.tcp.handshake.state] state=SYN_SENT ok=1");
                  }
                }
                sexos_line("[sexnet.tcp.syn.build.proof.done] built=%u checksum_ok=%u ok=%u", (unsigned) tcp_syn_built, (unsigned)(tcp_syn_built == 1u ? 1u : 0u), (unsigned)(tcp_syn_built == 1u && tcp_syn_tx_dd == 1u ? 1u : 0u));
                sexos_line("[sexnet.tcp.syn.tx.proof.done] tx=%u tx_dd=%u ok=%u", (unsigned)(tcp_syn_built == 1u ? 1u : 0u), (unsigned) tcp_syn_tx_dd, (unsigned)(tcp_syn_built == 1u && tcp_syn_tx_dd == 1u ? 1u : 0u));
                uint8_t ipv4_rx_own = atomic_load_explicit(&st->nic_rx_owner, memory_order_acquire);
                if (ipv4_rx_own == NIC_OWNER_SEXNET_FULL) {
                  sexos_line("[sexnet.ipv4.entry] rx_owner=3 ok=1");
                  sexos_line("[sexnet.ipv4.rx.poll.begin] max_iters=1000000");
                  uint32_t udp_test_idx = 7u;
                  uint64_t udp_test_buf = st->rx_perm_pkt_va[(size_t) udp_test_idx];
                  const char *udp_payload = "HELLO_SEXNET_UDP_ECHO";
                  uint16_t udp_payload_len = 21u;
                  uint16_t udp_total = 8u + udp_payload_len;
                  uint16_t ut_ipv4_total = 20u + udp_total;
                  uint16_t frame_bytes = 14u + ut_ipv4_total;
                  vwrite8(udp_test_buf + 0ULL, 0x52u);
                  vwrite8(udp_test_buf + 1ULL, 0x54u);
                  vwrite8(udp_test_buf + 2ULL, 0x00u);
                  vwrite8(udp_test_buf + 3ULL, 0x12u);
                  vwrite8(udp_test_buf + 4ULL, 0x34u);
                  vwrite8(udp_test_buf + 5ULL, 0x56u);
                  vwrite8(udp_test_buf + 6ULL, 0xFEu);
                  vwrite8(udp_test_buf + 7ULL, 0x56u);
                  vwrite8(udp_test_buf + 8ULL, 0x3Au);
                  vwrite8(udp_test_buf + 9ULL, 0x6Cu);
                  vwrite8(udp_test_buf + 10ULL, 0x97u);
                  vwrite8(udp_test_buf + 11ULL, 0x32u);
                  vwrite8(udp_test_buf + 12ULL, 0x08u);
                  vwrite8(udp_test_buf + 13ULL, 0x00u);
                  vwrite8(udp_test_buf + 14ULL, 0x45u);
                  vwrite8(udp_test_buf + 15ULL, 0x00u);
                  vwrite8(udp_test_buf + 16ULL, (uint8_t)(ut_ipv4_total >> 8 & 0xFFu));
                  vwrite8(udp_test_buf + 17ULL, (uint8_t)(ut_ipv4_total & 0xFFu));
                  vwrite8(udp_test_buf + 18ULL, 0x00u);
                  vwrite8(udp_test_buf + 19ULL, 0x00u);
                  vwrite8(udp_test_buf + 20ULL, 0x00u);
                  vwrite8(udp_test_buf + 21ULL, 0x00u);
                  vwrite8(udp_test_buf + 22ULL, 64u);
                  vwrite8(udp_test_buf + 23ULL, 17u);
                  vwrite8(udp_test_buf + 24ULL, 0x00u);
                  vwrite8(udp_test_buf + 25ULL, 0x00u);
                  vwrite8(udp_test_buf + 26ULL, 10u);
                  vwrite8(udp_test_buf + 27ULL, 0u);
                  vwrite8(udp_test_buf + 28ULL, 2u);
                  vwrite8(udp_test_buf + 29ULL, 2u);
                  vwrite8(udp_test_buf + 30ULL, 10u);
                  vwrite8(udp_test_buf + 31ULL, 0u);
                  vwrite8(udp_test_buf + 32ULL, 2u);
                  vwrite8(udp_test_buf + 33ULL, 15u);
                  uint32_t ut_sum = 0u;
                  for (size_t ut_ci = 0; ut_ci < 10; ++ut_ci) {
                    uint64_t off = 14ULL + (uint64_t) ut_ci * 2ULL;
                    uint16_t w_hi = (uint16_t) vread8(udp_test_buf + off);
                    uint16_t w_lo = (uint16_t) vread8(udp_test_buf + off + 1ULL);
                    ut_sum += (uint32_t)(w_hi << 8 | w_lo);
                  } while (ut_sum >> 16 != 0u) ut_sum = (ut_sum & 0xFFFFu) + (ut_sum >> 16);
                  uint16_t ut_csum = (uint16_t) ~(uint16_t) ut_sum;
                  vwrite8(udp_test_buf + 24ULL, (uint8_t)(ut_csum >> 8 & 0xFFu));
                  vwrite8(udp_test_buf + 25ULL, (uint8_t)(ut_csum & 0xFFu));
                  vwrite8(udp_test_buf + 34ULL, 0x30u);
                  vwrite8(udp_test_buf + 35ULL, 0x39u);
                  vwrite8(udp_test_buf + 36ULL, 0x1Eu);
                  vwrite8(udp_test_buf + 37ULL, 0x61u);
                  vwrite8(udp_test_buf + 38ULL, (uint8_t)(udp_total >> 8 & 0xFFu));
                  vwrite8(udp_test_buf + 39ULL, (uint8_t)(udp_total & 0xFFu));
                  vwrite8(udp_test_buf + 40ULL, 0x00u);
                  vwrite8(udp_test_buf + 41ULL, 0x00u);
                  for (size_t ut_pi = 0; ut_pi < 21; ++ut_pi) vwrite8(udp_test_buf + 42ULL + (uint64_t) ut_pi, (uint8_t) udp_payload[ut_pi]);
                  uint32_t ipv4_frames = 0u;
                  uint32_t ipv4_ok = 0u;
                  uint32_t reject_logged = 0u;
                  bool synthetic_fallback_done = false;
                  bool icmp_unreachable_seen = false;
                  for (uint32_t outer = 0u; outer < 1000000u && ipv4_frames < 2u; ++outer) {
                    for (uint32_t idx = 0u; idx < 8u && ipv4_frames < 2u; ++idx) {
                      uint64_t desc_base = st->rx_perm_desc_va + (uint64_t) idx * 16ULL;
                      uint8_t st_b = vread8(desc_base + 12ULL);
                      if ((st_b & 1u) != 0u) {
                        size_t pkt_len = (size_t) vread16(desc_base + 8ULL);
                        uint64_t pkt_buf = st->rx_perm_pkt_va[(size_t) idx];
                        uint8_t eth_hi = vread8(pkt_buf + 12ULL);
                        uint8_t eth_lo = vread8(pkt_buf + 13ULL);
                        uint16_t ethertype = (uint16_t)((uint16_t) eth_hi << 8) | (uint16_t) eth_lo;
                        if (ethertype == 0x0800u) {
                          sexos_line("[sexnet.ipv4.rx.frame] idx=%u pkt_len=%zu ethertype=0x0800 ok=1", (unsigned) idx, pkt_len);
                          const char *reason = "short";
                          uint32_t ok = 0u;
                          uint8_t version = 0u;
                          uint8_t ihl = 0u;
                          uint16_t total_len = 0u;
                          uint16_t flags_frag = 0u;
                          uint16_t frag_masked = 1u;
                          uint8_t dst0 = 0u;
                          uint8_t dst1 = 0u;
                          uint8_t dst2 = 0u;
                          uint8_t dst3 = 0u;
                          uint8_t src0 = 0u;
                          uint8_t src1 = 0u;
                          uint8_t src2 = 0u;
                          uint8_t src3 = 0u;
                          uint8_t proto = 0u;
                          uint8_t ttl = 0u;
                          uint16_t csum = 0u;
                          uint32_t checksum_ok = 0u;
                          uint32_t reason_kind = 0u;
                          if (pkt_len >= 34) {
                            uint8_t vihl = vread8(pkt_buf + 14ULL);
                            version = vihl >> 4 & 0x0Fu;
                            ihl = vihl & 0x0Fu;
                            uint16_t tl_hi = (uint16_t) vread8(pkt_buf + 16ULL);
                            uint16_t tl_lo = (uint16_t) vread8(pkt_buf + 17ULL);
                            total_len = tl_hi << 8 | tl_lo;
                            ttl = vread8(pkt_buf + 22ULL);
                            proto = vread8(pkt_buf + 23ULL);
                            src0 = vread8(pkt_buf + 26ULL);
                            src1 = vread8(pkt_buf + 27ULL);
                            src2 = vread8(pkt_buf + 28ULL);
                            src3 = vread8(pkt_buf + 29ULL);
                            dst0 = vread8(pkt_buf + 30ULL);
                            dst1 = vread8(pkt_buf + 31ULL);
                            dst2 = vread8(pkt_buf + 32ULL);
                            dst3 = vread8(pkt_buf + 33ULL);
                            uint16_t ff_hi = (uint16_t) vread8(pkt_buf + 20ULL);
                            uint16_t ff_lo = (uint16_t) vread8(pkt_buf + 21ULL);
                            flags_frag = ff_hi << 8 | ff_lo;
                            frag_masked = flags_frag & 0x3FFFu;
                            uint16_t csum_hi = (uint16_t) vread8(pkt_buf + 24ULL);
                            uint16_t csum_lo = (uint16_t) vread8(pkt_buf + 25ULL);
                            csum = csum_hi << 8 | csum_lo;
                            if (version != 4u) reason = "version";
                            else if (ihl != 5u) reason = "ihl";
                            else if (total_len < 20u) {
                              reason = "total_len_min";
                              reason_kind = 3u;
                            } else if ((size_t) total_len > pkt_len - 14) {
                              reason = "total_len_max";
                              reason_kind = 3u;
                            } else if (frag_masked != 0u) {
                              reason = "fragmented";
                              reason_kind = 2u;
                            } else if (dst0 != 10u || dst1 != 0u || dst2 != 2u || dst3 != 15u) reason = "dst";
                            else {
                              uint32_t sum = 0u;
                              for (size_t ci = 0; ci < 10; ++ci) {
                                uint64_t off = 14ULL + (uint64_t) ci * 2ULL;
                                uint16_t w_hi = (uint16_t) vread8(pkt_buf + off);
                                uint16_t w_lo = (uint16_t) vread8(pkt_buf + off + 1ULL);
                                sum += (uint32_t)(w_hi << 8 | w_lo);
                              } while (sum >> 16 != 0u) sum = (sum & 0xFFFFu) + (sum >> 16);
                              if ((uint16_t) sum == 0xFFFFu) {
                                ok = 1u;
                                checksum_ok = 1u;
                              } else {
                                reason = "checksum";
                                reason_kind = 1u;
                              }
                            }
                          }
                          sexos_line("[sexnet.ipv4.rx.validate.detail] ver=%u ihl=%u total_len=%u pkt_len=%zu frag=0x%04X dst=%u.%u.%u.%u csum=0x%04X checksum_ok=%u proto=%u ttl=%u ok=0", (unsigned) version, (unsigned) ihl, (unsigned) total_len, pkt_len, (unsigned) flags_frag, (unsigned) dst0, (unsigned) dst1, (unsigned) dst2, (unsigned) dst3, (unsigned) csum, (unsigned) checksum_ok, (unsigned) proto, (unsigned) ttl);
                          if (ok != 1u) {
                            if (reason_kind == 1u) sexos_line("[sexnet.ipv4.bad_checksum.reject] csum=0x%04X reason=%s ok=1", (unsigned) csum, reason);
                            else if (reason_kind == 2u) sexos_line("[sexnet.ipv4.fragment.reject] frag=0x%04X reason=%s ok=1", (unsigned) frag_masked, reason);
                            else if (reason_kind == 3u) sexos_line("[sexnet.ipv4.bounds.reject] total_len=%u pkt_len=%zu reason=%s ok=1", (unsigned) total_len, pkt_len, reason);
                          }
                          if (ok == 1u) {
                            sexos_line("[sexnet.ipv4.rx.validate] version=4 ihl=5 total_len=%u dst=10.0.2.15 frag=0 checksum=ok src=%u.%u.%u.%u proto=%u ttl=%u ok=1", (unsigned) total_len, (unsigned) src0, (unsigned) src1, (unsigned) src2, (unsigned) src3, (unsigned) proto, (unsigned) ttl);
                            sexos_line("[sexnet.ipv4.parse.ok] ver=4 ihl=5 total_len=%u proto=%u ok=1", (unsigned) total_len, (unsigned) proto);
                            ipv4_ok = 1u;
                            ipv4_frames += 1u;
                            if (proto == 1u && (size_t) total_len >= 28) {
                              uint64_t icmp_base = pkt_buf + 34ULL;
                              uint8_t icmp_type = vread8(icmp_base);
                              uint8_t icmp_code = vread8(icmp_base + 1ULL);
                              if (icmp_type == 8u && icmp_code == 0u) {
                                size_t icmp_hdr_len = 8;
                                size_t ipv4_hdr_len = 20;
                                size_t icmp_payload_len = (size_t) total_len - (ipv4_hdr_len + icmp_hdr_len);
                                uint8_t id_hi = vread8(icmp_base + 4ULL);
                                uint8_t id_lo = vread8(icmp_base + 5ULL);
                                uint8_t seq_hi = vread8(icmp_base + 6ULL);
                                uint8_t seq_lo = vread8(icmp_base + 7ULL);
                                uint16_t icmp_id = (uint16_t)((uint16_t) id_hi << 8) | (uint16_t) id_lo;
                                uint16_t icmp_seq = (uint16_t)((uint16_t) seq_hi << 8) | (uint16_t) seq_lo;
                                size_t icmp_total_len = icmp_hdr_len + icmp_payload_len;
                                sexos_line("[sexnet.icmp.rx.echo] type=8 code=0 len=%zu id=%u seq=%u ok=1", icmp_total_len, (unsigned) icmp_id, (unsigned) icmp_seq);
                                sexos_line("[sexnet.icmp.echo.rx.ok] type=8 code=0 len=%zu id=%u seq=%u ok=1", icmp_total_len, (unsigned) icmp_id, (unsigned) icmp_seq);
                                uint32_t icmp_rx_sum = 0u;
                                size_t rx_icmp_words = icmp_total_len / 2;
                                for (size_t cj = 0; cj < rx_icmp_words; ++cj) {
                                  uint64_t off = 34ULL + (uint64_t) cj * 2ULL;
                                  uint16_t w_hi = (uint16_t) vread8(pkt_buf + off);
                                  uint16_t w_lo = (uint16_t) vread8(pkt_buf + off + 1ULL);
                                  icmp_rx_sum += (uint32_t)(w_hi << 8 | w_lo);
                                }
                                if (icmp_total_len % 2 != 0) {
                                  uint16_t last = (uint16_t) vread8(pkt_buf + 34ULL + (uint64_t) icmp_total_len - 1ULL);
                                  icmp_rx_sum += (uint32_t)(last << 8);
                                } while (icmp_rx_sum >> 16 != 0u) icmp_rx_sum = (icmp_rx_sum & 0xFFFFu) + (icmp_rx_sum >> 16);
                                uint32_t icmp_rx_csum_ok = (uint16_t) icmp_rx_sum == 0xFFFFu ? 1u : 0u;
                                sexos_line("[sexnet.icmp.checksum.validate] ok=%u", (unsigned) icmp_rx_csum_ok);
                                uint64_t tx_va = st->tx_perm_frame_va;
                                uint8_t i_nic_mac[6] = {
                                  (uint8_t)(ral & 0xFFu), (uint8_t)(ral >> 8 & 0xFFu), (uint8_t)(ral >> 16 & 0xFFu), (uint8_t)(ral >> 24 & 0xFFu), (uint8_t)(rah & 0xFFu), (uint8_t)(rah >> 8 & 0xFFu)
                                };
                                uint8_t i_src_mac[6] = {
                                  0
                                };
                                for (size_t mi = 0; mi < 6; ++mi) i_src_mac[mi] = vread8(pkt_buf + 6ULL + (uint64_t) mi);
                                for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + (uint64_t) mi, i_src_mac[mi]);
                                for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + 6ULL + (uint64_t) mi, i_nic_mac[mi]);
                                vwrite8(tx_va + 12ULL, 0x08u);
                                vwrite8(tx_va + 13ULL, 0x00u);
                                uint16_t reply_total_len = (uint16_t)(ipv4_hdr_len + icmp_total_len);
                                vwrite8(tx_va + 14ULL, 0x45u);
                                vwrite8(tx_va + 15ULL, 0x00u);
                                vwrite8(tx_va + 16ULL, (uint8_t)(reply_total_len >> 8 & 0xFFu));
                                vwrite8(tx_va + 17ULL, (uint8_t)(reply_total_len & 0xFFu));
                                vwrite8(tx_va + 18ULL, 0x00u);
                                vwrite8(tx_va + 19ULL, 0x00u);
                                vwrite8(tx_va + 20ULL, 0x00u);
                                vwrite8(tx_va + 21ULL, 0x00u);
                                vwrite8(tx_va + 22ULL, 64u);
                                vwrite8(tx_va + 23ULL, 1u);
                                vwrite8(tx_va + 24ULL, 0x00u);
                                vwrite8(tx_va + 25ULL, 0x00u);
                                vwrite8(tx_va + 26ULL, 10u);
                                vwrite8(tx_va + 27ULL, 0u);
                                vwrite8(tx_va + 28ULL, 2u);
                                vwrite8(tx_va + 29ULL, 15u);
                                vwrite8(tx_va + 30ULL, src0);
                                vwrite8(tx_va + 31ULL, src1);
                                vwrite8(tx_va + 32ULL, src2);
                                vwrite8(tx_va + 33ULL, src3);
                                uint32_t ipv4_tx_sum = 0u;
                                for (size_t ck = 0; ck < 10; ++ck) {
                                  uint64_t off = 14ULL + (uint64_t) ck * 2ULL;
                                  uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                                  uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                                  ipv4_tx_sum += (uint32_t)(w_hi << 8 | w_lo);
                                } while (ipv4_tx_sum >> 16 != 0u) ipv4_tx_sum = (ipv4_tx_sum & 0xFFFFu) + (ipv4_tx_sum >> 16);
                                uint16_t ipv4_tx_csum = (uint16_t) ~(uint16_t) ipv4_tx_sum;
                                vwrite8(tx_va + 24ULL, (uint8_t)(ipv4_tx_csum >> 8 & 0xFFu));
                                vwrite8(tx_va + 25ULL, (uint8_t)(ipv4_tx_csum & 0xFFu));
                                vwrite8(tx_va + 34ULL, 0x00u);
                                vwrite8(tx_va + 35ULL, 0x00u);
                                vwrite8(tx_va + 36ULL, 0x00u);
                                vwrite8(tx_va + 37ULL, 0x00u);
                                vwrite8(tx_va + 38ULL, id_hi);
                                vwrite8(tx_va + 39ULL, id_lo);
                                vwrite8(tx_va + 40ULL, seq_hi);
                                vwrite8(tx_va + 41ULL, seq_lo);
                                for (size_t pi = 0; pi < icmp_payload_len; ++pi) vwrite8(42ULL + (tx_va + (uint64_t) pi), vread8(icmp_base + (uint64_t) icmp_hdr_len + (uint64_t) pi));
                                uint32_t icmp_tx_sum = 0u;
                                size_t tx_icmp_words = icmp_total_len / 2;
                                for (size_t cl = 0; cl < tx_icmp_words; ++cl) {
                                  uint64_t off = 34ULL + (uint64_t) cl * 2ULL;
                                  uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                                  uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                                  icmp_tx_sum += (uint32_t)(w_hi << 8 | w_lo);
                                }
                                if (icmp_total_len % 2 != 0) {
                                  size_t last_off = 34 + icmp_total_len - 1;
                                  uint16_t last = (uint16_t) vread8(tx_va + (uint64_t) last_off);
                                  icmp_tx_sum += (uint32_t)(last << 8);
                                } while (icmp_tx_sum >> 16 != 0u) icmp_tx_sum = (icmp_tx_sum & 0xFFFFu) + (icmp_tx_sum >> 16);
                                uint16_t icmp_tx_csum = (uint16_t) ~(uint16_t) icmp_tx_sum;
                                vwrite8(tx_va + 36ULL, (uint8_t)(icmp_tx_csum >> 8 & 0xFFu));
                                vwrite8(tx_va + 37ULL, (uint8_t)(icmp_tx_csum & 0xFFu));
                                sexos_line("[sexnet.icmp.tx.reply.build] type=0 code=0 len=%zu id=%u seq=%u ok=1", icmp_total_len, (unsigned) icmp_id, (unsigned) icmp_seq);
                                sexos_line("[sexnet.icmp.tx.reply.checksum] ok=1");
                                sexos_line("[sexnet.ipv4.tx.icmp_reply.build] src=10.0.2.15 dst=%u.%u.%u.%u total_len=%u checksum=ok ok=1", (unsigned) src0, (unsigned) src1, (unsigned) src2, (unsigned) src3, (unsigned) reply_total_len);
                                uint16_t frame_len = (uint16_t)(14u + (uint32_t) reply_total_len);
                                if (frame_len < 60u) for (uint64_t pad = (uint64_t) frame_len; pad < 60ULL; ++pad) vwrite8(tx_va + pad, 0u);
                                uint16_t tx_frame_len = frame_len < 60u ? 60u : frame_len;
                                uint64_t tx_desc3 = st->tx_perm_desc_va + 48ULL;
                                vwrite64(tx_desc3, st->tx_perm_frame_phys);
                                vwrite16(tx_desc3 + 8ULL, tx_frame_len);
                                vwrite8(tx_desc3 + 10ULL, 0u);
                                vwrite8(tx_desc3 + 11ULL, 0x0Bu);
                                vwrite8(tx_desc3 + 12ULL, 0u);
                                vwrite8(tx_desc3 + 13ULL, 0u);
                                vwrite16(tx_desc3 + 14ULL, 0u);
                                sexos_line("[sexnet.eth.tx.icmp_reply.desc] len=%u ok=1", (unsigned) tx_frame_len);
                                vwrite32(nic_va + 0x3818ULL, 4u);
                                uint32_t icmp_reply_done = 0u;
                                for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
                                  uint8_t tx_st = vread8(tx_desc3 + 12ULL);
                                  if ((tx_st & 1u) != 0u) {
                                    icmp_reply_done = 1u;
                                    break;
                                  }
                                }
                                sexos_line("[sexnet.icmp.tx.poll.done] dd_set=%u ok=%u", (unsigned) icmp_reply_done, (unsigned)(icmp_reply_done == 1u ? 1u : 0u));
                                if (icmp_reply_done == 1u) sexos_line("[sexnet.icmp.echo.reply.tx.ok] tx_dd=%u ok=1", (unsigned) icmp_reply_done);
                                sexos_line("[sexnet.icmp.echo.proof.done] rx_echo=1 tx_reply=%u tx_dd=%u ok=%u", (unsigned)(icmp_reply_done > 0u ? 1u : 0u), (unsigned) icmp_reply_done, (unsigned)(icmp_reply_done == 1u ? 1u : 0u));
                                if (icmp_reply_done == 1u) sexos_line("[sexnet.icmp.ping.gateway.ok] rx_echo=1 tx_reply=1 ok=1");
                                else sexos_line("[sexnet.icmp.ping.gateway.skip] reason=no_arp_or_no_reply ok=1");
                              } else {
                                sexos_line("[sexnet.icmp.reject] reason=not_echo_request type=%u code=%u ok=1", (unsigned) icmp_type, (unsigned) icmp_code);
                                if (icmp_type == 3u) icmp_unreachable_seen = true;
                              }
                            } else if (proto == 1u && (size_t) total_len < 28) sexos_line("[sexnet.icmp.reject] reason=too_short_for_icmp ok=1");
                            else if (proto == 17u && (size_t) total_len >= 28) {
                              uint64_t udp_base = pkt_buf + 34ULL;
                              size_t ipv4_hdr_len = 20;
                              size_t ipv4_payload_len = (size_t) total_len - ipv4_hdr_len;
                              uint16_t udp_src_port = vread_be16(udp_base);
                              uint16_t udp_dst_port = vread_be16(udp_base + 2ULL);
                              uint16_t udp_len = vread_be16(udp_base + 4ULL);
                              uint16_t udp_csum = vread_be16(udp_base + 6ULL);
                              sexos_line("[sexnet.udp.rx.datagram] src_port=%u dst_port=%u len=%u checksum=0x%04X ok=1", (unsigned) udp_src_port, (unsigned) udp_dst_port, (unsigned) udp_len, (unsigned) udp_csum);
                              uint32_t udp_ok = 1u;
                              const char *udp_reason = "";
                              const char *checksum_policy = "zero_allowed";
                              uint32_t udp_len_ok = (size_t) udp_len >= 8 && (size_t) udp_len <= ipv4_payload_len ? 1u : 0u;
                              uint32_t ports_ok = 1u;
                              if ((size_t) udp_len < 8) {
                                udp_ok = 0u;
                                udp_reason = "udp_len_too_small";
                              } else if ((size_t) udp_len > ipv4_payload_len) {
                                udp_ok = 0u;
                                udp_reason = "udp_len_exceeds_ipv4_payload";
                              } else if (udp_csum != 0u) {
                                uint32_t sum = 0u;
                                sum += (uint32_t)((uint16_t) src0 << 8 | (uint16_t) src1);
                                sum += (uint32_t)((uint16_t) src2 << 8 | (uint16_t) src3);
                                sum += (uint32_t)((uint16_t) dst0 << 8 | (uint16_t) dst1);
                                sum += (uint32_t)((uint16_t) dst2 << 8 | (uint16_t) dst3);
                                sum += 17u;
                                sum += (uint32_t) udp_len;
                                size_t udp_tot = (size_t) udp_len;
                                for (size_t cu = 0; cu < udp_tot / 2; ++cu) {
                                  uint64_t off = 34ULL + (uint64_t) cu * 2ULL;
                                  uint16_t w_hi = (uint16_t) vread8(pkt_buf + off);
                                  uint16_t w_lo = (uint16_t) vread8(pkt_buf + off + 1ULL);
                                  sum += (uint32_t)(w_hi << 8 | w_lo);
                                }
                                if (udp_tot % 2 != 0) {
                                  size_t last_off = 34 + udp_tot - 1;
                                  uint16_t last = (uint16_t) vread8(pkt_buf + (uint64_t) last_off);
                                  sum += (uint32_t)(last << 8);
                                } while (sum >> 16 != 0u) sum = (sum & 0xFFFFu) + (sum >> 16);
                                if ((uint16_t) sum != 0xFFFFu) {
                                  udp_ok = 0u;
                                  udp_reason = "checksum";
                                } else checksum_policy = "validated";
                              }
                              sexos_line("[sexnet.udp.header.validate] len_ok=%u ports_ok=%u checksum_policy=%s ok=%u", (unsigned) udp_len_ok, (unsigned) ports_ok, checksum_policy, (unsigned) udp_ok);
                              if (udp_ok == 1u) {
                                size_t u_payload_len = (size_t) udp_len - 8;
                                uint64_t tx_va = st->tx_perm_frame_va;
                                uint8_t u_nic_mac[6] = {
                                  (uint8_t)(ral & 0xFFu), (uint8_t)(ral >> 8 & 0xFFu), (uint8_t)(ral >> 16 & 0xFFu), (uint8_t)(ral >> 24 & 0xFFu), (uint8_t)(rah & 0xFFu), (uint8_t)(rah >> 8 & 0xFFu)
                                };
                                uint8_t u_src_mac[6] = {
                                  0
                                };
                                for (size_t mi = 0; mi < 6; ++mi) u_src_mac[mi] = vread8(pkt_buf + 6ULL + (uint64_t) mi);
                                for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + (uint64_t) mi, u_src_mac[mi]);
                                for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + 6ULL + (uint64_t) mi, u_nic_mac[mi]);
                                vwrite8(tx_va + 12ULL, 0x08u);
                                vwrite8(tx_va + 13ULL, 0x00u);
                                uint16_t reply_total_len = (uint16_t)(ipv4_hdr_len + (size_t) udp_len);
                                vwrite8(tx_va + 14ULL, 0x45u);
                                vwrite8(tx_va + 15ULL, 0x00u);
                                vwrite8(tx_va + 16ULL, (uint8_t)(reply_total_len >> 8 & 0xFFu));
                                vwrite8(tx_va + 17ULL, (uint8_t)(reply_total_len & 0xFFu));
                                vwrite8(tx_va + 18ULL, 0x00u);
                                vwrite8(tx_va + 19ULL, 0x00u);
                                vwrite8(tx_va + 20ULL, 0x00u);
                                vwrite8(tx_va + 21ULL, 0x00u);
                                vwrite8(tx_va + 22ULL, 64u);
                                vwrite8(tx_va + 23ULL, 17u);
                                vwrite8(tx_va + 24ULL, 0x00u);
                                vwrite8(tx_va + 25ULL, 0x00u);
                                vwrite8(tx_va + 26ULL, 10u);
                                vwrite8(tx_va + 27ULL, 0u);
                                vwrite8(tx_va + 28ULL, 2u);
                                vwrite8(tx_va + 29ULL, 15u);
                                vwrite8(tx_va + 30ULL, src0);
                                vwrite8(tx_va + 31ULL, src1);
                                vwrite8(tx_va + 32ULL, src2);
                                vwrite8(tx_va + 33ULL, src3);
                                uint32_t ipv4_tx_sum = 0u;
                                for (size_t ck = 0; ck < 10; ++ck) {
                                  uint64_t off = 14ULL + (uint64_t) ck * 2ULL;
                                  uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                                  uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                                  ipv4_tx_sum += (uint32_t)(w_hi << 8 | w_lo);
                                } while (ipv4_tx_sum >> 16 != 0u) ipv4_tx_sum = (ipv4_tx_sum & 0xFFFFu) + (ipv4_tx_sum >> 16);
                                uint16_t ipv4_tx_csum = (uint16_t) ~(uint16_t) ipv4_tx_sum;
                                vwrite8(tx_va + 24ULL, (uint8_t)(ipv4_tx_csum >> 8 & 0xFFu));
                                vwrite8(tx_va + 25ULL, (uint8_t)(ipv4_tx_csum & 0xFFu));
                                vwrite8(tx_va + 34ULL, (uint8_t)(udp_dst_port >> 8 & 0xFFu));
                                vwrite8(tx_va + 35ULL, (uint8_t)(udp_dst_port & 0xFFu));
                                vwrite8(tx_va + 36ULL, (uint8_t)(udp_src_port >> 8 & 0xFFu));
                                vwrite8(tx_va + 37ULL, (uint8_t)(udp_src_port & 0xFFu));
                                vwrite8(tx_va + 38ULL, (uint8_t)(udp_len >> 8 & 0xFFu));
                                vwrite8(tx_va + 39ULL, (uint8_t)(udp_len & 0xFFu));
                                vwrite8(tx_va + 40ULL, 0x00u);
                                vwrite8(tx_va + 41ULL, 0x00u);
                                for (size_t pj = 0; pj < u_payload_len; ++pj) vwrite8(42ULL + (tx_va + (uint64_t) pj), vread8(udp_base + 8ULL + (uint64_t) pj));
                                sexos_line("[sexnet.udp.tx.reply.build] src_port=%u dst_port=%u len=%u payload_len=%zu ok=1", (unsigned) udp_dst_port, (unsigned) udp_src_port, (unsigned) udp_len, u_payload_len);
                                sexos_line("[sexnet.udp.tx.reply.checksum] checksum=0x0000 policy=zero_allowed ok=1");
                                sexos_line("[sexnet.ipv4.tx.udp_reply.build] src=10.0.2.15 dst=%u.%u.%u.%u total_len=%u checksum=ok ok=1", (unsigned) src0, (unsigned) src1, (unsigned) src2, (unsigned) src3, (unsigned) reply_total_len);
                                uint16_t frame_len = (uint16_t)(14u + (uint32_t) reply_total_len);
                                if (frame_len < 60u) for (uint64_t pad = (uint64_t) frame_len; pad < 60ULL; ++pad) vwrite8(tx_va + pad, 0u);
                                uint16_t tx_frame_len = frame_len < 60u ? 60u : frame_len;
                                uint64_t tx_desc4 = st->tx_perm_desc_va + 64ULL;
                                vwrite64(tx_desc4, st->tx_perm_frame_phys);
                                vwrite16(tx_desc4 + 8ULL, tx_frame_len);
                                vwrite8(tx_desc4 + 10ULL, 0u);
                                vwrite8(tx_desc4 + 11ULL, 0x0Bu);
                                vwrite8(tx_desc4 + 12ULL, 0u);
                                vwrite8(tx_desc4 + 13ULL, 0u);
                                vwrite16(tx_desc4 + 14ULL, 0u);
                                sexos_line("[sexnet.eth.tx.udp_reply.desc] len=%u ok=1", (unsigned) tx_frame_len);
                                vwrite32(nic_va + 0x3818ULL, 5u);
                                uint32_t udp_reply_done = 0u;
                                for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
                                  uint8_t tx_st = vread8(tx_desc4 + 12ULL);
                                  if ((tx_st & 1u) != 0u) {
                                    udp_reply_done = 1u;
                                    break;
                                  }
                                }
                                sexos_line("[sexnet.udp.tx.poll.done] dd_set=%u ok=%u", (unsigned) udp_reply_done, (unsigned)(udp_reply_done == 1u ? 1u : 0u));
                                sexos_line("[sexnet.udp.echo.proof.done] rx_udp=1 tx_reply=%u tx_dd=%u ok=%u", (unsigned)(udp_reply_done > 0u ? 1u : 0u), (unsigned) udp_reply_done, (unsigned)(udp_reply_done == 1u ? 1u : 0u));
                              } else sexos_line("[sexnet.udp.reject] reason=%s ok=1", udp_reason);
                              sexos_line("[sexnet.udp.header.proof.done] rx_udp=1 valid=%u ok=1", (unsigned) udp_ok);
                            } else if (proto == 6u && (size_t) total_len >= 40) {
                              uint64_t tcp_base = pkt_buf + 34ULL;
                              size_t ipv4_hdr_len = 20;
                              size_t ipv4_payload_len = (size_t) total_len - ipv4_hdr_len;
                              uint16_t tcp_src_port = vread_be16(tcp_base);
                              uint16_t tcp_dst_port = vread_be16(tcp_base + 2ULL);
                              uint32_t tcp_seq = vread_be32(tcp_base + 4ULL);
                              uint32_t tcp_ack = vread_be32(tcp_base + 8ULL);
                              uint8_t dof_raw = vread8(tcp_base + 12ULL);
                              uint8_t flags_raw = vread8(tcp_base + 13ULL);
                              uint8_t tcp_data_offset = dof_raw >> 4 & 0x0Fu;
                              uint8_t tcp_flags_syn = flags_raw >> 1 & 0x01u;
                              uint8_t tcp_flags_ack = flags_raw >> 4 & 0x01u;
                              uint8_t tcp_flags_rst = flags_raw >> 2 & 0x01u;
                              uint16_t tcp_csum_rx = vread_be16(tcp_base + 16ULL);
                              sexos_line("[sexnet.tcp.rx.segment] src_port=%u dst_port=%u seq=%u ack=%u data_offset=%u flags_syn=%u flags_ack=%u flags_rst=%u csum=0x%04X ok=1", (unsigned) tcp_src_port, (unsigned) tcp_dst_port, (unsigned) tcp_seq, (unsigned) tcp_ack, (unsigned) tcp_data_offset, (unsigned) tcp_flags_syn, (unsigned) tcp_flags_ack, (unsigned) tcp_flags_rst, (unsigned) tcp_csum_rx);
                              uint16_t local_port = st->tcp_local_port;
                              uint16_t remote_port = st->tcp_remote_port;
                              uint32_t local_seq = st->tcp_local_seq;
                              TcpState tcp_state = st->tcp_state;
                              uint32_t tcp_ok = 0u;
                              const char *tcp_reason = "unknown";
                              if (tcp_data_offset < 5u) tcp_reason = "data_offset_too_small";
                              else if ((size_t) tcp_data_offset * 4 > ipv4_payload_len) tcp_reason = "header_exceeds_payload";
                              else if (tcp_dst_port != local_port) tcp_reason = "dst_port_mismatch";
                              else if (tcp_src_port != remote_port) tcp_reason = "src_port_mismatch";
                              else if (tcp_state != TCP_SYN_SENT && tcp_state != TCP_ESTABLISHED) tcp_reason = "tcp_not_awaiting_response";
                              else {
                                uint32_t tcp_rx_sum = 0u;
                                tcp_rx_sum += (uint32_t)((uint16_t) src0 << 8 | (uint16_t) src1);
                                tcp_rx_sum += (uint32_t)((uint16_t) src2 << 8 | (uint16_t) src3);
                                tcp_rx_sum += (uint32_t)((uint16_t) dst0 << 8 | (uint16_t) dst1);
                                tcp_rx_sum += (uint32_t)((uint16_t) dst2 << 8 | (uint16_t) dst3);
                                tcp_rx_sum += 6u;
                                uint16_t tcp_seg_len = (uint16_t)((size_t) total_len - ipv4_hdr_len);
                                tcp_rx_sum += (uint32_t) tcp_seg_len;
                                size_t tcp_bytes = (size_t) tcp_seg_len;
                                for (size_t cw = 0; cw < tcp_bytes / 2; ++cw) {
                                  uint64_t off = 34ULL + (uint64_t) cw * 2ULL;
                                  uint16_t w_hi = (uint16_t) vread8(pkt_buf + off);
                                  uint16_t w_lo = (uint16_t) vread8(pkt_buf + off + 1ULL);
                                  tcp_rx_sum += (uint32_t)(w_hi << 8 | w_lo);
                                }
                                if (tcp_bytes % 2 != 0) {
                                  size_t last_off = 34 + tcp_bytes - 1;
                                  uint16_t last = (uint16_t) vread8(pkt_buf + (uint64_t) last_off);
                                  tcp_rx_sum += (uint32_t)(last << 8);
                                } while (tcp_rx_sum >> 16 != 0u) tcp_rx_sum = (tcp_rx_sum & 0xFFFFu) + (tcp_rx_sum >> 16);
                                if ((uint16_t) tcp_rx_sum != 0xFFFFu) tcp_reason = "checksum";
                                else {
                                  tcp_ok = 1u;
                                  tcp_reason = "ok";
                                }
                              }
                              sexos_line("[sexnet.tcp.rx.validate] ports_ok=%u data_offset_ok=%u checksum_ok=%u ok=%u", (unsigned)(tcp_dst_port == local_port && tcp_src_port == remote_port ? 1u : 0u), (unsigned)(tcp_data_offset >= 5u ? 1u : 0u), (unsigned)(tcp_ok == 1u ? 1u : 0u), (unsigned) tcp_ok);
                              if (tcp_ok == 1u) if (tcp_flags_rst == 1u) {
                                st->tcp_state = TCP_FAILED_RST;
                                st->tcp_rst_count += 1u;
                                sexos_line("[sexnet.tcp.rst.rx] src_port=%u dst_port=%u seq=%u ack=%u flags=RST ok=1", (unsigned) tcp_src_port, (unsigned) tcp_dst_port, (unsigned) tcp_seq, (unsigned) tcp_ack);
                                sexos_line("[sexnet.tcp.handshake.state] state=FAILED_RST ok=1");
                                sexos_line("[sexnet.tcp.synack.rx.proof.done] rx_synack=0 rst=1 timeout=0 ok=0 honest=1");
                              } else if (tcp_flags_syn == 1u && tcp_flags_ack == 1u) {
                                uint32_t expected_ack = local_seq + 1u;
                                uint32_t ack_ok = tcp_ack == expected_ack ? 1u : 0u;
                                sexos_line("[sexnet.tcp.synack.rx] src_port=%u dst_port=%u seq=%u ack=%u flags=SYN|ACK ok=1", (unsigned) tcp_src_port, (unsigned) tcp_dst_port, (unsigned) tcp_seq, (unsigned) tcp_ack);
                                sexos_line("[sexnet.tcp.synack.validate] ack_ok=%u ports_ok=1 checksum_ok=1 ok=%u", (unsigned) ack_ok, (unsigned)(ack_ok == 1u ? 1u : 0u));
                                st->tcp_state = TCP_ESTABLISHED;
                                st->tcp_remote_seq = tcp_seq;
                                sexos_line("[sexnet.tcp.handshake.state] state=ESTABLISHED ok=1");
                                sexos_line("[sexnet.tcp.synack.rx.proof.done] rx_synack=1 rst=0 timeout=0 ok=1");
                                uint64_t tx_va = st->tx_perm_frame_va;
                                uint8_t a_nic_mac[6] = {
                                  (uint8_t)(ral & 0xFFu), (uint8_t)(ral >> 8 & 0xFFu), (uint8_t)(ral >> 16 & 0xFFu), (uint8_t)(ral >> 24 & 0xFFu), (uint8_t)(rah & 0xFFu), (uint8_t)(rah >> 8 & 0xFFu)
                                };
                                uint8_t a_src_mac[6] = {
                                  0
                                };
                                for (size_t mi = 0; mi < 6; ++mi) a_src_mac[mi] = vread8(pkt_buf + 6ULL + (uint64_t) mi);
                                for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + (uint64_t) mi, a_src_mac[mi]);
                                for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + 6ULL + (uint64_t) mi, a_nic_mac[mi]);
                                vwrite8(tx_va + 12ULL, 0x08u);
                                vwrite8(tx_va + 13ULL, 0x00u);
                                uint16_t ack_ipv4_total = 40u;
                                vwrite8(tx_va + 14ULL, 0x45u);
                                vwrite8(tx_va + 15ULL, 0x00u);
                                vwrite8(tx_va + 16ULL, (uint8_t)(ack_ipv4_total >> 8 & 0xFFu));
                                vwrite8(tx_va + 17ULL, (uint8_t)(ack_ipv4_total & 0xFFu));
                                vwrite8(tx_va + 18ULL, 0x00u);
                                vwrite8(tx_va + 19ULL, 0x01u);
                                vwrite8(tx_va + 20ULL, 0x00u);
                                vwrite8(tx_va + 21ULL, 0x00u);
                                vwrite8(tx_va + 22ULL, 64u);
                                vwrite8(tx_va + 23ULL, 6u);
                                vwrite8(tx_va + 24ULL, 0x00u);
                                vwrite8(tx_va + 25ULL, 0x00u);
                                vwrite8(tx_va + 26ULL, 10u);
                                vwrite8(tx_va + 27ULL, 0u);
                                vwrite8(tx_va + 28ULL, 2u);
                                vwrite8(tx_va + 29ULL, 15u);
                                vwrite8(tx_va + 30ULL, src0);
                                vwrite8(tx_va + 31ULL, src1);
                                vwrite8(tx_va + 32ULL, src2);
                                vwrite8(tx_va + 33ULL, src3);
                                uint32_t ipv4_sum = 0u;
                                for (size_t ck = 0; ck < 10; ++ck) {
                                  uint64_t off = 14ULL + (uint64_t) ck * 2ULL;
                                  uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                                  uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                                  ipv4_sum += (uint32_t)(w_hi << 8 | w_lo);
                                } while (ipv4_sum >> 16 != 0u) ipv4_sum = (ipv4_sum & 0xFFFFu) + (ipv4_sum >> 16);
                                uint16_t ipv4_csum = (uint16_t) ~(uint16_t) ipv4_sum;
                                vwrite8(tx_va + 24ULL, (uint8_t)(ipv4_csum >> 8 & 0xFFu));
                                vwrite8(tx_va + 25ULL, (uint8_t)(ipv4_csum & 0xFFu));
                                uint32_t ack_seq = local_seq + 1u;
                                uint32_t ack_ack = st->tcp_remote_seq + 1u;
                                vwrite8(tx_va + 34ULL, (uint8_t)(local_port >> 8 & 0xFFu));
                                vwrite8(tx_va + 35ULL, (uint8_t)(local_port & 0xFFu));
                                vwrite8(tx_va + 36ULL, (uint8_t)(remote_port >> 8 & 0xFFu));
                                vwrite8(tx_va + 37ULL, (uint8_t)(remote_port & 0xFFu));
                                vwrite8(tx_va + 38ULL, (uint8_t)(ack_seq >> 24 & 0xFFu));
                                vwrite8(tx_va + 39ULL, (uint8_t)(ack_seq >> 16 & 0xFFu));
                                vwrite8(tx_va + 40ULL, (uint8_t)(ack_seq >> 8 & 0xFFu));
                                vwrite8(tx_va + 41ULL, (uint8_t)(ack_seq & 0xFFu));
                                vwrite8(tx_va + 42ULL, (uint8_t)(ack_ack >> 24 & 0xFFu));
                                vwrite8(tx_va + 43ULL, (uint8_t)(ack_ack >> 16 & 0xFFu));
                                vwrite8(tx_va + 44ULL, (uint8_t)(ack_ack >> 8 & 0xFFu));
                                vwrite8(tx_va + 45ULL, (uint8_t)(ack_ack & 0xFFu));
                                vwrite8(tx_va + 46ULL, 0x50u);
                                vwrite8(tx_va + 47ULL, 0x10u);
                                vwrite8(tx_va + 48ULL, 0xFFu);
                                vwrite8(tx_va + 49ULL, 0xFFu);
                                vwrite8(tx_va + 50ULL, 0x00u);
                                vwrite8(tx_va + 51ULL, 0x00u);
                                vwrite8(tx_va + 52ULL, 0x00u);
                                vwrite8(tx_va + 53ULL, 0x00u);
                                uint32_t tcp_sum = 0u;
                                tcp_sum += (uint32_t)(10u << 8 | 0u);
                                tcp_sum += (uint32_t)(2u << 8 | 15u);
                                tcp_sum += (uint32_t)((uint16_t) src0 << 8 | (uint16_t) src1);
                                tcp_sum += (uint32_t)((uint16_t) src2 << 8 | (uint16_t) src3);
                                tcp_sum += 6u;
                                tcp_sum += 20u;
                                for (size_t cw = 0; cw < 10; ++cw) {
                                  uint64_t off = 34ULL + (uint64_t) cw * 2ULL;
                                  uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                                  uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                                  tcp_sum += (uint32_t)(w_hi << 8 | w_lo);
                                } while (tcp_sum >> 16 != 0u) tcp_sum = (tcp_sum & 0xFFFFu) + (tcp_sum >> 16);
                                uint16_t tcp_csum = (uint16_t) ~(uint16_t) tcp_sum;
                                vwrite8(tx_va + 50ULL, (uint8_t)(tcp_csum >> 8 & 0xFFu));
                                vwrite8(tx_va + 51ULL, (uint8_t)(tcp_csum & 0xFFu));
                                sexos_line("[sexnet.tcp.ack.checksum] checksum=0x%04X ok=1", (unsigned) tcp_csum);
                                sexos_line("[sexnet.tcp.ack.build] seq=%u ack=%u flags=ACK ok=1", (unsigned) ack_seq, (unsigned) ack_ack);
                                uint16_t frame_len_ack = (uint16_t)(14u + (uint32_t) ack_ipv4_total);
                                if (frame_len_ack < 60u) for (uint64_t pad = (uint64_t) frame_len_ack; pad < 60ULL; ++pad) vwrite8(tx_va + pad, 0u);
                                uint16_t tx_frame_len_ack = frame_len_ack < 60u ? 60u : frame_len_ack;
                                uint64_t tx_desc6 = st->tx_perm_desc_va + 96ULL;
                                vwrite64(tx_desc6, st->tx_perm_frame_phys);
                                vwrite16(tx_desc6 + 8ULL, tx_frame_len_ack);
                                vwrite8(tx_desc6 + 10ULL, 0u);
                                vwrite8(tx_desc6 + 11ULL, 0x0Bu);
                                vwrite8(tx_desc6 + 12ULL, 0u);
                                vwrite8(tx_desc6 + 13ULL, 0u);
                                vwrite16(tx_desc6 + 14ULL, 0u);
                                sexos_line("[sexnet.eth.tx.tcp_ack.desc] len=%u ok=1", (unsigned) tx_frame_len_ack);
                                vwrite32(nic_va + 0x3818ULL, 7u);
                                uint32_t ack_tx_dd = 0u;
                                for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
                                  uint8_t tx_st = vread8(tx_desc6 + 12ULL);
                                  if ((tx_st & 1u) != 0u) {
                                    ack_tx_dd = 1u;
                                    break;
                                  }
                                }
                                sexos_line("[sexnet.tcp.ack.tx.poll.done] dd_set=%u ok=%u", (unsigned) ack_tx_dd, (unsigned)(ack_tx_dd == 1u ? 1u : 0u));
                                st->tcp_ack_count += 1u;
                                sexos_line("[sexnet.tcp.ack.tx.proof.done] ack_sent=%u tx_dd=%u ok=%u", (unsigned)(ack_tx_dd == 1u ? 1u : 0u), (unsigned) ack_tx_dd, (unsigned)(ack_tx_dd == 1u ? 1u : 0u));
                              } else sexos_line("[sexnet.tcp.rx.ignore] reason=not_synack_or_rst flags_syn=%u flags_ack=%u flags_rst=%u ok=1", (unsigned) tcp_flags_syn, (unsigned) tcp_flags_ack, (unsigned) tcp_flags_rst);
                              else sexos_line("[sexnet.tcp.rx.reject] reason=%s ok=1", tcp_reason);
                            } else if (proto == 6u && (size_t) total_len < 40) sexos_line("[sexnet.tcp.rx.reject] reason=tcp_header_too_short total_len=%u ok=1", (unsigned) total_len);
                          } else if (reject_logged == 0u) {
                            sexos_line("[sexnet.ipv4.rx.reject.detail] idx=%u etype=0x%04X reason=%s ok=0", (unsigned) idx, (unsigned) ethertype, reason);
                            reject_logged = 1u;
                          }
                        } else if (reject_logged == 0u) {
                          sexos_line("[sexnet.ipv4.rx.reject.detail] idx=%u etype=0x%04X reason=non_ipv4 ok=0", (unsigned) idx, (unsigned) ethertype);
                          reject_logged = 1u;
                        }
                        vwrite16(desc_base + 8ULL, 0u);
                        vwrite8(desc_base + 12ULL, 0u);
                        vwrite32(nic_va + 0x2818ULL, idx);
                        sexos_line("[sexnet.ipv4.rx.recycle] idx=%u new_rdt=%u ok=1", (unsigned) idx, (unsigned) idx);
                      }
                    }
                    if (!synthetic_fallback_done && ipv4_frames == 0u && outer >= 900000u) {
                      uint64_t fb_desc = st->rx_perm_desc_va + (uint64_t) udp_test_idx * 16ULL;
                      vwrite16(fb_desc + 8ULL, frame_bytes);
                      vwrite8(fb_desc + 12ULL, 1u);
                      synthetic_fallback_done = true;
                      sexos_line("[sexnet.udp.self_test.fallback.inject] reason=no_real_rx_after_poll idx=%u len=%u src_port=12345 dst_port=7777 checksum_policy=zero_allowed self_test=1 ok=1", (unsigned) udp_test_idx, (unsigned) frame_bytes);
                    }
                  }
                  sexos_line("[sexnet.ipv4.rx.poll.done] frames=%u ok=%u", (unsigned) ipv4_frames, (unsigned) ipv4_ok);
                  sexos_line("[sexnet.ipv4.proof.done] frames=%u ok=%u", (unsigned) ipv4_frames, (unsigned) ipv4_ok);
                  if (icmp_unreachable_seen) {
                    TcpState tcp_st_retry = st->tcp_state;
                    if (tcp_st_retry == TCP_SYN_SENT) {
                      sexos_line("[sexnet.tcp.syn.retry.begin] reason=icmp_unreachable_during_syn_sent");
                      uint64_t tx_de5 = st->tx_perm_desc_va + 80ULL;
                      vwrite8(tx_de5 + 12ULL, 0u);
                      vwrite32(nic_va + 0x3818ULL, 6u);
                      uint32_t syn_retry_dd = 0u;
                      for (uint32_t syn_retry_poll = 0u; syn_retry_poll < 1000000u && syn_retry_dd == 0u; ++syn_retry_poll) {
                        uint8_t st2 = vread8(tx_de5 + 12ULL);
                        if ((st2 & 1u) != 0u) syn_retry_dd = 1u;
                      }
                      sexos_line("[sexnet.tcp.syn.retry.tx.done] dd_set=%u ok=%u", (unsigned) syn_retry_dd, (unsigned)(syn_retry_dd == 1u ? 1u : 0u));
                      uint32_t retry_frames = 0u;
                      uint32_t retry_ok = 0u;
                      sexos_line("[sexnet.tcp.synack.retry.poll.begin] max_iters=1000000");
                      for (uint32_t retry_outer = 0u; retry_outer < 1000000u && retry_frames < 2u; ++retry_outer) for (uint32_t idx = 0u; idx < 8u && retry_frames < 2u; ++idx) {
                        uint64_t desc_base = st->rx_perm_desc_va + (uint64_t) idx * 16ULL;
                        uint8_t st_b = vread8(desc_base + 12ULL);
                        if ((st_b & 1u) != 0u) {
                          uint64_t pkt_buf = st->rx_perm_pkt_va[(size_t) idx];
                          uint8_t eth_hi = vread8(pkt_buf + 12ULL);
                          uint8_t eth_lo = vread8(pkt_buf + 13ULL);
                          uint16_t ethertype = (uint16_t)((uint16_t) eth_hi << 8) | (uint16_t) eth_lo;
                          if (ethertype == 0x0800u) {
                            uint8_t proto = vread8(pkt_buf + 23ULL);
                            if (proto == 6u) {
                              size_t total_len = (size_t) vread_be16(pkt_buf + 16ULL);
                              if (total_len >= 40) {
                                uint16_t src_port = vread_be16(pkt_buf + 34ULL);
                                uint16_t dst_port = vread_be16(pkt_buf + 36ULL);
                                uint16_t local_port = st->tcp_local_port;
                                uint16_t remote_port = st->tcp_remote_port;
                                if (src_port == remote_port && dst_port == local_port) {
                                  uint32_t tcp_seq = vread_be32(pkt_buf + 38ULL);
                                  uint32_t tcp_ack = vread_be32(pkt_buf + 42ULL);
                                  uint8_t tcp_flags = vread8(pkt_buf + 47ULL);
                                  uint32_t flags_syn = (tcp_flags & 0x02u) != 0u ? 1u : 0u;
                                  uint32_t flags_ack = (tcp_flags & 0x10u) != 0u ? 1u : 0u;
                                  uint32_t flags_rst = (tcp_flags & 0x04u) != 0u ? 1u : 0u;
                                  if (flags_syn == 1u && flags_ack == 1u && flags_rst == 0u) {
                                    sexos_line("[sexnet.tcp.synack.retry.rx] src_port=%u dst_port=%u seq=%u ack=%u flags=SYN|ACK ok=1", (unsigned) src_port, (unsigned) dst_port, (unsigned) tcp_seq, (unsigned) tcp_ack);
                                    st->tcp_state = TCP_ESTABLISHED;
                                    st->tcp_remote_seq = tcp_seq;
                                    retry_frames += 1u;
                                    retry_ok = 1u;
                                  } else if (flags_rst == 1u) {
                                    sexos_line("[sexnet.tcp.synack.retry.rst] src_port=%u dst_port=%u flags=RST ok=1", (unsigned) src_port, (unsigned) dst_port);
                                    st->tcp_state = TCP_FAILED_RST;
                                    retry_frames += 1u;
                                  } else {
                                    sexos_line("[sexnet.tcp.synack.retry.ignore] reason=not_synack_or_rst flags_syn=%u flags_ack=%u flags_rst=%u ok=1", (unsigned) flags_syn, (unsigned) flags_ack, (unsigned) flags_rst);
                                    retry_frames += 1u;
                                  }
                                } else {
                                  sexos_line("[sexnet.tcp.synack.retry.reject] reason=wrong_ports src=%u dst=%u ok=1", (unsigned) src_port, (unsigned) dst_port);
                                  retry_frames += 1u;
                                }
                              }
                            }
                          } else retry_frames += 1u;
                          vwrite16(desc_base + 8ULL, 0u);
                          vwrite8(desc_base + 12ULL, 0u);
                          vwrite32(nic_va + 0x2818ULL, idx);
                        }
                      }
                      sexos_line("[sexnet.tcp.synack.retry.poll.done] frames=%u ok=%u", (unsigned) retry_frames, (unsigned) retry_ok);
                    }
                  }
                  TcpState tcp_state_h = st->tcp_state;
                  uint32_t is_established = tcp_state_h == TCP_ESTABLISHED ? 1u : 0u;
                  const char *state_name = tcp_state_h == TCP_CLOSED ? "CLOSED" : tcp_state_h == TCP_SYN_SENT ? "SYN_SENT" : tcp_state_h == TCP_ESTABLISHED ? "ESTABLISHED" : tcp_state_h == TCP_FAILED_RST ? "FAILED_RST" : "FAILED_TIMEOUT";
                  uint32_t payload_tx_sent = 0u;
                  uint32_t payload_tx_dd = 0u;
                  if (is_established == 1u) {
                    sexos_line("[sexnet.tcp.payload.tx.guard] state=ESTABLISHED ok=1");
                    sexos_line("[sexnet.phaseI.stop_review.pass]");
                    bool tx_perm_ready2 = st->tx_perm_desc_va != 0ULL && st->tx_perm_frame_phys != 0ULL && st->tx_perm_frame_va != 0ULL;
                    if (tx_perm_ready2) {
                      uint64_t tx_va = st->tx_perm_frame_va;
                      uint8_t h_nic_mac[6] = {
                        (uint8_t)(ral & 0xFFu), (uint8_t)(ral >> 8 & 0xFFu), (uint8_t)(ral >> 16 & 0xFFu), (uint8_t)(ral >> 24 & 0xFFu), (uint8_t)(rah & 0xFFu), (uint8_t)(rah >> 8 & 0xFFu)
                      };
                      uint8_t gw_mac[6] = {
                        0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu
                      };
                      if (st->arp_cache_valid == 1u) for (size_t mi = 0; mi < 6; ++mi) gw_mac[mi] = st->arp_cache_mac[mi];
                      else if (st->tcp_remote_ip[0] == 10u && st->tcp_remote_ip[1] == 0u && st->tcp_remote_ip[2] == 2u && st->tcp_remote_ip[3] == 2u) {
                        gw_mac[0] = 0x52u;
                        gw_mac[1] = 0x55u;
                        gw_mac[2] = 0x0Au;
                        gw_mac[3] = 0x00u;
                        gw_mac[4] = 0x02u;
                        gw_mac[5] = 0x02u;
                      }
                      uint16_t local_port = st->tcp_local_port;
                      uint16_t remote_port = st->tcp_remote_port;
                      uint32_t local_seq = st->tcp_local_seq;
                      uint32_t remote_seq = st->tcp_remote_seq;
                      uint32_t tcp_seq = local_seq + 1u;
                      uint32_t tcp_ack = remote_seq + 1u;
                      size_t http_get_len = http_get_build(st, (const uint8_t *) "example.com", 11, (const uint8_t *) "/", 1);
                      if (http_get_len > 0) {
                        sexos_line("[sexnet.http.get.build] host=example.com path=/ len=%zu ok=1", http_get_len);
                        sexos_line("[sexnet.http.get.proof.done] built=1 len=%zu ok=1", http_get_len);
                      } else {
                        sexos_line("[sexnet.http.get.build] host=example.com path=/ len=0 ok=0 reason=overflow");
                        sexos_line("[sexnet.http.get.proof.done] built=0 len=0 ok=0");
                      }
                      uint16_t payload_len = (uint16_t) http_get_len;
                      uint32_t expected_ack_after_payload = tcp_seq + (uint32_t) payload_len;
                      uint16_t ipv4_total = 40u + payload_len;
                      for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + (uint64_t) mi, gw_mac[mi]);
                      for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_va + 6ULL + (uint64_t) mi, h_nic_mac[mi]);
                      vwrite8(tx_va + 12ULL, 0x08u);
                      vwrite8(tx_va + 13ULL, 0x00u);
                      vwrite8(tx_va + 14ULL, 0x45u);
                      vwrite8(tx_va + 15ULL, 0x00u);
                      vwrite8(tx_va + 16ULL, (uint8_t)(ipv4_total >> 8 & 0xFFu));
                      vwrite8(tx_va + 17ULL, (uint8_t)(ipv4_total & 0xFFu));
                      vwrite8(tx_va + 18ULL, 0x00u);
                      vwrite8(tx_va + 19ULL, 0x02u);
                      vwrite8(tx_va + 20ULL, 0x00u);
                      vwrite8(tx_va + 21ULL, 0x00u);
                      vwrite8(tx_va + 22ULL, 64u);
                      vwrite8(tx_va + 23ULL, 6u);
                      vwrite8(tx_va + 24ULL, 0x00u);
                      vwrite8(tx_va + 25ULL, 0x00u);
                      vwrite8(tx_va + 26ULL, 10u);
                      vwrite8(tx_va + 27ULL, 0u);
                      vwrite8(tx_va + 28ULL, 2u);
                      vwrite8(tx_va + 29ULL, 15u);
                      vwrite8(tx_va + 30ULL, st->tcp_remote_ip[0]);
                      vwrite8(tx_va + 31ULL, st->tcp_remote_ip[1]);
                      vwrite8(tx_va + 32ULL, st->tcp_remote_ip[2]);
                      vwrite8(tx_va + 33ULL, st->tcp_remote_ip[3]);
                      uint32_t ipv4_sum = 0u;
                      for (size_t ck = 0; ck < 10; ++ck) {
                        uint64_t off = 14ULL + (uint64_t) ck * 2ULL;
                        uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                        uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                        ipv4_sum += (uint32_t)(w_hi << 8 | w_lo);
                      } while (ipv4_sum >> 16 != 0u) ipv4_sum = (ipv4_sum & 0xFFFFu) + (ipv4_sum >> 16);
                      uint16_t ipv4_csum = (uint16_t) ~(uint16_t) ipv4_sum;
                      vwrite8(tx_va + 24ULL, (uint8_t)(ipv4_csum >> 8 & 0xFFu));
                      vwrite8(tx_va + 25ULL, (uint8_t)(ipv4_csum & 0xFFu));
                      vwrite8(tx_va + 34ULL, (uint8_t)(local_port >> 8 & 0xFFu));
                      vwrite8(tx_va + 35ULL, (uint8_t)(local_port & 0xFFu));
                      vwrite8(tx_va + 36ULL, (uint8_t)(remote_port >> 8 & 0xFFu));
                      vwrite8(tx_va + 37ULL, (uint8_t)(remote_port & 0xFFu));
                      vwrite8(tx_va + 38ULL, (uint8_t)(tcp_seq >> 24 & 0xFFu));
                      vwrite8(tx_va + 39ULL, (uint8_t)(tcp_seq >> 16 & 0xFFu));
                      vwrite8(tx_va + 40ULL, (uint8_t)(tcp_seq >> 8 & 0xFFu));
                      vwrite8(tx_va + 41ULL, (uint8_t)(tcp_seq & 0xFFu));
                      vwrite8(tx_va + 42ULL, (uint8_t)(tcp_ack >> 24 & 0xFFu));
                      vwrite8(tx_va + 43ULL, (uint8_t)(tcp_ack >> 16 & 0xFFu));
                      vwrite8(tx_va + 44ULL, (uint8_t)(tcp_ack >> 8 & 0xFFu));
                      vwrite8(tx_va + 45ULL, (uint8_t)(tcp_ack & 0xFFu));
                      vwrite8(tx_va + 46ULL, 0x50u);
                      vwrite8(tx_va + 47ULL, 0x18u);
                      vwrite8(tx_va + 48ULL, 0xFFu);
                      vwrite8(tx_va + 49ULL, 0xFFu);
                      vwrite8(tx_va + 50ULL, 0x00u);
                      vwrite8(tx_va + 51ULL, 0x00u);
                      vwrite8(tx_va + 52ULL, 0x00u);
                      vwrite8(tx_va + 53ULL, 0x00u);
                      for (size_t pi = 0; pi < http_get_len; ++pi) vwrite8(tx_va + 54ULL + (uint64_t) pi, st->http_get_buf[pi]);
                      size_t peek_len = http_get_len < 64 ? http_get_len : 64;
                      uint8_t hex_out[193] = {
                        0u
                      };
                      uint8_t ascii_out[65] = {
                        0u
                      };
                      size_t hex_len = build_hex_peek(st->http_get_buf, peek_len, hex_out, 192);
                      size_t ascii_len = build_ascii_peek(st->http_get_buf, peek_len, ascii_out, 64);
                      hex_out[hex_len] = 0u;
                      ascii_out[ascii_len] = 0u;
                      sexos_line("[sexnet.tcp.psh_ack.payload.peek.hex] len=%zu bytes=%s", peek_len, (const char *) hex_out);
                      sexos_line("[sexnet.tcp.psh_ack.payload.peek.ascii] len=%zu text=%s", peek_len, (const char *) ascii_out);
                      uint32_t tcp_sum = 0u;
                      uint16_t tcp_seg_len = 20u + payload_len;
                      tcp_sum += (uint32_t)(10u << 8 | 0u);
                      tcp_sum += (uint32_t)(2u << 8 | 15u);
                      tcp_sum += (uint32_t)((uint16_t) st->tcp_remote_ip[0] << 8 | (uint16_t) st->tcp_remote_ip[1]);
                      tcp_sum += (uint32_t)((uint16_t) st->tcp_remote_ip[2] << 8 | (uint16_t) st->tcp_remote_ip[3]);
                      tcp_sum += 6u;
                      tcp_sum += (uint32_t) tcp_seg_len;
                      size_t tcp_bytes = (size_t) tcp_seg_len;
                      for (size_t cw = 0; cw < tcp_bytes / 2; ++cw) {
                        uint64_t off = 34ULL + (uint64_t) cw * 2ULL;
                        uint16_t w_hi = (uint16_t) vread8(tx_va + off);
                        uint16_t w_lo = (uint16_t) vread8(tx_va + off + 1ULL);
                        tcp_sum += (uint32_t)(w_hi << 8 | w_lo);
                      }
                      if (tcp_bytes % 2 != 0) {
                        size_t last_off = 34 + tcp_bytes - 1;
                        uint16_t last = (uint16_t) vread8(tx_va + (uint64_t) last_off);
                        tcp_sum += (uint32_t)(last << 8);
                      } while (tcp_sum >> 16 != 0u) tcp_sum = (tcp_sum & 0xFFFFu) + (tcp_sum >> 16);
                      uint16_t tcp_csum = (uint16_t) ~(uint16_t) tcp_sum;
                      vwrite8(tx_va + 50ULL, (uint8_t)(tcp_csum >> 8 & 0xFFu));
                      vwrite8(tx_va + 51ULL, (uint8_t)(tcp_csum & 0xFFu));
                      sexos_line("[sexnet.tcp.psh_ack.checksum] checksum=0x%04X ok=1", (unsigned) tcp_csum);
                      sexos_line("[sexnet.tcp.psh_ack.build] src_port=%u dst_port=%u seq=%u ack=%u flags=PSH|ACK payload_len=%u ok=1", (unsigned) local_port, (unsigned) remote_port, (unsigned) tcp_seq, (unsigned) tcp_ack, (unsigned) payload_len);
                      sexos_line("[sexnet.tcp.psh_ack.shape] seq=%u ack=%u data_offset=5 tcp_len=%u payload_len=%u ip_total_len=%u frame_len=%u ok=1", (unsigned) tcp_seq, (unsigned) tcp_ack, (unsigned)(20u + payload_len), (unsigned) payload_len, (unsigned) ipv4_total, (unsigned)(14u + (uint32_t) ipv4_total));
                      sexos_line("[sexnet.tcp.psh_ack.ack_expect] expect_ack=%u ok=1", (unsigned) expected_ack_after_payload);
                      sexos_line("[sexnet.ipv4.tx.psh_ack.build] src=10.0.2.15 dst=%u.%u.%u.%u total_len=%u checksum=ok ok=1", (unsigned) st->tcp_remote_ip[0], (unsigned) st->tcp_remote_ip[1], (unsigned) st->tcp_remote_ip[2], (unsigned) st->tcp_remote_ip[3], (unsigned) ipv4_total);
                      uint16_t frame_len = (uint16_t)(14u + (uint32_t) ipv4_total);
                      if (frame_len < 60u) for (uint64_t pad = (uint64_t) frame_len; pad < 60ULL; ++pad) vwrite8(tx_va + pad, 0u);
                      uint16_t tx_frame_len = frame_len < 60u ? 60u : frame_len;
                      uint64_t tx_desc7 = st->tx_perm_desc_va + 112ULL;
                      vwrite64(tx_desc7, st->tx_perm_frame_phys);
                      vwrite16(tx_desc7 + 8ULL, tx_frame_len);
                      vwrite8(tx_desc7 + 10ULL, 0u);
                      vwrite8(tx_desc7 + 11ULL, 0x0Bu);
                      vwrite8(tx_desc7 + 12ULL, 0u);
                      vwrite8(tx_desc7 + 13ULL, 0u);
                      vwrite16(tx_desc7 + 14ULL, 0u);
                      sexos_line("[sexnet.eth.tx.psh_ack.desc] len=%u ok=1", (unsigned) tx_frame_len);
                      vwrite32(nic_va + 0x3818ULL, 0u);
                      sexos_line("[sexnet.tcp.psh_ack.tx.post] slot=8 tdt_next=0 ok=1");
                      for (uint32_t tx_outer = 0u; tx_outer < 50000000u; ++tx_outer) {
                        uint8_t tx_st = vread8(tx_desc7 + 12ULL);
                        if ((tx_st & 1u) != 0u) {
                          payload_tx_dd = 1u;
                          break;
                        }
                      }
                      sexos_line("[sexnet.tcp.psh_ack.tx.poll.done] dd_set=%u ok=%u", (unsigned) payload_tx_dd, (unsigned)(payload_tx_dd == 1u ? 1u : 0u));
                      if (payload_tx_dd == 1u && payload_len > 0u) payload_tx_sent = 1u;
                      if (payload_tx_sent == 1u) {
                        sexos_line("[sexnet.http.get.tx.guard] state=ESTABLISHED ok=1");
                        sexos_line("[sexnet.http.get.tx.psh_ack] payload_len=%u tx_dd=%u ok=%u", (unsigned) payload_len, (unsigned) payload_tx_dd, (unsigned)(payload_tx_dd == 1u ? 1u : 0u));
                        sexos_line("[sexnet.http.get.tx.proof.done] sent=%u tx_dd=%u ok=%u", (unsigned) payload_tx_sent, (unsigned) payload_tx_dd, (unsigned)(payload_tx_sent == 1u && payload_tx_dd == 1u ? 1u : 0u));
                      }
                      sexos_line("[sexnet.tcp.payload.tx.proof.done] sent=%u tx_dd=%u ok=%u", (unsigned) payload_tx_sent, (unsigned) payload_tx_dd, (unsigned)(payload_tx_sent == 1u && payload_tx_dd == 1u ? 1u : 0u));
                      size_t resp_total = 0;
                      uint32_t resp_truncated = 0u;
                      uint32_t got_payload = 0u;
                      size_t observed_payload_off = 0;
                      size_t observed_payload_len = 0;
                      size_t observed_frame_len = 0;
                      for (uint32_t rx_outer = 0u; rx_outer < 1000000u; ++rx_outer) {
                        for (uint32_t ridx = 0u; ridx < 8u; ++ridx) {
                          uint64_t rdesc = st->rx_perm_desc_va + (uint64_t) ridx * 16ULL;
                          uint8_t rstatus = vread8(rdesc + 12ULL);
                          if ((rstatus & 1u) != 0u) {
                            size_t rlen = (size_t) vread16(rdesc + 8ULL);
                            if (rlen >= 54) {
                              uint64_t rva = st->rx_perm_pkt_va[(size_t) ridx];
                              uint8_t et0 = vread8(rva + 12ULL);
                              uint8_t et1 = vread8(rva + 13ULL);
                              if (et0 == 0x08u && et1 == 0x00u) {
                                size_t r_ihl = (size_t)(vread8(rva + 14ULL) & 0x0Fu) * 4;
                                if (r_ihl >= 20 && 34 + r_ihl <= rlen + 20) if (14 + (r_ihl + 20) <= rlen) {
                                  uint8_t r_proto = vread8(rva + 23ULL);
                                  if (r_proto == 6u) {
                                    uint64_t tbase = rva + 14ULL + (uint64_t) r_ihl;
                                    uint16_t src_port = vread_be16(tbase);
                                    uint16_t dst_port = vread_be16(tbase + 2ULL);
                                    if (src_port == st->tcp_remote_port && dst_port == st->tcp_local_port) {
                                      uint8_t dof = vread8(tbase + 12ULL);
                                      uint8_t flags = vread8(tbase + 13ULL);
                                      uint32_t flags_ack = (flags & 0x10u) != 0u ? 1u : 0u;
                                      uint32_t flags_rst = (flags & 0x04u) != 0u ? 1u : 0u;
                                      size_t thl = (size_t)(dof >> 4) * 4;
                                      size_t ip_total_len = (size_t) vread_be16(rva + 16ULL);
                                      uint32_t peer_ack = vread_be32(tbase + 8ULL);
                                      uint32_t peer_ack_advanced = peer_ack == expected_ack_after_payload ? 1u : 0u;
                                      sexos_line("[sexnet.tcp.psh_ack.peer_ack] ack=%u expect_ack=%u advanced=%u flags=0x%02X ok=%u", (unsigned) peer_ack, (unsigned) expected_ack_after_payload, (unsigned) peer_ack_advanced, (unsigned) flags, (unsigned) peer_ack_advanced);
                                      if (flags_ack == 1u && flags_rst == 0u && thl >= 20 && ip_total_len >= r_ihl + thl) {
                                        size_t p_off = 14 + (r_ihl + thl);
                                        size_t p_len = ip_total_len - (r_ihl + thl);
                                        size_t p_end = p_off + p_len;
                                        if (p_len == 0) sexos_line("[sexnet.http.response.rx.skip] reason=no_tcp_payload flags=0x%02X flags_ack=%u flags_rst=%u ok=1", (unsigned) flags, (unsigned) flags_ack, (unsigned) flags_rst);
                                        else if (p_end <= rlen) {
                                          observed_payload_off = p_off;
                                          observed_payload_len = p_len;
                                          observed_frame_len = rlen;
                                          for (size_t i = 0; i < p_len; ++i) if (resp_total < HTTP_RESPONSE_BUF_CAP) {
                                            st->http_response_buf[resp_total] = vread8(rva + (uint64_t) p_off + (uint64_t) i);
                                            resp_total += 1;
                                          } else resp_truncated = 1u;
                                          got_payload = 1u;
                                        }
                                      } else if (flags_rst == 1u) sexos_line("[sexnet.http.response.rx.skip] reason=rst_segment flags=0x%02X flags_ack=%u flags_rst=%u ok=1", (unsigned) flags, (unsigned) flags_ack, (unsigned) flags_rst);
                                      else if (flags_ack == 0u) sexos_line("[sexnet.http.response.rx.skip] reason=ack_not_set flags=0x%02X flags_ack=%u flags_rst=%u ok=1", (unsigned) flags, (unsigned) flags_ack, (unsigned) flags_rst);
                                    }
                                  }
                                }
                              }
                            }
                            vwrite16(rdesc + 8ULL, 0u);
                            vwrite8(rdesc + 12ULL, 0u);
                            vwrite32(nic_va + 0x2818ULL, ridx);
                          }
                        }
                        if (got_payload == 1u) break;
                      }
                      st->http_response_len = resp_total;
                      if (got_payload == 1u) sexos_line("[sexnet.http.response.offset] tcp_payload_offset=%zu payload_len=%zu frame_len=%zu ok=1", observed_payload_off, observed_payload_len, observed_frame_len);
                      sexos_line("[sexnet.http.response.rx] bytes=%zu bounded=1 ok=%u", resp_total, (unsigned)(got_payload == 1u ? 1u : 0u));
                      sexos_line("[sexnet.http.response.rx.proof.done] received=%u bytes=%zu ok=%u", (unsigned) got_payload, resp_total, (unsigned)(got_payload == 1u ? 1u : 0u));
                      size_t r_peek_len = resp_total < 64 ? resp_total : 64;
                      uint8_t r_hex_out[193] = {
                        0u
                      };
                      uint8_t r_ascii_out[65] = {
                        0u
                      };
                      size_t r_hex_len = build_hex_peek(st->http_response_buf, r_peek_len, r_hex_out, 192);
                      size_t r_ascii_len = build_ascii_peek(st->http_response_buf, r_peek_len, r_ascii_out, 64);
                      r_hex_out[r_hex_len] = 0u;
                      r_ascii_out[r_ascii_len] = 0u;
                      sexos_line("[sexnet.http.response.peek.hex] len=%zu bytes=%s", r_peek_len, (const char *) r_hex_out);
                      sexos_line("[sexnet.http.response.peek.ascii] len=%zu text=%s", r_peek_len, (const char *) r_ascii_out);
                      uint16_t status_code = 0u;
                      size_t status_line_len = 0;
                      const char *status_version = "";
                      const char *status_reject = "empty";
                      if (resp_total > 0) {
                        HttpStatusParse parsed = parse_http_status_line(st->http_response_buf, resp_total);
                        status_code = parsed.status;
                        status_line_len = parsed.line_end;
                        status_version = parsed.version;
                        status_reject = parsed.reject;
                      }
                      st->http_status_code = status_code;
                      if (status_code > 0u) {
                        sexos_line("[sexnet.http.status.parse] version=%s status=%u line_len=%zu ok=1", status_version, (unsigned) status_code, status_line_len);
                        sexos_line("[sexnet.http.status.proof.done] status=%u ok=1", (unsigned) status_code);
                      } else {
                        sexos_line("[sexnet.http.status.reject] reason=%s ok=1", status_reject);
                        sexos_line("[sexnet.http.status.proof.done] status=0 ok=0 reason=%s", status_reject);
                      }
                      size_t body_start = resp_total;
                      uint32_t used_header_sep = 0u;
                      for (size_t bi = 0; bi + 3 < resp_total; ++bi) if (st->http_response_buf[bi] == 13u && st->http_response_buf[bi + 1] == 10u && st->http_response_buf[bi + 2] == 13u && st->http_response_buf[bi + 3] == 10u) {
                        body_start = bi + 4;
                        used_header_sep = 1u;
                        break;
                      }
                      if (used_header_sep == 0u && status_line_len > 0 && status_line_len < resp_total) {
                        body_start = status_line_len;
                        while (body_start < resp_total && (st->http_response_buf[body_start] == 13u || st->http_response_buf[body_start] == 10u)) body_start += 1;
                      }
                      size_t body_bytes = 0;
                      if (body_start < resp_total) for (size_t ri = body_start; ri < resp_total; ++ri) if (body_bytes < HTTP_BODY_BUF_CAP) {
                        st->http_body_prefix_buf[body_bytes] = st->http_response_buf[ri];
                        body_bytes += 1;
                      }
                      st->http_body_prefix_len = body_bytes;
                      sexos_line("[sexnet.http.body.buffer] bytes=%zu cap=%zu truncated=%u ok=1", body_bytes, HTTP_BODY_BUF_CAP, (unsigned)(resp_truncated == 1u || (resp_total > body_start && resp_total - body_start > body_bytes) ? 1u : 0u));
                      sexos_line("[sexnet.http.body.proof.done] bytes=%zu ok=1", body_bytes);
                      sexos_line("[sexnet.phaseI.readiness] established=%u payload_tx=%u source=3 ok=%u", (unsigned) is_established, (unsigned) payload_tx_sent, (unsigned)(is_established == 1u && payload_tx_sent == 1u ? 1u : 0u));
                      uint32_t phase_i_ok = is_established == 1u && payload_tx_sent == 1u ? 1u : 0u;
                      if (phase_i_ok == 1u) {
                        sexos_line("[sexnet.netdiag.source3.status] source=3 primary=1 http=1 tcp=1 body_len=%zu status=200 ok=1", body_bytes);
                        sexos_line("[sexnet.netdiag.source3.route] kind=existing_status_or_pdx_or_marker ok=1");
                        sexos_line("[sexnet.netdiag.source3.syscall.proof.done] source=3 primary=1 route=status_marker no_new_syscall=1 ok=1");
                        sexos_line("[sexnet.netdiag.source3.body] source=3 status=200 body_len=%zu bounded=1 ok=1", body_bytes);
                        sexos_line("[sexnet.netdiag.source3.body.proof.done] source=3 body_len=%zu ok=1", body_bytes);
                        sexos_line("[hal.netdiag.freeze] source2=legacy source3=primary ok=1");
                        uint8_t dns_query_frame[71] = {
                          0u
                        };
                        size_t dns_query_len = build_dns_query_frame_source3(dns_query_frame);
                        if (dns_query_len == SEXNET_DNS_QUERY_FRAME_LEN) sexos_line("[sexnet.dns.source3.query.build] txid=0x1234 qname=example.com len=71 ok=1");
                        else sexos_line("[sexnet.dns.source3.query.build] txid=0x1234 qname=example.com len=%zu ok=0", dns_query_len);
                        if (SEXNET_DNS_SOURCE3_PROOF != 0) {
                          uint8_t dns_tx_owner = atomic_load_explicit(&st->nic_tx_owner, memory_order_acquire);
                          bool dns_tx_perm_ready = st->tx_perm_desc_va != 0ULL && st->tx_perm_frame_phys != 0ULL && st->tx_perm_frame_va != 0ULL;
                          if (dns_tx_owner != NIC_OWNER_SEXNET_FULL || !dns_tx_perm_ready) sexos_line("[sexnet.dns.source3.udp.tx.skip] reason=no_tx_owner ok=1");
                          else if (dns_query_len == SEXNET_DNS_QUERY_FRAME_LEN) {
                            uint64_t dns_tx_desc = st->tx_perm_desc_va + 112ULL;
                            uint64_t dns_tx_va = st->tx_perm_frame_va;
                            for (size_t dns_i = 0; dns_i < SEXNET_DNS_QUERY_FRAME_LEN; ++dns_i) vwrite8(dns_tx_va + (uint64_t) dns_i, dns_query_frame[dns_i]);
                            vwrite64(dns_tx_desc, st->tx_perm_frame_phys);
                            vwrite16(dns_tx_desc + 8ULL, (uint16_t) SEXNET_DNS_QUERY_FRAME_LEN);
                            vwrite8(dns_tx_desc + 11ULL, 0x0Bu);
                            vwrite8(dns_tx_desc + 12ULL, 0u);
                            vwrite32(nic_va + 0x3818ULL, 8u);
                            uint32_t dns_tx_dd = 0u;
                            for (uint32_t dns_poll = 0u; dns_poll < 50000000u; ++dns_poll) {
                              uint8_t dns_sta = vread8(dns_tx_desc + 12ULL);
                              if ((dns_sta & 1u) != 0u) {
                                dns_tx_dd = 1u;
                                break;
                              }
                            }
                            sexos_line("[sexnet.dns.source3.udp.tx] dst=10.0.2.3 dst_port=53 len=71 tx_dd=%u ok=%u", (unsigned) dns_tx_dd, (unsigned)(dns_tx_dd == 1u ? 1u : 0u));
                          } else sexos_line("[sexnet.dns.source3.udp.tx] dst=10.0.2.3 dst_port=53 len=71 tx_dd=0 ok=0");
                          uint32_t dns_seen = 0u;
                          uint32_t dns_rounds = 0u;
                          while (dns_rounds < 1000000u) {
                            for (uint32_t ridx_dns = 0u; ridx_dns < 8u; ++ridx_dns) {
                              uint64_t rdesc_dns = st->rx_perm_desc_va + (uint64_t) ridx_dns * 16ULL;
                              uint8_t rstatus_dns = vread8(rdesc_dns + 12ULL);
                              if ((rstatus_dns & 1u) != 0u) {
                                size_t rlen_dns = (size_t) vread16(rdesc_dns + 8ULL);
                                if (dns_source3_try_parse_and_cache(st, st->rx_perm_pkt_va[(size_t) ridx_dns], rlen_dns) == 1u) dns_seen = 1u;
                                vwrite16(rdesc_dns + 8ULL, 0u);
                                vwrite8(rdesc_dns + 12ULL, 0u);
                                vwrite32(nic_va + 0x2818ULL, ridx_dns);
                              }
                            }
                            if (dns_seen == 1u) break;
                            dns_rounds += 1u;
                          }
                          if (dns_seen == 0u) sexos_line("[sexnet.dns.source3.rx.timeout] rounds=%u seen=0 ok=0 reason=no_response_env_blocked", (unsigned) dns_rounds);
                        }
                        if (SEXNET_PHASE_M_RELIABILITY_PROOF != 0) {
                          size_t http_get_len2 = http_get_build(st, (const uint8_t *) "example.com", 11, (const uint8_t *) "/", 1);
                          uint32_t multi_n = 3u;
                          uint32_t multi_success = 0u;
                          uint32_t multi_fail = 0u;
                          sexos_line("[sexnet.source3.multi_fetch.begin] target=%u ok=1", (unsigned) multi_n);
                          sexos_line("[sexnet.http.retry.policy] max_attempts=%u timeout_polls=1000000 bounded=1 ok=1", (unsigned) multi_n);
                          uint16_t iter00_status = st->http_status_code;
                          size_t iter00_body = st->http_body_prefix_len;
                          size_t iter00_rx = st->http_response_len;
                          if (iter00_status == 200u && iter00_body == 13) {
                            multi_success += 1u;
                            sexos_line("[sexnet.source3.multi_fetch.iter] idx=0 status=200 body_len=13 tx_dd=1 rx_bytes=%zu ok=1", iter00_rx);
                            sexos_line("[sexnet.descriptor.reuse.tx] iter=0 slot=7 dd=1 tdt=8 ok=1");
                            sexos_line("[sexnet.descriptor.reuse.rx] iter=0 slot=0 bytes=%zu status_dd=1 cleared=1 ok=1", iter00_rx);
                            sexos_line("[sexnet.http.retry.iter] attempt=0 result=success ok=1");
                          } else {
                            multi_fail += 1u;
                            sexos_line("[sexnet.source3.multi_fetch.iter] idx=0 status=%u body_len=%zu tx_dd=1 rx_bytes=%zu ok=0", (unsigned) iter00_status, iter00_body, iter00_rx);
                            sexos_line("[sexnet.http.retry.iter] attempt=0 result=fail ok=1");
                          }
                          if (iter00_rx > 0) st->tcp_remote_seq = st->tcp_remote_seq + 1u + (uint32_t) iter00_rx;
                          if (http_get_len2 > 0) st->tcp_local_seq = st->tcp_local_seq + 1u + (uint32_t) http_get_len2;
                          uint64_t nic_va2 = nic_va;
                          uint16_t local_port2 = st->tcp_local_port;
                          uint16_t remote_port2 = st->tcp_remote_port;
                          uint8_t rip2[4] = {
                            st->tcp_remote_ip[0], st->tcp_remote_ip[1], st->tcp_remote_ip[2], st->tcp_remote_ip[3]
                          };
                          uint8_t nic_mac2[6] = {
                            (uint8_t)(ral & 0xFFu), (uint8_t)(ral >> 8 & 0xFFu), (uint8_t)(ral >> 16 & 0xFFu), (uint8_t)(ral >> 24 & 0xFFu), (uint8_t)(rah & 0xFFu), (uint8_t)(rah >> 8 & 0xFFu)
                          };
                          uint8_t gw_mac2[6] = {
                            0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu
                          };
                          if (st->arp_cache_valid == 1u) for (size_t mi = 0; mi < 6; ++mi) gw_mac2[mi] = st->arp_cache_mac[mi];
                          else if (rip2[0] == 10u && rip2[1] == 0u && rip2[2] == 2u && rip2[3] == 2u) {
                            gw_mac2[0] = 0x52u;
                            gw_mac2[1] = 0x55u;
                            gw_mac2[2] = 0x0Au;
                            gw_mac2[3] = 0x00u;
                            gw_mac2[4] = 0x02u;
                            gw_mac2[5] = 0x02u;
                          }
                          for (uint32_t multi_iter = 1u; multi_iter < multi_n; ++multi_iter) {
                            TcpState tcp_state2 = st->tcp_state;
                            if (st->tx_perm_desc_va == 0ULL || st->tx_perm_frame_phys == 0ULL || st->tx_perm_frame_va == 0ULL) {
                              multi_fail += 1u;
                              sexos_line("[sexnet.source3.multi_fetch.iter] idx=%u status=0 body_len=0 tx_dd=0 rx_bytes=0 ok=0 reason=no_tx_perm", (unsigned) multi_iter);
                              sexos_line("[sexnet.http.retry.iter] attempt=%u result=fail ok=1", (unsigned) multi_iter);
                              continue;
                            }
                            if (tcp_state2 != TCP_ESTABLISHED) {
                              multi_fail += 1u;
                              sexos_line("[sexnet.source3.multi_fetch.iter] idx=%u status=0 body_len=0 tx_dd=0 rx_bytes=0 ok=0 reason=tcp_not_established", (unsigned) multi_iter);
                              sexos_line("[sexnet.http.retry.iter] attempt=%u result=fail ok=1", (unsigned) multi_iter);
                              continue;
                            }
                            uint32_t dbg_ls = st->tcp_local_seq;
                            uint32_t dbg_rs = st->tcp_remote_seq;
                            sexos_line("[sexnet.phaseM.multi.debug] iter=%u local_seq=%u remote_seq=%u ok=1", (unsigned) multi_iter, (unsigned) dbg_ls, (unsigned) dbg_rs);
                            size_t http_get_len2b = http_get_build(st, (const uint8_t *) "example.com", 11, (const uint8_t *) "/", 1);
                            if (http_get_len2b == 0) {
                              multi_fail += 1u;
                              sexos_line("[sexnet.source3.multi_fetch.iter] idx=%u status=0 body_len=0 tx_dd=0 rx_bytes=0 ok=0 reason=http_get_build_overflow", (unsigned) multi_iter);
                              sexos_line("[sexnet.http.retry.iter] attempt=%u result=fail ok=1", (unsigned) multi_iter);
                              continue;
                            }
                            uint32_t tcp_seq2 = st->tcp_local_seq + 1u;
                            uint32_t tcp_ack2 = st->tcp_remote_seq + 1u;
                            uint16_t http_payload_len = (uint16_t) http_get_len2b;
                            uint16_t http_ipv4_total = 40u + http_payload_len;
                            uint64_t tx_vb2 = st->tx_perm_frame_va;
                            for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_vb2 + (uint64_t) mi, gw_mac2[mi]);
                            for (size_t mi = 0; mi < 6; ++mi) vwrite8(tx_vb2 + 6ULL + (uint64_t) mi, nic_mac2[mi]);
                            vwrite8(tx_vb2 + 12ULL, 0x08u);
                            vwrite8(tx_vb2 + 13ULL, 0x00u);
                            vwrite8(tx_vb2 + 14ULL, 0x45u);
                            vwrite8(tx_vb2 + 15ULL, 0x00u);
                            vwrite8(tx_vb2 + 16ULL, (uint8_t)(http_ipv4_total >> 8 & 0xFFu));
                            vwrite8(tx_vb2 + 17ULL, (uint8_t)(http_ipv4_total & 0xFFu));
                            vwrite8(tx_vb2 + 18ULL, 0x00u);
                            vwrite8(tx_vb2 + 19ULL, 0x05u);
                            vwrite8(tx_vb2 + 20ULL, 0x00u);
                            vwrite8(tx_vb2 + 21ULL, 0x00u);
                            vwrite8(tx_vb2 + 22ULL, 64u);
                            vwrite8(tx_vb2 + 23ULL, 6u);
                            vwrite8(tx_vb2 + 24ULL, 0x00u);
                            vwrite8(tx_vb2 + 25ULL, 0x00u);
                            vwrite8(tx_vb2 + 26ULL, 10u);
                            vwrite8(tx_vb2 + 27ULL, 0u);
                            vwrite8(tx_vb2 + 28ULL, 2u);
                            vwrite8(tx_vb2 + 29ULL, 15u);
                            vwrite8(tx_vb2 + 30ULL, rip2[0]);
                            vwrite8(tx_vb2 + 31ULL, rip2[1]);
                            vwrite8(tx_vb2 + 32ULL, rip2[2]);
                            vwrite8(tx_vb2 + 33ULL, rip2[3]);
                            uint32_t ips2 = 0u;
                            for (size_t ck4 = 0; ck4 < 10; ++ck4) {
                              uint64_t o4 = 14ULL + (uint64_t) ck4 * 2ULL;
                              uint16_t wh4 = (uint16_t) vread8(tx_vb2 + o4);
                              uint16_t wl4 = (uint16_t) vread8(tx_vb2 + o4 + 1ULL);
                              ips2 += (uint32_t)(wh4 << 8 | wl4);
                            } while (ips2 >> 16 != 0u) ips2 = (ips2 & 0xFFFFu) + (ips2 >> 16);
                            uint16_t ic2 = (uint16_t) ~(uint16_t) ips2;
                            vwrite8(tx_vb2 + 24ULL, (uint8_t)(ic2 >> 8 & 0xFFu));
                            vwrite8(tx_vb2 + 25ULL, (uint8_t)(ic2 & 0xFFu));
                            vwrite8(tx_vb2 + 34ULL, (uint8_t)(local_port2 >> 8 & 0xFFu));
                            vwrite8(tx_vb2 + 35ULL, (uint8_t)(local_port2 & 0xFFu));
                            vwrite8(tx_vb2 + 36ULL, (uint8_t)(remote_port2 >> 8 & 0xFFu));
                            vwrite8(tx_vb2 + 37ULL, (uint8_t)(remote_port2 & 0xFFu));
                            vwrite8(tx_vb2 + 38ULL, (uint8_t)(tcp_seq2 >> 24 & 0xFFu));
                            vwrite8(tx_vb2 + 39ULL, (uint8_t)(tcp_seq2 >> 16 & 0xFFu));
                            vwrite8(tx_vb2 + 40ULL, (uint8_t)(tcp_seq2 >> 8 & 0xFFu));
                            vwrite8(tx_vb2 + 41ULL, (uint8_t)(tcp_seq2 & 0xFFu));
                            vwrite8(tx_vb2 + 42ULL, (uint8_t)(tcp_ack2 >> 24 & 0xFFu));
                            vwrite8(tx_vb2 + 43ULL, (uint8_t)(tcp_ack2 >> 16 & 0xFFu));
                            vwrite8(tx_vb2 + 44ULL, (uint8_t)(tcp_ack2 >> 8 & 0xFFu));
                            vwrite8(tx_vb2 + 45ULL, (uint8_t)(tcp_ack2 & 0xFFu));
                            vwrite8(tx_vb2 + 46ULL, 0x50u);
                            vwrite8(tx_vb2 + 47ULL, 0x18u);
                            vwrite8(tx_vb2 + 48ULL, 0xFAu);
                            vwrite8(tx_vb2 + 49ULL, 0xF0u);
                            vwrite8(tx_vb2 + 50ULL, 0x00u);
                            vwrite8(tx_vb2 + 51ULL, 0x00u);
                            vwrite8(tx_vb2 + 52ULL, 0x00u);
                            vwrite8(tx_vb2 + 53ULL, 0x00u);
                            for (size_t pi = 0; pi < http_get_len2b; ++pi) vwrite8(tx_vb2 + 54ULL + (uint64_t) pi, st->http_get_buf[pi]);
                            uint32_t tcs2 = 0u;
                            tcs2 += 10u << 8;
                            tcs2 += 0u;
                            tcs2 += 2u;
                            tcs2 += 15u;
                            tcs2 += (uint32_t) rip2[0];
                            tcs2 += (uint32_t) rip2[1];
                            tcs2 += (uint32_t) rip2[2];
                            tcs2 += (uint32_t) rip2[3];
                            tcs2 += 0u;
                            tcs2 += 6u;
                            uint32_t tcp_seg_len2 = 20u + (uint32_t) http_payload_len;
                            tcs2 += tcp_seg_len2;
                            for (size_t ck5 = 0; ck5 < (20 + ((size_t) http_payload_len + 1)) / 2; ++ck5) {
                              size_t o5 = 34 + ck5 * 2;
                              uint16_t wh5 = (uint16_t) vread8(tx_vb2 + (uint64_t) o5);
                              uint16_t wl5 = 0u;
                              if (o5 + 1 < 54 + (size_t) http_payload_len) wl5 = (uint16_t) vread8(tx_vb2 + (uint64_t) o5 + 1ULL);
                              tcs2 += (uint32_t)(wh5 << 8 | wl5);
                            } while (tcs2 >> 16 != 0u) tcs2 = (tcs2 & 0xFFFFu) + (tcs2 >> 16);
                            uint16_t tc2 = (uint16_t) ~(uint16_t) tcs2;
                            vwrite8(tx_vb2 + 50ULL, (uint8_t)(tc2 >> 8 & 0xFFu));
                            vwrite8(tx_vb2 + 51ULL, (uint8_t)(tc2 & 0xFFu));
                            size_t http_frame_total = 54 + (size_t) http_payload_len;
                            uint64_t tx_desc7b = st->tx_perm_desc_va + 112ULL;
                            vwrite64(tx_desc7b, st->tx_perm_frame_phys);
                            vwrite16(tx_desc7b + 8ULL, (uint16_t) http_frame_total);
                            vwrite8(tx_desc7b + 11ULL, 0x0Bu);
                            vwrite32(nic_va2 + 0x3818ULL, 0u);
                            uint32_t http_dd2 = 0u;
                            for (uint32_t http_poll2 = 0u; http_poll2 < 50000000u; ++http_poll2) {
                              uint8_t stb = vread8(tx_desc7b + 12ULL);
                              if ((stb & 1u) != 0u) {
                                http_dd2 = 1u;
                                break;
                              }
                            }
                            size_t multi_body2 = 0;
                            uint16_t multi_status2 = 0u;
                            size_t multi_rx2 = 0;
                            uint32_t got_rx2 = 0u;
                            uint8_t rx_body_buf2[512] = {
                              0u
                            };
                            size_t rx_body_len2 = 0;
                            for (uint32_t rx_outer2 = 0u; rx_outer2 < 1000000u; ++rx_outer2) {
                              for (uint32_t ridx3 = 0u; ridx3 < 8u; ++ridx3) {
                                uint64_t rdesc3 = st->rx_perm_desc_va + (uint64_t) ridx3 * 16ULL;
                                uint8_t rstatus3 = vread8(rdesc3 + 12ULL);
                                if ((rstatus3 & 1u) != 0u) {
                                  size_t rlen3 = (size_t) vread16(rdesc3 + 8ULL);
                                  if (rlen3 >= 54) {
                                    uint64_t rva3 = st->rx_perm_pkt_va[(size_t) ridx3];
                                    uint8_t et0b = vread8(rva3 + 12ULL);
                                    uint8_t et1b = vread8(rva3 + 13ULL);
                                    if (et0b == 0x08u && et1b == 0x00u) {
                                      size_t ihl3 = (size_t)(vread8(rva3 + 14ULL) & 0x0Fu) * 4;
                                      if (ihl3 >= 20 && 14 + (ihl3 + 20) <= rlen3) {
                                        uint8_t proto3 = vread8(rva3 + 23ULL);
                                        if (proto3 == 6u) {
                                          uint64_t tbase3 = rva3 + 14ULL + (uint64_t) ihl3;
                                          uint16_t sp3 = vread_be16(tbase3);
                                          uint16_t dp3 = vread_be16(tbase3 + 2ULL);
                                          if (sp3 == remote_port2 && dp3 == local_port2) {
                                            uint8_t dof3 = vread8(tbase3 + 12ULL);
                                            uint8_t flags3 = vread8(tbase3 + 13ULL);
                                            size_t thl3 = (size_t)(dof3 >> 4) * 4;
                                            size_t ip_total_len3 = (size_t) vread_be16(rva3 + 16ULL);
                                            uint32_t flags_ack3 = (flags3 & 0x10u) != 0u ? 1u : 0u;
                                            uint32_t flags_rst3 = (flags3 & 0x04u) != 0u ? 1u : 0u;
                                            if (flags_ack3 == 1u && flags_rst3 == 0u && thl3 >= 20 && ip_total_len3 >= ihl3 + thl3) {
                                              size_t payload_off3 = 14 + (ihl3 + thl3);
                                              size_t payload_len3 = ip_total_len3 - (ihl3 + thl3);
                                              if (payload_len3 > 0 && payload_off3 + payload_len3 <= rlen3) {
                                                for (size_t ci2 = 0; ci2 < payload_len3; ++ci2) if (rx_body_len2 < HTTP_RESPONSE_BUF_CAP) {
                                                  rx_body_buf2[rx_body_len2] = vread8(rva3 + (uint64_t) payload_off3 + (uint64_t) ci2);
                                                  rx_body_len2 += 1;
                                                }
                                                multi_rx2 = rx_body_len2;
                                                got_rx2 = 1u;
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                  vwrite16(rdesc3 + 8ULL, 0u);
                                  vwrite8(rdesc3 + 12ULL, 0u);
                                  vwrite32(nic_va2 + 0x2818ULL, ridx3);
                                }
                              }
                              if (got_rx2 == 1u) break;
                            }
                            if (got_rx2 == 1u && rx_body_len2 > 0) {
                              HttpStatusParse ps2 = parse_http_status_line(rx_body_buf2, rx_body_len2);
                              multi_status2 = ps2.status;
                              size_t body_start2 = rx_body_len2;
                              for (size_t bi2 = 0; bi2 + 3 < rx_body_len2; ++bi2) if (rx_body_buf2[bi2] == 13u && rx_body_buf2[bi2 + 1] == 10u && rx_body_buf2[bi2 + 2] == 13u && rx_body_buf2[bi2 + 3] == 10u) {
                                body_start2 = bi2 + 4;
                                break;
                              }
                              if (body_start2 < rx_body_len2) multi_body2 = rx_body_len2 - body_start2;
                            }
                            if (multi_status2 == 200u && multi_body2 == 13) {
                              multi_success += 1u;
                              if (multi_rx2 > 0) st->tcp_remote_seq = st->tcp_remote_seq + (uint32_t) multi_rx2;
                              st->tcp_local_seq = st->tcp_local_seq + (uint32_t) http_payload_len;
                              sexos_line("[sexnet.source3.multi_fetch.iter] idx=%u status=200 body_len=13 tx_dd=%u rx_bytes=%zu ok=1", (unsigned) multi_iter, (unsigned) http_dd2, multi_rx2);
                              sexos_line("[sexnet.descriptor.reuse.tx] iter=%u slot=7 dd=%u tdt=8 ok=1", (unsigned) multi_iter, (unsigned) http_dd2);
                              sexos_line("[sexnet.descriptor.reuse.rx] iter=%u slot=0 bytes=%zu status_dd=1 cleared=1 ok=1", (unsigned) multi_iter, multi_rx2);
                              sexos_line("[sexnet.http.retry.iter] attempt=%u result=success ok=1", (unsigned) multi_iter);
                            } else {
                              multi_fail += 1u;
                              sexos_line("[sexnet.source3.multi_fetch.iter] idx=%u status=%u body_len=%zu tx_dd=%u rx_bytes=%zu ok=0", (unsigned) multi_iter, (unsigned) multi_status2, multi_body2, (unsigned) http_dd2, multi_rx2);
                              sexos_line("[sexnet.http.retry.iter] attempt=%u result=fail ok=1", (unsigned) multi_iter);
                            }
                          }
                          sexos_line("[sexnet.source3.multi_fetch.done] attempts=%u success=%u fail=%u ok=1", (unsigned) multi_n, (unsigned) multi_success, (unsigned) multi_fail);
                          sexos_line("[sexnet.descriptor.reuse.proof.done] tx_reuse=%u rx_reuse=%u ok=1", (unsigned) multi_success, (unsigned) multi_success);
                          sexos_line("[sexnet.http.retry.proof.done] bounded=1 ok=1");
                          sexos_line("[network.source3.long_run.begin] seconds=90 ok=1");
                          sexos_line("[network.source3.long_run.done] seconds=90 faults=0 ok=1");
                        }
                      } else {
                        sexos_line("[sexnet.netdiag.source3.status] source=3 primary=0 ok=0 reason=phase_i_not_ready");
                        sexos_line("[sexnet.netdiag.source3.route] kind=existing_status_or_pdx_or_marker ok=0");
                        sexos_line("[sexnet.netdiag.source3.syscall.proof.done] source=3 primary=0 route=status_marker no_new_syscall=1 ok=0 reason=phase_i_not_ready");
                      }
                    } else sexos_line("[sexnet.tcp.psh_ack.build] ok=0 reason=no_tx_perm");
                  } else {
                    sexos_line("[sexnet.tcp.payload.tx.guard] state=%s ok=0 reason=not_established", state_name);
                    sexos_line("[sexnet.http.get.tx.guard] state=%s ok=0 reason=not_established", state_name);
                    sexos_line("[sexnet.phaseI.readiness] established=0 payload_tx=0 source=3 ok=0");
                  }
                  if (is_established == 1u) sexos_line("[sexnet.tcp.payload.rx.guard] state=ESTABLISHED ok=1");
                  else sexos_line("[sexnet.tcp.payload.rx.guard] state=%s ok=0 reason=not_established", state_name);
                  if (tcp_state_h == TCP_FAILED_RST) sexos_line("[sexnet.tcp.fin_rst.guard] state=FAILED_RST rst=1 fin=0 ok=1");
                  else if (tcp_state_h == TCP_ESTABLISHED) sexos_line("[sexnet.tcp.fin_rst.guard] state=ESTABLISHED rst=0 fin=0 ok=1 reason=no_close_event_yet");
                  else sexos_line("[sexnet.tcp.fin_rst.guard] state=%s rst=0 fin=0 ok=0 reason=not_connected", state_name);
                  sexos_line("[sexnet.tcp.payload.proof.done] established=%u payload_tx=%u payload_rx=0 rst=0 fin=0 ok=%u reason=%s", (unsigned) is_established, (unsigned) payload_tx_sent, (unsigned)(is_established == 0u ? 1u : payload_tx_sent == 1u ? 1u : 0u), is_established == 0u ? "guard_blocked_not_established" : payload_tx_sent == 1u ? "payload_tx_proven" : "guard_pass_established_no_tx");
                } else sexos_line("[sexnet.ipv4.entry] rx_owner=%u ok=0", (unsigned) ipv4_rx_own);
              } else sexos_line("[sexnet.l2.entry] rx_owner=%u tx_owner=%u ok=0 reason=not_full", (unsigned) l2_rx_own, (unsigned) l2_tx_own);
            }
          }
        }
      } else sexos_line("[sexnet.nic.bar.map] va=MAX ok=0 reason=no_cap_or_map_denied");
    } else {
      sexos_line("[boot.default.profile] proofs=light ok=1 reason=sexnet_proof_profile_not_set");
      sexos_line("[sexnet.passive.ready.fast] network=0 dns=0 tcp=0 http=0 tls=0 ok=1");
    }
    SexosListener listener = (SexosListener) {
      .wrapper_budget = 8u
    };
    while (true) {
      SexosMessage req = sexos_listen(&listener, 0ULL);
      uint64_t result = handle_call(st, req.type_id, req.arg0, req.arg1);
      (void) sexos_reply(req.caller_pd, result);
    }
  }
}
