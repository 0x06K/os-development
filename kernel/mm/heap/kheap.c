#include <kernel/mm/heap/kheap.h>

// Internal heap tracking variables
static uint32_t heap_start;     // start of the heap
static uint32_t heap_end;       // current end of allocated/mapped heap
static uint32_t heap_max;       // maximum heap address

typedef struct __attribute__((aligned)) heap_header {
    uint32_t magic;              // 0xDEADBEEF for sanity check
    uint8_t  is_free;            // 0 = allocated, 1 = free
    uint32_t size;               // size of the data area
    struct heap_header* next;    // pointer to next block
    struct heap_header* prev;    // pointer to previous block
} heap_header_t;

static void expand(uint32_t new_heap_max) {
    while (heap_end < new_heap_max) {
        vmm_map_page(heap_end, pmm_alloc_frame());
        heap_end += 0x1000;
    }
}

void kheap_init(uint32_t start_addr, uint32_t end_addr) {
    // align start_addr and end_addr
    if (start_addr % PAGE_SIZE != 0) {
        start_addr = ((start_addr / PAGE_SIZE) + 1) * PAGE_SIZE;
    }
    end_addr = (end_addr / PAGE_SIZE) * PAGE_SIZE;

    // initialize the variables
    heap_start = start_addr;
    heap_end   = start_addr;
    heap_max   = end_addr;

    // calculating the number of pages
    uint32_t num_pages = ((end_addr - start_addr) / PAGE_SIZE);

    // allocating the pages for our heap
    vmm_alloc_npage(start_addr, num_pages, PAGE_PRESENT | PAGE_WRITE);

    // update heap_end after allocation
    heap_end = end_addr;

    // create the header for the heap
    heap_header_t* first = (heap_header_t*)heap_start;
    first->magic   = 0xDEADBEEF;
    first->is_free = 1;
    first->size    = heap_max - heap_start - sizeof(heap_header_t);
    first->next    = NULL;
    first->prev    = NULL;

    // resetting memory
    uint32_t* ptr = (uint32_t*)((uint32_t)first + sizeof(heap_header_t));
    uint32_t remaining_bytes = (heap_max - (uint32_t)ptr) / 4;
    for (uint32_t i = 0; i < remaining_bytes; i++) {
        ptr[i] = 0;
    }
}

void* kmalloc(uint32_t size) {
    if (size == 0)
        return NULL;

    heap_header_t* current = (heap_header_t*)heap_start;

    // Search for a free block big enough
    while (current) {
        if (current->magic != 0xDEADBEEF) {
            // Heap corruption
            return NULL;
        }

        if (current->is_free && current->size >= size) {
            // Found space → allocate
            current->is_free = 0;

            // Return usable pointer (after header)
            return (void*)((uint32_t)current + sizeof(heap_header_t));
        }

        // Move to next block
        current = current->next;
    }

    // Out of memory in heap range
    return NULL;
}
#include <kernel/mm/heap/kheap.h>

// Internal heap tracking variables
static uint32_t heap_start;     // start of the heap
static uint32_t heap_end;       // current end of allocated/mapped heap
static uint32_t heap_max;       // maximum heap address

typedef struct heap_header {
    uint32_t magic;              // 0xDEADBEEF for sanity check
    uint8_t  is_free;            // 0 = allocated, 1 = free
    uint32_t size;               // size of the data area
    struct heap_header* next;    // pointer to next block
    struct heap_header* prev;    // pointer to previous block
} heap_header_t;

static void expand(uint32_t new_heap_max) {
    while (heap_end < new_heap_max) {
        vmm_map_page(heap_end, pmm_alloc_frame());
        heap_end += 0x1000;
    }
}

