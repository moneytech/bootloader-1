#ifndef _MEMLAYOUT_H_

#define INIT_SEG            0x07C0      
#define INIT_ESP            0xFFF0      
#define STAGE2_START        0x0200      
#define STAGE3_START        0x1000      
#define TRACK_BUFFER        0xB000      
#define TRACK_BUFFER_SIZE   0x4000      
#define INITRD_START        0xFFFF// initrd を配置する場所
#define KERNEL_START        0x00100000  
#define IMAGE_START         0x00380000  
#define HEAP_START          0x00500000  
#define HEAP_END            0x00800000  

#define _MEMLAYOUT_H_
#endif