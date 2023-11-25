// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange_nk(void *pa_start, void *pa_end);

extern char end_nk[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem_nk;

void
kinit_nk()
{
  initlock(&kmem_nk.lock, "kmem_nk");
  freerange_nk(end_nk, (void*)NKSTOP);
}

void
freerange_nk(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree_nk(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree_nk(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end_nk || (uint64)pa >= NKSTOP)
    panic("kfree in nk");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem_nk.lock);
  r->next = kmem_nk.freelist;
  kmem_nk.freelist = r;
  release(&kmem_nk.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc_nk(void)
{
  struct run *r;

  acquire(&kmem_nk.lock);
  r = kmem_nk.freelist;
  if(r)
    kmem_nk.freelist = r->next;
  release(&kmem_nk.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
