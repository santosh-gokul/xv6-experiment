#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"

static int loadseg_nk(pde_t *, uint64, struct inode *, uint, uint);
extern char trampoline[], uservec[], userret[], uservec_NK[], trampoline_nk[], userret_NK[];
struct proc *protected_proc;


void exec_nk(char *path, char **argv){
  char *s, *last;
  int i, off;
  uint64 argc, sz = 0, sp, ustack_temp[MAXARG], stackbase;
  struct elfhdr elf;
  char* ustack = (char*) kalloc();
  memmove(ustack, ustack_temp, MAXARG);

  struct inode *ip;
  struct proghdr ph;
  pagetable_t pagetable = 0, oldpagetable;
  struct proc *p = myproc();

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    p->trapframe->a0 = -1;
  }
  ilock(ip);


  // Check ELF header
  if(readi(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;

  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pagetable = proc_pagetable_nk(p)) == 0)
    goto bad;

  /*  uint64 trampoline_userret = TRAMPOLINENK + (userret_NK - trampoline_nk); */
  /* ((void (*)(uint64))trampoline_userret)(MAKE_SATP(p->pagetable)); */

  // Load program into memory.
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    uint64 sz1;
    if((sz1 = uvmalloc_nk(pagetable, sz, ph.vaddr + ph.memsz, flags2perm(ph.flags))) == 0)
      goto bad;
    sz = sz1;
    if(loadseg_nk(pagetable, ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }

  iunlockput(ip);
  end_op();
  ip = 0;

  p = myproc();
  uint64 oldsz = p->sz;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible as a stack guard.
  // Use the second as the user stack.

  sz = PGROUNDUP(sz);
  uint64 sz1;
  if((sz1 = uvmalloc_nk(pagetable, sz, sz + 2*PGSIZE, PTE_W)) == 0)
    goto bad;
  sz = sz1;

  uvmclear_nk(pagetable, sz-2*PGSIZE);

  sp = sz;
  stackbase = sp - PGSIZE;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp -= strlen(argv[argc]) + 1;
    sp -= sp % 16; // riscv sp must be 16-byte aligned
    if(sp < stackbase)
      goto bad;
    if(copyout_nk(pagetable, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[argc] = sp;
  }

  ustack[argc] = 0;

  // push the array of argv[] pointers.
  sp -= (argc+1) * sizeof(uint64);
  sp -= sp % 16;
  if(sp < stackbase)
    goto bad;

  if(copyout_nk(pagetable, sp, (char *)ustack, (argc+1)*sizeof(uint64)) < 0){
      goto bad;
    }

  // arguments to user main(argc, argv)
  // argc is returned via the system call return
  // value, which goes in a0.

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(p->name, last, sizeof(p->name));

  // Commit to the user image.
  oldpagetable = p->pagetable;

  init_protected_proc(p);
  mycpu()->proc = protected_proc;
  p = protected_proc;
  set_protected_proc_trapframe(p, 15, sp);
  //p->trapframe->a1 = sp;


  set_protected_proc_pagetable(p, pagetable);
  set_protected_proc_sz(p, sz);


  set_protected_proc_trapframe(p, 3, elf.entry);
  set_protected_proc_trapframe(p, 6, sp);
  //p->trapframe->epc = elf.entry;  // initial program counter = main
  //p->trapframe->sp = sp; // initial stack pointer

  proc_freepagetable(oldpagetable, oldsz);
  set_protected_proc_trapframe(p, 14, argc);
  //printf("Control is here: %d\n", *pte);
  //p->trapframe->a0 = argc; // this ends up in a0, the first argument to main(argc, argv)
  printf("Exiting exec\n");
  usertrapret_NK();

 bad:
  if(pagetable)
    proc_freepagetable(pagetable, sz);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  p->trapframe->a0 = -1;
}

// Load a program segment into pagetable at virtual address va.
// va must be page-aligned
// and the pages from va to va+sz must already be mapped.
// Returns 0 on success, -1 on failure.
static int
loadseg_nk(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint sz)
{
  uint i, n;
  uint64 pa;

  for(i = 0; i < sz; i += PGSIZE){
    pa = walkaddr_nk(pagetable, va + i);
    if(pa == 0)
      panic("loadseg: address should exist");
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if(readi_nk(ip, 0, (uint64)pa, offset+i, n) != n)
      return -1;
  }

  return 0;
}
