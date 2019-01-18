#include "realmode.h"

int copy_block(unsigned int src, unsigned int dst, unsigned short wcount) 
{
    volatile char gdt[ 48 ];
    volatile int i, ret;
        
    for(i = 0; i < sizeof(gdt); i++) gdt[i] = 0;

    *((unsigned short*) (gdt+0x10)) = 0xffff;
    *((unsigned long*)  (gdt+0x12)) = src;
    *((unsigned char*)  (gdt+0x15)) = 0x92;

    *((unsigned short*) (gdt+0x18)) = 0xffff; 
    *((unsigned long*)  (gdt+0x1a)) = dst;   
    *((unsigned char*)  (gdt+0x1d)) = 0x92;  

    asm volatile(
        "int    $0x15 "
        : "=a"  (ret)
        : "a"   (0x8700),
          "c"   (wcount),
          "S"   (gdt)
    );
    return (ret >> 8) & 255;
}