#include "pipes.h"
#include "mips32.h"
#include "consts.h"
#include "stdio.h"

static struct preg_if_id   if_id;
static struct preg_id_exe  id_exe;
static struct preg_exe_mem exe_mem;
static struct preg_mem_wb  mem_wb;

/* Code attempts to model the processor layout on page 320 (Patterson & Hennessy)
 * As closly as possible (within reasonable limits)
 */

static bool flush;              // Signal IF to fetch a NOP
static bool jump;               // Control signal to PC mux (left) (TODO : Rename)
static uint32_t jump_target;    // Jump address bus (TODO : Rename)

// Run cycle
int cycle() {
    int ret;

    // NOTE : These are lines, unlike the pipeline registers they fall to low when not held high
    // In the real world all states happen in parallel (thats the point)
    jump = false;
    flush = false;

    // Run pipeline
    interp_wb();
    interp_mem();
    if ((ret = interp_exe())) return ret;
    if ((ret = interp_id())) return ret;
    interp_if();

    // Hazard detection
    forward();

    // Update PC (could be moved to IF)
    if (jump)  {
        D printf("DEBUG : Cycle : Perform jump : Target=[0x%x]\n", jump_target);
        PC = jump_target;
    }
    return ret;
}

// PIPELINE : Instruction fetch
void interp_if() {
    D printf("DEBUG : Pipeline : Instruction Fetch\n");

    if_id.next_pc = PC + INSTRUCTION_SIZE;
    if_id.inst = GET_BIGWORD(mem, PC);

    if (flush) {
        D printf("DEBUG   - Insert NOP\n");
        if_id.inst = 0x0;
        return;
    }

    // Check Load-use hazards
    // NOTE : This is more strict than it need be, for instance an I-Type instruction will
    // use RS as an operand and RT as destination - and it does not matter what value the destination holds.
    // In reality, who cares?
    D printf("DEBUG   - Loaded instruction 0x%08x\n", if_id.inst);
    if (id_exe.mem_read) {
        if (GET_RS(if_id.inst) == id_exe.rt) {
            D printf("DEBUG   - Load-use hazard detected for RS\n");
            if_id.inst = 0x0;
            return;
        } else if (GET_RT(if_id.inst) == id_exe.rt) {
            D printf("DEBUG   - Load-use hazard detected for RT\n");
            if_id.inst = 0x0;
            return;
        }
    }

    // Carry next PC
    PC = if_id.next_pc;
    instr_cnt++;

    D printf("DEBUG   - Next PC 0x%x\n", PC);
    return;
}

