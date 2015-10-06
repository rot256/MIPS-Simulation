# Post
# t0 = 0xcafe
# v0 = 0x195fc

.set noreorder

li $t0, 0xCAFE # Uhm coffee?

jal wait_for_it
add $v0, $t0, $t0
j end
nop

wait_for_it:
  jr $ra

end:
  nop
  syscall
  nop
  nop
