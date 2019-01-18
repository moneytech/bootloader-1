//
// m b u n z i p 2 . c
//
// On memory bunzip2 function (from libbzip2) testing program
//
// BUILD: gcc-4.8 -Wall -O2 -o mbunzip2 mbunzip2.c libbz2.o
//

#include <stdlib.h>                     // exit()
#include <unistd.h>                     // read(), write(), close()
#include <fcntl.h>                      // open()
#include "bzlib.h"                      // BZ2_bzBufftoBuffDecompress()

#define MAX_SRC_SIZE    0x200000
#define MAX_DST_SIZE    0x400000

// Supplement functions for libbz2.o

void bz_internal_error(int error) {
    printf("Internal error was occured %d\n", error);
    exit(error);
}

// BZ2_bzBufftoBuffDecompress() testing code

int main(int argc, char const *argv[])
{
    void* src;                          // Source compressed image buffer
    void* dst;                          // Decompressed image buffer
    void* buf;                          // Buffer pointer
    int fd, ret = 1;
    ssize_t cnt, ssize = 0, dsize = MAX_DST_SIZE;

    if (argc != 3) {
        printf("Usage: mbunzip2 bzipped_image decompressed_image\n");
        return 1;
    }

    // Prepare rooms for source and destination buffer

    src = malloc(MAX_SRC_SIZE);
    dst = malloc(MAX_DST_SIZE);
    if (! src || ! dst) {
        printf("Memory allocation error.\n");
        return 1;
    }

    // Read into source compressed image

    buf = src;
    fd = open(argv[ 1 ], O_RDONLY);
    if (fd < 0) {
        printf("File open failure.\n");
        goto release_mem;
    }
    while ((cnt = read(fd, buf, 4096)) > 0) {
        buf += cnt;
        ssize += cnt;
        if ((ssize + 4096) > MAX_SRC_SIZE) {
            printf("Source buffer overrun!\n");
            goto release_mem;
        }
    }
    if (cnt < 0) {
        printf("File read error.\n");
        close(fd);
        goto release_mem;
    }
    printf("%s: %d bytes read.\n", argv[ 1 ], ssize);
    close(fd);

    // Do decompress

    ret = BZ2_bzBuffToBuffDecompress(dst, &dsize, src, ssize, 0, 0);
    if (ret) {
        printf("Decompression error %d.\n", ret);
        close(fd);
        goto release_mem;
    }
    printf("Decompressed image size id %d bytes.\n", dsize);

    // Output decompressed image to user specified file

    fd = creat(argv[ 2 ], 0666);
    if (fd < 0) {
        printf("File open failure.\n");
        close(fd);
        goto release_mem;
    }
    cnt = write(fd, dst, dsize);
    if (cnt < 0) {
        printf("File write error.\n");
        close(fd);
        goto release_mem;
    }
    close(fd);

release_mem:
    free(src);
    free(dst);

    return ret;
}
