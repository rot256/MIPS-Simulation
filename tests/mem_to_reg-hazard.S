# A MEM hazard where the instruction exiting the MEM stage has read from memory
# (instead of having computed a value using the ALU in the EX stage).
#
# t1 gets the value of t0 (through memory)
# v0 gets t1 + t1
# Post
# t0 = 0x1337
# t1 = 0x1337
# v0 = 0x266e
# v1 = 0x2600

# Setup

li $v1, 0x2600
li $t0, 0x1337
li $t1, 0x1300

# Test

sw  $t0, -4($sp)  # | IF | ID | EX | MEM | WB  |
lw  $t1, -4($sp)  #      | IF | ID | EX  | MEM | t1 loaded from memory
nop               #           | IF | ID  | EX  |
add $v0, $t1, $t1 #                | IF  | ID  | MEM RS + MEM RT hazard

nop
nop
nop
nop
syscall
nop
nop
nop
nop
