# v0 remains at 0, while v1 is set to double the value of t0.
# t0 must be non-zero.

  beq $0, $0, true
  nop # branch-delay slot
  add $v0, $t0, $t0
true:
  add $v1, $t0, $t0
  nop # nop the MEM stage to let add get to WB
  syscall # Detected in EX stage
  nop # nop the ID stage
  nop # nop the IF stage (never reached, due to inverse pipeline order)
