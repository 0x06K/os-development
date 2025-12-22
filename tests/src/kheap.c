#include "kheap.h"

typedef struct heap_header_t {
    uint8_t  is_free;
    uint32_t sizeofblock;
    uint32_t magic_number;

    struct heap_header_t* next;  // must use 'struct' here
    struct heap_header_t* prev;
} __attribute__((aligned(8))) heap_header, *heap_header_p;


heap_header_p first_heap_header;

void kheap_init(uint32_t start, uint32_t end){
  first_heap_header = (heap_header_p)start;
  first_heap_header->is_free = 1;
  first_heap_header->sizeofblock = ((uint32_t)end - (uint32_t)start) - sizeof(heap_header);
  first_heap_header->magic_number = 0xABCD1234;
  first_heap_header->next = NULL;
  first_heap_header->prev = NULL;
}

void* kmalloc(uint32_t size){
  if(size == 0) return NULL;
  heap_header_p header = first_heap_header;
  uint32_t total_required_size = size + sizeof(heap_header);

  // 1. Search for a suitable FREE block (First-Fit)
  while(header) {
    // Check if block is free AND big enough (including space for a potential new header)
    if (header->is_free && header->sizeofblock >= total_required_size) {
      break; // Found a block!
    }
    header = header->next;
  }

  // 2. If no free block is found, return NULL (out of memory).
  if (header == NULL) {
    return NULL;
  }

  // 3. SPLIT the block (create a new header for the remaining free space)
  if (header->sizeofblock >= total_required_size) {

    // Calculate the address of the NEW header (next block)
    // Address of current header + size of current header + size of requested data block
    heap_header_p new_header = (heap_header_p)((uint32_t)header + sizeof(heap_header) + size);

    // Initialize the NEW header
    new_header->is_free = 1; // Remaining space is free
    new_header->sizeofblock = header->sizeofblock - total_required_size; // Remaining size
    new_header->magic_number = 0xABCD1234;

    // Link the list: New header goes between the current and the current's next
    new_header->next = header->next;
    new_header->prev = header;

    if (new_header->next != NULL) {
      new_header->next->prev = new_header; // Update the old 'next' block's prev pointer
    }

    // Update the CURRENT (allocated) header
    header->sizeofblock = size;
    header->next = new_header;
  }

  // 4. Mark the current block as allocated
  header->is_free = 0;

  // 5. Return the address of the data region (after the header).
  return (void*)((uint32_t)header + sizeof(heap_header));
}

void* kmalloc_a(uint32_t size){
  //  later will implement these functions right now i don't need these.
  size = 0;
  if(size) 
  return NULL;
  return NULL;
}

void* kmalloc_p(uint32_t size){
  void* virt = kmalloc(size);
  if(virt == NULL) return NULL;
  return (void*)(page_get_phys((uint32_t)virt));
}

void* kmalloc_ap(uint32_t size){
  void* virt = kmalloc_a(size);
  if(virt == NULL) return NULL;
  return (void*)(page_get_phys((uint32_t)virt));
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    heap_header_p header_ptr = (heap_header_p)ptr - 1;
    if (header_ptr->magic_number != 0xABCD1234) {
        // This is a critical error (double-free or heap corruption)
        return;
    }
    // Checking for double free
    if (header_ptr->is_free == 1) {
        return;
    }
    header_ptr->is_free = 1;

    // --- Coalescing (Merging Adjacent Free Blocks) ---

    // Forward Coalescing (Check the NEXT block)
    heap_header_p next_block = header_ptr->next;

    if (next_block != NULL && next_block->is_free == 1) {
        // A. Merge block sizes: Add the size of the *next* block,
        // including its header, to the current block's size.
        // The total size of the block being absorbed is its user data + its header size.
        header_ptr->sizeofblock += next_block->sizeofblock + sizeof(struct heap_header_t);

        // B. Update list pointers to bypass the now-merged 'next_block'
        header_ptr->next = next_block->next;

        // C. Update the 'prev' pointer of the block *after* the absorbed one
        if (next_block->next != NULL) {
            next_block->next->prev = header_ptr;
        }
    }

    // Backward Coalescing (Check the PREVIOUS block)
    heap_header_p prev_block = header_ptr->prev;

    if (prev_block != NULL && prev_block->is_free == 1) {
        // A. Merge block sizes: Add the size of the *current* block,
        // including its header, to the previous block's size.
        // The block being absorbed is 'header_ptr'.
        prev_block->sizeofblock += header_ptr->sizeofblock + sizeof(struct heap_header_t);

        // B. Update list pointers: The 'prev_block' now links directly to
        // the block *after* the current one.
        prev_block->next = header_ptr->next;

        // C. Update the 'prev' pointer of the block *after* the absorbed one
        if (header_ptr->next != NULL) {
            header_ptr->next->prev = prev_block;
        }

        // The memory represented by 'header_ptr' is now conceptually gone,
        // and the free operation concludes with 'prev_block' as the head of the new large block.
    }
}
