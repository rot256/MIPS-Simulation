#Testing overflow with the different adds
#This tests add, addu, addi, addiu, li, ori, lui
li $t1, 32767
li $t2, 32767
li $t3, 2147483647

addiu $t4, $t3, 32767
#addi $t5, $t3, 32767
addi $t6, $t1, 32767

#add $t1, $t1, $t3
add $t7, $t1, $t1
addu $t2, $t2, $t3

nop
#syscall

## Expected outcome
# t2 overflowed to 0x80007ffe
# t4 overflowed to 0x80007ffe
# t1, t5 are undefined
# t6 has the value 0x0000fffe
# t7 has the value 0x0000fffe