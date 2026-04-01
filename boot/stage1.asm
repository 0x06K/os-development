[ORG 0x7C00]
[BITS 16]

jmp 0:start

start:
    cli                                     ; clear interrupts
    xor ax, ax                              ; Zero out AX
    mov ds, ax                              ; Data Segment = 0
    mov es, ax                              ; Extra Segment = 0
    mov ss, ax                              ; Stack Segment = 0

                                            ;===============================================================
                                            ; stack grows downward. 0x0500−0x7BFF is guaranteed to be free.
                                            ; this is roughly 30 KB which can be used for stack. so we ca
    mov sp, 0x7C00                          ; the stack grows downward in x86. So if we set SP = 0x7C00
                                            ; the stack doesn't start at 0x7C00 it starts just above it
                                            ; PUSH instruction will decrement SP first to 0x7BFE.
                                            ; and then write data there.
                                            ;===============================================================

    sti                                     ; it will set interrupt flag



    ; ---------------------------------------------------------------
    ; Save boot drive number and Print Welcome Message
    ; ---------------------------------------------------------------
    mov [boot_drive], dl                   ; BIOS puts boot drive number in DL, will be used for reading kernel
    mov si, welcome_msg                    ; point SI to welcome string
    call print                             ; print it



    ; ---------------------------------------------------------------
    ; Load kernel into 0x10000 from sector 1
    ; ---------------------------------------------------------------
    push ds                                ; save DS, INT 13h may clobber it
    mov ah, 0x42                           ; BIOS extended read function
    mov dl, [boot_drive]                   ; drive to read from
    mov si, dap                           ; DS:SI points to Disk Address Packet
    int 0x13                               ; call BIOS — reads sectors into 0x10000
    pop ds                                 ; restore DS after BIOS call



    ; ---------------------------------------------------------------
    ; Enabling A20
    ; ---------------------------------------------------------------
    mov ax, 0x2401              ; AH=0x24 function, AL=0x01 enable
    int 0x15



    ; ---------------------------------------------------------------
    ; Getting memory map via 0xE820
    ; ---------------------------------------------------------------
    xor ebx, ebx                ; EBX = 0, first call marker
    xor bp, bp                  ; BP = 0, entry counter
    mov edx, 0x534D4150         ; 'SMAP' magic
    mov di, buffer              ; ES:DI points to buffer

.loop:
    mov eax, 0xE820             ; reload — BIOS trashes EAX each call
    mov ecx, 24                 ; reload — BIOS may trash ECX too
    int 0x15                    ; write one entry at ES:DI
    jc  .done                   ; carry = error or no more entries
    add di, 24                  ; advance buffer to next slot
    inc bp                      ; count this entry
    test ebx, ebx               ; EBX = 0 means last entry
    jnz .loop                   ; more entries — go back

.done:
    mov [mem_entry_count], bp   ; save total entry count







;=================================================================
; DATA
;=================================================================


boot_drive      db 0                       ; boot drive number saved from DL
mem_entry_count dw 0                       ; number of E820 memory map entries detected

buffer equ 0x0500                          

welcome_msg  db 'Bootloader stage 1 started', 0x0D, 0x0A, 0   ; startup message

dap:
    db 0x10, 0x00                          ; DAP size (16 bytes), reserved zero
    dw 5                                   ; number of sectors to read
    dw 0x0000                              ; destination offset
    dw 0x1000                              ; destination segment → physical 0x10000
    dq 1                                   ; LBA address of first sector to read


;=================================================================
; GDT — Global Descriptor Table (flat 32-bit model)
;=================================================================
gdt_start:
    dq 0x0000000000000000                  ; null descriptor — required as first entry

gdt_code:
    dw 0xFFFF, 0x0000                      ; limit 4GB, base 0
    db 0x00, 0b10011010, 0b11001111, 0x00  ; present, ring0, code, executable, readable, 4KB gran, 32-bit

gdt_data:
    dw 0xFFFF, 0x0000                      ; limit 4GB, base 0
    db 0x00, 0b10010010, 0b11001111, 0x00  ; present, ring0, data, writable, 4KB gran, 32-bit

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1            ; GDT size in bytes minus 1 (limit field)
    dd gdt_start                           ; linear base address of GDT


;=================================================================
; PRINT — prints null-terminated string pointed to by SI
;=================================================================
print:
    mov ah, 0x0E                           ; BIOS teletype output function
.loop:
    lodsb                                  ; load byte at DS:SI into AL, advance SI
    cmp al, 0                              ; check for null terminator
    je  .done                              ; if null, we are done
    int 0x10                              ; print character in AL
    jmp .loop                              ; next character
.done:
    ret


;=================================================================
; PADDING + BOOT SIGNATURE
;=================================================================
[BITS 16]
times 510 - ($ - $$) db 0                 ; pad remaining bytes to reach byte 510
dw 0xAA55                                 ; boot signature — BIOS checks this to confirm bootable disk