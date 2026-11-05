#if !defined(SEXOS_SEX_GRAPHICS_H)
#define SEXOS_SEX_GRAPHICS_H 1

#include <stdint.h>

#include <stddef.h>

#include <stdbool.h>

/*2D framebuffer primitives and 8x8 bitmap font, from crates/sex-graphics.*/

#define SEX_GRAPHICS_CHAR_W UINT32_C(8)

#define SEX_GRAPHICS_CHAR_H UINT32_C(8)

#define SEX_GRAPHICS_FONT_GLYPHS ((size_t)128)

#define SEX_GRAPHICS_FONT_ROWS ((size_t)8)

/*repr(C): 16 bytes, matches sex_pdx::Rect.*/

typedef struct SexGraphicsRect {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
}
SexGraphicsRect;

/*repr(C): 24 bytes on 64-bit targets, matches sex_graphics::WindowBuffer.*/

typedef struct WindowBuffer {
  uint32_t *virtual_address;
  uint32_t width;
  uint32_t height;
  uint32_t stride;
}
WindowBuffer;

_Static_assert(sizeof(SexGraphicsRect) == 16 && __builtin_offsetof(SexGraphicsRect, height) == 12, "SexGraphicsRect ABI");

_Static_assert(sizeof(WindowBuffer) == 24 && __builtin_offsetof(WindowBuffer, width) == 8 && __builtin_offsetof(WindowBuffer, stride) == 16, "WindowBuffer ABI");

extern const uint8_t SEX_GRAPHICS_FONT[SEX_GRAPHICS_FONT_GLYPHS][SEX_GRAPHICS_FONT_ROWS];

WindowBuffer window_buffer_new(uint32_t *virtual_address, uint32_t width, uint32_t height, uint32_t stride);

void window_buffer_draw_pixel(WindowBuffer *wb, uint32_t x, uint32_t y, uint32_t color);

uint32_t window_buffer_draw_pixel_safe(WindowBuffer *wb, uint32_t x, uint32_t y, uint32_t color);

uint32_t window_buffer_read_pixel_safe(const WindowBuffer *wb, uint32_t x, uint32_t y);

void window_buffer_draw_rect(WindowBuffer *wb, SexGraphicsRect rect, uint32_t color);

void window_buffer_clear(WindowBuffer *wb, uint32_t color);

void sex_graphics_draw_char(WindowBuffer *fb, uint32_t x, uint32_t y, uint8_t ch, uint32_t fg, bool has_bg, uint32_t bg);

uint32_t sex_graphics_draw_str(WindowBuffer *fb, uint32_t x, uint32_t y, const uint8_t *s, size_t len, uint32_t fg, bool has_bg, uint32_t bg);

uint32_t sex_graphics_str_width(size_t byte_len);

#endif
