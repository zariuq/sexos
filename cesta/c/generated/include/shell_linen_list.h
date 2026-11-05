
#ifndef SEXOS_SHELL_LINEN_LIST_H
#define SEXOS_SHELL_LINEN_LIST_H
#include "shell_text_delivery.h"

enum {
    SHELL_LINEN_LIST_CAPACITY = 16,
    SHELL_LINEN_LIST_NAME_CAPACITY = 126
};

typedef struct ShellLinenListItem {
    uint64_t id;
    uint8_t name[126];
    size_t len;
} ShellLinenListItem;

typedef struct ShellLinenItems {
    ShellLinenListItem items[16];
    size_t count;
} ShellLinenItems;

typedef struct ShellLinenLine {
    uint8_t bytes[128];
    size_t len;
} ShellLinenLine;

typedef struct ShellLinenList {
    ShellTextBytes text;
    size_t first, rows;
    bool has_selected;
    size_t selected_row;
    uint32_t row_y, row_h;
} ShellLinenList;

ShellLinenList shell_linen_list(const ShellLinenItems *items, uint64_t selected, uint32_t width,
                                uint32_t height);
#endif
