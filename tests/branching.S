# Post
# t1 = 0x28
# t2 = 0x3c
# t3 = 0x46
# t4 = 0x50
# t5 = 0x64

.set noreorder

li $t1, 50
li $t2, 50
beq $t1, 50, Branch1
addiu $t2, $zero, 60
addiu $t2, $zero, 50
nop
nop
b Nofall
nop

Branch1:

bne $t2, 50, Branch2
addiu $t1, $zero, 40
addiu $t1, $zero, 30

nop
nop
b Nofall

Branch2:
addiu $t3, $zero, 70
beq $t1, 40, First
nop
beq $t4, 80, Second

nop
nop
b Nofall

First:
addiu $t4, $zero, 80
nop
nop
b Nofall

Second:
addiu $t5, $zero, 100

Nofall:

nop
syscall
nop
nop
