# t1 gets the value of t0 (through memory).

sw $t0, -4($sp)
lw $t1, -4($sp)
nop # nop the MEM stage to let lw get to WB
syscall # Detected in EX stage
nop # nop the ID stage
nop # nop the IF stage (never reached, due to inverse pipeline order)
