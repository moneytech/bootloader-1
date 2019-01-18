#include "memlayout.h"

.code16gcc                      
.text                           

.extern main                    

.global _start                  
_start:
    ljmp    $INIT_SEG, $reentry 
reentry:                        
    cli                         
    movw    $INIT_SEG, %ax      
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movl    $INIT_ESP, %esp     
    sti                         
    call    main                
loop:
    jmp     loop                