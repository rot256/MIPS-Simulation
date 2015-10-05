#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "elf.h"
#include "mips32.h"
#include "consts.h"
#include "pipes.h"

// Advance the program counter
static inline void advance_pc(int32_t offset) {
    PC  =  nPC;
    nPC += offset;
}

// Print simulation status
void print_status() {
    printf("Executed %zu instruction(s).\n", instr_cnt);
    printf("%zu cycle(s) elapsed.\n", cycles);
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

int interp() {
    int ret = 0;
    while(ret == 0) {
        D printf("\n\n\n");
        D printf("DEBUG : Cycle %lu \n", cycles);

        ret = cycle();
        D printf("DEBUG : Press enter to continue...");
        D getchar();
        cycles++;
    }
    D printf("\n\n\n");
    return ret;
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
        exit(-1);
    }

    // Read config file and print inital status
    ret = read_config(argv[1]);
    if (ret != 0) {
        printf("Failed to load config, with code %d\n", ret);
        exit(ret);
    }

    // Parse elf file
    alloc = KB*640;
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
    regs[29] = alloc + MIPS_RESERVE + sizeof(uint32_t);
    D print_status();

    // Set next program counter
    nPC = PC + INSTRUCTION_SIZE;

    // Run
    ret = interp();
    print_status();
    if (ret == ERROR_UNKNOWN_OPCODE) {
        printf("Found unknown opcode!\n");
        exit(ret);
    } else if (ret == ERROR_UNKNOWN_FUNCT) {
        printf("Found unknown funct code!\n");
        exit(ret);
    } else if (ret < 0) {
        printf("Encountered unhandled error! [%d]\n", ret);
        exit(ret);
    } else if (ret != SIG_HALT_PROGRAM) {
        printf("Terminated with signal: [%d]\n", ret);
    }
    return 0;
}
