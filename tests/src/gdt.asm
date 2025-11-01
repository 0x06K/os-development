[BITS 32]
global gdt_flush

section .text
gdt_flush:
    mov eax, [esp + 4]    ; get the pointer argument (to gdt_ptr)
    lgdt [eax]

    ; Reload segment registers
    mov ax, 0x10          ; Kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush_done  ; Far jump to reload CS
.flush_done:
    ret