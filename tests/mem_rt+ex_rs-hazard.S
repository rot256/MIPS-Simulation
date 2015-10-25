# An RT hazard with the MEM stage, an RS hazard with the EX stage.
# t2 gets 2 * t0
# t3 gets 2 * t1
# t4 gets t3 + t2 (in that order)
# t0 and t1 must be non-zero, and different(!) for this to have a clear effect.
#
# Post
# t2 = 0x2
# t3 = 0x4
# t4 = 0x6

# Setup

li $t0, 1
li $t1, 2

# Test

add $t2, $t0, $t0 # | IF | ID | EX | MEM | (the RT register is in MEM)
add $t3, $t1, $t1 #      | IF | ID | EX  | (the RS register is in EX)
add $t4, $t3, $t2 #           | IF | ID  | MEM RT hazard + EX RS hazard

nop
nop
nop
nop
syscall
nop
nop
nop
nop
