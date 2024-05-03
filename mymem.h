#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <sys/wait.h>

#define PAGE_SIZE getpagesize()

// Structure for the memory block header
typedef struct block_header {
    size_t size;
    struct block_header *next;
    bool is_free;
} BlockHeader;

// Global variables
BlockHeader *block_list = NULL;
BlockHeader *last_choice = NULL;
void *heap_start = NULL;
size_t heap_size = 0;

// Function prototypes
int InitMyMalloc(int HeapSize);
void *MyMalloc(int size, int strategy);
int MyFree(void *ptr);
void DumpFreeList();