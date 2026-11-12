#ifndef CESTA_OWNERSHIP_ABI_PROFILE
#define CESTA_OWNERSHIP_ABI_PROFILE 1
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#endif
#include "network_web_open.h"

NetworkEngineError network_web_open_init(NetworkWebOpen *service, NetConfig config,
                                         NetworkEngineMac mac, uint64_t now) {
    network_web_init(&service->owners);
    return network_open_init(&service->engine, config, mac, now);
}

void network_web_open_dispose(NetworkWebOpen *service) {
    network_open_dispose(&service->engine);
    network_web_dispose(&service->owners);
}

NetworkEngineError network_web_open_connect(NetworkWebOpen *service, NetworkJobIdentity owner,
                                            uint8_t number, uint32_t address, uint16_t port) {
    NetworkWebAcquire acquired = network_web_flow_acquire(&service->owners, owner, number);
    if (!acquired.present)
        return NETWORK_ENGINE_BUSY;
    if (acquired.created) {
        size_t index = network_open_add_flow(&service->engine);
        if (index == NETWORK_OPEN_NO_INDEX) {
            (void)network_web_flow_release(&service->owners, owner, number);
            return NETWORK_ENGINE_BUSY;
        }
        if (index != acquired.index) {
            service->engine.count = acquired.index;
            (void)network_web_flow_release(&service->owners, owner, number);
            return NETWORK_ENGINE_PROTOCOL;
        }
    } else if (acquired.index >= service->engine.count) {
        (void)network_web_flow_release(&service->owners, owner, number);
        return NETWORK_ENGINE_PROTOCOL;
    }
    NetworkEngineError error = network_open_connect(&service->engine, acquired.index, address, port);
    if (error) {
        network_open_close(&service->engine, acquired.index);
        (void)network_web_flow_release(&service->owners, owner, number);
    }
    return error;
}

NetworkOpenState network_web_open_state(NetworkWebOpen *service, NetworkJobIdentity owner,
                                        uint8_t number) {
    size_t index = network_web_flow_find(&service->owners, owner, number);
    return index == NETWORK_WEB_NO_INDEX ? NETWORK_OPEN_CLOSED :
           network_open_state(&service->engine, index);
}

NetworkTcpClientCount network_web_open_send(NetworkWebOpen *service, NetworkJobIdentity owner,
                                            uint8_t number, const NetworkEnginePayload *payload) {
    size_t index = network_web_flow_find(&service->owners, owner, number);
    if (index == NETWORK_WEB_NO_INDEX)
        return (NetworkTcpClientCount){false, 0, NETWORK_TCP_CLIENT_STATE};
    return network_open_send(&service->engine, index, payload);
}

NetworkTcpClientReceive network_web_open_receive(NetworkWebOpen *service, NetworkJobIdentity owner,
                                                 uint8_t number, size_t maximum) {
    size_t index = network_web_flow_find(&service->owners, owner, number);
    if (index == NETWORK_WEB_NO_INDEX)
        return (NetworkTcpClientReceive){false, 0, NETWORK_TCP_CLIENT_STATE, {0}};
    return network_open_receive(&service->engine, index, maximum);
}

void network_web_open_shutdown(NetworkWebOpen *service, NetworkJobIdentity owner, uint8_t number) {
    size_t index = network_web_flow_find(&service->owners, owner, number);
    if (index != NETWORK_WEB_NO_INDEX)
        network_open_shutdown(&service->engine, index);
}

bool network_web_open_close(NetworkWebOpen *service, NetworkJobIdentity owner, uint8_t number) {
    size_t index = network_web_flow_find(&service->owners, owner, number);
    if (index == NETWORK_WEB_NO_INDEX)
        return false;
    network_open_close(&service->engine, index);
    return network_web_flow_release(&service->owners, owner, number);
}

NetworkWebRetirement network_web_open_retire(NetworkWebOpen *service, NetworkJobIdentity owner) {
    for (size_t i = 0; i < service->owners.flow_count; ++i) {
        NetworkWebFlow *flow = &service->owners.flows[i];
        if (flow->present && flow->owner.pd == owner.pd && flow->owner.task == owner.task)
            network_open_close(&service->engine, i);
    }
    return network_web_retire(&service->owners, owner);
}

void network_web_open_advance(NetworkWebOpen *service, uint64_t now) {
    network_open_advance(&service->engine, now);
}

NetworkEngineError network_web_open_submit(NetworkWebOpen *service,
                                           const NetworkEngineFrame *frame) {
    return network_open_submit(&service->engine, frame);
}

NetworkEngineFrameResult network_web_open_take_frame(NetworkWebOpen *service) {
    return network_open_take_frame(&service->engine);
}
