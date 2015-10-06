# Post
# t1 = 0x258
# t2 = 0x258
# t3 = 0x1f4

.set noreorder

b start
nop

branch:
li $t3, 500
b exit
nop

start:
li $t1, 600
sw $t1, 268500992
lw $t2, 268500992
beq $t1, $t2, branch


exit:
nop
syscall
nop
nop

