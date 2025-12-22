#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define STACK_SIZE 8192

typedef struct task_t {
    uint32_t id;
    uint32_t *stack_base;
    uint32_t stack_top;   // ESP VALUE
    struct task_t *next;
} task_t;


extern volatile uint32_t system_ticks;

void scheduler_init(void);
void scheduler_start(void);
void run_scheduler(uint32_t *out_old_stack_ptr, uint32_t *out_new_stack_val);
task_t* create_task(void (*task_func)(void));
void task_exit(void);

#endif
