#include "realmode.h"
#include "memlayout.h"
#include "bios_hd.h"
#include "diskrec.h"
#include "bios_mem.h"
#include "bios_tty.h"

#define DRV         0x80    
#define DAP_SIZE    0x10    
#define DAP_ZERO    0x00    
#define SECTOR_SIZE 512     
#define max_sector  18      

static const unsigned char Extendedtags[] = { 5, 0x85, 0x0f };
int isExtended(int type) {
  int i = 0;
  while (Extendedtags[ i ]) {
    if (Extendedtags[ i ] == type) return 0;
    else i++;
  }
  return 1;
}

static 
lba32 getlba(union disk_record *mbr) {
    int i, ret, recordcnt, extndx;
    dap_t dap;
    lba32 dr_offset = 0, epbr_offset = 0;
    recordcnt = 0;
    union disk_record dr = *mbr;
    for (i = 0; i < MaxPrimary; i++) {
        if(dr.record.partTbl[i].boot)
            return dr.record.partTbl[i].startLBA;
    }
    recordcnt++;
    while ( 1 ) {
        
        extndx = 0;
        for( i = 0; i < MaxPrimary; i++) {
            if(!isExtended(dr.record.partTbl[i].type)) {
                extndx = i; 
            }
        }
        if (recordcnt > MaxRecord) {
            prints("Too many disk records\r\n");
            return -1;
        }
        if (extndx) {
            dap.size = DAP_SIZE; dap.zero = DAP_ZERO;
            dap.sectors = 1;
            dr_offset = dr.record.partTbl[extndx].startLBA;
            if( recordcnt < 2)
                epbr_offset = dr_offset;
            else
                dr_offset += epbr_offset;
            dap.startlba = dr_offset;
            dap.buffer = (int) dr.image;
            dap.buffer |= 0x07C0<<16;
            ret = hd_read(DRV, (dap_t*) &dap);
            if (ret < 0) { prints("disk_err\r\n"); return -1; }
            for (i = 0; i < 2; i++) {
                if(dr.record.partTbl[i].boot)
                    return (dr_offset + dr.record.partTbl[i].startLBA);
            }
            recordcnt++;
        }
        else break; 
    }
    return -1;
}

static
int read_image(int image_size, void* dst, lba32 slba) {
    int ret;
    dap_t dap;
    dap.size = DAP_SIZE; dap.zero = DAP_ZERO;
    dap.buffer   = (TRACK_BUFFER|0x07C0<<16);
    dap.startlba = slba; 
    while (image_size > 0) {
        
        dap.sectors = max_sector;               
        if (dap.sectors > image_size) dap.sectors = image_size;
        
        ret = hd_read(DRV, (dap_t*) &dap); 
        if (ret < 0) { 
            prints("Track read error\r\n"); 
            return -1; 
        }
        
        ret = copy_block((INIT_SEG << 4) + TRACK_BUFFER, (unsigned) dst, \
                            (unsigned short) (dap.sectors * 256));
        if (ret) {
            prints("Memory transfer error!\r\n");
            return -1;
        }
        dst += (SECTOR_SIZE * dap.sectors);   
        printc('.');                            
        
        image_size -= dap.sectors;
        dap.startlba += dap.sectors;
    }
    prints("\r\n");
    return 0;
}

int load_images(union disk_record* mbr, int stage3_size, \
                int kernel_size, int initrd_size) 
{
    int ret;
    lba32 slba; 
    
    
    ret = getlba(mbr);
    if (ret < 0) return ret;
    slba = ret;
    
    if (stage3_size < 1) goto kernel;
    prints("  Loading stage3 ");    
    ret = read_image(stage3_size, (void*) (INIT_SEG << 4) + STAGE3_START, slba);
    if (ret) return -1;
    
kernel:
    if (kernel_size < 1) goto end;
    prints("  Loading compressed kernel image ");
    slba += stage3_size;
    ret = read_image(kernel_size, (void*) IMAGE_START, slba);
    if (ret) return -1;

initrd:
    if (initrd_size < 1) goto end;
    prints("  Loading initrd ");
    slba += kernel_size;
    ret = read_image(initrd_size, (void*) INITRD_START, slba);
    if (ret) return -1;
    
end:
    return ret;
}
