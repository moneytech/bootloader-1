#
# Makefile for "boot disk"
# tested with  gcc 5.X  or  gcc 4.8

CC = gcc-4.8
CFLAGS = -m32 -fno-common -Wall -Os -c        \
		 -finhibit-size-directive -fno-ident  \
		 -fomit-frame-pointer -fcall-used-ebx \
		 -fno-asynchronous-unwind-tables      \
		 -fno-stack-protector                 \
		 -fno-delete-null-pointer-checks      \
		 -fno-pie
TARGET   = specify!
FIRST    = stage1
SECOND   = stage2
THIRD    = stage3
KERNEL   = specify!
BZKERNEL = kernel.bz2
OBJS1    = crt.o bios_hd.o stage1.o
OBJS2    = stage2.o param.o load_images.o bios_tty.o bios_hd.o bios_mem.o
OBJS3    = stage3.o bz_decomp.o bios_tty.o start_32.o
CONFIG   = config
BUILD    = build

all: ${FIRST} ${SECOND} ${THIRD}

${FIRST} : ${OBJS1} stage1.ls
	ld -m elf_i386 -T stage1.ls -o $@ ${OBJS1} \
	-Map ${FIRST}.map --cref

crt.o : crt.S memlayout.h
	${CC} ${CFLAGS} $<

bios_tty.o : bios_tty.c realmode.h
	${CC} ${CFLAGS} $<

bios_hd.o : bios_hd.c realmode.h
	${CC} ${CFLAGS} $<

bios_mem.o : bios_mem.c realmode.h
	${CC} ${CFLAGS} $<

stage1.o : stage1.c realmode.h bios_hd.h diskrec.h
	${CC} ${CFLAGS} $<


${SECOND} : ${OBJS2} stage2.ls
	ld -m elf_i386 -T stage2.ls -o $@ ${OBJS2} \
	-Map ${SECOND}.map --cref

stage2.o : stage2.c realmode.h bios_tty.h load_images.h memlayout.h param.h
	${CC} ${CFLAGS} $<

param.o : param.c
	${CC} ${CFLAGS} $<

load_images.o : load_images.c realmode.h memlayout.h \
				bios_hd.h bios_mem.h bios_tty.h
	${CC} ${CFLAGS} $<

${THIRD}: ${OBJS3} stage3.ls set_*.o
	ld -m elf_i386 -T stage3.ls -o $@ ${OBJS3} set_*.o \
	-Map ${THIRD}.map

stage3.o : stage3.c realmode.h memlayout.h bios_tty.h
	${CC} ${CFLAGS} $<

bz_decomp.o :
	ln -sf ./bz2/bz_decomp.o bz_decomp.o

set_*.o : set_*.c realmode.h setup.h
	${CC} ${CFLAGS} set_*.c

start_32.o : start_32.c memlayout.h bzlib.h
	${CC} ${CFLAGS} $<


install: ${TARGET} ${CONFIG} ${BUILD}

${BZKERNEL} : ${KERNEL}
	cp ${KERNEL} kernel.bin
	#objcopy -O binary -S -R .comment -R .note kernel.bin
	bzip2 -zcfk1 kernel.bin > ${BZKERNEL}

${TARGET} : ${FIRST} ${SECOND} ${THIRD} ${BZKERNEL} ${CONFIG} ${BUILD}
	./${BUILD} $@ ${FIRST} ${SECOND} ${THIRD} ${BZKERNEL}

${CONFIG} : config.c
	gcc -Wall -s -o $@ $<

${BUILD} : build.c
	gcc -Wall -s -o $@ $<

clean:
	rm -rf *.o *.map ${FIRST} ${SECOND} ${THIRD} \
	${BZKERNEL} ${CONFIG} ${BUILD} kernel.bin
