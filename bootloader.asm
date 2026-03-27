[org 0x7C00]

hang:
    jmp hang

times 510 - ($ - $$) db 0
dw 0xAA55
