#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

// in kernelvec.S, calls kerneltrap().
void kernelvec();
void okernelvec();
void okernelret();
void kerneltrap();
extern char trampoline_nk[]; // trampoline.S
extern char* shared_space;
extern void timervec();
extern void external_intr();
void uservec_NK();
void syscall_handler();


// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void
okerneltrap()
{
   uint64 mcause = r_mcause();
   w_pmpcfg0(0x0F0D0F);

   //Timer interrupt from M mode.
   if(mcause ==  0x8000000000000007L){
     if((r_mstatus() & ~MSTATUS_MPP_U) != 0)
       timervec();
     
     struct proc* p = myproc();
     if(!strncmp(p->name,"test_nk", 2)){
       //Save user level registers into its trapframe. - Done automatically now
       //Now, delegate the timer request to the supervisor mode.
        w_mtvec((uint64)okernelvec);
        w_mepc((uint64) yield_nk);
        unsigned long x = r_mstatus();
        x &= ~MSTATUS_MPP_MASK;
        x |= MSTATUS_MPP_S;
        w_mstatus(x);
     }
     else{
        timervec();
     }

   }

   else if(mcause ==  0x800000000000000BL){
     int irq = plic_mclaim();
     plic_mcomplete(irq);

     w_mepc((uint64) kernelvec);
     external_intr();

     /* printf("Hellon\n"); */
     /* int irq = plic_mclaim(); */
     /* if(irq == UART0_IRQ){ */
     /*   uartintr(); */
     /* } else if(irq == VIRTIO0_IRQ){ */
     /*   virtio_disk_intr_nk(); */
     /* } else if(irq){ */
     /*   printf("unexpected interrupt irq=%d\n", irq); */
     /* } */
     /* if(irq){ */
     /*  plic_mcomplete(irq); */
     /* }  */
   }

   else if(mcause == 8){
      /*
      ** Disable PMP permissions before delegating the ecall to S mode.
      */

     // send interrupts and exceptions to kerneltrap(),
     // since we're now in the kernel.
     w_mtvec((uint64)okernelvec);
     struct proc *p = myproc();
     if(p->trapframe->a7 == 15){
       copyinstr(p->pagetable, &shared_space, p->trapframe->a0, MAXPATH);
     }
     p->trapframe->epc = r_mepc() + 4;
     unsigned long x = r_mstatus();
     x &= ~MSTATUS_MPP_MASK;
     x |= MSTATUS_MPP_S;
     w_mstatus(x);
     w_mepc((uint64)syscall_handler);
     w_sp(p->trapframe->kernel_sp);
     asm volatile("mret");                      
   }
   
   // Environment call from Supervisor mode
   else if(mcause ==  0x0000000000000009L){
      uint64 *scratch = r_mscratch();

      int a7 = scratch[16];
      uint64 a0 = scratch[9];
      uint64 a1 = scratch[10];
      uint64 a2 = scratch[11];
      uint64 a3 = scratch[12];
      uint64 a4 = scratch[13];
     if(a7 == 1){
       struct proc* p = (struct proc*) a0;
       scratch[9] = proc_pagetable_nk_impl(p);
       goto ret;
     }
     else if(a7 == 2){
      // Perform any checks necessary.
      scratch[9] = uvmalloc_nk_impl(a0, a1, a2, a3);
      goto ret;
     }
     else if(a7 == 3){
        scratch[9] = walkaddr_nk_impl(a0, a1);
        goto ret;
     }
     else if(a7 == 4){
       scratch[9] = readi_nk_impl(a0, a1, a2, a3, a4);
       goto ret;
     }
     else if(a7 == 5){
        scratch[9] = copyout_nk_impl(a0, a1, a2, a3);
        goto ret;
     }
     else if(a7 == 6){
        scratch[9] = either_copyout_nk_impl(a0, a1, a2, a3);
        goto ret;
     }
     else if(a7 == 7){
       uvmclear_nk_impl(a0, a1);
       goto ret;
     }
     else if(a7 == 8){
       char *path = a0;
       char **argv = a1;
       printf("Argv in string is : %s\n", argv[0]);
       exec_nk_impl(path, argv);
       goto ret;
     }
     else if(a7 == 9){
       forkret_NK_impl();
     }
     else if(a7 == 10){
        usertrapret_NK_impl();
        okernelret();
     }
     else if(a7 == 11){
       printf("Done hereee\n");
       struct proc* p = myproc();
       w_mepc(p->m_to_s);
       okernelret();
     }
     else if(a7 == 12){
       struct proc *p = myproc();
        w_mtvec((uint64) uservec_NK);
        w_mepc(p->trapframe->epc);
        unsigned long x = r_mstatus();
        x &= ~MSTATUS_MPP_MASK;
        x |= MSTATUS_MPP_U;
        w_mstatus(x);
        uint64 satp = MAKE_SATP(p->pagetable);
        w_satp(satp);
        w_pmpcfg0(0x0F0F0F);
        userret_NK(p->trapframe);
     }
     else if(a7 == 13){
       w_mie(r_mie() & ~MIE_ECU);
       w_medeleg(0xfdff);
       w_mepc((uint64) sched);
       okernelret();
       
       
     }
   }

 ret:
   w_mepc(r_mepc()+4);
   okernelret();
}
