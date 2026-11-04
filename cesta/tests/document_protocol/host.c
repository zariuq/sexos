#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sexos_document_protocol.h"

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

int main(void) {
    SexosDocumentRequest request = {
        UINT32_C(0x123456), SEXOS_DOCUMENT_DELETE, UINT8_C(7),
        UINT32_C(0x654321), UINT64_C(0x0123456789abcdef),
        UINT64_C(0xfedcba9876543210)
    };
    uint64_t header = sexos_document_header(request);
    CHECK(header == UINT64_C(0x076543210b123456));

    SexosDocumentDecode decoded = sexos_document_decode(header, request.a, request.b);
    CHECK(decoded.ok);
    CHECK(decoded.request.seq == request.seq);
    CHECK(decoded.request.kind == request.kind);
    CHECK(decoded.request.object == request.object);
    CHECK(decoded.request.offset == request.offset);
    CHECK(decoded.request.a == request.a && decoded.request.b == request.b);
    CHECK(!sexos_document_decode(header & ~UINT64_C(0xffffff), request.a, request.b).ok);
    CHECK(!sexos_document_decode((header & ~UINT64_C(0xff000000)) |
                                 (UINT64_C(12) << 24), request.a, request.b).ok);

    uint64_t success = sexos_document_reply(request.seq, true, UINT32_C(0x89abcdef));
    uint64_t failure = sexos_document_reply(request.seq, false, SEXOS_DOCUMENT_STALE);
    CHECK(success == UINT64_C(0x5512345689abcdef));
    CHECK(failure == UINT64_C(0xe512345600000003));
    SexosDocumentReply reply = sexos_document_decode_reply(success, request.seq);
    CHECK(reply.ok && reply.success && reply.value == UINT32_C(0x89abcdef));
    reply = sexos_document_decode_reply(failure, request.seq);
    CHECK(reply.ok && !reply.success && reply.value == SEXOS_DOCUMENT_STALE);
    CHECK(!sexos_document_decode_reply(success, request.seq + 1).ok);
    CHECK(!sexos_document_decode_reply(success ^ (UINT64_C(0x11) << 56), request.seq).ok);

    SexosDocumentPayload payload = { .bytes = { 'a', 'b', 'c' }, .len = 3 };
    CHECK(sexos_document_hash(&payload) == UINT64_C(0xe71fa2190541574b));
    payload.len = 0;
    CHECK(sexos_document_hash(&payload) == UINT64_C(0xcbf29ce484222325));
    payload.len = SEXOS_DOCUMENT_MAX + 1;
    CHECK(sexos_document_hash(&payload) == 0);
    CHECK(sexos_document_hash(NULL) == 0);

    puts("document protocol controls passed");
    return 0;
}
