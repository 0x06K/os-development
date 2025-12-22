#ifndef CONFIG_H
#define CONFIG_H

#define     FRAME_SIZE                  0x1000
#define     PAGE_SIZE                   0x1000
#define     MEM_SIZE                    0xFFFFFFFF

#define     KERNEL_START                0x00100000
#define     KERNEL_END                  0x00200000
#define     KERNEL_PHYS_BASE            0x00010000
#define     KERNEL_VIRTUAL_BASE         0xC0010000

#define     STACK_PAGES                 0x100
#define     KERNEL_STACK_SIZE           0x100000
#define     KERNEL_STACK_BOTTOM         0x00F00000
#define     KERNEL_STACK_TOP            0x00FFFFFF    // (KERNEL_STACK_BOTTOM + KERNEL_STACK_SIZE)

#define     KERNEL_STACK_VIRTUAL_BASE   0xC0F00000
#define     KERNEL_STACK_VIRTUAL_TOP    0xC0FFFFFF  // (KERNEL_STACK_VIRTUAL_BASE + KERNEL_STACK_SIZE)

#define     KHEAP_PHYS_BASE             0x00300000
#define     KHEAP_VIRTUAL_BASE          0x0C030000
#define     KHEAP_INITIAL_SIZE          0x00C00000  // 12MB
#define     KHEAP_MAX                   0xCEFFFFFF

#define     TOTAL_FRAMES                0x100000    // ((MEM_SIZE + 1) / FRAME_SIZE)
#define     BITMAP_SIZE                 0x8000      // (TOTAL_FRAMES + 31) / 32
#define     BITMAP_PHYS_BASE            0x2F8000    // we are placing it at the end of 2MB
#define     BITMAP_VIRTUAL_BASE         0xC02F8000

#define     PAGE_PRESENT                0x1
#define     PAGE_RW                     0x2
#define     PAGE_USER                   0x4

#define PIC1        0x20
#define PIC2        0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA   (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA   (PIC2 + 1)

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01


#endif
