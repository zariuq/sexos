#ifndef KALEIDOSCOPE_NAVIGATION_H
#define KALEIDOSCOPE_NAVIGATION_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define KALEIDOSCOPE_HTTP_PATH_MAX ((size_t)48)
typedef struct KaleidoscopeHttpPath {uint8_t bytes[KALEIDOSCOPE_HTTP_PATH_MAX];size_t len;} KaleidoscopeHttpPath;
typedef struct KaleidoscopeNavigation {KaleidoscopeHttpPath visited[3];uint8_t count;bool active;} KaleidoscopeNavigation;
void kaleidoscope_http_path_initial(KaleidoscopeHttpPath *out);
bool kaleidoscope_http_path_parse(const KaleidoscopeHttpPath *input,KaleidoscopeHttpPath *out);
bool kaleidoscope_http_path_equal(const KaleidoscopeHttpPath *left,const KaleidoscopeHttpPath *right);
void kaleidoscope_navigation_init(KaleidoscopeNavigation *state);
void kaleidoscope_navigation_begin(KaleidoscopeNavigation *state);
void kaleidoscope_navigation_current(const KaleidoscopeNavigation *state,KaleidoscopeHttpPath *out);
bool kaleidoscope_navigation_active(const KaleidoscopeNavigation *state);
size_t kaleidoscope_navigation_hops(const KaleidoscopeNavigation *state);
bool kaleidoscope_navigation_follow(KaleidoscopeNavigation *state,const KaleidoscopeHttpPath *path);
void kaleidoscope_navigation_complete(KaleidoscopeNavigation *state);
void kaleidoscope_navigation_cancel(KaleidoscopeNavigation *state);
#endif
