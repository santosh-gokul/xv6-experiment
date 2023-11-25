#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"


extern char trampoline_nk[];
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

pagetable_t
proc_pagetable_nk_impl(struct proc* p){
    pagetable_t pagetable;
    //kfree(p->trapframe);
    //p->trapframe = (struct trapframe *)kalloc_nk();

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
