#include <stdint.h>
#include "idt.h"

// Exception messages for debugging
static const char* exception_messages[32] = {
    "Division by zero",                 // 0
    "Debug",                           // 1
    "Non-maskable interrupt",          // 2
    "Breakpoint",                      // 3
    "Overflow",                        // 4
    "Bound range exceeded",            // 5
    "Invalid opcode",                  // 6
    "Device not available",            // 7
    "Double fault",                    // 8
    "Coprocessor segment overrun",     // 9
    "Invalid TSS",                     // 10
    "Segment not present",             // 11
    "Stack-segment fault",             // 12
    "General protection fault",        // 13
    "Page fault",                      // 14
    "Reserved",                        // 15
    "x87 floating-point exception",    // 16
    "Alignment check",                 // 17
    "Machine check",                   // 18
    "SIMD floating-point exception",   // 19
    "Virtualization exception",        // 20
    "Control protection exception",    // 21
    "Reserved",                        // 22
    "Reserved",                        // 23
    "Reserved",                        // 24
    "Reserved",                        // 25
    "Reserved",                        // 26
    "Reserved",                        // 27
    "Reserved",                        // 28
    "Reserved",                        // 29
    "Security exception",              // 30
    "Reserved"                         // 31
};

// Structure representing the stack frame created by our ISR stub
typedef struct {
    // Pushed by isr_common_stub
    uint32_t ds;                        // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;         // Interrupt number and error code
    // Pushed automatically by CPU
    uint32_t eip, cs, eflags, useresp, ss; // Only if privilege change occurred
} registers_t;

// Forward declaration for screen output (you'll need to implement these)
extern void screen_write(const char* str);
extern void screen_write_hex(uint32_t value);
extern void screen_write_dec(uint32_t value);

// Main ISR handler called from assembly
void isr_handler(registers_t* regs)
{
    // Handle the interrupt based on interrupt number
    if (regs->int_no < 32) {
        // CPU Exception
        screen_write("CPU Exception: ");
        screen_write(exception_messages[regs->int_no]);
        screen_write("\n");
        
        // Print additional debug information
        screen_write("Interrupt: ");
        screen_write_dec(regs->int_no);
        screen_write(", Error Code: ");
        screen_write_hex(regs->err_code);
        screen_write("\n");
        
        screen_write("EIP: ");
        screen_write_hex(regs->eip);
        screen_write(", CS: ");
        screen_write_hex(regs->cs);
        screen_write(", EFLAGS: ");
        screen_write_hex(regs->eflags);
        screen_write("\n");
        
        screen_write("EAX: ");
        screen_write_hex(regs->eax);
        screen_write(", EBX: ");
        screen_write_hex(regs->ebx);
        screen_write(", ECX: ");
        screen_write_hex(regs->ecx);
        screen_write(", EDX: ");
        screen_write_hex(regs->edx);
        screen_write("\n");
        
        screen_write("ESP: ");
        screen_write_hex(regs->esp);
        screen_write(", EBP: ");
        screen_write_hex(regs->ebp);
        screen_write(", ESI: ");
        screen_write_hex(regs->esi);
        screen_write(", EDI: ");
        screen_write_hex(regs->edi);
        screen_write("\n");
        
        // For page faults, print the faulting address (stored in CR2)
        if (regs->int_no == 14) {
            uint32_t faulting_address;
            asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
            screen_write("Page fault at address: ");
            screen_write_hex(faulting_address);
            screen_write("\n");
            
            // Decode page fault error code
            screen_write("Error details: ");
            if (!(regs->err_code & 0x1)) screen_write("Page not present ");
            if (regs->err_code & 0x2) screen_write("Write operation ");
            else screen_write("Read operation ");
            if (regs->err_code & 0x4) screen_write("User mode ");
            else screen_write("Kernel mode ");
            if (regs->err_code & 0x8) screen_write("Reserved bit set ");
            if (regs->err_code & 0x10) screen_write("Instruction fetch ");
            screen_write("\n");
        }
        
        // Halt the system for unhandled exceptions
        screen_write("System halted due to unhandled exception.\n");
        asm volatile("cli; hlt"); // Disable interrupts and halt
    }
}

// Helper function to handle specific exceptions if needed
void handle_page_fault(registers_t* regs)
{
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    
    // Your page fault handling logic here
    // This could involve:
    // - Checking if the fault is in valid memory
    // - Loading pages from swap
    // - Growing the stack
    // - Handling copy-on-write
    
    screen_write("Page fault handler called\n");
}

// Function to register custom exception handlers
typedef void (*isr_handler_func_t)(registers_t* regs);
static isr_handler_func_t custom_handlers[32] = {0};

void register_exception_handler(uint8_t exception, isr_handler_func_t handler)
{
    if (exception < 32) {
        custom_handlers[exception] = handler;
    }
}

// Enhanced ISR handler with custom handler support
void isr_handler_enhanced(registers_t* regs)
{
    if (regs->int_no < 32) {
        // Check if there's a custom handler registered
        if (custom_handlers[regs->int_no] != 0) {
            custom_handlers[regs->int_no](regs);
            return;
        }
        
        // Fall back to default handler
        isr_handler(regs);
    }
}