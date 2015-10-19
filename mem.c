#include "consts.h"
#include "mem.h"
#include "elf.h"
#include "mips32.h"
#include <stdio.h>

static unsigned char mem[MEMSZ];

int mem_init(const char *path, uint32_t *PC) {
    return elf_dump(path, PC, &mem[0], MEMSZ);
}

int inst_read(uint32_t addr, uint32_t *read_inst) {
    D printf("DEBUG   - Cache : Read from instruction memory [0x%x]\n", addr);
    printf("0x%x 0x%x\n", addr, MIPS_RESERVE);
    if (addr > LAST_MEM_ADDR || addr < MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Reading instruction from outside memory\n");
        return ERROR_INVALID_MEM_ADDR;
    }
    *read_inst = GET_BIGWORD(mem, addr);
    return 0;
}

int data_read(uint32_t addr, uint32_t *read_data) {
    D printf("DEBUG   - Cache : Read from data memory [0x%x]\n", addr);
    if (addr > LAST_MEM_ADDR || addr < MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Reading instruction from outside memory\n");
        return ERROR_INVALID_MEM_ADDR;
    }
    *read_data = GET_BIGWORD(mem, addr);
    return 0;
}

int data_write(uint32_t addr, uint32_t data) {
    D printf("DEBUG   - Cache : Write to data memory [0x%x] = [0x%x]\n", addr, data);
    if (addr > LAST_MEM_ADDR || addr < MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Writing to data outside memory\n");
        return ERROR_INVALID_MEM_ADDR;
    }
    SET_BIGWORD(mem, addr, data);
    return 0;
}

