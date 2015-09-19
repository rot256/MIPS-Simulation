
#failing ascii art
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
# The MIPS32-Simulator

## What works
All of the tests in the folder tests. are being simulated correctly. So all of the basic instructions should
simulate correctly.


## What doesn't work
Currently syscall will only cause the simulator to stop. And not perform an actual simulation of the syscall.

## Design choices

While we were debugging, we used a rather hackish way to quickly add lines we only wanted to run if debug was enabled.
So that instead of having to write
```c
#ifdef
 DEBUG CODE
#endif
```
We could just write
```c
D CODE
```

We also chose to implement the instructions a one big switch statement. Which in hindsight isn't a terribly clean way to do it.


# Testing

In SegfaultSimulator (SS) we focused on great tests and exceptionally high code quality,
we therefore have thorough test of every possible edge case with systematic unit tests.
In all seriousness, there are 3 tests.

    fib_rec.S    : A recursive implementation of fibonacci to test the stack (Load Word & Store Word)
    arithmetic.S : Some shitty attempt at implementing a LFSR
    jumps.S      : Testing branching.

Our tests are pretty complex - because we were too lazy to test every instruction seperatly.
All three tests are passed, but this is by no means a garantee that SegfaultSimulator won't live up to it's name.

