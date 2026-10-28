/* The kernel image, at the reference kernel's addresses: the higher half, from 2 MiB. */
OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);
    rodata PT_LOAD FLAGS(4);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0xffffffff80000000 + 0x200000;

    .text : ALIGN(4K) { *(.text .text.*) } :text

    . = ALIGN(4K);
    .rodata : ALIGN(4K) { *(.rodata .rodata.*) } :rodata

    . = ALIGN(4K);
    .data : ALIGN(4K) {
        /* The Limine request block stays in one writable segment. */
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
        *(.data .data.*)
    } :data

    .bss : ALIGN(4K) { *(.bss .bss.*) *(COMMON) } :data

    /DISCARD/ : { *(.eh_frame*) *(.note .note.*) *(.comment) }
}
