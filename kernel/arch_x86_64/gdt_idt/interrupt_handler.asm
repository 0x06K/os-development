; idt.asm - IDT Assembly Implementation
; NASM syntax for 32-bit x86

[BITS 32]

; External C function
extern isr_handler

; Global symbols for C code
global idt_flush
global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

; ===== IDT Flush Function =====
; Load IDT with LIDT instruction
; void idt_flush(uint32_t idt_ptr)
idt_flush:
    mov eax, [esp + 4]      ; Get pointer to IDT pointer structure
    lidt [eax]              ; Load IDT
    ret

; ===== ISR Macros =====
; Macro for ISRs without error codes
%macro ISR_NOERRORCODE 1
isr%1:
    cli                     ; Disable interrupts
    push byte 0             ; Push dummy error code
    push byte %1            ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Macro for ISRs with error codes (CPU pushes error code automatically)
%macro ISR_ERRORCODE 1
isr%1:
    cli                     ; Disable interrupts
    push byte %1            ; Push interrupt number
    jmp isr_common_stub
%endmacro

; ===== ISR Definitions =====
; CPU Exceptions 0-31

; No error code exceptions
ISR_NOERRORCODE 0   ; Division by zero
ISR_NOERRORCODE 1   ; Debug
ISR_NOERRORCODE 2   ; Non-maskable interrupt
ISR_NOERRORCODE 3   ; Breakpoint
ISR_NOERRORCODE 4   ; Overflow
ISR_NOERRORCODE 5   ; Bound range exceeded
ISR_NOERRORCODE 6   ; Invalid opcode
ISR_NOERRORCODE 7   ; Device not available

; Error code exceptions
ISR_ERRORCODE   8   ; Double fault
ISR_NOERRORCODE 9   ; Coprocessor segment overrun (legacy)
ISR_ERRORCODE   10  ; Invalid TSS
ISR_ERRORCODE   11  ; Segment not present
ISR_ERRORCODE   12  ; Stack-segment fault
ISR_ERRORCODE   13  ; General protection fault
ISR_ERRORCODE   14  ; Page fault

; No error code exceptions
ISR_NOERRORCODE 15  ; Reserved
ISR_NOERRORCODE 16  ; x87 floating-point exception
ISR_ERRORCODE   17  ; Alignment check
ISR_NOERRORCODE 18  ; Machine check
ISR_NOERRORCODE 19  ; SIMD floating-point exception
ISR_NOERRORCODE 20  ; Virtualization exception
ISR_ERRORCODE   21  ; Control protection exception

; Reserved exceptions (22-31)
ISR_NOERRORCODE 22
ISR_NOERRORCODE 23
ISR_NOERRORCODE 24
ISR_NOERRORCODE 25
ISR_NOERRORCODE 26
ISR_NOERRORCODE 27
ISR_NOERRORCODE 28
ISR_NOERRORCODE 29
ISR_ERRORCODE   30  ; Security exception
ISR_NOERRORCODE 31

; ===== Common ISR Handler =====
; This stub is called by all ISR stubs
; It saves processor state and calls the C handler
isr_common_stub:
    ; Save all general-purpose registers
    pusha                   ; Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    ; Save segment registers
    mov ax, ds
    push eax                ; Save data segment
    
    ; Load kernel data segment
    mov ax, 0x10            ; Kernel data segment selector
    mov ds, ax
    mov es, axs
    mov fs, ax
    mov gs, ax
    
    ; Call C handler
    ; Stack now contains: [interrupt_number] [error_code] [saved_registers...]
    ; We need to pass interrupt_number and error_code as parameters
    push esp                ; Pass pointer to stack (for accessing all registers if needed)
    call isr_handler        ; Call C function: isr_handler(stack_ptr)
    add esp, 4              ; Clean up stack (remove stack pointer parameter)
    
    ; Restore segment registers
    pop eax                 ; Get original data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore general-purpose registers
    popa                    ; Pop EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    
    ; Clean up error code and interrupt number from stack
    add esp, 8              ; Remove error_code and interrupt_number
    
    ; Enable interrupts and return
    sti                     ; Re-enable interrupts
    iret                    ; Return from interrupt