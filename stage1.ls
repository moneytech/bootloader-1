/*
 * s t a g e 1 . l s
 *
 * Linker script for "stage1" program
 */

OUTPUT_FORMAT("binary")         /* We want raw binary image */
OUTPUT_ARCH(i386)               /* CPU is i386 family */

/* Define memory layout */

MEMORY {
    body    :   org =   0, len = 444
    size    :   org = 444, len = 2
}

/* Specify input and output sections */

SECTIONS {
    .text   :   { *(.text) }    > body      /* Excutable code */
    .rodata :   { *(.rodata*) } > body      /* Constants (R/O) */
    .data   :   { *(.data) }    > body      /* Initialized data */
    .bss    :   { *(.bss) }     > body      /* Unitnitialized data */
    .size   :   { *(.size) }    > size      /* Boot parameter table */
}