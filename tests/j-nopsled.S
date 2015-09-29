# v0 should *not* be set to double the value of t0.
# t0 must be something other than zero for this to work.
# The nops are there to get the instructions through the pipeline, one at a
# time.

.set noreorder

  j end
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
  # Nothing to nop here..
  syscall # Detected in EX stage
  nop # nop the ID stage
  nop # nop the IF stage (never reached, due to inverse pipeline order)
