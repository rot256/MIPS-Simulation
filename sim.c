#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "elf.h"
#include "mips32.h"

//DEBUG 
#define DEBUG

#ifdef DEBUG
    #define D
#else
    #define D if(0)
#endif

// https://www.cs.umd.edu/class/sum2003/cmsc311/Notes/Mips/format.html
// http://www.mrc.uidaho.edu/mrc/people/jff/digital/MPCSir.html

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
    nPC  += offset;
}

// Print simulation status
void print_status() {
    printf("Executed %zu instruction(s).\n", instr_cnt);
    printf("pc = 0x%x\n", PC);
    printf("at = 0x%x\n", regs[r_at]);
    printf("v0 = 0x%x\n", regs[r_v0]);
    printf("v1 = 0x%x\n", regs[r_v1]);
    for(int i = 0; i < 8; i++) printf("t%d = 0x%x\n", i, regs[i+8]);
    printf("sp = 0x%x\n", regs[r_sp]);
    printf("ra = 0x%x\n", regs[r_ra]);
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
    D printf("%08x\n", inst);
    D printf("Des: %u, Src1: %u Src2: %u\n", rs, rt, rd);
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
            regs[rd] = regs[rs] << GET_SHAMT(inst);
            advance_pc(INSTRUCTION_SIZE);
            break;

        // Shift right
        case FUNCT_SRL:
            regs[rd] = regs[rs] >> GET_SHAMT(inst);
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
        //debug
        D print_status();
        D printf("Program counter = %x\n", PC); 
        D if(PC == 0x40004c){ printf("reached\n"); exit(0);}   

        inst = GET_BIGWORD(mem, PC);
        D printf("Instruction = %x\n", inst);
        switch(GET_OPCODE(inst)) {
            // R type instruction
            case OPCODE_R:
                ret = interp_r(inst);
                if (ret != 0) return ret;
                break;

            // Unconditional jump
            case OPCODE_J:
                PC = nPC;
                nPC = GET_ADDRESS(inst);
                break;

            // Jump and link
            case OPCODE_JAL:
                D print_status();
                D printf("%x\n", nPC);
                regs[r_ra] = nPC + INSTRUCTION_SIZE;
                PC = nPC;
                nPC = (PC & MS_4B) | (GET_ADDRESS(inst) << 2);
                D print_status();
                D printf("%x\n", nPC);
                break;

            // Branch on equal
            case OPCODE_BEQ:
                if (GET_RS(inst) == GET_RT(inst)) {
                    advance_pc(GET_IMM(inst) << 2);
                    D print_status();
                    D exit(0);
                } else {
                    advance_pc(INSTRUCTION_SIZE);
                }
                break;

            // Branch on not equal
            case OPCODE_BNE:
                if (GET_RS(inst) != GET_RT(inst)) {
                    advance_pc(GET_IMM(inst) << 2);
                } else {
                    advance_pc(INSTRUCTION_SIZE);
                }
                break;

            // Add immediate unsigned (no overflow)
            case OPCODE_ADDIU:
                D printf("ADDIU GET_IMM %x (%d)\n", GET_IMM(inst),(int32_t) (int16_t) GET_IMM(inst));
                regs[GET_RS(inst)] += (int32_t) (int16_t) GET_IMM(inst);
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

            // Load upper immediate
            case OPCODE_LUI:
                regs[GET_RT(inst)] = GET_IMM(inst) << 16;
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Load word
            case OPCODE_LW:
                regs[GET_RS(inst)] = GET_BIGWORD(mem, GET_IMM(inst));
                advance_pc(INSTRUCTION_SIZE);
                break;

            // Store word
            case OPCODE_SW:
                D printf("Storing %x GET_RS %x GET_IMM %x\n", regs[GET_RT(inst)], regs[GET_RS(inst)], GET_IMM(inst));

                SET_BIGWORD(mem, regs[GET_RS(inst)] + GET_IMM(inst), regs[GET_RT(inst)]);
                advance_pc(INSTRUCTION_SIZE);
                break;

            default:
                printf("%x\n", inst);
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

    // Parse elf file (why the F does the elf parser write to stdout?)
    alloc = KB*1024;
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
    print_status();

    // Set next program counter
    nPC = PC + INSTRUCTION_SIZE;

    // Run
    ret = interp();
    if (ret == ERROR_UNKNOWN_OPCODE) {
        printf("Found unknown opcode!\n");
        exit(ret);
    } else  if (ret == SIG_HALT_PROGRAM) {
        printf("Program has stopped (encountered a syscall)\n");
    }
    print_status();

    printf("%s\n", mem);
    printf("%u\n", PC);

    return 0;
}