void kheap_init(uint32_t start_addr, uint32_t end_addr) {
    // align start_addr and end_addr
    if (start_addr % PAGE_SIZE != 0) {
        start_addr = ((start_addr / PAGE_SIZE) + 1) * PAGE_SIZE;
    }
    end_addr = (end_addr / PAGE_SIZE) * PAGE_SIZE;

    // initialize the variables
    heap_start = start_addr;
    heap_end   = start_addr;
    heap_max   = end_addr;

    // calculating the number of pages
    uint32_t num_pages = ((end_addr - start_addr) / PAGE_SIZE);

    // allocating the pages for our heap
    vmm_alloc_npage(start_addr, num_pages, PAGE_PRESENT | PAGE_WRITE);

    // update heap_end after allocation
    heap_end = end_addr;

    // create the header for the heap
    heap_header_t* first = (heap_header_t*)heap_start;
    first->magic   = 0xDEADBEEF;
    first->is_free = 1;
    first->size    = heap_max - heap_start - sizeof(heap_header_t);
    first->next    = NULL;
    first->prev    = NULL;

    // resetting memory
    uint32_t* ptr = (uint32_t*)((uint32_t)first + sizeof(heap_header_t));
    uint32_t remaining_bytes = (heap_max - (uint32_t)ptr) / 4;
    for (uint32_t i = 0; i < remaining_bytes; i++) {
        ptr[i] = 0;
    }
}

void* kmalloc(uint32_t size) {
    if (size == 0)
        return NULL;

    heap_header_t* current = (heap_header_t*)heap_start;

    // Search for a free block big enough
    while (current) {
        if (current->magic != 0xDEADBEEF) {
            // Heap corruption
            return NULL;
        }

        if (current->is_free && current->size >= size) {
            // Found space → allocate
            current->is_free = 0;

            // Return usable pointer (after header)
            return (void*)((uint32_t)current + sizeof(heap_header_t));
        }

        // Move to next block
        current = current->next;
    }

    // Out of memory in heap range
    return NULL;
}

void* kmalloc_a(uint32_t size) {
    if (size == 0)
        return NULL;

    heap_header_t* current = (heap_header_t*)heap_start;

    // Search for a free block big enough with alignment
    while (current) {
        if (current->magic != 0xDEADBEEF) {
            return NULL;
        }

        if (current->is_free && current->size >= size) {
            // Calculate aligned address (after header)
            uint32_t addr = (uint32_t)current + sizeof(heap_header_t);
            uint32_t aligned_addr = addr;
            
            // Align to page boundary if needed
            if (aligned_addr % PAGE_SIZE != 0) {
                aligned_addr = ((aligned_addr / PAGE_SIZE) + 1) * PAGE_SIZE;
            }
            
            uint32_t alignment_offset = aligned_addr - addr;
            
            // Check if block is still large enough after alignment
            if (current->size >= size + alignment_offset) {
                current->is_free = 0;
                return (void*)aligned_addr;
            }
        }

        current = current->next;
    }

    return NULL;
}

void* kmalloc_p(uint32_t size, uint32_t* phys) {
    void* addr = kmalloc(size);
    
    if (addr && phys) {
        // Get physical address from virtual address
        *phys = vmm_get_physical_address((uint32_t)addr);
    }
    
    return addr;
}

void* kmalloc_ap(uint32_t size, uint32_t* phys) {
    void* addr = kmalloc_a(size);
    
    if (addr && phys) {
        // Get physical address from virtual address
        *phys = vmm_get_physical_address((uint32_t)addr);
    }
    
    return addr;
}

void kfree(void* ptr) {
    if (!ptr)
        return;

    // Get the header from the pointer
    heap_header_t* block = (heap_header_t*)((uint32_t)ptr - sizeof(heap_header_t));

    // Verify block integrity
    if (block->magic != 0xDEADBEEF) {
        // Invalid pointer or heap corruption
        return;
    }

    if (block->is_free) {
        // Double free detected
        return;
    }

    // Mark block as free
    block->is_free = 1;

    // Merge with next block if it's free
    if (block->next && block->next->is_free) {
        block->size += sizeof(heap_header_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    // Merge with previous block if it's free
    if (block->prev && block->prev->is_free) {
        block->prev->size += sizeof(heap_header_t) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}