# Post
# v0 = 0xa

.set noreorder

li $t0, 0x5
jal end
add $v0, $t0, $t0
add $v0, $v0, $t0

end:
  nop
  syscall
  nop
  nop
