#include "realmode.h"
#include "setup.h"

#define HD0_PARAM       0x080           
#define HD1_PARAM       0x090   

static
void set_hd_params(void) {
    asm volatile(
        "xorw   %%ax, %%ax          \n\t"
        "movw   %%ax, %%ds          \n\t"
        "movw   $0x1500, %%ax       \n\t"
        "movb   $0x80, %%dl         \n\t"
        "int    $0x13               \n\t"
        "jc     1f                  \n\t"
        "cmpb   $3, %%ah            \n\t"
        "jne    1f                  \n\t"
        "ldsw   (4 * 0x41), %%si    \n\t"
        "movw   $0x80, %%di         \n\t"
        "movw   $0x10, %%cx         \n\t"
        "cld                        \n\t"
        "rep                        \n\t"
        "movsb                      \n"
    "1:                             \n\t"
        "movw   $0x1500, %%ax       \n\t"
        "movb   $0x81, %%dl         \n\t"
        "int    $0x13               \n\t"
        "jc     2f                  \n\t"
        "cmpb   $3, %%ah            \n\t"
        "jne    2f                  \n\t"
        "ldsw   (4 * 0x46), %%si    \n\t"
        "movw   $0x90, %%di         \n\t"
        "movw   $0x10, %%cx         \n\t"
        "cld                        \n\t"
        "rep                        \n\t"
        "movsb                      \n"
    "2:                             \n\t"
        "movw   %%cs, %%ax          \n\t"
        "movw   %%ax, %%ds          \n"
        :
        :
        : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
}

__setupcall(set_hd_params);