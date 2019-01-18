#include "bzlib.h"
#include "memlayout.h"

#define KERNEL_SIZE     0x100           

asm(".code32");

void start_32(void) {
    int buf_size = (IMAGE_START - KERNEL_START);
    int ret;
    
    ret = BZ2_bzBuffToBuffDecompress((char*) (KERNEL_START - (INIT_SEG << 4)), \
                                    (unsigned int*) &(buf_size), \
                                    (char*) (IMAGE_START - (INIT_SEG << 4)), \
                                    *((unsigned int*) KERNEL_SIZE), \
                                    0, 0);
    if (ret) {
        while (1);
    }
    
    asm("movl   $0x7C00, %esi");            
    asm("ljmp   $0x10,   $0x100000");
}

void* malloc(size_t size) {
    static unsigned int cur_ptr = HEAP_START;
    void* ptr;
    if ((cur_ptr + (unsigned int) size) > HEAP_END)
        return 0;
    
    ptr = (void*) cur_ptr;
    cur_ptr += (unsigned int) size;
    return ptr;
}

void free(void* ptr) {
}

void bz_internal_error(int error) {
    while(1);
}