#include "uspinlock.h"
#include <stdlib.h>
#include <stdio.h>


void
panic_thread(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}

void
uinitlock(struct uspinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->tid = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void
uacquire(struct uspinlock *lk)
{
 if(holding(lk)){
    panic_thread("PANIC: release");
  }


  // On RISC-V, sync_lock_test_and_set turns into an atomic swap:
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while(__sync_lock_test_and_set(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen strictly after the lock is acquired.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize();

  lk->tid = get_tid();
  // Record info about lock acquisition for holding() and debugging.
  //lk->cpu = mycpu();
}

// Release the lock.
void
urelease(struct uspinlock *lk)
{
 if(!holding(lk)){
   panic_thread("release");
 }

  lk->tid = 0;
  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released,
  // and that loads in the critical section occur strictly before
  // the lock is released.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code doesn't use a C assignment, since the C standard
  // implies that an assignment might be implemented with
  // multiple store instructions.
  // On RISC-V, sync_lock_release turns into an atomic swap:
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)
  __sync_lock_release(&lk->locked);
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int
holding(struct uspinlock *lk)
{
  int r;
  r = (lk->locked && lk->tid == get_tid());
  return r;
}
