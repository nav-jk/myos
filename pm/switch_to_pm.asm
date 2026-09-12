[bits 16]

switch_to_pm:
    cli                 ; Disable all interrupts for now

    lgdt[gdt_descriptor]

    mov eax, cr0            ; To make the switch to protected mode, we set
    or eax, 0x1             ; the first bit of CR0, a control register
    mov cr0, eax

    jmp CODE_SEG:init_pm    ; Make a far jump (i.e. to a new segment) to our 32-bit
                            ; code. This also forces the CPU to flush its cache of
                            ; pre-fetched and real-mode decoded instructions, which can
                            ; cause problems.

[bits 32]

; Init registers and stack to defined segments in gdt

init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax      
    mov es, ax      
    mov fs, ax     
    mov gs, ax  

    mov ebp, 0x90000    ; Update our stack position so it is right
    mov esp, ebp        ; at the top of the free space.

    call BEGIN_PM       ; Finally, call some well-known label
