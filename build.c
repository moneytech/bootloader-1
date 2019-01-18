//
// b u i l d . c
//
// Boot disk image build tool
//

#include <unistd.h>                         // stat()
#include <sys/stat.h>                       // struct stat {}
#include <sys/types.h>                      // off_t
#include <stdlib.h>                         // system()
#include <stdio.h>                          // fprintf()
#include <string.h>                         // strlen(), strncpy(), strncat()
#include <fcntl.h>
#include "diskrec.h"

#define SectorSize      512
#define max_stage1      446                 // Maximum number of stage1 size
#define MAX_CMDLINE     256                 // Maximum command line length

#define IMAGE       argv[ 1 ]               // Boot disk image filename
#define STAGE1      argv[ 2 ]               // stage1 filename
#define STAGE2      argv[ 3 ]               // stage2 filename
#define STAGE3      argv[ 4 ]               // stage3 filename
#define KERNEL      argv[ 5 ]               // Compressed kernel filename

#define DEF_CMD     "\"\""                  // Default kernel command
#define DEF_MOUNT   0                       // Default mount flag
#define DEF_ROOT    16676                   // Default root disk info
#define DEF_DEV     0x0200                  // Default root device (/dev/sda1)

char cmd_line[ MAX_CMDLINE + 1 ];           // Command line buffer

static const unsigned char Extendedtags[] = { 5, 0x85, 0x0f };
int isExtended(int type) {
  int i = 0;
  while (Extendedtags[ i ]) {
    if (Extendedtags[ i ] == type) return 0;
    else i++;
  }
  return 1;
}

// lba map
struct map_t {
    lba32 stage2lba;
    lba32 startlba;
    lba32 sizelba;
} map;

// Get file size

static
off_t get_size(const char* file) {
    struct stat st;
    int ret;

    ret = stat(file, &st);
    if (ret) {
        fprintf(stderr, "Could not get status of %s!\n", file);
        exit(1);
    }
    return (st.st_size);
}


static 
int getlba(int fd, union disk_record *dr, struct map_t* mapp) {
    int i, ret, recordcnt, extndx;
    lba32 dr_offset = 0, epbr_offset = 0;
    recordcnt = 0;

    mapp->stage2lba = dr->record.partTbl[0].startLBA;

    for (i = 0; i < MaxPrimary; i++) {
        if(dr->record.partTbl[i].boot) {
            mapp->startlba = dr->record.partTbl[i].startLBA;
            mapp->sizelba  = dr->record.partTbl[i].size;
            return 0;
        }
    }

    recordcnt++;
    while ( 1 ) {
        
        extndx = 0;
        for( i = 0; i < MaxPrimary; i++) {
            if(!isExtended(dr->record.partTbl[i].type)) {
                extndx = i; 
            }
        }
        if (recordcnt > MaxRecord) {
            fprintf(stderr, "Too many disk records\r\n");
            return -1;
        }
        if (extndx) {
            dr_offset = dr->record.partTbl[extndx].startLBA;
            if( recordcnt < 2)
                epbr_offset = dr_offset;
            else
                dr_offset += epbr_offset;
            
            ret = lseek(fd, dr_offset, SEEK_SET);
            if (ret != dr_offset) {
                fprintf(stderr, "Seek error");
                return -1;
            }
            ret = read(fd, &(dr->image), SectorSize);
            if (ret != SectorSize) {
                fprintf(stderr, "MBR read error");
                return -1;
            }

            for (i = 0; i < 2; i++) {
                if(dr->record.partTbl[i].boot) {
                    mapp->startlba = dr_offset + dr->record.partTbl[i].startLBA;
                    mapp->sizelba  = dr->record.partTbl[i].size;
                    return 0;
                }
            }
            recordcnt++;
        }
        else break; 
    }
    return -1;
}


