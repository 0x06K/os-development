/* linker.ld */

ENTRY(kmain)

SECTIONS
{
    /* Kernel loaded at 1MB physical address */
    . = 0x0010000;

    .text : {
        */kmain.o(.text)     /* kernel.o's .text first */
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }
}
