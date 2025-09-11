.section .text
.globl tea_encrypt

tea_encrypt:
    addi sp, sp, -16
    sw   s0, 12(sp)
    sw   s1, 8(sp)



    lw   t0, 0(a0)
    lw   t1, 4(a0)

    lw   t2, 0(a1)
    lw   t3, 4(a1)
    lw   t4, 8(a1)
    lw   t5, 12(a1)



    slli t6, t1, 4
    add  t6, t6, t2
    add  s0, t1, a2
    srli s1, t1, 5
    add  s1, s1, t3
    xor  t6, t6, s0
    xor  t6, t6, s1
    add  t0, t0, t6



    slli t6, t0, 4
    add  t6, t6, t4
    add  s0, t0, a2
    srli s1, t0, 5
    add  s1, s1, t5
    xor  t6, t6, s0
    xor  t6, t6, s1
    add  t1, t1, t6



    sw t0, 0(a0)
    sw t1, 4(a0)



    lw   s1, 8(sp)
    lw   s0, 12(sp)
    addi sp, sp, 16
    ret
