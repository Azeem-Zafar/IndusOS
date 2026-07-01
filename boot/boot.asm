BITS 16
ORG 0x7C00

start:
    CLI
    XOR AX, AX
    MOV DS, AX
    MOV ES, AX
    MOV SS, AX
    MOV SP, 0x7BFF
    STI

    MOV AH, 0x02
    MOV AL, 0x20
    MOV CH, 0x00
    MOV CL, 0x02
    MOV DH, 0x00
    MOV DL, 0x00
    MOV BX, 0x0000
    MOV ES, BX
    MOV BX, 0x1000
    INT 0x13

    CLI
    LGDT [gdt_descriptor]
    MOV EAX, CR0
    OR EAX, 1
    MOV CR0, EAX
    JMP 8:pm

gdt_start:
    DQ 0
gdt_code:
    DW 0xFFFF,0,0x9A00,0xCF
gdt_data:
    DW 0xFFFF,0,0x9200,0xCF
gdt_end:

gdt_descriptor:
    DW gdt_end-gdt_start-1
    DD gdt_start

BITS 32
pm:
    MOV AX, 16
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    MOV SS, AX
    MOV ESP, 0x90000
    JMP 0x1000

TIMES 510-($-$$) db 0
DW 0xAA55
