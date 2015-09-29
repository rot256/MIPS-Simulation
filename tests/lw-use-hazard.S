# lw, use hazard
#
# t0 must be non-zero for this to work.
# t1 gets the value of t0 (through memory)
# t2 gets t0 * 2
# You should see an extra clock-cycle wasted.

sw $t0, -4($sp)
lw $t1, -4($sp)       # lw t1
add $t2, $t1, $t1     # use t1
nop # nop the MEM stage to let add get to WB
syscall # Detected in EX stage
nop # nop the ID stage
nop # nop the IF stage (never reached, due to inverse pipeline order)
