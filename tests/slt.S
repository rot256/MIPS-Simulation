# Post
# t1 = 0x32
# t2 = 0x3c
# t3 = 0x1
# t4 = 0x0

li $t1, 50
li $t2, 60
li $t4, 20
slt $t3, $t1, $t2
slt $t4, $t2, $t1

# Expected outcome
# t1 = 50
# t2 = 60
# t3 = 1
# t4 = 0
nop
syscall
nop
nop

