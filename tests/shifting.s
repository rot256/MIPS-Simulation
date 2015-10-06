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