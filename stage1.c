#include "realmode.h"
#include "bios_hd.h"
#include "diskrec.h"

#define ATTR __attribute__ ((section(".size"), aligned(1)))

unsigned short stage2_size ATTR;
union disk_record* mbr = (union disk_record *) 0x000;
dap_t dap;

char welcome[]  = "Stage1: ";
char disk_err[] = "I/O error!";
char loaded[]   = "  sectors loaded.\r\n";

void inform(char* msg) {
    char c;
    while((c = *msg++))
        asm volatile(
            "int $0x10"
            : 
            : "a"   (0x0e00 | (c & 0xff)),
              "b"   (7)
        );
}

int main(void) {
    int ret;
    int (*setup)(union disk_record*) = \
        (int (*)(union disk_record*)) 0x280;
    inform(welcome);
    
    ret = hd_reset(0x80);
    if (ret < 0) { inform(disk_err); return 1; }
    
    dap.size = 0x10; dap.zero = 0x00;
    dap.sectors = stage2_size;
    dap.buffer = (0x0200|0x07C0<<16);
    dap.startlba = 1;
    ret = hd_read(0x80, (dap_t*) &dap);
    if (ret < 0) { inform(disk_err); return 1; }
    loaded[0] = stage2_size + '0';
    inform(loaded);
    
    ret = (*setup) (mbr);
    return ret;
}
