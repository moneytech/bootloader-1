#include "realmode.h"
#include "memlayout.h"
#include "bios_tty.h"
#include "setup.h"

typedef struct desc_tbl_t {             
    unsigned short limit;               
    unsigned long  base;                
} desc_tbl;

typedef unsigned long long x86_desc;
x86_desc gdt[ 6 ] = {                   
    0x0000000000000000,                 
    0x0000000000000000,                 
    0x00CF9A000000FFFF,
    0x00CF92000000FFFF,                 
    0x00CF9A007C00FFFF,                 
    0x00CF92007C00FFFF,                 
};

desc_tbl idtr = { 0, 0 };               
desc_tbl gdtr = { 0x8000, 0 };          
        
#define KERNEL_SIZE     0x100           
#define MOUNT_FLAG      0x1F2           
#define ROOT_PARAM      0x1F8           
#define VGA_MODE        0x1EA           
#define ROOT_DEVICE     0x1FC           
#define COMMAND_LINE    0x228           
#define KEY_RDWR        0x60            
#define KEY_STATUS      0x64            
#define KEY_CMD         0x64            
#define KEY_OUT_FULL    0x01            
#define KEY_IN_FULL     0x02            
#define KEY_WRITE_OUT   0xD1            
#define KEY_ENABLE_A20  0xDF            

static void clear_bss(void);
static void clear_0to2k(void);
static void clear_2to4k(void);
static void do_setupcalls(void);
static void do_basic_setup(int, int, int, int);
static int  check_a20(void);
static void io_delay(void);
static int  input_byte(int);
static void output_byte(int, int);
static void flush_8042(void);
static void enable_a20(void);
static void move_to_protect(void);

int __attribute__((section(".first"))) stages(int kernel_size, char* cmd_line, \
                                              int mount_flag, int root_param,  \
                                              int root_dev) 
{
    char* ptr;

    clear_bss();
    
    clear_2to4k();                          
    ptr = (char*)(0x800);                   
    while(*cmd_line) 
        *ptr++ = *cmd_line++;
    clear_0to2k();                          
    
    prints("Stage3:\r\n");                  
    prints("  Initializing system.\r\n");

    do_basic_setup(kernel_size, mount_flag, root_dev, root_dev);
    enable_a20();

    prints("  Decompressing kernel ...");
    move_to_protect();
}

static 
void  __attribute__((noinline)) do_basic_setup(int kernel_size, int mount_flag, \
                int root_param, int root_dev) {

    do_setupcalls();                   

    *((short int*) MOUNT_FLAG)     = (short int) mount_flag;
    *((short int*) ROOT_PARAM)     = (short int) (root_param & 0xFFFF);
    *((short int*) VGA_MODE)       = 0xFFFF;
    *((short int*) ROOT_DEVICE)    = (short int) root_dev;
    *((unsigned int*) KERNEL_SIZE) = ((unsigned int) kernel_size) * 512;
    *((long int*) COMMAND_LINE)    = (INIT_SEG << 4) + 0x800;
    
}

static 
void __attribute__((noinline)) do_setupcalls(void)
{
	setupcall_t *call;

	call = &__setupcall_start;
	do {
		(*call)();
		call++;
	} while (call < &__setupcall_end);
}

static
void __attribute__((noinline)) clear_bss(void) {
    asm volatile(
        "xorl   %eax, %eax       \n\t"
        "movl   $_data_end, %edi \n\t"
        "movl   $_bss_end, %ecx  \n\t"
        "cmpl   %edi, %ecx       \n\t"
        "je     1f               \n\t"
        "subl   %edi, %ecx       \n\t"
        "cld                     \n\t"
        "rep                     \n\t"
        "stosb                   \n"
    "1:                          \n"
    );
}

static
void __attribute__((noinline)) clear_0to2k(void) {
    int i;
    char* ptr = (char*) (0);                
    for (i = 0; i < 0x800; i++)
        *ptr++ = 0;
}

static 
void __attribute__((noinline)) clear_2to4k(void) {
    int i;
    char* ptr = (char*) (0x800);            
    for (i = 0; i < 0x800; i++)
        *ptr++ = 0;
}

