#pragma GCC target("general-regs-only")
#include <filesystem/filesystem.h>
#include <keyboard/keyboard.h>
#include <vga/vga.h>
#include "handlers.h"

extern void keyboard_push(uint8_t scancode);


#define PIC1_COMMAND  0x20   // master PIC
#define PIC2_COMMAND  0xA0   // slave PIC
#define PIC_EOI       0x20   // EOI command byte


uint8_t inb(uint16_t port)
{
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);  // must also ack slave PIC
    outb(PIC1_COMMAND, PIC_EOI);      // always ack master PIC
}


__attribute__((interrupt)) 
void keyboard_handler(interrupt_frame *frame) {
    uint8_t scancode = inb(0x60);
    send_eoi(1);                   // EOI first — re-enables PIC line
    keyboard_push(scancode);       // then process
}
__attribute__((interrupt))
void timer_handler(interrupt_frame *frame)
{
    (void)frame;
    outb(0x20, 0x20);
}



// table of kernel functions, index = syscall number
typedef int (*syscall_fn)(uint32_t, uint32_t, uint32_t);
static syscall_fn syscall_table[] = {
    [0]  = (syscall_fn)kprintf,
    [1]  = (syscall_fn)kgets,
    [2]  = (syscall_fn)fs_cat,
    [3]  = (syscall_fn)fs_create,
    [4]  = (syscall_fn)fs_unlink,
    [5]  = (syscall_fn)fs_rename,
    [6]  = (syscall_fn)fs_cp,
    [7]  = (syscall_fn)ls_dir,
    [8]  = (syscall_fn)fs_mkdir,
    [9]  = (syscall_fn)fs_mkdir_p,
    [10] = (syscall_fn)fs_rmdir,
    [11] = (syscall_fn)fs_rmdir_r,
    [12] = (syscall_fn)fs_chdir,
    [13] = (syscall_fn)fs_getcwd,
    [14] = (syscall_fn)fs_stat,
    [15] = (syscall_fn)fs_find,
    [16] = (syscall_fn)fs_df,
    [17] = (syscall_fn)fs_du,
    [18] = (syscall_fn)fs_sync,
    [19] = (syscall_fn)fs_echo,
    [20] = (syscall_fn)clear
};

void syscall_handler(registers_t *regs) {
    uint32_t num = regs->eax;  // get syscall number

    // bounds check + null check
    if (num < sizeof(syscall_table)/sizeof(syscall_table[0]) && syscall_table[num]) {
        // call the function, pass EBX, ECX, EDX as args
        regs->eax = syscall_table[num](regs->ebx, regs->ecx, regs->edx);
    } else {
        regs->eax = -1;  // unknown syscall
    }
}

__attribute__((naked))
void syscall_stub(void) {
    asm volatile(
        "pusha              \n"
        "push %esp         \n"
        "call syscall_handler\n"
        "add $4, %esp       \n"
        "popa               \n"
        "iret               \n"
    );
}

extern volatile uint8_t irq_fired;
__attribute__((interrupt))
void irq14_handler(interrupt_frame* frame) {
    outb(0x20, 0x20);   // EOI to master PIC
    outb(0xA0, 0x20);   // EOI to slave PIC    
    (void)frame;
    irq_fired = 1;
}