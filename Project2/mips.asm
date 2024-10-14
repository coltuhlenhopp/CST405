.data
newline: .asciiz "\n"
.text
.globl main
main:
    li $t0, 25
    li $t1, 87
    move $a0, $t0
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, newline
    syscall
    move $a0, $t1
    li $v0, 1
    syscall
    li $v0, 4
    la $a0, newline
    syscall
    li $v0, 10
    syscall