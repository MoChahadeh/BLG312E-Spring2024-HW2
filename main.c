#include "mymem.c"

char* getstrategy(int i) {

    if (i == 0) {
        return "Best Fit\n";
    }
    else if (i == 1) {
        return "Worst Fit\n";
    }
    else if (i == 2) {
        return "First Fit\n";
    }
    else if (i == 3) {
        return "Next Fit\n";
    }
    else {
        return "Invalid Strategy\n";
    }

}

int main() {
    
    int requested_heap_size;

    printf("Enter the size of the heap: ");
    scanf("%d", &requested_heap_size);
    printf("\n");

    for (int i = 0; i < 4 ; i++) {

        pid_t pid = fork();

        if(pid == 0) {

            printf("\n--------------------------------\n");
            printf("Child Process %d, PID: %d, Strategy: %s\n", i+1, getpid(), getstrategy(i));
            
            if (InitMyMalloc(requested_heap_size) == -1) {
                printf("Memory allocation failed!\n");
                return 1;
            }

            printf("\nMemory allocation successful!\n\n");

            printf("Free list before Allocation:\n");
            DumpFreeList();


            void *ptr1 = MyMalloc(2048, i);
            void *ptr2 = MyMalloc(500, i);

            if(ptr1 == NULL || ptr2 == NULL) {
                printf("Memory allocation failed!\n");
                return 1;
            }

            printf("ptr1: %p, size: %zu\n", ptr1, ((BlockHeader *)((char *)ptr1 - sizeof(BlockHeader)))->size);
            printf("ptr2: %p, size: %zu\n", ptr2, ((BlockHeader *)((char *)ptr2 - sizeof(BlockHeader)))->size);

            DumpFreeList();


            printf("Freeing ptr1..\n");
            MyFree(ptr1);

            DumpFreeList();

            void *ptr3 = MyMalloc(300, i);
            printf("ptr3: %p, size: %zu\n", ptr3,  ((BlockHeader *)((char *)ptr3 - sizeof(BlockHeader)))->size);

            DumpFreeList();


            // allocating 300 bytes
            void *ptr4 = MyMalloc(300, i);
            printf("ptr4: %p, size: %zu\n", ptr4,  ((BlockHeader *)((char *)ptr4 - sizeof(BlockHeader)))->size);

            DumpFreeList();

            printf("Freeing ptr2..\n");
            MyFree(ptr2);

            DumpFreeList();

            
            printf("Freeing ptr3..\n");
            MyFree(ptr3);

            DumpFreeList();

            if (munmap(heap_start, heap_size) == -1) {
                printf("Memory deallocation failed!\n");
                return 1;
            }

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
    
    return 0;
}
