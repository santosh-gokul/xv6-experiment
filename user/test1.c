#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

//#include "user/uspinlock.h"
//
void* heappages;

#include <stdarg.h>

void start_func(void) {
    /* Start the thread here. */
    printf("[.] started the thread function (tid = %d) \n", get_tid());
    //printf("[.] started the thread function (tid = %d) \n", get_tid());
    for (int i = 0; i < 0*1000000000; i++);
    int count = 2;
    int* a;


    /* Notify for a thread exit. */
    for (int i = 0; i < 1; i++) {
        printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = count;
            a++;
        }
        count++;
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }

    /* Notify for a thread exit. */
    printf("Destroying thread 1\n");
    //void* heappages = sbrk(4096*1);
    thread_destroy();
}

void start_func_2(int a, int b, int c, int d, int e, int f) {
    /* Start the thread here. */
    printf("Function 2 Arguments = %d %d %d %d %d %d\n", a, b, c, d, e, f);
    printf("[.] started the thread function (tid = %d)\n", get_tid());
    for (int i = 0; i < 0*1000000000; i++);

    int count = 1;
    int* a1;
    /* Notify for a thread exit. */
    for (int i = 0; i < 1; i++) {
        a1 = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 2\n", *a1);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a1 = count;
            a1++;
        }
        count++;
        a1 = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 2\n", *a1);
    }

    printf("Destroying thread 2\n");
    thread_destroy();
}

void start_func_3(void) {
    /* Start the thread here. */
    printf("[.] started the thread function 3 (tid = %d)\n", get_tid());
    for (int i = 0; i < 0*1000000000; i++);

    /* Notify for a thread exit. */
    printf("Destroying thread 3\n");
    thread_destroy();
}

// Block until all the other threads are done executing
void thread_join() {
  while(get_active_threads() != 1){
  }
}

int
main(int argc, char *argv[])
{
    printf("Running Test 1\n");
    heappages = sbrk(4096*1);
    
    /* Initialize the user-level threading library */
    thread_init();


    /* Create a user-level thread */
    uint64 args[6] = {31,0,57,0,98,0};    
    uint64 args2[6] = {10,25,77,80,-65,9};
    thread_create((uint64) start_func, (uint64) args);
    thread_create((uint64) start_func_2, (uint64) args2);
    thread_create((uint64) start_func_3, (uint64) args);
    //void* heappages = sbrk(4096*1);
    thread_join();

    printf("[*] OS-Assisted Threading Test #1 Complete.\n");
    return 0;
}
