#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE getpagesize()

// Structure for the memory block header
typedef struct block_header {
    size_t size;
    struct block_header *next;
} BlockHeader;

// Global variables
BlockHeader *free_list = NULL;
void *heap_start = NULL;

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

    size_t rounded_heap_size = roundUpToPageSize(HeapSize + sizeof(BlockHeader));
    heap_start = mmap(NULL, rounded_heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (heap_start == MAP_FAILED) {
        return -1;
    }

    free_list = (BlockHeader *)heap_start;
    free_list->size = rounded_heap_size - sizeof(BlockHeader);
    free_list->next = NULL;

    return 0;
}

// Function to allocate memory
void *MyMalloc(int size, int strategy) {
    if (size <= 0 || heap_start == NULL) {
        return NULL;
    }

    BlockHeader *prev_block = NULL;
    BlockHeader *curr_block = free_list;
    BlockHeader *best_fit = NULL;

    // Find the best fit block
    while (curr_block != NULL) {
        if (curr_block->size >= size) {
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
                if (curr_block >= free_list && (best_fit == NULL || curr_block->size < best_fit->size)) {
                    best_fit = curr_block;
                }
            }
        }
        prev_block = curr_block;
        curr_block = curr_block->next;
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
        best_fit->size = size;
        best_fit->next = new_block;
    }

    // Remove the allocated block from the free list and put the new_block in its place
    if (prev_block == NULL || prev_block == best_fit) {
        free_list = best_fit->next;
    } else if(free_list != best_fit) {
        prev_block->next = best_fit->next;
    }

    return (void *)((char *)best_fit + sizeof(BlockHeader));
}

// Function to free allocated memory
int MyFree(void *ptr) {
    if (ptr == NULL || heap_start == NULL) {
        return -1;
    }

    BlockHeader *block_to_free = (BlockHeader *)((char *)ptr - sizeof(BlockHeader));

    // Coalesce free blocks
    BlockHeader *curr_block = free_list;
    BlockHeader *prev_block = NULL;
    while (curr_block != NULL && curr_block < block_to_free) {
        prev_block = curr_block;
        curr_block = curr_block->next;
    }

    if (prev_block == NULL) {
        block_to_free->next = free_list;
        free_list = block_to_free;
    } else {
        block_to_free->next = prev_block->next;
        prev_block->next = block_to_free;
    }

    // Merge adjacent free blocks
    curr_block = free_list;
    while (curr_block != NULL) {
        if ((char *)curr_block + curr_block->size + sizeof(BlockHeader) == (char *)curr_block->next) {
            curr_block->size += curr_block->next->size + sizeof(BlockHeader);
            curr_block->next = curr_block->next->next;
        } else {
            curr_block = curr_block->next;
        }
    }

    return 0;
}

// Function to print the free list for debugging
void DumpFreeList() {
    BlockHeader *curr_block = free_list;
    printf("Addr\tSize\tStatus\n");
    while (curr_block != NULL) {
        printf("%p\t%lu\tEmpty\n", curr_block, curr_block->size);
        curr_block = curr_block->next;
    }

    printf("\n\n");
}

int main() {
    int heap_size;
    int strategy;

    printf("Enter the heap size: ");
    scanf("%d", &heap_size);

    printf("Enter the strategy type (0: BF, 1: WF, 2: FF, 3: NF): ");
    scanf("%d", &strategy);

    if (InitMyMalloc(heap_size) == -1) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    printf("Heap size: %d\n", free_list->size);

    DumpFreeList();
    
    printf("\n\n");

    // Process allocation
    void *p1 = MyMalloc(100, strategy);
    void *p2 = MyMalloc(200, strategy);
    void *p3 = MyMalloc(150, strategy);
    void *p4 = MyMalloc(300, strategy);

    printf("Strategy: %d\n", strategy);
    printf("Process P1: %s, Address: %p\n", p1 != NULL ? "Succeed" : "Failed", p1);
    printf("Process P2: %s, Address: %p\n", p2 != NULL ? "Succeed" : "Failed", p2);
    printf("Process P3: %s, Address: %p\n", p3 != NULL ? "Succeed" : "Failed", p3);
    printf("Process P4: %s, Address: %p\n", p4 != NULL ? "Succeed" : "Failed", p4);
    printf("\n\n");

    printf("Free list after allocation:\n");
    DumpFreeList();
    
    // Freeing memory
    printf("Freeing P2\n");
    MyFree(p2);
    DumpFreeList();

    printf("Freeing P4\n");
    MyFree(p4);
    DumpFreeList();

    printf("Freeing P1\n");
    MyFree(p1);
    DumpFreeList();

    printf("Freeing P3\n");
    MyFree(p3);
    DumpFreeList();

    printf("Free list after deallocation:\n");
    DumpFreeList();

    printf("Freeing the heap at: %p\n", heap_start);
    free(heap_start);
    return 0;
}
