[ORG 0x7C00]
[BITS 16]

;It performs a far jump to set CS = 0 and IP = offset of start
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
; ENTER PROTECTED MODE
;=================================================================

    ; ---------------------------------------------------------------
    ; Load GDT into GDTR
    ; ---------------------------------------------------------------
    cli                             ; interrupts MUST be off before switching modes
    lgdt [gdt_descriptor]           ; load GDT register — tells CPU where our GDT lives


    ; ---------------------------------------------------------------
    ; Set CR0.PE = 1 — this flips the CPU into protected mode
    ; You cannot write CR0 directly; you must go through a GPR
    ; ---------------------------------------------------------------
    mov eax, cr0
    or  eax, 0x1                    ; set bit 0 (Protection Enable)
    mov cr0, eax


    ; ---------------------------------------------------------------
    ; Far jump — two purposes:
    ;   1. Flushes the prefetch queue (CPU may have decoded real-mode
    ;      instructions ahead — we need to discard them)
    ;   2. Loads CS with our code segment selector (0x08 = GDT entry 1)
    ;      A segment selector is: index<<3 | TI | RPL
    ;      0x08 = 0000 0000 0000 1000 → index=1, TI=0 (GDT), RPL=0 (ring 0)
    ; ---------------------------------------------------------------
    jmp 0x08:protected_mode_entry   ; CS = 0x08, EIP = physical address of label below


;=================================================================
; WE ARE NOW IN 32-BIT PROTECTED MODE
;=================================================================
[BITS 32]

protected_mode_entry:
    ; ---------------------------------------------------------------
    ; CS is set by the far jump above.
    ; But DS, ES, FS, GS, SS still contain real-mode garbage selectors.
    ; Load them all with our data segment selector: 0x10
    ;   0x10 = index=2, TI=0, RPL=0 → gdt_data
    ; ---------------------------------------------------------------
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax


    ; ---------------------------------------------------------------
    ; Set up a 32-bit stack below the kernel load address.
    ; Kernel is at 0x10000. Anything below that and above 0x7E00
    ; (end of bootloader) is safe. 0xC000 is a clean midpoint.
    ; Stack grows DOWN so ESP = top of stack region.
    ; ---------------------------------------------------------------
    mov esp, 0x0000C000


    ; ---------------------------------------------------------------
    ; Jump to kernel loaded at physical address 0x10000
    ; (segment 0x1000, offset 0x0000 from INT 13h DAP earlier)
    ; In protected mode, 0x08 code segment is flat — base=0, limit=4GB
    ; so physical address == logical offset directly.
    ; ---------------------------------------------------------------
    jmp 0x10000                     ; transfer control to your kernel entry point


    ; ---------------------------------------------------------------
    ; Should never reach here. Halt the CPU if we do.
    ; ---------------------------------------------------------------
    cli
    hlt



[BITS 16]


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
; PADDING + BOOT SIGNATURE
;=================================================================

times 510 - ($ - $$) db 0                 ; pad remaining bytes to reach byte 510
dw 0xAA55                                 ; boot signature — BIOS checks this to confirm bootable disk