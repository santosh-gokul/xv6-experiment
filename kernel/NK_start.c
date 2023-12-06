#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/defs.h"

void main();
void timerinit();
void nkernelvec();
volatile static int started = 0;
// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// a scratch area per CPU for machine-mode timer interrupts.
uint64 timer_scratch[NCPU][34];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();
char* shared_space;

// entry.S jumps here in machine mode on stack0.
void
start()
{
  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  w_mstatus(x);


  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany
  w_mepc((uint64)main);

  // disable paging for now.
  w_satp(0);

  // Just don't delegate the ecall from S mode to S mode itself. Now its trapped by the M mode
  // Execptions all are delegated to S mode.
  w_medeleg(0xfdff);
  w_mideleg(0xffff);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);


  // configure Physical Memory Protection to give supervisor mode
  // access to all of physical memory.
  //w_pmpaddr0(0x3fffffffffffffull);

  //Region from 0x81000000 to 0x81000000 are allocated for NK region.
  //16MB region in the memory.
  w_pmpaddr0(0x0000000080000000>>2);
  w_pmpaddr1(0x0000000081000000>>2);
  w_pmpaddr2(0x0000000088000000>>2);
  w_pmpcfg0(0x0F0D0F);

  // D = Only R/X

  if(r_mhartid() == 0){
  kinit(); //For unprotected region memory init.
  kinit_nk();
  shared_space = (char*) kalloc();
  kvminit_nk();
  started = 1;
  kvminithart_nk();
  syscall_logs_init();
   }else{
    while(started == 0);
      __sync_synchronize();
      kvminithart_nk();
   }

  // ask for clock interrupts.
  timerinit();
  w_mtvec((uint64)nkernelvec);

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);

  asm volatile("call _entry");
}

// arrange to receive timer interrupts.
// they will arrive in machine mode at
// at timervec in kernelvec.S,
// which turns them into software interrupts for
// devintr() in trap.c.
void
timerinit()
{
  // each CPU has a separate source of timer interrupts.
  int id = r_mhartid();

  // ask the CLINT for a timer interrupt.
  int interval = 1000000; // cycles; about 1/10th second in qemu.
  *(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval;

  // prepare information in scratch[] for timervec.
  // scratch[0..2] : space for timervec to save registers.
  // scratch[3] : address of CLINT MTIMECMP register.
  // scratch[4] : desired interval (in cycles) between timer interrupts.
  uint64 *scratch = &timer_scratch[id][0];
  scratch[31] = CLINT_MTIMECMP(id);
  scratch[32] = interval;
  w_mscratch((uint64)scratch);

  // set the machine-mode trap handler.
  //w_mtvec((uint64)timervec);

  // enable machine-mode interrupts.
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // ECS stands for Env call from Supervisor model
  // enable machine-mode timer interrupts.
   w_mie(r_mie() | MIE_MTIE | MIE_ECS);

}
