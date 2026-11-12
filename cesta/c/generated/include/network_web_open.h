
#ifndef SEXOS_NETWORK_WEB_OPEN_H
#define SEXOS_NETWORK_WEB_OPEN_H
#include "network_open.h"
#include "network_web.h"

typedef struct NetworkWebOpen {
    NetworkWebService owners;
    NetworkOpenEngine engine;
} NetworkWebOpen;

NetworkEngineError network_web_open_init(NetworkWebOpen *service, NetConfig config,
                                         NetworkEngineMac mac, uint64_t now);
void network_web_open_dispose(NetworkWebOpen *service);
NetworkEngineError network_web_open_connect(NetworkWebOpen *service, NetworkJobIdentity owner,
                                            uint8_t number, uint32_t address, uint16_t port);
NetworkOpenState network_web_open_state(NetworkWebOpen *service, NetworkJobIdentity owner,
                                        uint8_t number);
NetworkTcpClientCount network_web_open_send(NetworkWebOpen *service, NetworkJobIdentity owner,
                                            uint8_t number, const NetworkEnginePayload *payload);
NetworkTcpClientReceive network_web_open_receive(NetworkWebOpen *service, NetworkJobIdentity owner,
                                                 uint8_t number, size_t maximum);
void network_web_open_shutdown(NetworkWebOpen *service, NetworkJobIdentity owner, uint8_t number);
bool network_web_open_close(NetworkWebOpen *service, NetworkJobIdentity owner, uint8_t number);
NetworkWebRetirement network_web_open_retire(NetworkWebOpen *service, NetworkJobIdentity owner);
void network_web_open_advance(NetworkWebOpen *service, uint64_t now);
NetworkEngineError network_web_open_submit(NetworkWebOpen *service,
                                           const NetworkEngineFrame *frame);
NetworkEngineFrameResult network_web_open_take_frame(NetworkWebOpen *service);
#endif
