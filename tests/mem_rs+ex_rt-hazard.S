# An RS hazard with the MEM stage, an RT hazard with the EX stage.
# t2 gets 2 * t0
# t3 gets 2 * t1
# t4 gets t2 + t3 (in that order)
# t0 and t1 must be non-zero, and different(!) for this to have a clear effect.
#
# Post
# t0 = 0x10
# t1 = 0x8
# t2 = 0x20
# t3 = 0x10
# t4 = 0x30

# Setup

li $t0, 0x10
li $t1, 0x08

# Test

add $t2, $t0, $t0 # | IF | ID | EX | MEM |
add $t3, $t1, $t1 #      | IF | ID | EX  |
add $t4, $t2, $t3 #           | IF | ID  | MEM RS hazard + EX RT hazard

nop
nop
nop
nop
syscall
nop
nop
nop
nop
