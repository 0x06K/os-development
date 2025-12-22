#include "scheduler.h"
#include "vga.h"
#include "kheap.h"
#include <stdint.h>

task_t *ready_queue_head = NULL;
task_t *ready_queue_tail = NULL;
task_t *current_task = NULL;
uint32_t task_switch_count = 0;


// Disable/Enable interrupts
static inline void cli(void) { __asm__ volatile("cli"); }
static inline void sti(void) { __asm__ volatile("sti"); }

// Queue functions
void enqueue(task_t *task) {
    task->next = NULL;
    if (!ready_queue_tail) {
        ready_queue_head = ready_queue_tail = task;
    } else {
        ready_queue_tail->next = task;
        ready_queue_tail = task;
        vga_writestring("i am here.");
    }
}

task_t* dequeue(void) {
    if (!ready_queue_head) return NULL;
    task_t *task = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    if (!ready_queue_head) ready_queue_tail = NULL;
    task->next = NULL;
    return task;
}

// Task exit handler
void task_exit(void) {
    vga_writestring("\nTask exited - hanging\n");
    while (1) { __asm__ volatile("hlt"); }
}

// Task creation
unsigned int next_task_id = 1;

task_t* create_task(void (*task_func)(void)) {
    cli();

    task_t* task = (task_t*)kmalloc(sizeof(task_t));
    if (!task) { sti(); return NULL; }

    task->stack_base = (uint32_t*)kmalloc(STACK_SIZE);
    if (!task->stack_base) { kfree(task); sti(); return NULL; }

    task->id = next_task_id++;
    task->next = NULL;

    // Setup stack (grows downward)
    uint32_t *stack = (uint32_t*)((uint32_t)task->stack_base + STACK_SIZE);

    // Reserve space for registers to match ISR pop order: EBP,EAX,EBX,ECX,EDX,ESI,EDI,EFLAGS
    stack -= 8;         // push 8 dummy values (for popfd/pop regs)
    stack[0] = (uint32_t)task_exit;       // dummy EBP
    stack[1] = 0;       // EAX
    stack[2] = 0;       // EBX
    stack[3] = 0;       // ECX
    stack[4] = 0;       // EDX
    stack[5] = 0;       // ESI
    stack[6] = 0;       // EDI
    stack[7] = 0x200;   // EFLAGS (IF=1)

    // Push initial EIP (task function)
    stack--;
    *stack = (uint32_t)task_func;

    task->stack_top = stack;

    enqueue(task);
    sti();
    return task;
}

void run_scheduler(uint32_t* out_old_stack_ptr, uint32_t* out_new_stack_val) {
    task_t *prev_task = current_task;

    static uint32_t last_switch = 0;
    if (current_task && (system_ticks - last_switch) < 1)
        return;
    last_switch = system_ticks;

    task_t *next_task = dequeue();

    if (!next_task) {
        if (prev_task) {
            *out_old_stack_ptr = (uint32_t)&prev_task->stack_top;
            *out_new_stack_val = prev_task->stack_top;
            enqueue(prev_task);
        }
        return;
    }

    if (prev_task)
        enqueue(prev_task);

    current_task = next_task;

    *out_old_stack_ptr = prev_task ? (uint32_t)&prev_task->stack_top : 0;
    *out_new_stack_val = current_task->stack_top;
}


// Start the scheduler
extern void context_switch(uint32_t *old_ptr, uint32_t new_esp);




void scheduler_init(void) {
    cli();
    ready_queue_head = ready_queue_tail = current_task = NULL;
    task_switch_count = 0;

    vga_writestring("Scheduler initialized\n");
    sti();
}

void scheduler_start(void) {
    cli();
    if (!ready_queue_head) {
        vga_writestring("No tasks to run!\n");
        sti();
        return;
    }

    current_task = dequeue();
    context_switch(NULL, current_task->stack_top);
    
    vga_writestring("Returned from first task!?\n");
}
