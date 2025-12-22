#ifndef KERNEL_MM_PAGING_H
#define KERNEL_MM_PAGING_H

#include <stdint.h>

/*
 * Initialize paging:
 *  - kernel_phys_start..kernel_phys_end : identity map this physical range into the page tables
 *  - creates kernel_page_directory and enables paging
 *
 * NOTE: pass physical addresses (e.g. provided by linker map/loader).
 */
void paging_init(uint32_t kernel_phys_start, uint32_t kernel_phys_end);

/* switch CR3 to an already-created page directory (physical addr) */
void paging_switch_directory(uint32_t phys_pd);

/* load CR3 with current kernel directory (helper) */
uint32_t paging_get_current_cr3(void);

/* simple page fault handler stub (hook into your ISR) */
void paging_handle_fault(uint32_t err_code, uint32_t fault_addr);

#endif /* KERNEL_MM_PAGING_H */
