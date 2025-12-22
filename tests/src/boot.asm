; boot.asm - 16-bit Real Mode Bootloader -> Switch to 32-bit PM
[bits 16]
[org 0x7C00]

start:
    cli
    mov [boot_drive], dl        ; Save BIOS boot drive #

    ; Setup stack
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFF
    sti

    mov si, msg_loading
    call print_string

    ; -------------------------
    ; Load kernel at 0x10000
    ; -------------------------
    mov bx, 0x1000              ; segment
    mov es, bx
    xor bx, bx                  ; offset 0

    mov cx, 2                   ; Start from sector 2

.read_loop:
    mov ah, 0x02                ; BIOS read
    mov al, 1                   ; Read 1 sector at a time
    mov ch, 0                   ; cylinder 0
    mov cl, cl                  ; current sector
    mov dh, 0                   ; head 0
    mov dl, [boot_drive]

    int 0x13
    jc .done_reading            ; If error, we're done

    ; Move to next sector
    add bx, 512                 ; Advance buffer (512 bytes per sector)
    jnc .no_segment_wrap
    ; If bx wrapped, increment segment
    mov dx, es
    add dh, 0x10                ; Add 4KB to segment
    mov es, dx
    xor bx, bx

.no_segment_wrap:
    inc cl                      ; Next sector
    cmp cl, 18                  ; Max 18 sectors per track
    jbe .read_loop

.done_reading:
    mov si, msg_kernel
    call print_string

    ; -------------------------
    ; Enter Protected Mode
    ; -------------------------
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode

[bits 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x00F00000
    mov esp, ebp

    jmp 0x10000

[bits 16]
print_string:
    pusha
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x07
.next:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .next
.done:
    popa
    ret

gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

msg_loading db "Bootloader: Loading kernel...", 13, 10, 0
msg_kernel  db "Kernel loaded successfully.", 13, 10, 0
boot_drive  db 0

times 510 - ($ - $$) db 0
dw 0xAA55
