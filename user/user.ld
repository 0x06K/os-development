ENTRY(user_program)

SECTIONS {
    . = 0x40000000;
    .text  : { *(.text)  }
    .data  : { *(.data)  }
    .bss   : { *(.bss)   }
}
