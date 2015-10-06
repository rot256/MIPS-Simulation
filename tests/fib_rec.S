# Stack test (recursive fibonacci)
#
# 21 should be in v0 after termination
#
# This code also executes in the MARS simulator

.data

.text

# Calculate fib
li $a0, 8
jal fib

# Stop
syscall

fib:
    # Prolog : Push return address to stack
    subu    $sp, $sp, 4
    sw      $ra, ($sp)

    # Check base case
    li      $v0, 1
    beq     $a0, 1, fib_epi
    beq     $a0, 2, fib_epi

    # Save $s0
    subu    $sp, $sp, 4
    sw      $s0, ($sp)

    # Calc fib(x-1)
    addu    $s0, $a0, 0 # Save a0
    subu    $a0, $s0, 1
    jal     fib

    # Calc fib(x-2)
    subu    $a0, $s0, 2
    addu    $s0, $v0, 0 # Save result
    jal     fib

    # Calc fib(x-2) + fib(x-1)
    addu    $v0, $v0, $s0

    # Restore $s0
    lw      $s0, ($sp)
    addu    $sp, $sp, 4

fib_epi:

    # Epilog : Jump to stored return address
    lw      $ra, ($sp)
    addu    $sp, $sp, 4
    jr      $ra

nop
syscall
nop
nop

