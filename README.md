# SegfaultSimulator

```
 _____             __            _ _   _____ _                 _       _
/  ___|           / _|          | | | /  ___(_)               | |     | |
\ `--.  ___  __ _| |_ __ _ _   _| | |_\ `--. _ _ __ ___  _   _| | __ _| |_ ___  _ __
 `--. \/ _ \/ _` |  _/ _` | | | | | __|`--. \ | '_ ` _ \| | | | |/ _` | __/ _ \| '__|
/\__/ /  __/ (_| | || (_| | |_| | | |_/\__/ / | | | | | | |_| | | (_| | || (_) | |
\____/ \___|\__, |_| \__,_|\__,_|_|\__\____/|_|_| |_| |_|\__,_|_|\__,_|\__\___/|_|
             __/ |
            |___/
```

## Design choices

### Overflow detection

ADD and SUB implements overflow detection, if an overflow in encountered in either of these instructions the program terminates with an error.

### Branching

We handle all I-Type branching in the ID stage (which reflects many implementations) this means that we avoid flushing to some degree since only the delay slot is fetched.
However we do not avoid flushing all together due to the JR instruction, which is an R-Type instruction which we process in EX (unlike the walk-though implementation - which mixes I- and R-Type instructions in ID).
When JR is in EX, its delay slot instruction is in ID (not IF), which means that we will get 2 delay slots, to avoid this we have no choice but to insert a nop.

To do this we created the "flush" variable, this is outside the pipelined registers, since it is not a register - its models a control line (its value is reset every cycle).
"flush" simply signals that IF should load a NOP instead of the next instruction - the same way we do for Load-Use hazards.

Another side effect of this design is that the edge case for JAL is avoided:

	"You will need to handle another case in addition to those discussed in [COD5e]. A jal
	instruction might be on it’s way to the WB stage, when we hit a jr instruction in the ID
	stage. In this case, the mem_wb.alu_res should be forwarded to id_ex.jump_target instead of id_ex.rs_value." - Assignment

Since we intrepid JR in EX (to separate I-Type and R-Type completely).

Handling branches in ID does have one major drawback, if the branch depends on the instructions immediately preceding it we must bubble. 
When the branch instruction is in ID stage the result it depends on will be in EX, we can not forward - for this would mean going backwards in time 
(the result will be available when the current cycle has completed).
We handle this by inserting a nop in IF when we detect this problem - there may be a smarter way to do this.

### Hazards

All hazards which can be mitigated using forwarding is handled in the **forward()** function.
Load-use hazards are handled directly in **interp_if()**

## Tests

In the tests folder. We have a bunch of tests checking if the basic instructions work as intended.
Some of the things we also test for are:

* We test whether our simulate correctly reports errors, when add,addi,sub overflow.
* We test whether instructions in the branch delay slot, gets correctly executed.
* We test if the basic instructions works as intended

To be able to test more effectively we also. We made a python script to run our tests automatic - it's not beautiful, it's not flawless but it gets the job done. 
It works by comparing the post conditions in our assembly file. To the output of our simulator.
