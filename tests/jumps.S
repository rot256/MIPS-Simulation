# Post
# t0 = 0x73

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

j ptr7
addu $t1, $t1, $t1
addu $t0, $t0, $t1

ptr8:

addu $t1, $t1, $t1
addu $t0, $t0, $t1
nop
syscall
nop
nop

ptr7:

j ptr8

ptr6:

addu $t0, $t0, $t1
addu $t1, $t1, $t1
beq  $t0, 3, ptr3

nop
syscall
nop
nop
