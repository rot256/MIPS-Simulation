# Post
# t0 = 0xbeef
# t1 = 0x17dde
# t2 = 0x2fbbc

li $t0, 0xbeef

add $t1, $t0, $t0
add $t2, $t1, $t1
nop
syscall
nop
nop

