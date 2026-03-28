[org 0x7C00]
[bits 16]

KERNEL_LOAD_ADDRESS equ 0x1000

start:
    ; Set up segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Load kernel from disk
    mov ah, 0x02            ; BIOS read sectors function
    mov al, KERNEL_SECTORS  ; number of sectors to read (injected by Makefile)
    mov ch, 0x00            ; cylinder 0
    mov cl, 0x02            ; start from sector 2 (sector 1 is our bootloader)
    mov dh, 0x00            ; head 0
    mov bx, KERNEL_LOAD_ADDRESS
    int 0x13                ; BIOS disk interrupt
    jc disk_error           ; if carry flag is set something went wrong

    ; Jump to kernel
    jmp KERNEL_LOAD_ADDRESS

disk_error:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

error_msg db 'Disk read failed. RIP.', 0

times 510 - ($ - $$) db 0
dw 0xAA55
