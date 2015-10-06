# Post
# t0 = 0x4
# t1 = 0x4
# t2 = 0x8

.set noreorder

li $t0, 0x4
sw $t0, -4($sp)
lw $t1, -4($sp)
add $t2, $t1, $t1
nop
syscall
nop
nop
