# MEM data hazard that should forward read_data instead of alu_res.

# t0 should be non-zero for this to work
# t1 should get the value of t0 (through memory)
# t2 should get the value of t0*2

sw $t0, -4($sp)
lw $t1, -4($sp)
nop # nop the MEM stage to let lw get to WB
add $t2, $t1, $t1
nop # nop the MEM stage to let add get to WB
syscall # Detected in EX stage
nop # nop the ID stage
nop # nop the IF stage (never reached, due to inverse pipeline order)
