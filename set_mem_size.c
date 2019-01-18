#include "realmode.h"
#include "setup.h"

#define MEM_SIZE        0x1E0           

static 
void set_mem_size(void) {
    int ret;
    asm volatile("int $0x15" : "=a" (ret) : "a" (0x8800));
    *((unsigned long*) MEM_SIZE) = (unsigned long) (ret & 0xFFFF);
}

__setupcall(set_mem_size);