// Excute a shell command
static
void execution(int len) {
    int ret;

    if (len < 0) {
        fprintf(stderr, "Command line is too long!\n");
        exit(1);
    }
    printf("Excuting: %s\n", cmd_line);
    ret = system(cmd_line);
    if (ret < 0) {
        fprintf(stderr, "Excution failed\n");
        exit(1);
    }
}

// build starts..

int main(int argc, char const *argv[])
{
    off_t stage1_size, stage2_size, stage3_size, kernel_size;
    int max_stage2, max_stages;
    union disk_record dr;
    struct map_t map;
    int seek = 0;
    int fd, ret;
    long pos;

    if (argc != 6) {
        fprintf(stderr,"Usage: build boot_image stage1 stage2 stage3 kernel\n");
        return 1;
    }

    // Get sector size of every file

    stage1_size =  get_size((const char*) argv[ 2 ]);
    stage2_size = (get_size((const char*) argv[ 3 ]) + 511) / 512;
    stage3_size = (get_size((const char*) argv[ 4 ]) + 511) / 512;
    kernel_size = (get_size((const char*) argv[ 5 ]) + 511) / 512;

    // Size checker for stage1, stage2, stage3

    fd = open(IMAGE, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Can't open the device");
        return 1;
    }
    pos = lseek(fd, 0L, SEEK_SET);
    if (pos != 0L) {
        close(fd);
        fprintf(stderr, "Seek error");
        return 1;
    }
    ret = read(fd, &(dr.image), SectorSize);
    if (ret != SectorSize) {
        fprintf(stderr, "MBR read error");
        return 1;
    }
    if (getlba(fd, &dr, &map) < 0) {
        fprintf(stderr, "Can not access any disk record\n");
        close(fd);
        return 1;
    }
    close(fd);
    max_stage2 = map.stage2lba - 1;
    max_stages = map.sizelba;

    if (stage1_size != max_stage1) {
        fprintf(stderr, "stage1 must be within 446 bytes!\n");
        return 1;
    }
    if ((stage2_size) > max_stage2) {
        fprintf(stderr, "stage2 size exceeds %d sectors\n", max_stage2);
        return 1;
    }

    if ((stage3_size + kernel_size) > max_stages) {
        fprintf(stderr, "stage3 size exceeds %d sectors\n", max_stages);
    }

    // Clear the boot image with zeroes

    execution(snprintf(cmd_line, sizeof(cmd_line), \
    "dd if=/dev/zero of=%s bs=1 count=%d conv=notrunc >/dev/null", \
                                            IMAGE, max_stage1));

    execution(snprintf(cmd_line, sizeof(cmd_line), \
    "dd if=/dev/zero of=%s bs=512 seek=1 count=%d conv=notrunc >/dev/null", \
                                            IMAGE, max_stage2));

    // Write out stage1

    execution(snprintf(cmd_line, sizeof(cmd_line), \
    "dd if=%s of=%s conv=notrunc > /dev/null", STAGE1, IMAGE));
    seek += 1;

    // Write out stage2

    execution(snprintf(cmd_line, sizeof(cmd_line), \
    "dd if=%s of=%s bs=512 seek=%d conv=notrunc >/dev/null", STAGE2, IMAGE, \
                                                                    seek));

    // Write out stage3
    seek = map.startlba;
    execution(snprintf(cmd_line, sizeof(cmd_line), \
    "dd if=%s of=%s bs=512 seek=%d conv=notrunc >/dev/null", STAGE3, IMAGE, \
                                                                    seek));
    seek += stage3_size;

    // Write out compressed kernel

    execution(snprintf(cmd_line, sizeof(cmd_line), \
    "dd if=%s of=%s conv=notrunc seek=%d >/dev/null", KERNEL, IMAGE, seek));

    // Write system parameters

    execution(snprintf(cmd_line, sizeof(cmd_line), \
    "./config %s %s %d %d %d %d %d %d &>/dev/null", IMAGE, DEF_CMD, \
    DEF_MOUNT, DEF_MOUNT, DEF_DEV, (int) stage2_size, (int) stage3_size, \
                                                (int) kernel_size));
    return 0;
}
