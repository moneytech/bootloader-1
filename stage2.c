#include "realmode.h"
#include "bios_tty.h"
#include "load_images.h"
#include "memlayout.h"
#include "param.h"

extern struct table_t table;

int setup(union disk_record* mbr)
{
    int ret;
    void (*stage3)(int, char*, int, int, int) = \
        (void (*)(int, char*, int, int, int)) STAGE3_START;
    prints("Stage2: ");
    prints("stage3_size = "); print_hex16(table.stage3_size); prints(" : ");
    prints("kernel size = "); print_hex16(table.kernel_size); prints("\r\n");
    
    
    ret = load_images(mbr, table.stage3_size, table.kernel_size);
    if (ret) {
        prints("Load error!");
        while(1) ;
    }
    
    (*stage3)(table.kernel_size, (char*) table.cmd_line, table.mount_flag, \
                                            table.root_disk, table.root_dev);
}
