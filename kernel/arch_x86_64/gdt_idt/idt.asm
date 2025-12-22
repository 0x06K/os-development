extern idt_ptr

lidt [idt_ptr]  ; Load IDT
sti             ; Enable interrupts after IDT is loaded
