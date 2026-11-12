#include "sext_service.h"

#if SEXOS_WORKER_PROBE
typedef struct {
    uint64_t status, value;
} WorkerReply;

static uint8_t worker_stack[65536] __attribute__((aligned(16)));
static volatile uint32_t start_word, done_word, timeout_word;
static uint64_t worker_tls_value, main_tls_value, observed_arg, observed_tls, observed_wait;

static WorkerReply worker_call(uint64_t op, uint64_t a, uint64_t b, uint64_t c)
{
    register uint64_t rax __asm__("rax") = 83;
    register uint64_t rdi __asm__("rdi") = op;
    register uint64_t rsi __asm__("rsi") = a;
    register uint64_t rdx __asm__("rdx") = b;
    register uint64_t r10 __asm__("r10") = c;
    register uint64_t r8 __asm__("r8");
    __asm__ volatile("syscall" : "+a"(rax), "+S"(rsi), "+d"(rdx), "+r"(r10), "=r"(r8)
                     : "D"(rdi) : "rcx", "r11", "memory", "cc");
    return (WorkerReply){rax, rsi};
}

static _Noreturn void worker_entry(uint64_t arg)
{
    observed_arg = arg;
    WorkerReply reply = worker_call(3, (uintptr_t)&start_word, 0, 0);
    observed_wait = reply.status;
    reply = worker_call(6, (uintptr_t)&worker_tls_value, 0, 0);
    if (reply.status != 0) sext_fatal("SEXT_WORKER_TLS", reply.status);
    __asm__ volatile("movq %%fs:0, %0" : "=r"(observed_tls));
    (void)worker_call(2, (uintptr_t)&done_word, 0, 0);
    sext_fatal("SEXT_WORKER_EXIT_RETURNED", 0);
}

static uint64_t tls_word(void)
{
    uint64_t value;
    __asm__ volatile("movq %%fs:0, %0" : "=r"(value));
    return value;
}

static uint32_t run_worker(uint64_t arg)
{
    start_word = 0;
    done_word = 0;
    observed_arg = observed_tls = observed_wait = 0;
    WorkerReply reply = worker_call(1, (uintptr_t)worker_entry,
                                    (uintptr_t)(worker_stack + sizeof worker_stack), arg);
    if (reply.status != 0 || !reply.value) {
        sexos_line("[sext.worker.probe.error] spawn status=%llu", (unsigned long long)reply.status);
        return 0;
    }
    uint32_t id = (uint32_t)reply.value;
    sexos_yield();
    start_word = 1;
    reply = worker_call(4, (uintptr_t)&start_word, 1, 0);
    if (reply.status != 0 || reply.value > 1) {
        sexos_line("[sext.worker.probe.error] wake status=%llu count=%llu",
                   (unsigned long long)reply.status, (unsigned long long)reply.value);
        return 0;
    }
    while (!done_word) {
        reply = worker_call(3, (uintptr_t)&done_word, 0, 0);
        if (reply.status != 0 && reply.status != 8) {
            sexos_line("[sext.worker.probe.error] join status=%llu", (unsigned long long)reply.status);
            return 0;
        }
        if (!done_word) sexos_yield();
    }
    if (observed_arg != arg || observed_tls != worker_tls_value ||
        (observed_wait != 0 && observed_wait != 8) || tls_word() != main_tls_value) {
        sexos_line("[sext.worker.probe.error] arg=%#llx tls=%#llx wait=%llu main_tls=%#llx",
                   (unsigned long long)observed_arg, (unsigned long long)observed_tls,
                   (unsigned long long)observed_wait, (unsigned long long)tls_word());
        return 0;
    }
    return id;
}

bool sext_worker_probe(void)
{
    worker_tls_value = UINT64_C(0xfeedface12345678);
    main_tls_value = UINT64_C(0xdecafbad87654321);
    WorkerReply reply = worker_call(6, (uintptr_t)&main_tls_value, 0, 0);
    if (reply.status != 0) return false;
    uint32_t first = run_worker(0x11111111);
    if (!first) return false;
    reply = worker_call(3, (uintptr_t)&timeout_word, 0, 5);
    if (reply.status != 0 && reply.status != 8) return false;
    uint32_t second = run_worker(0x22222222);
    if (!second || second == first) return false;
    reply = worker_call(5, 0, 0, 0);
    if (reply.status != 0 || reply.value != 1) return false;
    sexos_line("[sext.worker.probe] first=%u second=%u wait=2 wake=2 tls=2 join=2 cycles=2", first, second);
    return true;
}
#else
bool sext_worker_probe(void) { return true; }
#endif
