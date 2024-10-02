.data
newline: .asciiz "\n"
h: .word 0
g: .word 0
x: .word 0
.text
.globl main
main:
    sw t0, h
    sw t0, t1
    sw t1, g
    sw t0, t2
    sw t0, t3
    lw $t0, t2
    lw $t1, t3
    add $t2, $t0, $t1
    sw $t2, t4
    sw t0, t5
    sw t5, x
    sw t0, t7
    sw t0, t8
    lw $t0, t7
    lw $t1, t8
    add $t2, $t0, $t1
    sw $t2, t9
    sw t0, t6
    sw t0, t10
    lw $t0, t6
    lw $t1, t10
    add $t2, $t0, $t1
    sw $t2, t11
    sw t0, t12
    sw t12, h
    lw $a0, x
    li $v0, 1
    syscall
    lw $a0, h
    li $v0, 1
    syscall
    li $v0, 10
    syscall
