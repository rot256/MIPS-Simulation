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
    if_id.nPC = PC + INSTRUCTION_SIZE;

    // TODO : PC = Branch / PC+4
    PC += INSTRUCTION_SIZE;

    D printf("DEBUG   - Loaded instruction 0x%08x\n", if_id.inst);
    D printf("DEBUG   - Next PC 0x%x\n", PC);

    instr_cnt++;
    return 0;
}

// CONTROL :
int interp_control() {
    // Set control signals
    switch(GET_OPCODE(if_id.inst)) {
        // Memory
        case OPCODE_LW:
            D printf("DEBUG   - LW instruction\n");
            id_exe.mem_read = true;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;
            id_exe.mem_to_reg = true;

            id_exe.funct = FUNCT_ADD;
            id_exe.reg_dst = GET_RT(if_id.inst);
            break;

        case OPCODE_SW:
            D printf("DEBUG   - SW instruction\n");
            id_exe.mem_read = false;
            id_exe.mem_write = true;
            id_exe.reg_write = false;
            id_exe.alu_src = true;
            id_exe.mem_to_reg = false;

            id_exe.funct = FUNCT_ADD;
            id_exe.reg_dst = GET_RT(if_id.inst);
            break;

        // R-Type instructions
        case OPCODE_R:
            D printf("DEBUG   - R-type instruction\n");
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = false;
            id_exe.mem_to_reg = false;

            id_exe.funct = GET_FUNCT(if_id.inst);
            id_exe.reg_dst = GET_RD(if_id.inst);
            break;

        // Arithmetic
        case OPCODE_ADDIU:
            D printf("DEBUG   - Add Immediate Unsigned\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;                  // Source is imm
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            id_exe.funct = FUNCT_ADDU;              // Add no overflow
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        case OPCODE_SLTI:
            D printf("DEBUG   - Set on Less Than Immediate (signed)\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;                  // Source is imm
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            id_exe.funct = FUNCT_SLT;               // Set on less than (signed)
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        case OPCODE_ANDI:
            D printf("DEBUG   - AND with Immediate\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;                  // Source is imm
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            id_exe.funct = FUNCT_AND;               // Bitwise AND
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        case OPCODE_ORI:
            D printf("DEBUG   - OR with Immediate\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;                  // Source is imm
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            id_exe.funct = FUNCT_OR;                // Bitwise OR
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        case OPCODE_LUI:
            D printf("DEBUG   - XOR with Immediate\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;                  // Source is imm
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            id_exe.funct = FUNCT_SLL;               // Logical shift
            id_exe.shamt = 16;                      // Shift by 16
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        // Branching
        case OPCODE_BEQ:
            D printf("DEBUG   - Branch on Equal [%d]\n", (id_exe.rs_value == id_exe.rt_value) & 1);

            // JUMP
            if (id_exe.rs_value == id_exe.rt_value) {
                PC = if_id.nPC + id_exe.sign_ext_imm;
            }

            // Bubble
            id_exe.mem_write = false;
            id_exe.reg_write = false;

            break;

        case OPCODE_BNE:
            D printf("DEBUG   - Branch on Not Equal [%d]\n", (id_exe.rs_value != id_exe.rt_value) & 1);

            // JUMP
            if (id_exe.rs_value == id_exe.rt_value) {
                PC = if_id.nPC + id_exe.sign_ext_imm;
            }

            // Bubble
            id_exe.mem_write = false;
            id_exe.reg_write = false;

            break;

        case OPCODE_J:
            D printf("DEBUG   - Jump\n");

            // JUMP
            PC = (if_id.nPC & MS_4B) | (GET_ADDRESS(if_id.inst) << 2);

            // Bubble
            id_exe.mem_write = false;
            id_exe.reg_write = false;
            break;

        case OPCODE_JAL:
            D printf("DEBUG   - Jump And Link\n");

            // JUMP
            PC = (if_id.nPC & MS_4B) | (GET_ADDRESS(if_id.inst) << 2);

            // Write to RA
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.mem_to_reg = false;

            id_exe.reg_dst = r_ra;
            id_exe.funct = FUNCT_ADD;
            // TODO : Check this
            id_exe.rs_value = if_id.nPC;
            id_exe.rt_value = 0x4;
            break;

        default:
            D printf("DEBUG   - Unknown opcode 0x%08x\n", GET_OPCODE(if_id.inst));
            return ERROR_UNKNOWN_OPCODE;
    }

    // Debugging
    D printf("DEBUG   - MemRead  = [%d]\n", id_exe.mem_read);
    D printf("DEBUG   - MemWrite = [%d]\n", id_exe.mem_write);
    D printf("DEBUG   - RegWrite = [%d]\n", id_exe.reg_write);
    D printf("DEBUG   - ALUSrc   = [%d]\n", id_exe.alu_src);
    D printf("DEBUG   - MemToReg = [%d]\n", id_exe.mem_to_reg);
    D printf("DEBUG   - RegDst   = [%d]\n", id_exe.reg_dst);
    D printf("DEBUG   - FUNCT    = [0x%x]\n", id_exe.funct);
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
    id_exe.shamt = GET_SHAMT(if_id.inst);

    // Set control signals
    return interp_control();
}

// ALU
int alu() {
    // Select operands
    uint32_t op1 = id_exe.rs_value;
    uint32_t op2 = id_exe.alu_src ? id_exe.sign_ext_imm : id_exe.rt_value;
    D printf("DEBUG   - SrcVal1 = [0x%x]\n", op1);
    D printf("DEBUG   - SrcVal2 = [0x%x]\n", op2);

    // Complete compuation
    switch(id_exe.funct) {
        case FUNCT_AND:
            D printf("DEBUG   - OP = [FUNCT_AND] (Bitwise AND)\n");
            exe_mem.alu_res = op1 & op2;
            break;

        case FUNCT_NOR:
            D printf("DEBUG   - OP = [FUNCT_NOR] (Bitwise NOT OR)\n");
            exe_mem.alu_res = !(op1 | op2);
            break;

        case FUNCT_OR:
            D printf("DEBUG   - OP = [FUNCT_OR] (Bitwise OR)\n");
            exe_mem.alu_res = op1 | op2;
            break;

        case FUNCT_SLL:
            D printf("DEBUG   - OP = [FUNCT_SLL] (Shift left)\n");
            exe_mem.alu_res = op1 << id_exe.shamt;
            break;

        case FUNCT_SLT:
            D printf("DEBUG   - OP = [FUNCT_SLT] (Set on less than - signed)\n");
            exe_mem.alu_res = (int32_t) op1 < (int32_t) op2 ? 1 : 0;
            break;

        case FUNCT_SLTU:
            D printf("DEBUG   - OP = [FUNCT_SLTU] (Set on less than - unsigned)\n");
            exe_mem.alu_res = op1 < op2 ? 1 : 0;
            break;

        case FUNCT_SRL:
            D printf("DEBUG   - OP = [FUNCT_SRL] (Shift right)\n");
            exe_mem.alu_res = op1 >> id_exe.shamt;
            break;

        case FUNCT_SUB:
            D printf("DEBUG   - OP = [FUNCT_SUB] (Subtraction - check overflow)\n");
            exe_mem.alu_res = op1 - op2;

            // Underflow detection (TODO : Sanity check - dont code late)
            if ((uint32_t) exe_mem.alu_res > (uint32_t) op1) return ERROR_OVERFLOW;
            break;

        case FUNCT_SUBU:
            D printf("DEBUG   - OP = [FUNCT_SUBU] (Subtraction - no overflow)\n");
            exe_mem.alu_res = op1 - op2;
            break;

        case FUNCT_ADD:
            D printf("DEBUG   - OP = [FUNCT_ADD] (Add - check overflow)\n");
            exe_mem.alu_res = op1 + op2;

            // Overflow detection
            if ((uint32_t) exe_mem.alu_res < (uint32_t) op1) return ERROR_OVERFLOW;
            if ((uint32_t) exe_mem.alu_res < (uint32_t) op2) return ERROR_OVERFLOW;
            break;

        case FUNCT_ADDU:
            D printf("DEBUG   - OP = [FUNCT_ADDU] (Add - no overflow)\n");
            exe_mem.alu_res = op1 + op2;
            break;

        case FUNCT_SYSCALL:
            D printf("DEBUG   - OP = [FUNCT_SYSCALL] (Syscall)\n");
            return SIG_HALT_PROGRAM;

        default:
            D printf("DEBUG   - OP = UNKNOWN\n");
            return ERROR_UNKNOWN_FUNCT;
    }
    D printf("DEBUG   - AluRes = [0x%x]\n", exe_mem.alu_res);
    return 0;

}

// PIPELINE : Execute
int interp_exe() {
    D printf("DEBUG : Pipeline : Execution\n");

    // Carry forward
    exe_mem.mem_read = id_exe.mem_read;
    exe_mem.mem_write = id_exe.mem_write;
    exe_mem.reg_write = id_exe.reg_write;
    exe_mem.mem_to_reg = id_exe.mem_to_reg;

    exe_mem.rt = id_exe.rt;
    exe_mem.rt_value = id_exe.rt_value;
    exe_mem.reg_dst = id_exe.reg_dst;

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

    // Carry forward
    mem_wb.reg_write = exe_mem.reg_write;
    mem_wb.mem_to_reg = exe_mem.mem_to_reg;

    mem_wb.rt = exe_mem.rt;
    mem_wb.alu_res = exe_mem.alu_res;
    mem_wb.reg_dst = exe_mem.reg_dst;

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
    if (!mem_wb.reg_dst) return 0;

    D printf("DEBUG   - Writing to register bank\n");
    D printf("DEBUG   - MemToReg = [%d]\n", mem_wb.mem_to_reg);

    regs[mem_wb.reg_dst] = mem_wb.mem_to_reg ? mem_wb.read_data : mem_wb.alu_res;

    return 0;
}
