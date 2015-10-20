#include "consts.h"
#include "mem.h"
#include "elf.h"
#include "mips32.h"
#include <stdio.h>

static unsigned char mem[MEMSZ];

// "Fast" log2
static inline uint32_t logn_2(uint32_t a);
static inline uint32_t logn_2(uint32_t a) {
    uint32_t l;
    for(l = 0; a >>= 1; l++);
    return l;
}

// Setup memory (dumb wrapper)
int mem_init(const char *path, uint32_t *PC) {
    return elf_dump(path, PC, &mem[0], MEMSZ);
}

// Setup cache
int cache_init(struct cache *c) {
    // Check if values are good
    if ( ((uint32_t) 1 << logn_2(c->n_sets)) != c->n_sets ) return ERROR_INVALID_CACHE_SIZE;
    if ( ((uint32_t) 1 << logn_2(c->n_blocks)) != c->n_blocks ) return ERROR_INVALID_CACHE_SIZE;
    if ( ((uint32_t) 1 << logn_2(c->n_words_per_block)) != c->n_words_per_block ) return ERROR_INVALID_CACHE_SIZE;

    // Allocate memory
    c->blocks = (struct block*) calloc(c->n_sets, sizeof(struct block));
    c->data = (unsigned char*) malloc(N_BYTES(c));
    return 0;
}

// Read from instruction cache
int inst_read(uint32_t addr, uint32_t *read_inst) {
    if (addr >= END_OF_MEM || addr <= MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Reading instruction from outside memory\n");
        return ERROR_INVALID_MEM_ADDR;
    }



    *read_inst = GET_BIGWORD(mem, addr);
    return 0;
}

// Read from data cache
int data_read(uint32_t addr, uint32_t *read_data) {
    if (addr >= END_OF_MEM || addr <= MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Reading instruction from outside memory\n");
        return ERROR_INVALID_MEM_ADDR;
    }
    *read_data = GET_BIGWORD(mem, addr);
    return 0;
}

// Write to data cache
int data_write(uint32_t addr, uint32_t data) {
    if (addr >= END_OF_MEM || addr <= MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Writing to data outside memory\n");
        return ERROR_INVALID_MEM_ADDR;
    }
    SET_BIGWORD(mem, addr, data);
    return 0;
}
