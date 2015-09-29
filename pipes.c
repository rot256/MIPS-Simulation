#include "pipes.h"
#include "mips32.h"
#include "consts.h"

static struct preg_if_id  if_id;
static struct preg_id_exe id_exe;

// PIPELINE : Instruction fetch
int interp_if() {
    if_id.inst = GET_BIGWORD(mem, PC);
    if_id.PC = PC + INSTRUCTION_SIZE;

    // TODO : PC = Branch / PC+4

    instr_cnt++;
    return 0;
}

// PIPELINE : Instruction decode
int interp_id() {

	id_exe.sign_ext_imm = SIGN_EXTEND(GET_IMM(inst))

	return 0;
}

// PIPELINE : Execute
int interp_exe() {
	return 0;
}

// PIPELINE : Memory
int interp_mem() {
	return 0;
}

// PIPELINE : Writeback
int interp_wb() {
	return 0;
}