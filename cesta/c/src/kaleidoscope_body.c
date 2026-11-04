#include "kaleidoscope_body.h"
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
static void kaleidoscope_body_clean(const KaleidoscopeBodySource *input, KaleidoscopeBodyClean *content) {
  *content = (KaleidoscopeBodyClean) {
    .len = 0
  };
  if (input->len > KALEIDOSCOPE_BODY_CAPACITY) return;
  bool space = false;
  for (size_t i = 0; i < input->len; ++i) {
    uint8_t byte = input->bytes[i];
    byte = byte <= 32 || byte == 127 ? 32 : byte <= 126 ? byte : 63;
    if (byte == 32) {
      space = content->len != 0;
      continue;
    }
    size_t needed = 1 + (space ? 1 : 0);
    if (content->len + needed > 96) {
      for (size_t j = 93; j < 96; ++j) content->bytes[j] = 46;
      content->len = 96;
      break;
    }
    if (space) {
      content->bytes[content->len] = 32;
      content->len += 1;
      space = false;
    }
    content->bytes[content->len] = byte;
    content->len += 1;
  }
}
void kaleidoscope_body_build(const KaleidoscopeBodySource *body, const KaleidoscopeBodySource *network_summary, KaleidoscopeBodyPreview *preview) {
  static const uint8_t prefix[] = "Kaleidoscope\nCached / sample network text\n\n";
  static const uint8_t empty[] = "No text available.";
  *preview = (KaleidoscopeBodyPreview) {
    .len = 0
  };
  for (size_t i = 0; i < sizeof prefix - 1; ++i) {
    preview->bytes[preview->len] = prefix[i];
    preview->len += 1;
  }
  KaleidoscopeBodyClean content = {
    0
  };
  kaleidoscope_body_clean(body, &content);
  if (content.len == 0) kaleidoscope_body_clean(network_summary, &content);
  if (content.len == 0) for (size_t i = 0; i < sizeof empty - 1; ++i) {
    preview->bytes[preview->len] = empty[i];
    preview->len += 1;
  } else for (size_t i = 0; i < content.len; ++i) {
    preview->bytes[preview->len] = content.bytes[i];
    preview->len += 1;
  }
}
