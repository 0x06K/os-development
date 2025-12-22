; gdt.asm
global gdt_flush

; gdt_flush(uint32_t gdt_ptr_address)
; Argument: address of gdt_ptr structure is in EAX
gdt_flush:
    mov eax, [esp + 4]    ; get the argument (pointer to gdt_ptr)
    lgdt [eax]             ; load the GDT

    ; Reload segment registers
    mov ax, 0x10           ; kernel data segment selector (GDT entry 2)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS
    ; kernel code segment selector (GDT entry 1)
    jmp 0x08:flush_done

flush_done:
    ret