#include "realmode.h"
#include "setup.h"

static
void set_keyrepeat_max(void) {
    asm volatile("int $0x16" : : "a" (0x0305), "b" (0));
}

__setupcall(set_keyrepeat_max);