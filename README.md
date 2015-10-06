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
However we do not avoid flushing all together due to the JR instruction, which is an R-Type instruction which we process in EX (unlike the walkthough implementation - which mixes I- and R-Type instructions in ID).
When JR is in EX, its delayslot instruction is in ID (not IF), which means that we will get 2 delay slots, to avoid this we have no choice but to insert a nop.

To do this we created the "flush" variable, this is outside the pipelined registers, since it is not a register - its models a control line (its value is reset every cycle).
"flush" simply signals that IF should load a NOP insted of the next instruction - the same way we do for Load-Use hazards.

Another side effect of this design is that the edge case for JAL is avoided:

	"You will need to handle another case in addition to those discussed in [COD5e]. A jal
	instruction might be on it’s way to the WB stage, when we hit a jr instruction in the ID
	stage. In this case, the mem_wb.alu_res should be forwarded to id_ex.jump_target instead of id_ex.rs_value." - Assignment

Since we interped JR in EX (to seperate I-Type and R-Type completely).

### Hazards

All hazards which can be mitigated using forwarding is handled in the **forward()** function.
Load-use hazards are handled directly in **interp_if()**

## Tests