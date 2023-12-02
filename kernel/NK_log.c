#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "NK_log.h"

struct {
  struct spinlock log_lock;
  struct logging_params* logs[100];
  int log_head;
} syscall_logs;

syscall_logs_init(){
   initlock(&syscall_logs.log_lock, "log_lock_nk");
   syscall_logs.log_head = 0;
}

void log_syscalls(struct logging_params* logp){
  w_a0((uint64) logp);
  asm volatile("li a7, 29");
  asm volatile("ecall");
}

void log_syscalls_impl(struct logging_params* logp){
  struct logging_params *logp_nk = (struct logging_params*)kalloc_nk();
  *logp_nk = *logp;

  acquire_nk_impl(&syscall_logs.log_lock);
  syscall_logs.logs[syscall_logs.log_head] = logp_nk;
  syscall_logs.log_head++;
  release_nk_impl(&syscall_logs.log_lock);
}
void fetch_syscall_logs(struct logging_params** ok_syscall_logs){
  w_a0((uint64) ok_syscall_logs);
  asm volatile("li a7, 30");
  asm volatile("ecall");
}
int fetch_syscall_logs_impl(struct logging_params** ok_syscall_logs){
  /*
  ** This is not a problem - since the ok can read from the nk, but can't write any value though.!
  */

  acquire_nk_impl(&syscall_logs.log_lock);
  for(int iter = 0; iter<syscall_logs.log_head; iter++){
    ok_syscall_logs[iter] = syscall_logs.logs[iter];
  }
  int return_val = syscall_logs.log_head;
  release_nk_impl(&syscall_logs.log_lock);
  return return_val;
}
