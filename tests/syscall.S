# syscall
#
# The simulator should exit gracefully when meeting a `syscall` instruction.
# Especially if there are no other instructions but a `syscall` instruction.

syscall

# Expected simulator exit code: 0
# Expected simulator output:
#
# Ran 1 cycle(s).
# pc = 0x40001c
# at = 0x49fffc
# v0 = 0x0
# v1 = 0x0
# t0 = 0x0
# t1 = 0x0
# t2 = 0x0
# t3 = 0x0
# t4 = 0x2a
# t5 = 0x0
# t6 = 0x0
# t7 = 0x0
# sp = 0x49fffc
# ra = 0x0
