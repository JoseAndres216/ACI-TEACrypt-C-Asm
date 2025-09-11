.section .text
.globl tea_decrypt
.type tea_decrypt, @function

tea_decrypt:
    addi sp, sp, -16
    sw   s0, 12(sp)
    sw   s1, 8(sp)

    lw   t0, 0(a0)     # v0
    lw   t1, 4(a0)     # v1
    lw   t2, 0(a1)     # k0
    lw   t3, 4(a1)     # k1
    lw   t4, 8(a1)     # k2
    lw   t5, 12(a1)    # k3

    slli t6, t0, 4
    add  t6, t6, t4
    add  s0, t0, a2
    srli s1, t0, 5
    add  s1, s1, t5
    xor  t6, t6, s0
    xor  t6, t6, s1
    sub  t1, t1, t6

    slli t6, t1, 4
    add  t6, t6, t2
    add  s0, t1, a2
    srli s1, t1, 5
    add  s1, s1, t3
    xor  t6, t6, s0
    xor  t6, t6, s1
    sub  t0, t0, t6

    sw t0, 0(a0)
    sw t1, 4(a0)

    lw   s1, 8(sp)
    lw   s0, 12(sp)
    addi sp, sp, 16
    ret
