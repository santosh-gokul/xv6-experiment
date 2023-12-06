# Context switch
#
#   void swtch(struct context *old, struct context *new);
#
# Save current registers in old. Load from new.


.globl swtch_nk
swtch_nk:
        csrrw a2, mscratch, a2
        ld ra, 0(a2)
        ld sp, 8(a2)
        ld s0, 56(a2)
        ld s1, 64(a2)
        ld s2, 136(a2)
        ld s3, 144(a2)
        ld s4, 152(a2)
        ld s5, 160(a2)
        ld s6, 168(a2)
        ld s7, 176(a2)
        ld s8, 184(a2)
        ld s9, 192(a2)
        ld s10, 200(a2)
        ld s11, 208(a2)

        sd ra, 0(a0)
        sd sp, 8(a0)
        sd s0, 16(a0)
        sd s1, 24(a0)
        sd s2, 32(a0)
        sd s3, 40(a0)
        sd s4, 48(a0)
        sd s5, 56(a0)
        sd s6, 64(a0)
        sd s7, 72(a0)
        sd s8, 80(a0)
        sd s9, 88(a0)
        sd s10, 96(a0)
        sd s11, 104(a0)


        ld ra, 0(a1)
        ld sp, 8(a1)
        ld s0, 16(a1)
        ld s1, 24(a1)
        ld s2, 32(a1)
        ld s3, 40(a1)
        ld s4, 48(a1)
        ld s5, 56(a1)
        ld s6, 64(a1)
        ld s7, 72(a1)
        ld s8, 80(a1)
        ld s9, 88(a1)
        ld s10, 96(a1)
        ld s11, 104(a1)


        sd ra, 0(a2)
        sd sp, 8(a2)
        sd s0, 56(a2)
        sd s1, 64(a2)
        sd s2, 136(a2)
        sd s3, 144(a2)
        sd s4, 152(a2)
        sd s5, 160(a2)
        sd s6, 168(a2)
        sd s7, 176(a2)
        sd s8, 184(a2)
        sd s9, 192(a2)
        sd s10, 200(a2)
        sd s11, 208(a2)

        csrrw a2, mscratch, a2
        csrw mepc, ra
        j nkernelret
