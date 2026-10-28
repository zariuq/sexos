#include "network_service.h"
static const uint8_t guest_ip[4] = {
  10, 0, 2, 15
};
static const uint8_t server_ip[4] = {
  10, 0, 2, 2
};
static uint16_t be16(const uint8_t *p) {
  return (uint16_t)((uint16_t) p[0] << 8 | p[1]);
}
static uint32_t be32(const uint8_t *p) {
  return (uint32_t) p[0] << 24 | (uint32_t) p[1] << 16 | (uint32_t) p[2] << 8 | p[3];
}
static void put16(uint8_t *p, uint16_t value) {
  p[0] = (uint8_t)(value >> 8);
  p[1] = (uint8_t) value;
}
static void put32(uint8_t *p, uint32_t value) {
  p[0] = (uint8_t)(value >> 24);
  p[1] = (uint8_t)(value >> 16);
  p[2] = (uint8_t)(value >> 8);
  p[3] = (uint8_t) value;
}
static uint16_t checksum(const uint8_t *data, size_t length, uint32_t sum) {
  for (size_t i = 0; i < length; i += 2) sum += (uint32_t) data[i] << 8 | (i + 1 < length ? data[i + 1] : 0);
  while (sum >> 16) sum = (sum & 65535) + (sum >> 16);
  return (uint16_t) ~sum;
}
static uint32_t pseudo_sum(size_t tcp_length) {
  return (uint32_t) be16(guest_ip) + be16(guest_ip + 2) + be16(server_ip) + be16(server_ip + 2) + 6 + (uint32_t) tcp_length;
}
static bool arp_send(NetworkDevice *d, uint8_t peer_mac[6], const uint8_t target_ip[4], bool reply) {
  uint8_t packet[42] = {
    0
  };
  for (size_t i = 0; i < 6; ++i) {
    packet[i] = reply ? peer_mac[i] : 255;
    packet[6 + i] = packet[22 + i] = d->mac[i];
    packet[32 + i] = reply ? peer_mac[i] : 0;
  }
  put16(packet + 12, 0x0806);
  put16(packet + 14, 1);
  put16(packet + 16, 0x0800);
  packet[18] = 6;
  packet[19] = 4;
  put16(packet + 20, reply ? 2 : 1);
  memcpy(packet + 28, guest_ip, 4);
  memcpy(packet + 38, target_ip, 4);
  return network_device_send(d, (cesta_read_bytes) {
    packet, sizeof packet
  });
}
static bool arp_receive(NetworkDevice *d, NetworkFrame *frame, uint8_t peer_mac[6]) {
  const uint8_t *p = frame->data;
  if (frame->length < 42 || be16(p + 12) != 0x0806 || be16(p + 14) != 1 || be16(p + 16) != 0x0800 || p[18] != 6 || p[19] != 4 || memcmp(p + 38, guest_ip, 4)) return false;
  if (be16(p + 20) == 1) {
    uint8_t sender[6];
    memcpy(sender, p + 22, 6);
    (void) arp_send(d, sender, p + 28, true);
  }
  if (be16(p + 20) != 2 || memcmp(p + 28, server_ip, 4)) return false;
  memcpy(peer_mac, p + 22, 6);
  return true;
}
static bool tcp_send(NetworkDevice *d, const uint8_t peer_mac[6], uint32_t sequence, uint32_t acknowledgment, uint8_t flags, cesta_read_bytes payload) {
  if (payload.len > 1460) return false;
  uint8_t packet[1514] = {
    0
  };
  memcpy(packet, peer_mac, 6);
  memcpy(packet + 6, d->mac, 6);
  put16(packet + 12, 0x0800);
  uint8_t *ip = packet + 14, *tcp = packet + 34;
  ip[0] = 0x45;
  put16(ip + 2, (uint16_t)(40 + payload.len));
  put16(ip + 6, 0x4000);
  ip[8] = 64;
  ip[9] = 6;
  memcpy(ip + 12, guest_ip, 4);
  memcpy(ip + 16, server_ip, 4);
  put16(ip + 10, checksum(ip, 20, 0));
  put16(tcp, 7777);
  put16(tcp + 2, 18081);
  put32(tcp + 4, sequence);
  put32(tcp + 8, acknowledgment);
  tcp[12] = 0x50;
  tcp[13] = flags;
  put16(tcp + 14, 8192);
  if (payload.len) memcpy(tcp + 20, payload.data, payload.len);
  put16(tcp + 16, checksum(tcp, 20 + payload.len, pseudo_sum(20 + payload.len)));
  return network_device_send(d, (cesta_read_bytes) {
    packet, 54 + payload.len
  });
}
static const uint8_t *tcp_receive(const NetworkFrame *frame, size_t *header, size_t *length) {
  const uint8_t *p = frame->data;
  if (frame->length < 54 || be16(p + 12) != 0x0800) return NULL;
  const uint8_t *ip = p + 14;
  size_t ihl = (ip[0] & 15u) * 4u, total = be16(ip + 2);
  if (ip[0] >> 4 != 4 || ihl < 20 || total < ihl + 20 || total > frame->length - 14 || be16(ip + 6) & 0x3FFF || ip[9] != 6 || memcmp(ip + 12, server_ip, 4) || memcmp(ip + 16, guest_ip, 4) || checksum(ip, ihl, 0)) return NULL;
  const uint8_t *tcp = ip + ihl;
  *header = (tcp[12] >> 4) * 4u;
  *length = total - ihl;
  if (*header < 20 || *header > *length || be16(tcp) != 18081 || be16(tcp + 2) != 7777 || checksum(tcp, *length, pseudo_sum(*length))) return NULL;
  return tcp;
}
NetworkFailure network_fetch(NetworkDevice *d, HttpResponse *response) {
  http_response_init(response);
  uint8_t peer_mac[6] = {
    0
  };
  NetworkFrame frame;
  bool found = false;
  for (unsigned attempt = 0; attempt < 3 && !found; ++attempt) {
    if (!arp_send(d, peer_mac, server_ip, false)) return d->failure ? d->failure : NET_DEVICE_ERROR;
    for (unsigned remaining = 65536; remaining; --remaining) {
      NetworkStep step = network_device_receive(d, &frame);
      if (step == NET_FAILED) return NET_DEVICE_ERROR;
      if (step == NET_READY && arp_receive(d, &frame, peer_mac)) {
        found = true;
        break;
      }
      sexos_yield();
    }
  }
  if (!found) return NET_TIMEOUT;
  sexos_line("[network.arp.ready]");
  uint32_t local = 42, remote = 0;
  bool connected = false;
  for (unsigned attempt = 0; attempt < 3 && !connected; ++attempt) {
    if (!tcp_send(d, peer_mac, local, 0, 2, (cesta_read_bytes) {
      NULL, 0
    })) return NET_DEVICE_ERROR;
    for (unsigned remaining = 65536; remaining; --remaining) {
      NetworkStep step = network_device_receive(d, &frame);
      if (step == NET_FAILED) return NET_DEVICE_ERROR;
      if (step == NET_READY) {
        (void) arp_receive(d, &frame, peer_mac);
        size_t header, length;
        const uint8_t *tcp = tcp_receive(&frame, &header, &length);
        if (tcp && tcp[13] & 0x10 && be32(tcp + 8) == local + 1) {
          if (tcp[13] & 4) return NET_RESET;
          if ((tcp[13] & 0x12) == 0x12 && length == header) {
            remote = be32(tcp + 4) + 1;
            connected = true;
            break;
          }
        }
      }
      sexos_yield();
    }
  }
  if (!connected) return NET_TIMEOUT;
  ++local;
  if (!tcp_send(d, peer_mac, local, remote, 0x10, (cesta_read_bytes) {
    NULL, 0
  })) return NET_DEVICE_ERROR;
  sexos_line("[network.tcp.established]");
  static const uint8_t get[] = "GET / HTTP/1.1\r\nHost: 10.0.2.2:18081\r\nConnection: close\r\n\r\n";
  if (!tcp_send(d, peer_mac, local, remote, 0x18, (cesta_read_bytes) {
    get, sizeof get - 1
  })) return NET_DEVICE_ERROR;
  local += (uint32_t)(sizeof get - 1);
  for (unsigned remaining = 262144; remaining; --remaining) {
    NetworkStep step = network_device_receive(d, &frame);
    if (step == NET_FAILED) return NET_DEVICE_ERROR;
    if (step == NET_READY) {
      (void) arp_receive(d, &frame, peer_mac);
      size_t header, length;
      const uint8_t *tcp = tcp_receive(&frame, &header, &length);
      if (tcp && tcp[13] & 0x10 && be32(tcp + 8) == local) {
        if (tcp[13] & 4) return NET_RESET;
        size_t payload = length - header;
        uint32_t sequence = be32(tcp + 4);
        if (sequence != remote) {
          if (!tcp_send(d, peer_mac, local, remote, 0x10, (cesta_read_bytes) {
            NULL, 0
          })) return NET_DEVICE_ERROR;
        } else {
          if (payload) {
            (void) http_response_feed(response, (cesta_read_bytes) {
              tcp + header, payload
            });
            remote += (uint32_t) payload;
          }
          if (tcp[13] & 1) {
            ++remote;
            http_response_end(response);
          }
          if (payload || tcp[13] & 1) {
            if (!tcp_send(d, peer_mac, local, remote, 0x10, (cesta_read_bytes) {
              NULL, 0
            })) return NET_DEVICE_ERROR;
          }
          if (response->state == HTTP_FAILED) return NET_HTTP_ERROR;
          if (response->state == HTTP_COMPLETE) {
            if (!tcp_send(d, peer_mac, local, remote, 0x11, (cesta_read_bytes) {
              NULL, 0
            })) return NET_DEVICE_ERROR;
            return NET_OK;
          }
        }
      }
    }
    sexos_yield();
  }
  return NET_TIMEOUT;
}
