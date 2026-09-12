print_string:
    pusha
    mov ah,0x0e         ;BIOS Video ISR load

.loop:
    mov al, [bx]
    cmp al, 0
    je .done

    int 0x10
    add bx, 1
    jmp .loop

.done:
    popa
    ret