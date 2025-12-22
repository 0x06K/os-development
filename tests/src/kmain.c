#include "vga.h"
#include "frame.h"
#include "page_table.h"
#include "gdt.h"
#include "pic.h"
#include "idt.h"
#include "scheduler.h"
#include "kheap.h"
#include "myshell.h"

extern void idt_init();
void kmain(void);
extern void func1();
extern void func2();
extern void scheduler_start();
void kmain(void) {
    vga_initialize();
    gdt_init();
    vga_writestring("gdt_init done.\n");
    frame_init();
    vga_writestring("frame_init done.\n");
    pd_init();
    vga_writestring("pd_init done.\n");
    idt_init();
    kheap_init(0xC0F00000, 0xC0300000);
    vga_writestring("kheap_init done.\n");
    asm volatile("sti");
    scheduler_init();
    // start_scheduler();
    create_task(shell_init);
    scheduler_start();
    while(1);

}
