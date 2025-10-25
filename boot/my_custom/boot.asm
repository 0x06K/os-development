; boot.asm - 16-bit Real Mode Bootloader -> Switch to 32-bit PM
; Loads kernel (second-stage) from disk and jumps to it
; ---------------------------------------------------------------
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
    ; Load kernel (1 sector) at 0x10000 (segment 0x1000)
    ; -------------------------
    mov ah, 0x02                ; BIOS read
    mov al, 3                   ; number of sectors
    mov ch, 0                   ; cylinder
    mov cl, 2                   ; sector (start from 2)
    mov dh, 0                   ; head
    mov dl, [boot_drive]
    mov bx, 0x1000              ; segment
    mov es, bx
    xor bx, bx                  ; offset 0
    int 0x13
    jc disk_error
    
    cmp al, 3
    jne disk_error
    
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
    
    ; Far jump to flush pipeline and enter protected mode
    jmp 0x08:protected_mode

; -------------------------
; 32-bit code starts here
; -------------------------
[bits 32]
protected_mode:
    ; Load data segment selector into all segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Setup 32-bit stack
    mov ebp, 0x90000
    mov esp, ebp
    
    ; Jump to kernel entry point
    jmp 0x10000

; -------------------------
; Back to 16-bit for error handling
; -------------------------
[bits 16]
disk_error:
    mov si, msg_error
    call print_string
    jmp $

; -------------------------
; 16-bit BIOS Print Function
; -------------------------
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

; -------------------------
; Global Descriptor Table
; -------------------------
gdt_start:
    ; Null descriptor
    dq 0x0000000000000000
    
    ; Code segment descriptor (0x08)
    ; Base=0, Limit=0xFFFFF, 4KB granularity, 32-bit, executable, readable
    dq 0x00CF9A000000FFFF
    
    ; Data segment descriptor (0x10) 
    ; Base=0, Limit=0xFFFFF, 4KB granularity, 32-bit, writable
    dq 0x00CF92000000FFFF

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start                ; GDT base address

; -------------------------
; Data
; -------------------------
msg_loading db "Bootloader: Loading kernel...", 13, 10, 0
msg_kernel  db "Kernel loaded successfully.", 13, 10, 0
msg_error   db "Disk read error!", 13, 10, 0
boot_drive  db 0

; -------------------------
; Boot sector signature
; -------------------------
times 510 - ($ - $$) db 0
dw 0xAA55
