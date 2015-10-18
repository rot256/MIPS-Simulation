#include <stdint.h>
#include <stdbool.h>
#include "consts.h"

#ifndef MEM_H
#define MEM_H

// You win, we make it static
#define MEMSZ 640 * KB

struct cache {
    uint32_t n_sets;
    uint32_t n_blocks;
    uint32_t n_words_per_block;

    struct block *blocks;
    unsigned char *data;
};

struct block {
    bool valid;
    bool modified;
    uint32_t tag;
};

// Caches
extern struct cache icache;
extern struct cache dcache;
extern struct cache l2cache;
struct cache icache;
struct cache dcache;
struct cache l2cache;

// Setup memory
int mem_init(const char *path, uint32_t *PC);

// Cache operations
int inst_read(uint32_t addr, uint32_t *read_inst);
int data_read(uint32_t addr, uint32_t *read_data);
int data_write(uint32_t addr, uint32_t data);

#endif
