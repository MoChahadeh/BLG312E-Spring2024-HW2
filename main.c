#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>

#define PAGE_SIZE getpagesize()

// Structure for the memory block header
typedef struct block_header {
    size_t size;
    struct block_header *next;
    bool is_free;
} BlockHeader;

// Global variables
BlockHeader *block_list = NULL;
void *heap_start = NULL;
size_t heap_size = 0;

// Function prototypes
int InitMyMalloc(int HeapSize);
void *MyMalloc(int size, int strategy);
int MyFree(void *ptr);
void DumpFreeList();

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

    // Find the best fit block
    while (curr_block != NULL) {
        if (curr_block->is_free && curr_block->size >= size) {
            // printf("Here 1\n");
            if (strategy == 0) { // Best Fit
                if (best_fit == NULL || curr_block->size < best_fit->size) {
                    best_fit = curr_block;
                }
            } else if (strategy == 1) { // Worst Fit
                if (best_fit == NULL || curr_block->size > best_fit->size) {
                    best_fit = curr_block;
                }
            } else if (strategy == 2) { // First Fit
                best_fit = curr_block;
                break;
            } else if (strategy == 3) { // Next Fit
                if (curr_block >= block_list && (best_fit == NULL || curr_block->size < best_fit->size)) {
                    best_fit = curr_block;
                }
            }
        }
        prev_block = curr_block;
        curr_block = curr_block->next;
        // printf("Here 2\n");
    }

    // If no suitable block found, return NULL
    if (best_fit == NULL) {
        return NULL;
    }

    // printf("Here 3\n");

    // split the block if there is space left
    if (best_fit->size > size) {
        BlockHeader *new_block = (BlockHeader *)((char *)best_fit + sizeof(BlockHeader) + size);
        new_block->size = best_fit->size - size - sizeof(BlockHeader);
        new_block->next = best_fit->next;
        new_block->is_free = true;
        best_fit->size = size;
        best_fit->next = new_block;
        best_fit->is_free = false;
    }

    // printf("Here 4\n");

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
        printf("Here 6\n");
        if ((curr_block->is_free && curr_block->next != NULL && curr_block->next->is_free) && (char *)curr_block + curr_block->size + sizeof(BlockHeader) == (char *)curr_block->next) {
            curr_block->size += curr_block->next->size + sizeof(BlockHeader);
            curr_block->next = curr_block->next->next;
        } else {
            printf("Here 7\n");
            curr_block = curr_block->next;
        }
    }

    printf("Here 8\n");

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

int main() {
    
    int heap_size = 4096;

    for (int i = 0; i < 4 ; i++) {

        pid_t pid = fork();

        if(pid == 0) {

            printf("\n--------------------------------\n");
            printf("Child Process %d, PID: %d\n", i, getpid());
            
            if (InitMyMalloc(heap_size) == -1) {
                printf("Memory allocation failed!\n");
                return 1;
            }

            printf("\nMemory allocation successful!\n\n");

            printf("Free list before Allocation:\n");
            DumpFreeList();


            void *ptr1 = MyMalloc(100, i);
            void *ptr2 = MyMalloc(200, i);

            if(ptr1 == NULL || ptr2 == NULL) {
                printf("Memory allocation failed!\n");
                return 1;
            }

            printf("ptr1: %p, size: %d\n", ptr1, ((BlockHeader *)((char *)ptr1 - sizeof(BlockHeader)))->size);
            printf("ptr2: %p, size: %d\n", ptr2, ((BlockHeader *)((char *)ptr2 - sizeof(BlockHeader)))->size);

            DumpFreeList();


            printf("Freeing ptr1..\n");
            MyFree(ptr1);

            DumpFreeList();

            void *ptr3 = MyMalloc(300, i);
            printf("ptr3: %p, size: %d\n", ptr3, ((BlockHeader *)((char *)ptr3 - sizeof(BlockHeader)))->size);

            DumpFreeList();

            printf("Freeing ptr2..\n");
            MyFree(ptr2);

            DumpFreeList();

            printf("Freeing ptr3..\n");
            MyFree(ptr3);

            DumpFreeList();

            return 0;

        }
        else if (pid > 0) {
            wait(NULL);
        }
        else {
            printf("Fork failed!\n");
            return 1;
        }

    }
    
    if (munmap(heap_start, heap_size) == -1) {
        printf("Memory deallocation failed!\n");
        return 1;
    }
    return 0;
}
