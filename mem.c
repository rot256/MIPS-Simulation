#include "consts.h"
#include "mem.h"
#include "elf.h"
#include "mips32.h"

static unsigned char mem[MEMSZ];

int mem_init(const char *path, uint32_t *PC) {
    return elf_dump(path, PC, &mem[0], MEMSZ);
}

int inst_read(uint32_t addr, uint32_t *read_inst) {
    *read_inst = GET_BIGWORD(mem, addr);
    return 0;
}

int data_read(uint32_t addr, uint32_t *read_data) {
    *read_data = GET_BIGWORD(mem, addr);
    return 0;
}

int data_write(uint32_t addr, uint32_t data) {
    SET_BIGWORD(mem, addr, data);
    return 0;
}

