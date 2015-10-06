li $t1, 50
li $t2, 50
beq $t1, 50, Branch1
li $t2, 60
li $t2, 50
nop
nop
b Nofall

Branch1:
bne $t2, 50, Branch2
li $t1, 40
li $t1, 30

nop
nop
b Nofall

Branch2:
li $t3, 70
beq $t1, 40, First
beq $t4, 80, Second

nop
nop
b Nofall

First:
li $t4, 80
nop
nop
b Nofall

Second:
li $t5, 100

Nofall:



# Expected outcome
# t1 should be 40
# t2 should be 60
# t3 should be 70
# t4, t5 undefined