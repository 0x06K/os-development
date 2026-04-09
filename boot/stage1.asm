[ORG 0x7C00]
[BITS 16]

jmp 0:start

start:
    cli                         ; disable interrupts during setup
    xor ax, ax
    mov ds, ax                  ; zero all segments
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; stack grows down from bootloader base
    sti                         ; safe to re-enable interrupts now

    mov [boot_drive], dl        ; BIOS puts boot drive in DL, save it

    mov si, msg_start
    call print

    ; -------------------------------------------------------
    ; INT 13h AH=41h — check if BIOS supports LBA extensions
    ; BX=0x55AA is a magic number BIOS expects
    ; if carry is set — no LBA support, we hang
    ; -------------------------------------------------------
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc  hang

    mov si, msg_lba
    call print

    ; -------------------------------------------------------
    ; INT 13h AH=42h — extended read using DAP
    ; DS:SI points to the Disk Address Packet (DAP)
    ; DAP tells BIOS: read 1 sector from LBA 1 into 0x10000
    ; we push/pop DS because INT 13h may clobber it
    ; -------------------------------------------------------
    push ds
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap
    int 0x13
    pop ds

    mov si, msg_loaded
    call print

    ; -------------------------------------------------------
    ; INT 15h AX=2401h — enable A20 line
    ; without A20, address bit 20 is always 0
    ; meaning memory wraps at 1MB — we need it open for PM
    ; -------------------------------------------------------
    mov ax, 0x2401
    int 0x15

    mov si, msg_a20
    call print

    ; -------------------------------------------------------
    ; INT 15h EAX=E820h — query BIOS memory map
    ; each call fills one 24-byte entry at ES:DI
    ; EBX is a continuation token — 0 means last entry
    ; EDX must hold magic 'SMAP' = 0x534D4150
    ; BP counts how many entries we got
    ; entries are stored at physical 0x0500 (buffer)
    ; -------------------------------------------------------
    xor ebx, ebx                ; first call: EBX must be 0
    xor bp, bp                  ; entry counter
    mov edx, 0x534D4150         ; 'SMAP' magic
    mov di, buffer              ; ES:DI = destination

.e820_loop:
    mov eax, 0xE820             ; BIOS trashes EAX each call, reload
    mov ecx, 24                 ; BIOS may trash ECX too, reload
    int 0x15
    jc  .e820_done              ; carry = error or list exhausted
    add di, 24                  ; move to next entry slot
    inc bp                      ; count this entry
    test ebx, ebx               ; EBX=0 means this was the last entry
    jnz .e820_loop

.e820_done:
    mov [mem_entry_count], bp   ; save count so kernel can find it

    mov si, msg_mmap
    call print

    ; -------------------------------------------------------
    ; switch to 32-bit protected mode
    ; 1. interrupts must be off — IDT is not set up for PM yet
    ; 2. load GDTR with our GDT
    ; 3. set CR0 bit 0 (PE) to enter protected mode
    ; 4. far jump to flush prefetch queue and load CS with
    ;    code segment selector 0x08 (GDT entry 1)
    ; -------------------------------------------------------
    
    cli
    lgdt [gdt_descriptor]       ; load GDT register

    mov eax, cr0
    or  eax, 0x1                ; set Protection Enable bit
    mov cr0, eax

    jmp 0x08:protected_mode_entry   ; CS = 0x08, flush pipeline


[BITS 32]

protected_mode_entry:
    ; CS is already set by the far jump above
    ; load all data segments with data selector 0x10 (GDT entry 2)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x0000C000         ; set up 32-bit stack below kernel
    
    jmp 0x10000                 ; jump to kernel entry point


hang:
    cli
    hlt                         ; something went wrong — freeze


[BITS 16]

; -------------------------------------------------------
; print — prints null-terminated string pointed to by SI
; uses BIOS INT 10h AH=0Eh teletype output
; -------------------------------------------------------
print:
    mov ah, 0x0E
.loop:
    lodsb                       ; load byte at DS:SI into AL, advance SI
    cmp al, 0                   ; null terminator?
    je  .done
    int 0x10                    ; print character in AL
    jmp .loop
.done:
    ret


; -------------------------------------------------------
; data
; -------------------------------------------------------
boot_drive      db 0            ; saved boot drive number
mem_entry_count dw 0            ; number of E820 entries found

buffer equ 0x0500               ; E820 entries stored here (free conventional memory)

msg_start   db 'BL: started',     0x0D, 0x0A, 0
msg_lba     db 'BL: LBA ok',      0x0D, 0x0A, 0
msg_loaded  db 'BL: kernel loaded', 0x0D, 0x0A, 0
msg_a20     db 'BL: A20 enabled',  0x0D, 0x0A, 0
msg_mmap    db 'BL: mmap done',    0x0D, 0x0A, 0
msg_pm      db 'BL: entering PM',  0x0D, 0x0A, 0

; -------------------------------------------------------
; DAP — Disk Address Packet for INT 13h extended read
; size=16, reserved=0, sectors=1, dst=0x0000:0x1000 (0x10000), LBA=1
; -------------------------------------------------------
dap:
    db 0x10, 0x00               ; DAP size = 16 bytes, reserved
    dw 100                        ; number of sectors to read
    dw 0x0000                   ; destination offset
    dw 0x1000                   ; destination segment — physical 0x10000
    dq 1                        ; LBA sector number to start reading from

; -------------------------------------------------------
; GDT — flat 32-bit model, two descriptors after null
; code: base=0 limit=4GB executable readable ring0 32-bit
; data: base=0 limit=4GB writable ring0 32-bit
; -------------------------------------------------------
gdt_start:
    dq 0x0000000000000000       ; null descriptor — required
gdt_code:
    dw 0xFFFF, 0x0000
    db 0x00, 0b10011010, 0b11001111, 0x00
gdt_data:
    dw 0xFFFF, 0x0000
    db 0x00, 0b10010010, 0b11001111, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size minus 1 (limit field)
    dd gdt_start                ; linear base address of GDT

times 510 - ($ - $$) db 0      ; pad to 510 bytes
dw 0xAA55                      ; boot signature — BIOS checks this