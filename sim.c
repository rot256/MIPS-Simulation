#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "elf.h"
#include "mips32.h"

// DEBUG
// #define DEBUG

#ifdef DEBUG
    #define D
#else
    #define D if(0)
#endif

// Types
#define uchar unsigned char

// Signal codes
#define SIG_HALT_PROGRAM 1

// Error codes
#define ERROR_IO_ERROR       -1
#define ERROR_INVALID_CONFIG -2
#define ERROR_INVALID_ELF    -3
#define ERROR_UNKNOWN_OPCODE -4
#define ERROR_UNKNOWN_FUNCT  -5

// Index of special registers
#define r_at 1
#define r_v0 2
#define r_v1 3
#define r_sp 29
#define r_ra 31

#define KB 1024

#define INSTRUCTION_SIZE sizeof(uint32_t)

// Registers
uint32_t regs[32]; // General purpose
uint32_t PC;       // Program counter
uint32_t nPC;      // Next program counter
size_t instr_cnt;  // Number of executed instructions

// Memory
size_t alloc;
uchar* mem;

// Advance the program counter
static inline void advance_pc(int32_t offset) {
    PC  =  nPC;
    nPC += offset;
}

// Print simulation status
void print_status() {
    printf("Executed %zu instruction(s).\n", instr_cnt);
    printf("pc = 0x%x\n", PC);
    printf("at = 0x%x\n", regs[r_at]);
    printf("v0 = 0x%x\n", regs[r_v0]);
    printf("v1 = 0x%x\n", regs[r_v1]);
    for (int i = 0; i < 8; i++) printf("t%d = 0x%x\n", i, regs[i+8]);
    printf("sp = 0x%x\n", regs[r_sp]);
    printf("ra = 0x%x\n", regs[r_ra]);
}

void print_all_registers() {
    for (int i = 0; i < 4; i++) printf("a%d = 0x%x\n", i, regs[i+16]);
    for (int i = 0; i < 8; i++) printf("s%d = 0x%x\n", i, regs[i+16]);
}

// Read config file stream
int read_config_stream(FILE* f) {
    for (int i = 8; i < 16; i++) {
	if (fscanf(f, "%u", &regs[i]) != 1) {
	    if (errno == 0) return ERROR_INVALID_CONFIG;
	    return ERROR_IO_ERROR;
	}
    }
    return 0;
}

// Read config
int read_config(const char *path) {
    FILE* f = fopen(path, "r");
    if (f == NULL) return ERROR_IO_ERROR;
    if (read_config_stream(f) != 0) return ERROR_IO_ERROR;
    return fclose(f);
}

// Interpid R type instruction
int interp_r(uint32_t inst) {
    uint32_t rs = GET_RS(inst);
    uint32_t rt = GET_RT(inst);
    uint32_t rd = GET_RD(inst);
    switch(GET_FUNCT(inst)) {
        // Jump register
        case FUNCT_JR:
            PC = nPC;
            nPC = regs[rs];
            break;

        // Syscall (not implemented)
        case FUNCT_SYSCALL:
            return SIG_HALT_PROGRAM;

        // Add unsigned
        case FUNCT_ADDU:
            regs[rd] = regs[rs] + regs[rt];
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Subtract unsigned
        case FUNCT_SUBU:
            regs[rd] = regs[rs] - regs[rt];
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Bitwise and
        case FUNCT_AND:
            regs[rd] = regs[rs] & regs[rt];
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Bitwise or
        case FUNCT_OR:
            regs[rd] = regs[rs] | regs[rt];
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Bitwise xor
        case FUNCT_XOR:
            regs[rd] = regs[rs] ^ regs[rt];
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Bitwise not or
        case FUNCT_NOR:
            regs[rd] = !(regs[rs] | regs[rt]);
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Set on less than
        case FUNCT_SLT:
            regs[rd] = ((int32_t) regs[rs] < (int32_t) regs[rt]) ? 1 : 0;
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Shift left
        case FUNCT_SLL:
            regs[rd] = regs[rt] << GET_SHAMT(inst);
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Shift right
        case FUNCT_SRL:
            regs[rd] = regs[rt] >> GET_SHAMT(inst);
            advance_pc(INSTRUCTION_SIZE);
            break;

        default:
            return ERROR_UNKNOWN_FUNCT;
    }
    return 0;
}

