#include "realmode.h"
#include "setup.h"

static
void set_ps2_dev(void) {
    asm(
        "int    $0x11           \n\t"
        "xorb   %bl, %bl        \n\t"
        "testb  $4, %al         \n\t"
        "jz     1f              \n\t"
        "movb   $0xAA, %bl      \n"
    "1:                         \n\t"
        "movb   %bl, (0x1FF)    \n\t"
    );
}

__setupcall(set_ps2_dev);
