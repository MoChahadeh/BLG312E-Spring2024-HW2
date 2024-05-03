#include "mymem.h"

// Function to round up the size to the nearest multiple of page size
size_t roundUpToPageSize(size_t size) {
    return (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

// Function to initialize memory allocation system
int InitMyMalloc(int HeapSize) {
    if (heap_start != NULL || HeapSize <= 0) {
        return -1;
    }

    heap_size = roundUpToPageSize(HeapSize + sizeof(BlockHeader));
    heap_start = mmap(NULL, heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (heap_start == MAP_FAILED) {
        return -1;
    }

    block_list = (BlockHeader *)heap_start;
    block_list->size = heap_size - sizeof(BlockHeader);
    block_list->next = NULL;
    block_list->is_free = true;

    return 0;
}

// Function to allocate memory
void *MyMalloc(int size, int strategy) {
    if (size <= 0 || heap_start == NULL) {
        return NULL;
    }

    BlockHeader *prev_block = NULL;
    BlockHeader *curr_block = block_list;
    BlockHeader *best_fit = NULL;

    if (strategy == 3 && last_choice != NULL && last_choice->next != NULL) {
        curr_block = last_choice->next;
    }

    bool looped = false;

    // Find the best fit block
    while (curr_block != NULL) {
        if (curr_block->is_free && curr_block->size >= size) {
            if (strategy == 0) { // Best Fit
                if (best_fit == NULL || curr_block->size < best_fit->size) {
                    best_fit = curr_block;
                }
            } else if (strategy == 1) { // Worst Fit
                if (best_fit == NULL || curr_block->size > best_fit->size) {
                    best_fit = curr_block;
                }
            } else if (strategy == 2 || strategy == 3) { // First Fit or Next Fit
                best_fit = curr_block;
                break;
            } else {
                return NULL;
            }
        }
        prev_block = curr_block;
        curr_block = curr_block->next;

        if(strategy == 3 && curr_block == NULL && !looped) {
            curr_block = block_list;
            looped = true;
        }

    }

    // If no suitable block found, return NULL
    if (best_fit == NULL) {
        return NULL;
    }



    // split the block if there is space left
    if (best_fit->size > size) {
        BlockHeader *new_block = (BlockHeader *)((char *)best_fit + sizeof(BlockHeader) + size);
        new_block->size = best_fit->size - size - sizeof(BlockHeader);
        new_block->next = best_fit->next;
        new_block->is_free = true;
        last_choice = new_block;
        best_fit->size = size;
        best_fit->next = new_block;
        best_fit->is_free = false;
    } else {
        best_fit->is_free = false;
        last_choice = best_fit;
    }




    return (void *)((char *)best_fit + sizeof(BlockHeader));
}

// Function to free allocated memory
int MyFree(void *ptr) {
    if (ptr == NULL || heap_start == NULL) {
        return -1;
    }

    BlockHeader *block_to_free = (BlockHeader *)((char *)ptr - sizeof(BlockHeader));

    // Mark the block as free
    block_to_free->is_free = true;

    // Merge adjacent free blocks
    BlockHeader *curr_block = block_list;
    while (curr_block != NULL) {

        if ((curr_block->is_free && curr_block->next != NULL && curr_block->next->is_free) && (char *)curr_block + curr_block->size + sizeof(BlockHeader) == (char *)curr_block->next) {
            curr_block->size += curr_block->next->size + sizeof(BlockHeader);
            if(last_choice == curr_block->next) {
                last_choice = curr_block;
            }
            curr_block->next = curr_block->next->next;
        } else {
            curr_block = curr_block->next;
        }
    }


    return 0;
}

// Function to print the free list for debugging
void DumpFreeList() {
    BlockHeader *curr_block = block_list;
    printf("Addr\tSize\tStatus\n");
    while (curr_block != NULL) {
        printf("%p\t%lu\t%s\n", curr_block, curr_block->size, curr_block->is_free ? "Free" : "Allocated");
        curr_block = curr_block->next;
    }

    printf("\n\n");
}
