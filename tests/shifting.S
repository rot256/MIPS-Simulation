# Post
# t1 = 0x32
# t2 = 0x320
# t3 = 0x3
# t4 = 0x80000000
# t5 = 0x0


li $t1, 50
sll $t2, $t1, 4
srl $t3, $t1, 4

sll $t4, $t1, 30
srl $t5, $t1, 8


# Expected outcome
# t1 = 50
# t2 = 800
# t3 = 3
# t4 = -2147483648
# t5 = 0

nop
syscall
nop
nop

