# v0 *should* be set to double the value of t0.
# Also, ra should be something like 0x400020.
# t0 must be something other than zero for this to work.

# j and jr must work for this to work.

  jal wait_for_it
  add $v0, $t0, $t0
  j end
wait_for_it:
  jr $ra
end:
  nop # nop the MEM stage to let add get to WB
  syscall # Detected in EX stage
  nop # nop the ID stage
  nop # nop the IF stage (never reached, due to inverse pipeline order)
