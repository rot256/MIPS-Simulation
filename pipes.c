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
    PC += INSTRUCTION_SIZE;

    instr_cnt++;
    return 0;
}

// CONTROL :
int interp_control() {
    // NOP
    if (if_id.inst == 0) {
        id_exe.mem_read = false;
        id_exe.mem_write = false;
        id_exe.reg_write = false;
        return 0;
    }

    // Set control signals
    switch(GET_OPCODE(if_id.inst)) {
        case OPCODE_LW:
            id_exe.mem_read = true;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.funct = FUNCT_ADD;

        case OPCODE_SW:
            id_exe.mem_read = false;
            id_exe.mem_write = true;
            id_exe.reg_write = false;
            id_exe.funct = FUNCT_ADD;

        default:
            return ERROR_UNKNOWN_OPCODE;
    }

    return 0;
}

// PIPELINE : Instruction decode
int interp_id() {
    // Load fields from instruction
    id_exe.rs = GET_RS(if_id.inst);
    id_exe.rs_value = regs[GET_RS(if_id.inst)];
    id_exe.rt_value = regs[GET_RT(if_id.inst)];
    id_exe.sign_ext_imm = SIGN_EXTEND(GET_IMM(if_id.inst));

    // TODO : Branching here

    // Set control signals
    return interp_control();
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