// CONTROL :
int interp_control() {
    // Set control signal
    switch(GET_OPCODE(if_id.inst)) {
        // Memory
        case OPCODE_LW:
            D printf("DEBUG   - LW instruction\n");

            // Control
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

            // Control
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

            // Control
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
            D printf("DEBUG   - Add Immediate (ignore overflow)\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;                  // Source is imm
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            id_exe.funct = FUNCT_ADDU;              // Add no overflow
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        case OPCODE_ADDI:
            D printf("DEBUG   - Add Immediate (check overflow)\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;                  // Source is imm
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            id_exe.funct = FUNCT_ADD;               // Add with overflow
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

        case OPCODE_SLTIU:
            D printf("DEBUG   - Set on Less Than Immediate (unsigned)\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.alu_src = true;                  // Source is imm
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            id_exe.funct = FUNCT_SLTU;              // Set on less than (signed)
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        case OPCODE_ANDI:
            D printf("DEBUG   - AND with Immediate\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            // Insert raw immidate into RT (kinda hackish)
            id_exe.alu_src = false;                 // Source is imm (but not sign extended)
            id_exe.rt_value = GET_IMM(if_id.inst);  // Raw immidate
            id_exe.rt = 0;                          // Avoid hazard detection fucking us over

            id_exe.funct = FUNCT_AND;               // Bitwise AND
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        case OPCODE_ORI:
            D printf("DEBUG   - OR with Immediate\n");
            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.mem_to_reg = false;              // We want the output of the ALU

            // Insert raw immidate into RT (kinda hackish)
            id_exe.alu_src = false;                 // Source is imm (but not sign extended)
            id_exe.rt_value = GET_IMM(if_id.inst);  // Raw immidate
            id_exe.rt = 0;                          // Avoid hazard detection fucking us over

            id_exe.funct = FUNCT_OR;                // Bitwise OR
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        case OPCODE_LUI:
            D printf("DEBUG   - Load Upper Immediate\n");

            // Control
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.mem_to_reg = false;              // We want the output of the ALU
            id_exe.alu_src = true;                  // Source is imm

            id_exe.funct = FUNCT_SLL;               // Logical shift
            id_exe.shamt = 16;                      // Shift by 16
            id_exe.reg_dst = GET_RT(if_id.inst);    // Destination is in RT
            break;

        // Branching
        case OPCODE_BEQ:
            D printf("DEBUG   - Branch on Equal [%d]\n", (id_exe.rs_value == id_exe.rt_value) & 1);

            // JUMP
            if (id_exe.rs_value == id_exe.rt_value) {
                // TODO : Detect overflow
                jump = true;
                jump_target = if_id.next_pc + (id_exe.sign_ext_imm << 2);
            }

            // Bubble
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = false;

            break;

        case OPCODE_BNE:
            D printf("DEBUG   - Branch on Not Equal [%d]\n", (id_exe.rs_value != id_exe.rt_value) & 1);

            // JUMP
            if (id_exe.rs_value != id_exe.rt_value) {
                // TODO : Detect overflow
                jump = true;
                jump_target = if_id.next_pc + (id_exe.sign_ext_imm << 2);
            }

            // Bubble
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = false;

            break;

        case OPCODE_J:
            D printf("DEBUG   - Jump\n");

            // JUMP
            jump = true;
            jump_target = (if_id.next_pc & MS_4B) | (GET_ADDRESS(if_id.inst) << 2);

            // Bubble
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = false;
            break;

        case OPCODE_JAL:
            D printf("DEBUG   - Jump And Link\n");

            // JUMP
            jump = true;
            jump_target = (if_id.next_pc & MS_4B) | (GET_ADDRESS(if_id.inst) << 2);

            // Write to RA
            id_exe.mem_read = false;
            id_exe.mem_write = false;
            id_exe.reg_write = true;
            id_exe.mem_to_reg = false;

            id_exe.reg_dst = r_ra;
            id_exe.funct = FUNCT_ADD;
            // TODO : Check this
            id_exe.rs_value = if_id.next_pc;
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
    D printf("DEBUG   - Imm : 0x%x\n", id_exe.sign_ext_imm);
    return 0;
}

// PIPELINE : Instruction decode
int interp_id() {
    D printf("DEBUG : Pipeline : Instruction Decode\n");
    D printf("DEBUG   - Instruction = [0x%08x]\n", if_id.inst);

    // Load fields from instruction
    id_exe.rt = GET_RT(if_id.inst);
    id_exe.rs = GET_RS(if_id.inst);
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
    D printf("DEBUG   - ALUSrc  = [%d]\n", id_exe.alu_src);

    // Complete compuation
    switch(id_exe.funct) {
        case FUNCT_JR:
            D printf("DEBUG   - OP = [FUNCT_JR] (Jump Register)\n");
            // NOTE : Realistically this would not happen in the ALU

            // Jump
            jump = true;
            jump_target = op1;

            // NOP next IF
            flush = true;

            // Pass bubble
            exe_mem.mem_read = false;
            exe_mem.mem_write = false;
            exe_mem.reg_write = false;
            break;

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
            exe_mem.alu_res = op2 << id_exe.shamt;
            D printf("DEBUG 0x%x %d\n", op1, id_exe.shamt);
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
            if (EXT64I(op1) - EXT64I(op2) != EXT64I(exe_mem.alu_res)) return ERROR_OVERFLOW;
            break;

        case FUNCT_SUBU:
            D printf("DEBUG   - OP = [FUNCT_SUBU] (Subtraction - no overflow)\n");
            exe_mem.alu_res = op1 - op2;
            break;

        case FUNCT_ADD:
            D printf("DEBUG   - OP = [FUNCT_ADD] (Add - check overflow)\n");
            exe_mem.alu_res = op1 + op2;
            if (EXT64I(op1) + EXT64I(op2) != EXT64I(exe_mem.alu_res)) return ERROR_OVERFLOW;
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
void interp_mem() {
    D printf("DEBUG : Pipeline : Memory\n");
    D printf("DEBUG   - ALURes = [0x%x]\n", exe_mem.alu_res);

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
    return;
}

// PIPELINE : Writeback
void interp_wb() {
    D printf("DEBUG : Pipeline : Writeback\n");
    D printf("DEBUG   - ALURes = [0x%x]\n", mem_wb.alu_res);

    if (!mem_wb.reg_write) return;
    if (!mem_wb.reg_dst) return;

    D printf("DEBUG   - Writing to register bank\n");
    D printf("DEBUG   - MemToReg = [%d]\n", mem_wb.mem_to_reg);

    regs[mem_wb.reg_dst] = mem_wb.mem_to_reg ? mem_wb.read_data : mem_wb.alu_res;

    return;
}

// FORWARDING
int forward() {
    D printf("DEBUG : Hazard detection\n");

    // NOTE : We cannot terminate the function after detecting a single hazard,
    // although MIPS instructions can only write to 1 register, we might need to
    // forward one operand from MEM and one from WB!

    // Detect EX hazards
    if (exe_mem.reg_write && exe_mem.reg_dst != 0) {
        // Check RS
        if(exe_mem.reg_dst == id_exe.rs) {
            D printf("DEBUG   - EX hazard : Forward ALURes to RSValue [%d]\n", exe_mem.reg_dst);
            id_exe.rs_value = exe_mem.alu_res;
        }

        // Check RT
        if (exe_mem.reg_dst == id_exe.rt) {
            D printf("DEBUG   - EX hazard : Forward ALURes to RTValue [%d]\n", exe_mem.reg_dst);
            id_exe.rt_value = exe_mem.alu_res;
        }
    }

    // Detect MEM hazards
    if (mem_wb.reg_write && mem_wb.reg_dst != 0) {
        // Check RS
        if (mem_wb.reg_dst == id_exe.rs && exe_mem.reg_dst != id_exe.rs) {
            if (mem_wb.mem_to_reg) {
                D printf("DEBUG   - MEM hazard : Forward ReadData to RSValue [%d]\n", mem_wb.reg_dst);
                id_exe.rs_value = mem_wb.read_data;
            } else {
                D printf("DEBUG   - MEM hazard : Forward ALURes to RSValue [%d]\n", mem_wb.reg_dst);
                id_exe.rs_value = mem_wb.alu_res;

            }
        }
        // Check RT
        if (mem_wb.reg_dst == id_exe.rt && exe_mem.reg_dst != id_exe.rt) {
            if (mem_wb.mem_to_reg) {
                D printf("DEBUG   - MEM hazard : Forward ReadData to RTValue [%d]\n", mem_wb.reg_dst);
                id_exe.rt_value = mem_wb.read_data;
            } else {
                D printf("DEBUG   - MEM hazard : Forward ALURes to RTValue [%d]\n", mem_wb.reg_dst);
                id_exe.rt_value = mem_wb.alu_res;
            }
        }
    }

    return 0;
}

