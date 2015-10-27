# Testing andi and ori
# Post
# t0 = 0x1337
# t1 = 0x100000
# t2 = 0x101337
# t3 = 0x0
# t4 = 0x1337
# t5 = 0xdbff
# t6 = 0x1337

# Setup

li $t0, 0x1337
li $t1, 0x100000

# Tests

ori $t2, $t1, 0x1337
andi $t3, $t1, 0x1337
andi $t4, $t0, 0x1337
ori $t5, $t0, 0xcafe
and $t6, $t2, $t0

# Exit

nop
syscall
nop
nop
nop
nop