static
int __attribute__((noinline)) check_a20(void) {
    int ret;
    asm volatile (
        "movl   $1, %%ebx       \n\t"
        "xorw   %%ax, %%ax      \n\t"
        "movw   %%ax, %%fs      \n\t"
        "notw   %%ax            \n\t"
        "movw   %%ax, %%gs      \n\t"
        "movw   %%fs:0, %%ax    \n\t"
        "cmpw   %%gs:16, %%ax   \n\t"
        "jnz    1f              \n\t"
        "cli                    \n\t"
        "movw   %%ax, %%dx      \n\t"
        "notw   %%ax            \n\t"
        "movw   %%ax, %%fs:0    \n\t"
        "cmpw   %%gs:16, %%ax   \n\t"
        "movw   %%dx, %%fs:0    \n\t"
        "sti                    \n\t"
        "jnz    1f              \n\t"
        "xorl   %%ebx, %%ebx    \n"
    "1:                         \n\t"
        "movl   %%ebx, %%eax    \n\t"
        : "=a" (ret)
        :
    );
    return ret;
}

static
void __attribute__((noinline)) io_delay(void) {
    asm(
        "pushl  %eax            \n\t"
        "inb    $0x80, %al      \n\t"
        "inb    $0x80, %al      \n\t"
        "popl   %eax            \n\t"
    );
}

static
int __attribute__((noinline)) input_byte(int port) {
    unsigned char data;
    asm volatile(
        "inb    %%dx, %%al      \n\t"
        : "=a" (data)
        : "d" ((unsigned short) (port & 0xFFFF))
    );
    
    return (data & 255);
}

static
void __attribute__((noinline)) output_byte(int port, int data) {
    asm volatile(
        "outb   %%al, %%dx      \n\t"
        :
        : "a" (data & 255),
          "d" ((unsigned short) (port & 0xFFFF))
    );
}

static
void __attribute__((noinline)) flush_8042(void) {
    int stat;
loop:
    io_delay();
    stat = input_byte(KEY_STATUS);
    if (stat & KEY_OUT_FULL) {              
        io_delay(); input_byte(KEY_RDWR);   
        goto loop;
    }
    if (stat & KEY_IN_FULL)                 
        goto loop;
}

static
void __attribute__((noinline)) enable_a20(void) {
    int data;
    if (check_a20()) {
        prints("  A20 line is already active.\r\n");
        return;
    }
    
    flush_8042();
    output_byte(KEY_CMD, KEY_WRITE_OUT);
    flush_8042();
    output_byte(KEY_RDWR, KEY_ENABLE_A20);
    flush_8042();
    if (check_a20()) {
        prints("  A20 line is activated by classical method.\r\n");
        return;
    }
    
    data = input_byte(0x92);
    io_delay();
    output_byte(0x92, data | 2);
    while (! check_a20());                  
    prints("  A20 line is activated by fast method.\r\n");
}

static
void __attribute__((noinline)) move_to_protect(void) {
    extern void start_32(void);
    
    output_byte(0xF0, 0);                   
    output_byte(0xF1, 0);                   
    io_delay();
    
    asm("cli");                             
    output_byte(0x70, 0x80);                
    output_byte(0xA1, 0xFF);                
    io_delay();
    output_byte(0x21, 0xFB);                
    
    asm("lidt   idtr");                     
    
    asm("xorl   %eax, %eax");               
    asm("movw   %ds, %ax");                 
    asm("shll   $4, %eax");                 
    asm("addl   $gdt, %eax");
    asm("movl   %eax, (gdtr + 2)");
    asm("lgdt   gdtr");                     
    asm("movl   %cr0, %eax");               
    asm("orl    $1, %eax");                 
    asm("movl   %eax, %cr0");               
    asm("jmp    flushing");                 
    asm("flushing:");                       
    asm("movw   $0x28, %ax");               
    asm("movw   %ax, %ds");
    asm("movw   %ax, %es");
    asm("movw   %ax, %fs");
    asm("movw   %ax, %gs");
    asm("movw   %ax, %ss");
        
    asm("data32 ljmp    $0x20, $start_32");   
}
