global load_idt

; expecting the address of idt on stack
load_idt:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8] ; Load the address of the idt_ptr structure into EAX
    lidt [eax]       ; Load the IDT register using the address in EAX
    pop ebp
    ret
