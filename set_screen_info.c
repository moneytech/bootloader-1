#include "realmode.h"
#include "setup.h"

#define VIDEO_CURSOR    0x000           
#define VIDEO_PAGE      0x004           
#define VIDEO_MODE      0x006           
#define VIDEO_COLS      0x007           
#define VIDEO_LINES     0x00E           
#define VIDEO_VGA       0x00F           
#define VIDEO_FONT      0x010

static
void set_screen_info(void) {
    short int ret, mode, page;
    
    *((unsigned char*) (VIDEO_VGA)) = 1;
    
    asm volatile("int $0x10" : "=d" (ret) : "a" (0x0300), "b" (0));
    *((short int*) (VIDEO_CURSOR)) = ret;
    
    asm volatile(
        "int $0x10" 
        : "=a" (mode), 
          "=b" (page) 
        : "a" (0x0f00), 
          "b" (0)
    );
    *((short int*) (VIDEO_PAGE)) = page;
    *((short int*) (VIDEO_MODE)) = mode;
    
    asm volatile("xorw  %ax, %ax            \n");
    asm volatile("movw  %ax, %gs            \n");
    asm volatile("movw  %gs:(0x485), %ax    \n");
    asm volatile("movw  %ax, %cs:(0x10)     \n");   
    
    *((unsigned char*) (VIDEO_COLS))  = 80;
    *((unsigned char*) (VIDEO_LINES)) = 25;
}

__setupcall(set_screen_info);