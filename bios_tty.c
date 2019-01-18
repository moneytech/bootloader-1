#include "realmode.h"

void printc(char ch) {
    asm volatile(
        "int $0x10"
        :
        : "a" (0x0e00 | (ch&0xff)),
          "b" (7)
    );
}

void prints(const char* msg) {
    while (*msg)
        printc(*msg++);
}

void print_hex(int val) {
    printc((val < 10) ? val + '0' : val - 10 + 'A');
}

void print_hex8(int val) {
    print_hex((val & 0xf0) >> 4);
    print_hex((val & 0x0f));
}

void print_hex16(int val) {
    print_hex8((val & 0xff00) >> 8);
    print_hex8((val & 0x00ff));
}

void print_hex32(int val) {
    print_hex16((val & 0xffff0000) >> 16);
    print_hex16((val & 0xffff));
}
