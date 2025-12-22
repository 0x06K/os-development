global context_switch
; context_switch(uint32_t *old_esp_store, uint32_t new_esp)

context_switch:
    mov eax, [esp + 4]    ; old_esp_store
    mov edx, [esp + 8]    ; new_esp

    test eax, eax
    jz .load_new

    mov [eax], esp        ; save ESP value

.load_new:
    mov esp, edx          ; DIRECT load
    ret
