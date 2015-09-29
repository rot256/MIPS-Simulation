# MEM data hazard, the value needed in EX stage has only just passed MEM.

# t0 should be non-zero for this to work
# t1 should get t0*2
# t2 should get t0*4

add $t1, $t0, $t0
nop # nop the EX stage to get a MEM data hazard
add $t2, $t1, $t1
nop # nop the MEM stage to let add get to WB
syscall # Detected in EX stage
nop # nop the ID stage
nop # nop the IF stage (never reached, due to inverse pipeline order)
