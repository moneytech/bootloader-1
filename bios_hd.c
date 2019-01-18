#include "realmode.h"
#include "bios_hd.h"

int hd_reset(int drv) {
    int ret;
    asm volatile("int $0x13" : "=a" (ret) : "a" (0), "d" (drv));
    return -((ret >> 8) & 255);
}

int hd_read(int drv, dap_t* dap) {
    int ret;
    asm volatile(
        "int $0x13"
        : "=a"  (ret)
        : "a"   (0x4200),
          "S"   (dap),
          "d"   (drv)
    );
    return (ret & 0xff00) ? -((ret >> 8) & 255) : ret & 255;
}