# Stack test (recursive fibonacci)
#
# If the simulator terminates on syscalls,
# the 21 should be in a0!
#
# Otherwise "21" will also be written to stdout
#
# This code also executes in the MARS simulator

.data

.text

# Calculate fib
li $a0, 3
jal fib
nop

# Print result
addu    $a0, $v0, 0
li      $v0, 1
syscall

# Exit
li $v0, 10
li $a0, 0
syscall

fib:
    # Prolog : Push return address to stack
    subu    $sp, $sp, 4
    sw      $ra, ($sp)

    # Check base case
    li      $v0, 1
    beq     $a0, 1, fib_epi
    nop
    beq     $a0, 2, fib_epi
    nop

    # Save $s0
    subu    $sp, $sp, 4
    sw      $s0, ($sp)

    # Calc fib(x-1)
    addu    $s0, $a0, 0 # Save a0
    subu    $a0, $s0, 1
    jal     fib
    nop

    # Calc fib(x-2)
    subu    $a0, $s0, 2
    addu    $s0, $v0, 0 # Save result
    jal     fib
    nop

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
