# v0 gets the value of t0 plus t1.

add $v0, $t0, $t1
nop # nop the MEM stage to let add get to WB
syscall # Detected in EX stage
nop # nop the ID stage
nop # nop the IF stage (never reached, due to inverse pipeline order)
