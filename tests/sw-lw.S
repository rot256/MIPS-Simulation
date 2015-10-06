# Post
# t0 = 0xdead
# t1 = 0xdead

li $t0, 0xdead

sw $t0, -4($sp)
lw $t1, -4($sp)
nop
syscall
nop
nop
