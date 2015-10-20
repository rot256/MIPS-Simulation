#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "mips32.h"
#include "consts.h"
#include "pipes.h"
#include "mem.h"

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
    // Load registers
    for (int i = 8; i < 16; i++) {
	    if (fscanf(f, "%u", &regs[i]) != 1) {
	        if (errno == 0) return ERROR_INVALID_CONFIG;
	        return ERROR_IO_ERROR;
	    }
    }
    return 0;
}

// Read cache config
int read_cache_config(FILE *f, struct cache *c) {
    // Read config
    if (fscanf(f, "%u %u %u", &c->n_sets, &c->n_blocks, &c->n_words_per_block) < 0) {
        if (errno == 0) return ERROR_INVALID_CONFIG;
        return ERROR_IO_ERROR;
    }
    D printf("DEBUG   - Sets = [%u]\n", c->n_sets);
    D printf("DEBUG   - Blocks = [%u]\n", c->n_blocks);
    D printf("DEBUG   - Words Per Block = [%u]\n", c->n_words_per_block);
    return cache_init(c);
}

// Read config
int read_config(const char *path) {
    int ret;
    FILE* f = fopen(path, "r");
    if (f == NULL) return ERROR_IO_ERROR;
    if ((ret = read_config_stream(f))) return ret;
    D printf("DEBUG : Setup instruction cache\n");
    if ((ret = read_cache_config(f, &icache))) return ret;
    D printf("DEBUG : Setup data cache\n");
    if ((ret = read_cache_config(f, &dcache))) return ret;
    D printf("DEBUG : Setup shared l2 cache\n");
    if ((ret = read_cache_config(f, &l2cache))) return ret;
    D printf("\n\n\n");
    return fclose(f);
}

int interp() {
    int ret = 0;
    while(ret == 0) {
        D printf("\n\n\n");
        D printf("DEBUG : Cycle %zu \n", cycles);
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

    // Setup memory
    if ((ret = mem_init(argv[2], &PC))) return ret;

    // Set stack pointer
    regs[29] = END_OF_MEM;
    D print_status();

    // Run
    ret = interp();
    print_status();
    if (ret == ERROR_UNKNOWN_OPCODE) {
        printf("Found unknown opcode!\n");
    } else if (ret == ERROR_UNKNOWN_FUNCT) {
        printf("Found unknown funct code!\n");
    } else if (ret == ERROR_OVERFLOW) {
        printf("Encountered overflow!\n");
    } else if (ret == ERROR_INVALID_MEM_ADDR){
        printf("Accesssing invalid memory address\n");
    } else if (ret < 0) {
        printf("Encountered unhandled error! [%d]\n", ret);
    } else if (ret != SIG_HALT_PROGRAM) {
        printf("Terminated with signal: [%d]\n", ret);
        ret = 0;
    }
    return ret;
}
