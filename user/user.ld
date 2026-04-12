ENTRY(user_program)

SECTIONS {
    . = 0x00400000;
    .text  : { *(.text)  }
    .data  : { *(.data)  }
    .rodata: { *(.rodata)}
    .bss   : { *(.bss)   }
}
