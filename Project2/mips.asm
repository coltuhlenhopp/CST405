.data
newline: .asciiz "\n"  # String for newline
h: .word 0              # Variable h
g: .word 0              # Variable g
x: .word 0              # Variable x

.text
.globl main

main:
    li $t0, 5          # Load 5 into t0
    li $t1, 3          # Load 3 into t1 (not used further)

    sw $t0, h          # Store 5 into h
    sw $t0, g          # Store 5 into g
    sw $t0, x          # Store 5 into x

    lw $t2, h          # Load h into t2 (t2 = 5)
    lw $t3, g          # Load g into t3 (t3 = 5)
    add $t4, $t2, $t3  # Add t2 and t3 (t4 = 5 + 5 = 10)
    sw $t4, h          # Store 10 back into h

    lw $a0, x          # Load x into a0 (a0 = 5)
    li $v0, 1          # Prepare to print integer
    syscall             # Print x

    # Print newline after x
    la $a0, newline     # Load address of newline
    li $v0, 4          # Prepare to print string
    syscall             # Print newline

    lw $a0, h          # Load h into a0 (a0 = 10)
    li $v0, 1          # Prepare to print integer
    syscall             # Print h

    # Exit program
    li $v0, 10         # Prepare to exit
    syscall             # Exit
