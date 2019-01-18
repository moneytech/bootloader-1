#ifndef _BIOS_HD_H_

typedef struct __attribute__((packed)) {
    unsigned char size;
    unsigned char zero;
    unsigned short int sectors;
    unsigned long buffer;
    unsigned long long startlba;
} dap_t;

int hd_reset(int);
int hd_read(int, dap_t*);

#define _BIOS_HD_H_
#endif
