//
// c o n f i g . c
//
// Boot disk configuration tool
//

#include <fcntl.h>                              // open()
#include <unistd.h>                             // read()
#include <stdlib.h>                             // atoi()
#include <stdio.h>                              // printf()
#include <sys/types.h>                          // off_t
#include <string.h>                             // strncpy()

// Boot protocol parameter address

#define STAGE2  (unsigned short*) &buf[ 444 ]   // Stage1 sector length
#define COMMAND           (char*) &buf[ 512 ]   // command line address
#define STAGE3  (unsigned short*) &buf[ 630 ]   // stage2 sector length
#define KERNEL  (unsigned short*) &buf[ 632 ]   // kernel sector length
#define MOUNT   (unsigned short*) &buf[ 634 ]   // Root disk mount flag
#define ROOT    (unsigned short*) &buf[ 636 ]   // Root disk information
#define DEVICE  (unsigned short*) &buf[ 638 ]   // Root device

// config starts...

int main(int argc, char const *argv[])
{
    /* code */
    int fd;
    ssize_t cnt;
    off_t pos;
    char buf[ 640 ];

    if (argc != 2 && argc != 6 && argc != 9) {
        printf("Usage: config file [ command mount root device [ \
                                                stage2 stage3 kernel ]]\n");
        return 1;
    }

    fd = open(argv[ 1 ], O_RDWR);
    if (fd == -1) {
        printf("File open failure!\n");
        return 1;
    }

    cnt = read(fd, buf, 640);                   // Read boot sector
    if (cnt != 640) {
        printf("File read error!\n");
        close(fd);
        return 1;
    }
    if (argc == 2) {                            // Show current status
        printf("=== Current status of [ %s ]===\n", argv[ 1 ]);
        printf("       command line : \"%s\"\n", COMMAND);
        printf("         mount flag : %d\n",   *(MOUNT));
        printf("     root disk info : %04X\n", *(ROOT));
        printf("        root device : %04X\n", *(DEVICE));
        printf(" stage2 sector size : %d\n",   *(STAGE2));
        printf(" stage3 sector size : %d\n",   *(STAGE3));
        printf("        kernel size : %u\n",   *(KERNEL));
    } else {                                    // Update boot parameters
        pos = lseek(fd, 0, SEEK_SET);           // Reset file pointer
        strncpy(COMMAND, argv[ 2 ], 118);
        *(MOUNT)  = (unsigned short) (atoi(argv[ 3 ]) & 0xFFFF);
        *(ROOT)   = (unsigned short) (atoi(argv[ 4 ]) & 0xFFFF);
        *(DEVICE) = (unsigned short) (atoi(argv[ 5 ]) & 0xFFFF);
        if (argc == 9) {
            *(STAGE2) = (unsigned short) (atoi(argv[ 6 ]) & 0xFFFF);
            *(STAGE3) = (unsigned short) (atoi(argv[ 7 ]) & 0xFFFF);
            *(KERNEL) = (unsigned short) (atoi(argv[ 8 ]) & 0xFFFF);
        }
        cnt = write(fd, buf, 640);
        if (cnt != 640) {
            printf("File write error!\n");
            close(fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}
