global isr0
extern isr0_handler_c
extern run_scheduler
extern context_switch

isr0:
    cli

    ; save general registers + flags
    push ebp
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    pushfd

    call isr0_handler_c

    sub esp, 8              ; reserve locals: [esp] = old_ptr, [esp+4] = new_esp_val

    lea eax, [esp]          ; &old_ptr local
    lea ebx, [esp+4]        ; &new_esp_val local
    push ebx
    push eax
    call run_scheduler
    add esp, 8

    ; push args to context_switch: old_ptr, new_esp
    mov eax, [esp]          ; eax = old_ptr
    mov edx, [esp+4]        ; edx = new_esp
    push edx
    push eax
    call context_switch
    add esp, 8

    add esp, 8              ; remove sub esp,8 locals

    popfd
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ebp

    sti
    iretd

; --------------------------------------------------
; context_switch(old_ptr, new_esp)
; old_ptr = pointer to previous task->stack_top
; new_esp = new task stack_top
