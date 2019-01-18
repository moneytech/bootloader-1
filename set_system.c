#include "realmode.h"
#include "setup.h"

static
void set_system(void) {
    asm(
        "movb   $0xC0, %ah      \n\t"
        "int    $0x15           \n\t"
        "movw   %es, %ax        \n\t"
        "movw   %ax, %ds        \n\t"
        "movw   %bx, %si        \n\t"
        "movw   %cs, %ax        \n\t"
        "movw   %ax, %es        \n\t"
        "movw   $0xA0, %di      \n\t"
        "movw   $0x10, %cx      \n\t"
        "cld                    \n\t"
        "rep                    \n\t"
        "movsb                  \n\t"
        "movw   %cs, %ax        \n\t"
        "movw   %ax, %ds        \n\t"
        "movw   %ax, %es        \n\t"       
    );
}

__setupcall(set_system);