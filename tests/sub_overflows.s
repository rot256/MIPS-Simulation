li $t1, 100
li $t2, 50
li $t5, -2147483647
li $s1, -100
li $s2, -50

## Regular subtraction
sub $t3, $t1, $t2
## Go under 0
sub $t4, $zero, $t2
## Overflow causes exception
sub $t6, $t5, $t1

# Negative from positive
sub $t7, $t1, $s2
# Positive from negative
sub $v0, $s1, $t2
# Negative from negative
sub $v1, $s1, $s2

nop
syscall
nop
nop

