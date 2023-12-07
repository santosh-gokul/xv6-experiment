        #
        # low-level code to handle traps from kernel space into
        # the nested kernel, and returns from nested kernel to the kernel.
        #
        # the nested kernel maps the page holding this code
        # at the same virtual address (TRAMPOLINE_NK)
        # in okernel and nkernel space so that it continues
        # to work when it switches page tables.
        # kernel.ld causes this code to start at
        # a page boundary.
        #

#include "riscv.h"
#include "memlayout.h"

.section trampsec_nk
.globl trampoline_nk
trampoline_nk:
.align 4
.globl nkernelvec
nkernelvec:
	#
        # nk_trap.c sets mtvec to point here, so
        # traps from okernel space start here,
        # in machine mode, but with the
        # same page table.
        #

        # jump to nkerneltrap(), which does not return


        csrrw a0, mscratch, a0

        # save the registers.
        sd ra, 0(a0)
        sd sp, 8(a0)
        sd gp, 16(a0)
        sd tp, 24(a0)
        sd t0, 32(a0)
        sd t1, 40(a0)
        sd t2, 48(a0)
        sd s0, 56(a0)
        sd s1, 64(a0)
        sd a1, 80(a0)
        sd a2, 88(a0)
        sd a3, 96(a0)
        sd a4, 104(a0)
        sd a5, 112(a0)
        sd a6, 120(a0)
        sd a7, 128(a0)
        sd s2, 136(a0)
        sd s3, 144(a0)
        sd s4, 152(a0)
        sd s5, 160(a0)
        sd s6, 168(a0)
        sd s7, 176(a0)
        sd s8, 184(a0)
        sd s9, 192(a0)
        sd s10, 200(a0)
        sd s11, 208(a0)
        sd t3, 216(a0)
        sd t4, 224(a0)
        sd t5, 232(a0)
        sd t6, 240(a0)
        csrrw a0, mscratch, a0
        csrrw s11, mscratch, s11
        sd a0, 72(s11)
        csrrw s11, mscratch, s11


        la sp, stack_nk
        li a0, 1024*4
        csrr a1, mhartid
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0

        j nkerneltrap

.globl uservec_NK
uservec_NK:
	#
        # trap.c sets stvec to point here, so
        # traps from user space start here,
        # in supervisor mode, but with a
        # user page table.
        #

        # save user a0 in sscratch so
        # a0 can be used to get at TRAPFRAME.
        csrrw a0, sscratch, a0

        # each process has a separate p->trapframe memory area,
        # but it's mapped to the same virtual address
        # (TRAPFRAME) in every process's user page table.

        # save the user registers in TRAPFRAME
        sd ra, 40(a0)
        sd sp, 48(a0)
        sd gp, 56(a0)
        sd tp, 64(a0)
        sd t0, 72(a0)
        sd t1, 80(a0)
        sd t2, 88(a0)
        sd s0, 96(a0)
        sd s1, 104(a0)
        sd a1, 120(a0)
        sd a2, 128(a0)
        sd a3, 136(a0)
        sd a4, 144(a0)
        sd a5, 152(a0)
        sd a6, 160(a0)
        sd a7, 168(a0)
        sd s2, 176(a0)
        sd s3, 184(a0)
        sd s4, 192(a0)
        sd s5, 200(a0)
        sd s6, 208(a0)
        sd s7, 216(a0)
        sd s8, 224(a0)
        sd s9, 232(a0)
        sd s10, 240(a0)
        sd s11, 248(a0)
        sd t3, 256(a0)
        sd t4, 264(a0)
        sd t5, 272(a0)
        sd t6, 280(a0)

	# save the user a0 in p->trapframe->a0
        csrrw a0, sscratch, a0
        csrrw t2, sscratch, t2
        sd a0, 112(t2)


        la sp, stack_nk
        li a0, 1024*4
        csrr a1, mhartid
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0

        # make tp hold the current hartid, from p->trapframe->kernel_hartid
        ld tp, 32(t2)

        # fetch the kernel page table address, from p->trapframe->kernel_satp.
        ld t1, 0(t2)

        # load the address of usertrap(), from p->trapframe->kernel_trap
        ld t0, 16(t2)

        # wait for any previous memory operations to complete, so that
        # they use the user page table.
        sfence.vma zero, zero

        # install the kernel page table.
        csrw satp, t1

        # flush now-stale user entries from the TLB.
        sfence.vma zero, zero

        csrrw t2, sscratch, t2
        # jump to usertrap(), which does not return
        jr t0

