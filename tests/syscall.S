# This is the basic syscall instruction that should work for a pipelined
# simulator. The syscall is caught first in the EX stage, so we need a couple
# nops to get it there.

# Note, how the last nop is necessary for a *truly* pipelined simulator, but
# for our sequential implementation.

syscall # Detected in EX stage
nop # nop the ID stage
nop # nop the IF stage (never reached, due to inverse pipeline order)
