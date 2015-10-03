#include "pipes.h"
#include "mips32.h"
#include "consts.h"
#include "stdio.h"

static struct preg_if_id   if_id;
static struct preg_id_exe  id_exe;
static struct preg_exe_mem exe_mem;
static struct preg_mem_wb  mem_wb;

// PIPELINE : Instruction fetch
int interp_if() {
    D printf("DEBUG : Pipeline : Instruction Fetch\n");

    if_id.inst = GET_BIGWORD(mem, PC);
    if_id.PC = PC + INSTRUCTION_SIZE;

    // TODO : PC = Branch / PC+4
    PC += INSTRUCTION_SIZE;

    D printf("DEBUG   - Loaded instruction 0x%08x\n", if_id.inst);
    D printf("DEBUG   - Next PC 0x%x\n", PC);

    instr_cnt++;
    return 0;
}

// CONTROL :
int interp_control() {
    // NOP
    if (if_id.inst == 0) {
        D printf("DEBUG   - NOP instruction\n");
        id_exe.mem_read = false;
        id_exe.mem_write = false;
        id_exe.reg_write = false;
        return 0;
    }

    // Set control signals
    switch(GET_OPCODE(if_id.inst)) {
        case OPCODE_LW:
            D printf("DEBUG   - LW instruction\n");
            id_exe.mem_read = true;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.funct = FUNCT_ADD;
            break;

        case OPCODE_SW:
            D printf("DEBUG   - SW instruction\n");
            id_exe.mem_read = false;
            id_exe.mem_write = true;
            id_exe.reg_write = false;
            id_exe.funct = FUNCT_ADD;
            break;

        default:
            D printf("DEBUG   - Unknown opcode 0x%08x\n", GET_OPCODE(if_id.inst));
            return ERROR_UNKNOWN_OPCODE;
    }

    // Debugging
    D printf("DEBUG   - MemRead  = [%d]\n", id_exe.mem_read);
    D printf("DEBUG   - MemWrite = [%d]\n", id_exe.mem_write);
    D printf("DEBUG   - RegWrite = [%d]\n", id_exe.reg_write);
    return 0;
}

// PIPELINE : Instruction decode
int interp_id() {
    D printf("DEBUG : Pipeline : Instruction Decode\n");

    // Load fields from instruction
    id_exe.rt = GET_RT(if_id.inst);
    id_exe.rs_value = regs[GET_RS(if_id.inst)];
    id_exe.rt_value = regs[GET_RT(if_id.inst)];
    id_exe.sign_ext_imm = SIGN_EXTEND(GET_IMM(if_id.inst));
    id_exe.funct = GET_FUNCT(if_id.inst);

    // TODO : Branching here

    // Set control signals
    return interp_control();
}

// ALU
int alu() {
    switch(id_exe.funct) {
        case FUNCT_SLL:
            // TODO : Implement
            break;
        case FUNCT_ADD:
            exe_mem.alu_res = id_exe.rs_value + id_exe.sign_ext_imm;
            // if ((uint32_t) exe_mem.alu_res < (uint32_t) id_exe.rs_value) return ERROR_OVERFLOW;
            // if ((uint32_t) exe_mem.alu_res < (uint32_t) id_exe.sign_ext_imm) return ERROR_OVERFLOW;
            break;
        default:
            return ERROR_UNKNOWN_FUNCT;
    }
    return 0;

}

// PIPELINE : Execute
int interp_exe() {
    D printf("DEBUG : Pipeline : Execution\n");

    // Carry

    exe_mem.mem_read = id_exe.mem_read;
    exe_mem.mem_write = id_exe.mem_write;
    exe_mem.reg_write = id_exe.reg_write;
    exe_mem.rt = id_exe.rt;
    exe_mem.rt_value = id_exe.rt_value;

    // TODO : Add data hazard detection

    // Debugging
    D printf("DEBUG   - MemRead  = [%d]\n", exe_mem.mem_read);
    D printf("DEBUG   - MemWrite = [%d]\n", exe_mem.mem_write);
    D printf("DEBUG   - RegWrite = [%d]\n", exe_mem.reg_write);

    // Perform ALU operations
    return alu();
}

// PIPELINE : Memory
int interp_mem() {
    D printf("DEBUG : Pipeline : Memory\n");

    // Carry
    mem_wb.reg_write = exe_mem.reg_write;
    mem_wb.rt = exe_mem.rt;

    // Read from memory
    if (exe_mem.mem_read) {
        mem_wb.read_data = GET_BIGWORD(mem, exe_mem.alu_res);
        D printf("DEBUG   - Read value 0x%x from address 0x%x [LW]\n", mem_wb.read_data, exe_mem.alu_res);
    }

    // Write to memory
    if (exe_mem.mem_write) {
        SET_BIGWORD(mem, exe_mem.alu_res, exe_mem.rt_value);
        D printf("DEBUG   - Store value 0x%x into 0x%x [SW]\n", exe_mem.rt_value, exe_mem.alu_res);
    }
    return 0;
}

// PIPELINE : Writeback
int interp_wb() {
    D printf("DEBUG : Pipeline : Writeback\n");

    if (!mem_wb.reg_write) return 0;

    D printf("DEBUG   - Writing to register\n");

    // Write to destination register
    regs[mem_wb.rt] = mem_wb.read_data;

    // Make sure regs[0] is always zero
    regs[0] = 0;
    return 0;
}
