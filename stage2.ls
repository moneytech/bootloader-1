/*
 * s t a g e 2 . l s
 *
 * Linker script for "stage2" program
 */

OUTPUT_FORMAT("binary")

/* Specify input and output sections */

SECTIONS {
    . = 0x200;                      /* "setup" will be loaded after IPL */
    .table  :   { *(.table) }       /* boot parameter                   */
    .text   :   { *(.text) }        /* Excutable code                   */
    .rodata :   { *(.rodata*) }     /* Constants (R/O)                  */
    .data   :   { *(.data) }        /* Initialized data                 */
    .bss    :   { *(.bss) }         /* Uninitialized data               */
}