# v0 remains at 0, while v1 is set to double the value of t0.
# t0 must be non-zero.
# The nops are there to get the instructions through the pipeline, one at a
# time.

.set noreorder

  beq $0, $0, true
  nop
  nop
  nop
  nop
  add $v0, $t0, $t0
  nop
  nop
  nop
  nop
true:
  add $v1, $t0, $t0
  nop
  nop
  nop
  nop
  syscall # Detected in EX stage
  nop # nop the ID stage
  nop # nop the IF stage (never reached, due to inverse pipeline order)
