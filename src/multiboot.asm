section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, 0x90000
    call kernel_main
    hlt
