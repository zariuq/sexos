
#ifndef SEXOS_SOURCE_POLICY_H
#define SEXOS_SOURCE_POLICY_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define SOURCE_DENIED UINT64_MAX
#define SOURCE_UNAVAILABLE (UINT64_MAX-1)
#define SOURCE_BUSY (UINT64_MAX-2)
#define SOURCE_INVALID (UINT64_MAX-3)
#define SOURCE_MIN_SECONDS UINT64_C(1577836800)
#define SOURCE_MAX_SECONDS UINT64_C(4102444799)
enum { SOURCE_INFO=1, SOURCE_READ=2, SOURCE_ENTROPY_ATTEMPTS=8, SOURCE_CLOCK_ATTEMPTS=4, SOURCE_PRECISION=1 };
typedef struct {uint32_t pd,task;} SourceIdentity;
typedef struct {bool present;SourceIdentity holder,consumer;uint32_t source;} SourceGrant;
typedef struct {uint64_t status,value0,value1,value2,value3;} SourceReply;
typedef struct {bool present;uint64_t value;} SourceWord;
typedef struct {uint8_t sec,min,hour,day,month,year,century,status_a,status_b,status_d;} SourceClockRaw;
typedef struct {bool present;SourceClockRaw first,second;} SourceClockPair;
typedef struct {bool present;uint32_t value;} SourceNumber;
typedef struct {uint32_t profile,version;size_t root_bytes;} SourceTrustPolicy;
bool source_identity_valid(SourceIdentity identity);
bool source_identity_equal(SourceIdentity a,SourceIdentity b);
SourceReply source_entropy_invoke(SourceGrant grant,SourceIdentity caller,uint64_t kind,uint64_t a,uint64_t b,uint64_t c);
SourceReply source_clock_invoke(SourceGrant grant,SourceIdentity caller,uint64_t kind,uint64_t a,uint64_t b,uint64_t c);
SourceReply source_trust_invoke(SourceGrant grant,SourceIdentity caller,uint64_t kind,uint64_t a,uint64_t b,uint64_t c,SourceTrustPolicy policy);
SourceWord source_clock_seconds(SourceClockRaw raw);
bool source_clock_raw_equal(SourceClockRaw a,SourceClockRaw b);
bool source_entropy_available(void);
SourceWord source_entropy_sample(void);
SourceClockPair source_clock_pair(void);
#endif
