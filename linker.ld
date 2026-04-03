ENTRY(kernel_entry)
SECTIONS {
    . = 0x10000;
    .text :
    {
        *(.text.main)
        *(.text*)
    }
}
