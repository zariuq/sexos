ENTRY(_start)
PHYS_BASE = 0x40080000;
SECTIONS
{
    . = PHYS_BASE;
    __kernel_start = .;
    .text : ALIGN(4K) { KEEP(*(.text.boot)) *(.text .text.*) }
    .rodata : ALIGN(4K) { *(.rodata .rodata.*) }
    .data : ALIGN(4K) { *(.data .data.*) *(.data.rel.ro .data.rel.ro.*) }
    .bss : ALIGN(4K) {
        __bss_start = .;
        *(.bss .bss.*) *(COMMON)
        . = ALIGN(16);
        __boot_stack_bottom = .;
        . = . + 0x10000;
        __boot_stack_top = .;
        . = ALIGN(4K);
        __bss_end = .;
    }
    __kernel_end = .;
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
