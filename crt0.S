    .extern main
    .global __start

    .set mips32
    .set noreorder
    .set noat

    .section .init.text,"ax"

__start:
    # clear bss
    lui $t0, %hi(bssbegin)
    addiu $t0, %lo(bssbegin)
    lui $t1, %hi(bssend)
    addiu $t1, %lo(bssend)
    beq $t0, $t1, 2f
    nop

1:
    addiu $t0, 4
    bne $t0, $t1, 1b
    sw $0, -4($t0)

    # jump to main
2:
    b main
    nop