// Run simulation
int interp() {
    int ret;
    uint32_t inst;
    while(1) {
        D print_status();
        D print_all_registers();
        D printf("PC     : %x\n", PC);
        D printf("Next PC: %x\n", nPC);
        inst = GET_BIGWORD(mem, PC);
        D printf("Inst   : %x\n", inst);
        D getchar();
        switch(GET_OPCODE(inst)) {
            // R type instruction
            case OPCODE_R:
                ret = interp_r(inst);
                if (ret != 0) return ret;
                break;

            // Unconditional jump
            case OPCODE_J:
                PC = nPC;
                nPC = (PC & MS_4B) | (GET_ADDRESS(inst) << 2);
                break;

            // Jump and link
            case OPCODE_JAL:
                regs[r_ra] = nPC + INSTRUCTION_SIZE;
                PC = nPC;
                nPC = (PC & MS_4B) | (GET_ADDRESS(inst) << 2);
                break;

            // Branch on equal
            case OPCODE_BEQ:
                if (regs[GET_RS(inst)] == regs[GET_RT(inst)]) {
                    advance_pc(SIGN_EXTEND(GET_IMM(inst) << 2));
                } else {
                    advance_pc(INSTRUCTION_SIZE);
                }
                break;

            // Branch on not equal
            case OPCODE_BNE:
                if (regs[GET_RS(inst)] != regs[GET_RT(inst)]) {
                    advance_pc(SIGN_EXTEND(GET_IMM(inst) << 2));
                } else {
                    advance_pc(INSTRUCTION_SIZE);
                }
                break;

            // Add immediate unsigned (no overflow)
            case OPCODE_ADDIU:
                regs[GET_RT(inst)] = regs[GET_RS(inst)] + (SIGN_EXTEND( GET_IMM(inst)));
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Set on less than (signed)
            case OPCODE_SLTI:
                regs[GET_RT(inst)] = ((int32_t) regs[GET_RS(inst)] < (int32_t) GET_IMM(inst)) ? 1 : 0;
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Bitwise and on immediate
            case OPCODE_ANDI:
                regs[GET_RT(inst)] = regs[GET_RS(inst)] & GET_IMM(inst);
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Bitwise or on immediate
            case OPCODE_ORI:
                regs[GET_RT(inst)] = regs[GET_RS(inst)] | GET_IMM(inst);
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Bitwise xor on immediate
            case OPCODE_XORI:
                regs[GET_RT(inst)] = regs[GET_RS(inst)] ^ GET_IMM(inst);
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Load upper immediate
            case OPCODE_LUI:
                regs[GET_RT(inst)] = GET_IMM(inst) << 16;
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Load word
            case OPCODE_LW:
                regs[GET_RT(inst)] = GET_BIGWORD(mem, regs[GET_RS(inst)] + GET_IMM(inst));
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Store word
            case OPCODE_SW:
                SET_BIGWORD(mem, regs[GET_RS(inst)] + GET_IMM(inst), regs[GET_RT(inst)]);
                advance_pc(INSTRUCTION_SIZE);
                break;

            default:
                D printf("Unknown opcode 0x%x\n", GET_OPCODE(inst));
                return ERROR_UNKNOWN_OPCODE;
        }

        // Ensure that zero = 0
        regs[0] = 0;
        instr_cnt++;
    }
}

int main(int argc, char* argv[]) {
    int ret;

    // Print usage
    if (argc != 3) {
        printf("Usage:\n");
        printf("%s config_file mips_elf_file\n", argv[0]);
        printf("Error codes:\n");
        printf("IO ERROR       : %4d\n", ERROR_IO_ERROR);
        printf("INVALID CONFIG : %4d\n", ERROR_INVALID_CONFIG);
        exit(0);
    }

    // Read config file and print inital status
    ret = read_config(argv[1]);
    if (ret != 0) {
        printf("Failed to load config, with code %d\n", ret);
        exit(ret);
    }

    // Parse elf file
    alloc = KB*64;
    mem = (uchar*) malloc(alloc);
    while(1) {
        ret = elf_dump(argv[2], &PC, mem, alloc);
        if (ret == ELF_ERROR_OUT_OF_MEM) {
            alloc *= 2;
            mem = (uchar*) realloc(mem, alloc);
            continue;
        }
        if (ret != 0) {
            printf("Failed to parse elf file!");
            exit(ERROR_INVALID_ELF);
        }
        break;
    }

    // Set stack pointer
    regs[29] = alloc + MIPS_RESERVE - sizeof(uint32_t);
    SET_BIGWORD(mem, regs[29], regs[29]);
    D printf("GET_WORD %x regs_29 %x \n" , GET_BIGWORD(mem, regs[29]), regs[29]);
    D print_status();

    // Set next program counter
    nPC = PC + INSTRUCTION_SIZE;

    // Run
    ret = interp();
    if (ret == ERROR_UNKNOWN_OPCODE) {
        printf("Found unknown opcode!\n");
        exit(ret);
    } else if (ret == ERROR_UNKNOWN_FUNCT) {
        printf("Found unknown funct code!\n");
        exit(ret);
    }
    print_status();
    return 0;
}
