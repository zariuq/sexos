#define main adapter_main
#include "host.c"
#undef main

int main(void)
{
    reset();
    for (uint64_t epoch = 1; epoch <= EVENTS; ++epoch) batch_put(epoch);
    batch.retries = 4;
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_RETRY && batch.flushed_through == 2 &&
           !batch.retries);
    assert(epoch_files_journal.commit_generation == 2);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_RETRY && batch.flushed_through == 4 &&
           !batch.retries);
    assert(epoch_files_journal.commit_generation == 3);
    assert(epoch_files_flush(&batch) == EPOCH_JOURNAL_OK && batch.flushed_through == EVENTS &&
           !batch.retries);
    assert(epoch_files_journal.commit_generation == 4 && !batch.failed);
    assert(flush_entries == flush_exits && !flush_active);
    for (uint64_t epoch = 1; epoch <= EVENTS; ++epoch) expect(epoch);
    puts("epoch files: bounded flush chunks commit and resume without recursive failures");
    return 0;
}
