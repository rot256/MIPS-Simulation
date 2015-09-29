# v0 should *not* be set to double the value of t0.
# Also, ra should be something like 0x40001c.
# t0 must be something other than zero for this to work.
# The nops are there to get the instructions through the pipeline, one at a
# time.

.set noreorder

  jal end
  nop # branch-delay slot
  nop
  nop
  nop
  nop
  add $v0, $t0, $t0
  nop
  nop
  nop
  nop
end:
  nop # nop the MEM stage to let jal get to WB
  syscall # Detected in EX stage
  nop # nop the ID stage
  nop # nop the IF stage (never reached, due to inverse pipeline order)
