#include "mymem.c"

int main() {
    
    int requested_heap_size;

    printf("Enter the size of the heap: ");
    scanf("%d", &requested_heap_size);
    printf("\n");

    for (int i = 0; i < 4 ; i++) {

        pid_t pid = fork();

        if(pid == 0) {

            printf("\n--------------------------------\n");
            printf("Child Process %d, PID: %d\n", i+1, getpid());
            
            if (InitMyMalloc(requested_heap_size) == -1) {
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

            printf("ptr1: %p, size: %zu\n", ptr1, ((BlockHeader *)((char *)ptr1 - sizeof(BlockHeader)))->size);
            printf("ptr2: %p, size: %zu\n", ptr2, ((BlockHeader *)((char *)ptr2 - sizeof(BlockHeader)))->size);

            DumpFreeList();


            printf("Freeing ptr1..\n");
            MyFree(ptr1);

            DumpFreeList();

            void *ptr3 = MyMalloc(300, i);
            printf("ptr3: %p, size: %zu\n", ptr3,  ((BlockHeader *)((char *)ptr3 - sizeof(BlockHeader)))->size);

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
