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
void* heappages;

void start_func(void) {
  int *a;
  //heappages = sbrk(4096*1);
  //heappages = sbrk(4096*1);
    //uacquire(&data_lock);
    /* Start the thread here. */
    printf("[.] started the thread function 1 (tid = %d) %d \n", get_tid());
    for (int i = 0; i < 100000000; i++);

    printf("In thread 1 function\n");
    uacquire(&data_lock);
    //heappages = sbrk(4096*1);
    printf("After acquire in thread 1: %d\n", heappages);
    data = 200;
    for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }

    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 1\n");
    thread_destroy();
}

void start_func_2(int a, int b, int c, int d, int e, int f) {
  int *a1;
    /* Start the thread here. */
    for (int i = 0; i < 100000000; i++);
    printf("Function 2 Arguments = %d %d %d %d %d %d\n", a, b, c, d, e, f);
    printf("[.] started the thread function 2 (tid = %d)\n", get_tid());
    for (int i = 0; i < 100000000; i++);

    printf("In thread 2 function\n");
    uacquire(&data_lock);
    printf("After acquire in thread 2\n");
    data = 300;
    for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a1);
        a1 = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a1);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a1 = data;
            a1++;
        }
        a1 = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a1);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 100000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 2\n");
    thread_destroy();
}

void start_func_3(void) {
  int *a;
    /* Start the thread here. */
    printf("[.] started the thread function 3 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 3 function\n");

    uacquire(&data_lock);
    printf("After acquire in thread 3\n");
    data = 400;
     for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 3\n");
    thread_destroy();
}

void start_func_4(void) {
  int *a;
    /* Start the thread here. */
    printf("[.] started the thread function 4 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 4 function\n");

    uacquire(&data_lock);
     printf("After acquire in thread 4\n");
    data = 500;
      for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 4\n");
    thread_destroy();
}


void start_func_5(void) {
  int* a;
    /* Start the thread here. */
    printf("[.] started the thread function 5 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 5 function\n");

    uacquire(&data_lock);
     printf("After acquire in thread 5\n");
    data = 600;
      for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 5\n");
    thread_destroy();
}


void start_func_6(void) {
  int* a;
    /* Start the thread here. */
    printf("[.] started the thread function 6 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 6 function\n");

    uacquire(&data_lock);
     printf("After acquire in thread 6\n");
    data = 700;
      for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 6\n");
    thread_destroy();
}


void start_func_7(void) {
  int* a;
    /* Start the thread here. */
    printf("[.] started the thread function 7 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 7 function\n");

    uacquire(&data_lock);
     printf("After acquire in thread 7\n");
    data = 800;
      for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 7\n");
    thread_destroy();
}


void start_func_8(void) {
  int* a;
    /* Start the thread here. */
    printf("[.] started the thread function 8 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 8 function\n");

    uacquire(&data_lock);
     printf("After acquire in thread 8\n");
    data = 900;
      for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 8\n");
    thread_destroy();
}


void start_func_9(void) {
  int* a;
    /* Start the thread here. */
    printf("[.] started the thread function 9 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 9 function\n");

    uacquire(&data_lock);
     printf("After acquire in thread 9\n");
    data = 1000;
      for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 9\n");
    thread_destroy();
}


void start_func_10(void) {
  int* a;
    /* Start the thread here. */
    printf("[.] started the thread function 10 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 10 function\n");

    uacquire(&data_lock);
     printf("After acquire in thread 10\n");
    data = 1100;
      for (int i = 0; i < 1; i++) {
        //printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
         printf("Val at i = %d :: From func 1\n", *a);
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 10\n");
    thread_destroy();
}


void start_func_11(void) {
  int* a;
    /* Start the thread here. */
    printf("[.] started the thread function 11 (tid = %d)\n", get_tid());
    for (int i = 0; i < 1000000000; i++);

    printf("In thread 11 function\n");

    uacquire(&data_lock);
     printf("After acquire in thread 11\n");
    data = 1200;
      for (int i = 0; i < 1; i++) {
        printf("Val at i = %d :: From func 1\n", *a);
        a = ((int*) (heappages + i*PGSIZE));
        for (int j = 0; j < PGSIZE/sizeof(int); j++) {
            *a = data;
            a++;
        }
        a = ((int*) (heappages + i*PGSIZE));
        printf("Val at i = %d :: From func 1\n", *a);
    }
    printf("Setting data = %d\n", data);
    for(int i = 0; i < 1000000000; i++);
    urelease(&data_lock);

    /* Notify for a thread exit. */
    printf("Destroying thread 11\n");
    thread_destroy();
}


void thread_join() {
    while(get_active_threads() != 1);
}

int
main(int argc, char *argv[])
{
    printf("Running Test 2\n");
    heappages = sbrk(4096*1);

    uinitlock(&data_lock, "data_lock");

    /* Initialize the user-level threading library */
    thread_init();

    /* Create a user-level thread */
    uint64 args[6] = {31,0,57,0,98,0};
    uint64 args2[6] = {10,25,77,80,-65,9};
    thread_create((uint64) start_func, (uint64) args, -1);
    //heappages = sbrk(4096*1);
    thread_create((uint64) start_func_2, (uint64) args2, -1);
    thread_create((uint64) start_func_3, (uint64) args, -1);
    thread_create((uint64) start_func_4, (uint64) args, -1);
    thread_create((uint64) start_func_5, (uint64) args, -1);
    thread_create((uint64) start_func_6, (uint64) args, -1);
    thread_create((uint64) start_func_7, (uint64) args, -1);
    thread_create((uint64) start_func_8, (uint64) args, -1);
    thread_join();

    printf("[*] OS-Assited Threading Test #2 Complete.\n");
    return 0;
}
