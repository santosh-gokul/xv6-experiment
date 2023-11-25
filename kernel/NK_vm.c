#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"

pagetable_t nk_pagetable;
extern char etext[];
extern char etext_nk[];// kernel.ld sets this to end of kernel code.

extern char trampoline[]; // trampoline.S
extern char trampoline_nk[];


pagetable_t
kvmmake_nk(void)
{
  pagetable_t kpgtbl;

  kpgtbl = (pagetable_t) kalloc_nk();
  memset(kpgtbl, 0, PGSIZE);

  // uart registers
  kvmmap_nk(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);

  // virtio mmio disk interface
  kvmmap_nk(kpgtbl, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

  // PLIC
  kvmmap_nk(kpgtbl, PLIC, PLIC, 0x400000, PTE_R | PTE_W);

  // map kernel text executable and read-only.
  //printf("NK: %d", etext_nk);
  //printf("Size1: %d and Size2: %d\n", (uint64)etext_nk-NKBASE,  NKSTOP - (uint64)etext_nk);
  kvmmap_nk(kpgtbl, NKBASE, NKBASE, (uint64)etext_nk-NKBASE, PTE_R | PTE_X);
  kvmmap_nk(kpgtbl, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);
  kvmmap_nk(kpgtbl, (uint64)etext_nk, (uint64)etext_nk, NKSTOP - (uint64)etext_nk, PTE_R | PTE_W);
  // map kernel data and the physical RAM we'll make use of.
  kvmmap_nk(kpgtbl, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);

  // map the trampoline for trap entry/exit to
  // the highest virtual address in the kernel.
  //
  //Need a different trampoline for us here.
  kvmmap_nk(kpgtbl, TRAMPOLINENK, (uint64)trampoline_nk, PGSIZE, PTE_R | PTE_X);
  kvmmap_nk(kpgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);

  // allocate and map a kernel stack for each process.
  proc_mapstacks(kpgtbl);

  return kpgtbl;
}

// Initialize the one kernel_pagetable
void
kvminit_nk(void)
{
  nk_pagetable = kvmmake_nk();
}

// Switch h/w page table register to the kernel's page table,
// and enable paging.
void
kvminithart_nk()
{
  // wait for any previous writes to the page table memory to finish.
  sfence_vma();

  w_satp(MAKE_SATP(nk_pagetable));

  // flush stale entries from the TLB.
  sfence_vma();
}


pte_t *
walk_nk(pagetable_t pagetable, uint64 va, int alloc)
{
  if(va >= MAXVA)
    panic("walk");

  for(int level = 2; level > 0; level--) {
    pte_t *pte = &pagetable[PX(level, va)];
    if(*pte & PTE_V) {
      pagetable = (pagetable_t)PTE2PA(*pte);
    } else {
      if(!alloc || (pagetable = (pde_t*)kalloc_nk()) == 0)
        return 0;
      memset(pagetable, 0, PGSIZE);
      *pte = PA2PTE(pagetable) | PTE_V;
    }
  }
  return &pagetable[PX(0, va)];
}


// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64
walkaddr_nk_impl(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  uint64 pa;

  if(va >= MAXVA)
    return 0;

  pte = walk_nk(pagetable, va, 0);
  if(pte == 0)
    return 0;
  if((*pte & PTE_V) == 0)
    return 0;
  if((*pte & PTE_U) == 0)
    return 0;
  pa = PTE2PA(*pte);
  return pa;
}

uint64
walkaddr_nk(pagetable_t pagetable, uint64 va)
{
  asm volatile("li a7, 3");
  w_a0((uint64) pagetable);
  w_a1(va);
  asm volatile("ecall");
  return r_a0();
}



// add a mapping to the kernel page table.
// only used when booting.
// does not flush TLB or enable paging.
void
kvmmap_nk(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
{
  if(mappages_nk(kpgtbl, va, sz, pa, perm) != 0)
    panic("kvmmap nk");
}



int
mappages_nk(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  uint64 a, last;
  pte_t *pte;
  //printf("size: %d\n", size);

  if(size == 0)
    panic("nk mappages: size");

  a = PGROUNDDOWN(va);
  last = PGROUNDDOWN(va + size - 1);
  for(;;){
    if((pte = walk_nk(pagetable, a, 1)) == 0)
      return -1;
    if(*pte & PTE_V){
      //printf("PA: %d %d\n", a, size);
      panic("nk mappages: remap");
    }//printf("PA: %d\n", a);
    *pte = PA2PTE(pa) | perm | PTE_V;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}


// create an empty user page table.
// returns 0 if out of memory.
pagetable_t
uvmcreate_nk()
{
  pagetable_t pagetable;
  pagetable = (pagetable_t) kalloc_nk();
  if(pagetable == 0)
    return 0;
  memset(pagetable, 0, PGSIZE);
  return pagetable;
}


uint64
uvmalloc_nk_impl(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
{
  char *mem;
  uint64 a;

  if(newsz < oldsz)
    return oldsz;

  oldsz = PGROUNDUP(oldsz);
  for(a = oldsz; a < newsz; a += PGSIZE){
    mem = kalloc_nk();
    if(mem == 0){
      uvmdealloc(pagetable, a, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    if(mappages_nk(pagetable, a, PGSIZE, (uint64)mem, PTE_R|PTE_U|xperm) != 0){
      kfree_nk(mem);
      uvmdealloc(pagetable, a, oldsz);
      return 0;
    }
  }
  return newsz;
}

// Allocate PTEs and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
uint64
uvmalloc_nk(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
{
  asm volatile("li a7, 2");
  w_a0((uint64) pagetable);
  w_a1(oldsz);
  w_a2(newsz);
  w_a3(xperm);
  asm volatile("ecall");
  return r_a0();
}


// mark a PTE invalid for user access.
// used by exec for the user stack guard page.
void
uvmclear_nk_impl(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;

  pte = walk(pagetable, va, 0);
  if(pte == 0)
    panic("uvmclear");
  *pte &= ~PTE_U;
}


// mark a PTE invalid for user access.
// used by exec for the user stack guard page.
void
uvmclear_nk(pagetable_t pagetable, uint64 va)
{
  asm volatile("li a7, 7");
  w_a0((uint64) pagetable);
  w_a1(va);
  asm volatile("ecall");
}


// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
int
copyout_nk(pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
  asm volatile("li a7, 5");
  w_a0((uint64) pagetable);
  w_a1(dstva);
  w_a2((uint64) src);
  w_a3(len);

  asm volatile("ecall");
  return r_a0();
}

// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
int
copyout_nk_impl(pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
  uint64 n, va0, pa0;

  while(len > 0){
    va0 = PGROUNDDOWN(dstva);
    pa0 = walkaddr_nk_impl(pagetable, va0);

    if(pa0 == 0)
      return -1;
    n = PGSIZE - (dstva - va0);
    if(n > len)
      n = len;
    memmove((void *)(pa0 + (dstva - va0)), src, n);


    len -= n;
    src += n;
    dstva = va0 + PGSIZE;
  }

  return 0;
}
