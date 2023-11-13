#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;
  struct proc* p = myproc();

  argint(0, &n);

  if(p->parent_thread!=0){
    acquire(&p->parent_thread->lock);
    addr = p->parent_thread->sz;
    p->sz = p->parent_thread->sz;
  }
  else{
     addr = p->sz;
  }

  if(growproc(n) < 0)
    return -1;
  else{
    if(p->parent_thread!=0){
      p->parent_thread->sz = p->sz;
      release(&p->parent_thread->lock);
    }
  }
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_thread_init(void){
  struct proc *p = myproc();
  p->num_threads = 1;
  p->num_threads_static = 1;
  return 0;
}

uint64
sys_thread_create(void){
  uint64 ra;
  uint64 arg_addr;
  argaddr(0, &ra);
  argaddr(1, &arg_addr);
  struct proc *p = myproc();
  uint64 arg_values[6];
  uint64 arg;

  p->num_threads++;
  p->num_threads_static++;
  struct proc* pt = allocprocthread(p);
  //p->child_threads[p->num_threads] = pt;

  pt->trapframe->epc = ra;

  for(int var = 0; var<6; var++){
    copyin(p->pagetable, &arg, arg_addr+var*sizeof(uint64), sizeof(uint64));
    arg_values[var] = arg;
  }

  pt->trapframe->a0 = arg_values[0];
  pt->trapframe->a1 = arg_values[1];
  pt->trapframe->a2 = arg_values[2];
  pt->trapframe->a3 = arg_values[3];
  pt->trapframe->a4 = arg_values[4];
  pt->trapframe->a5 = arg_values[5];

    // increment reference counts on open file descriptors.
  for(int i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      pt->ofile[i] = filedup(p->ofile[i]);
  pt->cwd = idup(p->cwd);

  pt->state = RUNNABLE;
  release(&pt->lock);
  return 0;
}

uint64
sys_thread_destroy(void){
  struct proc *p = myproc();
  acquire(&p->parent_thread->lock);
  p->parent_thread->num_threads--;
  release(&p->parent_thread->lock);
  freeprocthread(p);
  thread_yield();
  return 0;
}

uint64
sys_get_tid(void){
  struct proc *p = myproc();
  return p->pid;
}

uint64
sys_get_active_threads(void){
  struct proc *p = myproc();
  return p->num_threads;
}
