#include <stdint.h>
#include <stdbool.h>
#include "consts.h"

#ifndef MEM_H
#define MEM_H

// You win, we make it static
#define MEMSZ (640 * KB)
#define END_OF_MEM (MEMSZ + MIPS_RESERVE)

struct cache {
    uint32_t n_sets;            // Number of sets
    uint32_t n_blocks_per_set;  //
    uint32_t n_words_per_block; //
    struct block *blocks;
};

struct block {
    bool valid;          // Block references valid data?
    bool modified;       // Block modified?
    uint32_t age;        // When was the block inserted (higher = older)
    uint32_t tag;        // Upper part of address
    uint32_t *data;      // Data segment of block
};

// Caches
extern struct cache icache;
extern struct cache dcache;
extern struct cache l2cache;
struct cache icache;
struct cache dcache;
struct cache l2cache;

// Setup memory and cache
int mem_init(const char *path, uint32_t *PC);
int cache_init(struct cache *c);

// Cache operations
int inst_read(uint32_t addr, uint32_t *read_inst);
int data_read(uint32_t addr, uint32_t *read_data);
int data_write(uint32_t addr, uint32_t data);

#endif
