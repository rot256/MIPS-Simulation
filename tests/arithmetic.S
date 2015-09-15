.data

.text

# Just arithmetic strangeness
# t0-t3 holds inital state
# v0 holds round counter

li $t0, 0
li $t1, 0xACE1
li $v0, 20      # Iterations

ptr1:

# Generate bit
li   $t2, 0
addu $t4, $t1, 0
addu $t2, $t2, $t4
srl  $t4, $t4, 2
addu $t2, $t2, $t4
srl  $t4, $t4, 1
addu $t2, $t2, $t4
srl  $t4, $t4, 2
addu $t2, $t2, $t4

# Progress
srl  $t1, $t1, 1
sll  $t4, $t2, 15
or   $t1, $t1, $t4

# Fetch last bit and add to accumulator
sll  $t2, $t2, 31
srl  $t2, $t2, 31
sll  $t0, $t0, 1
or   $t0, $t0, $t2

# Repeat
subu $v0, $v0, 1
bne  $v0, 0, ptr1
ori  $t0, $t0, 0xBEEF

syscall
