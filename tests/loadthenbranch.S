# Post
# t1 = 0x258
# t2 = 0x258
# t3 = 0x1f4
# t4 = 0

.set noreorder

li $t4, 0
b start
nop

mine:
li $t4, 1

branch:
li $t3, 500
b exit
nop

start:
li $t1, 600
sw $t1, -4($sp)
lw $t2, -4($sp)
beq $t1, $t2, branch


exit:
nop
syscall
nop
nop

