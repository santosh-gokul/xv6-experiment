#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "user/uspinlock.h"

#include <stdarg.h>

int data = 100;
struct uspinlock data_lock;

void start_func(void) {
    /* Start the thread here. */
    printf("[.] started the thread function 1 (tid = %d) \n", get_tid());
    for (int i = 0; i < 100000000; i++);

    printf("In thread 1 function\n");

    uacquire(&data_lock);
    data = 200;
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 1\n");
    thread_destroy();
}

void start_func_2(int a, int b, int c, int d, int e, int f) {
    /* Start the thread here. */
    for (int i = 0; i < 100000000; i++);
    printf("Function 2 Arguments = %d %d %d %d %d %d\n", a, b, c, d, e, f);
    printf("[.] started the thread function 2 (tid = %d)\n", get_tid());
    for (int i = 0; i < 100000000; i++);

    printf("In thread 2 function\n");
    uacquire(&data_lock);
    printf("After acquire in thread 2\n");
    data = 300;
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 100000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 2\n");
    thread_destroy();
}

void start_func_3(void) {
    /* Start the thread here. */
    printf("[.] started the thread function 3 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 3 function\n");

    uacquire(&data_lock);
    data = 400;
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 3\n");
    thread_destroy();
}

void thread_join() {
    while(get_active_threads() != 1);
}

int
main(int argc, char *argv[])
{
    printf("Running Test 2\n");

    uinitlock(&data_lock, "data_lock");

    /* Initialize the user-level threading library */
    thread_init();

    /* Create a user-level thread */
    uint64 args[6] = {31,0,57,0,98,0};
    uint64 args2[6] = {10,25,77,80,-65,9};
    thread_create((uint64) start_func, (uint64) args, -1);
    thread_create((uint64) start_func_2, (uint64) args2, -1);
    thread_create((uint64) start_func_3, (uint64) args, -1);

    thread_join();

    printf("[*] OS-Assited Threading Test #2 Complete.\n");
    return 0;
}
