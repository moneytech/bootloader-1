OUTPUT_FORMAT("binary")

SECTIONS {
    . = 0x1000;                 /* "stage3" will be loaded at 2nd page  */
    .first  : { *(.fisrt) }     /* Locate "stages" First Excutable code */
    __setupcall_start = .;
    .setupcall.init : { *(.setupcall.init) }
    __setupcall_end = .;
    .text   : { *(.text) }      /* Excutable code                       */
    .rodata : { *(.rodata) }    /* Constants (R/O)                      */
    .data   : { *(.data) }      /* Initialized data                     */
    _data_end = .;              /* The end of .data section             */
    .bss    : { *(.bss) }       /* Uninitialized data                   */
    _bss_end = .;               /* The end of .bss section              */
}

