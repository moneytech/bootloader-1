#
# Makefile for "libbzip2" decompression module
#

CC = gcc-4.8
CFLAGS = -m32 -fno-common -Wall -Os -c \
         -finhibit-size-directive -fno-ident \
         -fomit-frame-pointer -fcall-used-ebx \
		 -fno-asynchronous-unwind-tables \
		 -fno-stack-protector \
         -DBZ_NO_STDIO

TARGET  = bz_decomp.o
HEADERS = bzlib.h bzlib_private.h
OBJS    = huffman.o    \
		  crctable.o   \
		  randtable.o  \
		  decompress.o \
		  bzlib.o

all: ${TARGET}

${TARGET} : ${OBJS}
	ld -m elf_i386 -r -o $@ ${OBJS}

huffman.o: huffman.c ${HEADERS}
	${CC} ${CFLAGS} $<

crctable.o: crctable.c ${HEADERS}
	${CC} ${CFLAGS} $<

randtable.o: randtable.c ${HEADERS}
	${CC} ${CFLAGS} $<

decompress.o: decompress.c ${HEADERS}
	${CC} ${CFLAGS} $<

bzlib.o: bzlib.c ${HEADERS}
	${CC} ${CFLAGS} $<

clean:
	rm -f *.o
