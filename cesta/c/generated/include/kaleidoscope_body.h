#ifndef KALEIDOSCOPE_BODY_H
#define KALEIDOSCOPE_BODY_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define KALEIDOSCOPE_BODY_CAPACITY ((size_t)192)
#define KALEIDOSCOPE_BODY_CONTENT_LIMIT ((size_t)96)
typedef struct KaleidoscopeBodySource {uint8_t bytes[KALEIDOSCOPE_BODY_CAPACITY];size_t len;} KaleidoscopeBodySource;
typedef struct KaleidoscopeBodyClean {uint8_t bytes[KALEIDOSCOPE_BODY_CONTENT_LIMIT];size_t len;} KaleidoscopeBodyClean;
typedef struct KaleidoscopeBodyPreview {uint8_t bytes[KALEIDOSCOPE_BODY_CAPACITY];size_t len;} KaleidoscopeBodyPreview;
void kaleidoscope_body_build(const KaleidoscopeBodySource *body,const KaleidoscopeBodySource *network_summary,KaleidoscopeBodyPreview *preview);
#endif