.globl userret_NK
userret_NK:
        # userret(pagetable)
        # called by usertrapret() in trap.c to
        # switch from kernel to user.
        # a0: user page table, for satp.

        # switch to the user page table.
        # restore all but a0 from TRAPFRAME
        ld ra, 40(a0)
        ld sp, 48(a0)
        ld gp, 56(a0)
        ld tp, 64(a0)
        ld t0, 72(a0)
        ld t1, 80(a0)
        ld t2, 88(a0)
        ld s0, 96(a0)
        ld s1, 104(a0)
        ld a1, 120(a0)
        ld a2, 128(a0)
        ld a3, 136(a0)
        ld a4, 144(a0)
        ld a5, 152(a0)
        ld a6, 160(a0)
        ld a7, 168(a0)
        ld s2, 176(a0)
        ld s3, 184(a0)
        ld s4, 192(a0)
        ld s5, 200(a0)
        ld s6, 208(a0)
        ld s7, 216(a0)
        ld s8, 224(a0)
        ld s9, 232(a0)
        ld s10, 240(a0)
        ld s11, 248(a0)
        ld t3, 256(a0)
        ld t4, 264(a0)
        ld t5, 272(a0)
        ld t6, 280(a0)

	# restore user a0
        ld a0, 112(a0)

        # return to user mode and user pc.
        # usertrapret() set up sstatus and sepc.
        mret


.globl nkernelret
nkernelret:
        # nkernelret(pagetable)
        # called by nkerneltrapret() in NK_trap.c to
        # switch from nkernel to S mode.

        csrrw a0, mscratch, a0
        ld ra, 0(a0)
        ld sp, 8(a0)
        ld gp, 16(a0)
        ld tp, 24(a0)
        ld t0, 32(a0)
        ld t1, 40(a0)
        ld t2, 48(a0)
        ld s0, 56(a0)
        ld s1, 64(a0)
        ld a1, 80(a0)
        ld a2, 88(a0)
        ld a3, 96(a0)
        ld a4, 104(a0)
        ld a5, 112(a0)
        ld a6, 120(a0)
        ld a7, 128(a0)
        ld s2, 136(a0)
        ld s3, 144(a0)
        ld s4, 152(a0)
        ld s5, 160(a0)
        ld s6, 168(a0)
        ld s7, 176(a0)
        ld s8, 184(a0)
        ld s9, 192(a0)
        ld s10, 200(a0)
        ld s11, 208(a0)
        ld t3, 216(a0)
        ld t4, 224(a0)
        ld t5, 232(a0)
        ld t6, 240(a0)

        csrrw a0, mscratch, a0
        csrrw s11, mscratch, s11
        ld a0, 72(s11)
        csrrw s11, mscratch, s11

        # return to S mode and kernel pc.
        # nkerneltrapret() set up mstatus and mepc.
        mret

.globl timervec
timervec:
        # start.c has set up the memory that mscratch points to:
        # scratch[0,8,16] : register save area.
        # scratch[24] : address of CLINT's MTIMECMP register.
        # scratch[32] : desired interval between interrupts.

        csrrw a0, mscratch, a0

        # schedule the next timer interrupt
        # by adding interval to mtimecmp.
        ld a1, 248(a0) # CLINT_MTIMECMP(hart)
        ld a2, 256(a0) # interval
        ld a3, 0(a1)
        add a3, a3, a2
        sd a3, 0(a1)

        # arrange for a supervisor software interrupt
        # after this handler returns.
        li a1, 2
        csrw sip, a1
        csrrw a0, mscratch, a0

        j nkernelret

.globl external_intr
external_intr:
        li a1, 512
        csrw sip, a1
        j nkernelret
