.data

.text

# JUMPS!
# t0 should hold 0x33 after execution

li $t0, 0
li $t1, 1

ptr1:

or  $t0, $t0, $t1
sll $t1, $t1, 1
j ptr6

ptr2:

sll $t1, $t1, 1
or  $t0, $t0, $t1
bne $t1, 0x8, ptr4 

ptr3:

sll $t1, $t1, 1
jal ptr2

ptr4:

addu $t1, $t1, $t1
addu $t0, $t0, $t1

ptr5:

syscall

ptr6:

addu $t0, $t0, $t1
addu $t1, $t1, $t1
beq  $t0, 3, ptr3
