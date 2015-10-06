# v0 *should* be set to double the value of t0.
# Also, ra should be something like 0x400020.
# t0 must be something other than zero for this to work.

# j and jr must work for this to work.

# The nops are there to get the instructions through the pipeline, one at a
# time.

.set noreorder

  jal wait_for_it
  nop # branch-delay slot
  nop
  nop
  nop
  nop
  add $v0, $t0, $t0
  # Yo, I'ma let you finish
  nop
  nop
  nop
  nop
  j end
  nop # branch-delay slot
  nop
  nop
  nop
  nop
wait_for_it:
  nop
  nop
  nop
  nop
  jr $ra
  nop # branch-delay slot
  nop
  nop
  nop
  nop
end:
  # Nothing to nop here.
  syscall # Detected in EX stage
  nop # nop the ID stage
  nop # nop the IF stage (never reached, due to inverse pipeline order)
