#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"


extern struct proc proc[NPROC];
extern struct spinlock wait_lock;
extern char trampoline_nk[];
extern struct proc *initproc;
extern struct proc *protected_proc;
void okernelret();
extern char trampoline[]; // trampoline.S
void okernelvec();

void yield_nk(){
  yield();
  asm volatile("li a7, 10");
  asm volatile("ecall");
}

void sched_nk(){
  uint64 ra = r_ra();
  sched();
  asm volatile("mv a0, %0" : : "r" (ra));
  asm volatile("li a7, 11");
  asm volatile("ecall");
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
pagetable_t
proc_pagetable_nk(struct proc *p)
{
  //printf("Allocating pagetable for proc\n");
  asm volatile("li a7, 1");
  w_a0(p);
  asm volatile("ecall");
  return r_a0();

}

void
init_protected_proc_impl(struct proc* p){
  protected_proc = (struct proc*) kalloc_nk();
  *protected_proc = *p;
}

void
init_protected_proc(struct proc* p){
  asm volatile("li a7, 15");
  w_a0((uint64)p);
  asm volatile("ecall");
}
pagetable_t
proc_pagetable_nk_impl(struct proc* p){
    pagetable_t pagetable;
    kfree(p->trapframe);
    p->trapframe = (struct trapframe *)kalloc_nk();

       // An empty page table.
    pagetable = uvmcreate_nk();
    if(pagetable == 0)
        panic("No available region in protected region to allocate space for the pagetable.\n");

       // map the trampoline code (for system call return)
       // at the highest user virtual address.
       // only the supervisor uses it, on the way
       // to/from user space, so not PTE_U.
    if(mappages_nk(pagetable, TRAMPOLINENK, PGSIZE,
                  (uint64)trampoline_nk, PTE_R | PTE_X) < 0){
       panic("No available region in protected region to allocate space for the pagetable.\n");
       uvmfree(pagetable, 0);
    }

      if(mappages_nk(pagetable, TRAMPOLINE, PGSIZE,
              (uint64)trampoline, PTE_R | PTE_X) < 0){
        uvmfree(pagetable, 0);
      }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
    if(mappages_nk(pagetable, TRAPFRAME, PGSIZE,
                 (uint64)(p->trapframe), PTE_R | PTE_W) < 0){
      panic("No available region in protected region to allocate space for the pagetable.\n");
      uvmunmap(pagetable, TRAMPOLINE, 1, 0);
      uvmfree(pagetable, 0);
    }

    //printf("Leaving proc pagetable\n");
    //
    return pagetable;

}

void forkret_NK(void){
   asm volatile("li a7, 9");
   asm volatile("ecall");

}

void forkret_NK_impl(void){
  release(&myproc()->lock);
  usertrapret_NK_impl();
}

void usertrapret_NK(void){
  asm volatile("li a7, 10");
  asm volatile("ecall");
}

int
either_copyout_nk(int user_dst, uint64 dst, void *src, uint64 len)
{
  asm volatile("li a7, 6");
  w_a0(user_dst);
  w_a1(dst);
  w_a2((uint64) src);
  w_a3(len);
  asm volatile("ecall");
  return r_a0();
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int
either_copyout_nk_impl(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if(user_dst){
    return copyout_nk_impl(p->pagetable, dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
void
freeproc_nk_impl(struct proc *p)
{
  if(p->trapframe)
    kfree_nk((void*)p->trapframe); //Update this after shifting the trapframe to the protected space.
  p->trapframe = 0;
  if(p->pagetable)
      proc_freepagetable_nk_impl(p->pagetable, p->sz); //Change this to a syscall to delete the pagetable.

  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}

void
freeproc_nk(struct proc* p){
  asm volatile("li a7, 16");
  w_a0(p);
  asm volatile("ecall");
}

void
reparent_nk(struct proc* p){
  asm volatile("li a7, 17");
  w_a0(p);
  asm volatile("ecall");
}
// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void
reparent_nk_impl(struct proc *p)
{
  struct proc *pp;
  for(pp = proc; pp < &proc[NPROC]; pp++){
      if(pp->parent == p){
        pp->parent = initproc;
        wakeup(initproc);
      }
  }
}

void exit_nk_impl(int status){
  struct proc *p = myproc();

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  acquire(&wait_lock);

  // Give any children to init.
  reparent_nk_impl(p);

  // Parent might be sleeping in wait().
  wakeup(p->parent);

  acquire(&p->lock);

  p->xstate = status;
  p->state = ZOMBIE;

  release(&wait_lock);

  w_mie(r_mie() & ~MIE_ECU);
  w_medeleg(0xfdff);
  w_mepc((uint64) sched);
  printf("Done Exiting process\n");
  okernelret();

}

void exit_nk(int status){
  asm volatile("li a7, 18");
  w_a0(status);
  asm volatile("ecall");
}

void set_protected_proc_state_impl(struct proc* p, enum procstate state){
  p->state = state;
}

void set_protected_proc_state(struct proc* p, enum procstate state){
  asm volatile("li a7, 21");
  w_a0((uint64)p);
  w_a1(state);
  asm volatile("ecall");
}

void set_protected_proc_channel_impl(struct proc* p, void *chan){
  p->chan = chan;
}

void set_protected_proc_channel(struct proc* p, void *chan){
  asm volatile("li a7, 22");
  w_a0((uint64)p);
  w_a1((uint64)chan);
  asm volatile("ecall");
}

void set_protected_proc_pagetable_impl(struct proc* p, pagetable_t pagetable){
  p->pagetable = pagetable;
}

void set_protected_proc_pagetable(struct proc* p, pagetable_t pagetable){
  asm volatile("li a7, 23");
  w_a0((uint64)p);
  w_a1(pagetable);
  asm volatile("ecall");
}


void set_protected_proc_sz_impl(struct proc* p, uint64 sz){
  p->sz = sz;
}

void set_protected_proc_sz(struct proc* p, uint64 sz){
  asm volatile("li a7, 24");
  w_a0((uint64)p);
  w_a1((uint64)sz);
  asm volatile("ecall");
}

void set_protected_proc_ofile_impl(struct proc* p, int fd, struct file *f){
  p->ofile[fd] = f;
}

void set_protected_proc_ofile(struct proc* p, int fd, struct file *f){
  asm volatile("li a7, 25");
  w_a0((uint64)p);
  w_a1(fd);
  w_a2((uint64)f);
  asm volatile("ecall");
}

void swtch_nk_call(struct context *context1, struct context *context2){
  asm volatile("li a7, 26");
  w_a0((uint64)context1);
  w_a1((uint64)context2);
  asm volatile("ecall");
}
void set_protected_proc_trapframe_impl(struct proc* p, uint64 offset, uint64 value){
  uint64 *tf = (uint64*)p->trapframe;
  tf[offset] = value;
}

void set_protected_proc_trapframe(struct proc* p, uint64 offset, uint64 value){
  asm volatile("li a7, 27");
  w_a0((uint64)p);
  w_a1(offset);
  w_a2(value);
  asm volatile("ecall");
}
