#include "consts.h"
#include "mem.h"
#include "elf.h"
#include "mips32.h"
#include <stdio.h>
#include <string.h>

static unsigned char mem[MEMSZ];

// Addr = [Tag | Set Offset | Block Offset  | Byte Index]
//         X   | n_sets     | words / block | 2

// Byte index (in word)
#define RM_BYTE(addr) (addr >> 2)
#define GET_BYTE(addr) (addr & 0x3)

// Word offset in block
#define RM_BLOCK(c, addr) (addr >> log2n(c.n_words_per_block))
#define GET_BLOCK(c, addr) (RM_BYTE(addr) & (c.n_words_per_block - 1))

// Set offset in cache
#define RM_SET(c, addr) (addr >> log2n(c.n_sets))
#define GET_SET(c, addr) (RM_BLOCK(c, RM_BYTE(addr)) & (c.n_blocks_per_set - 1))

// Tag value
#define GET_TAG(c, addr) (RM_SET(c, RM_BLOCK(c, RM_BYTE(addr))))

//

// Total bytes in cache
#define N_BYTES(c) (c.n_sets * c.n_blocks_per_set * c.n_words_per_block * sizeof(uint32_t))

struct block* cache_load(struct cache c, uint32_t addr);

// Integer Log2
static inline uint32_t log2n(uint32_t a);
static inline uint32_t log2n(uint32_t a) {
    uint32_t l;
    for (l = 0; a >>= 1; l++);
    return l;
}

// Print binary number
/*
   static void print_binary(uint32_t a) {
   for (int i = 31; i >= 0; i--) {
   if (a & 1 << i) {
   printf("1");
   } else {
   printf("0");
   }
   if (i % 8 == 0) printf(" ");
   }
   printf(" [0x%08x]\n", a);
   }
   */

// Setup memory (dumb wrapper)
int mem_init(const char *path, uint32_t *PC) {
    return elf_dump(path, PC, &mem[0], MEMSZ);
}

// Setup cache
int cache_init(struct cache *c) {
    // Check if values are good
    if (c->n_sets == 0) return ERROR_INVALID_CACHE_SIZE;
    if (c->n_blocks_per_set == 0) return ERROR_INVALID_CACHE_SIZE;
    if (c->n_words_per_block == 0) return ERROR_INVALID_CACHE_SIZE;
    if ( ((uint32_t) 1 << log2n(c->n_sets)) != c->n_sets ) return ERROR_INVALID_CACHE_SIZE;
    if ( ((uint32_t) 1 << log2n(c->n_blocks_per_set)) != c->n_blocks_per_set ) return ERROR_INVALID_CACHE_SIZE;
    if ( ((uint32_t) 1 << log2n(c->n_words_per_block)) != c->n_words_per_block ) return ERROR_INVALID_CACHE_SIZE;

    // Allocate memory
    size_t block_cnt = c->n_sets * c->n_blocks_per_set;
    c->blocks = (struct block*) calloc(block_cnt, sizeof(struct block));
    for(size_t i = 0; i < block_cnt; i++) {
        (c->blocks[i]).data = (unsigned char*) malloc(N_BYTES((*c)));
    }

    return 0;
}

// Finds block of address (if in cache)
struct block* cache_find_block(struct cache c, uint32_t addr) {
    struct block *res = NULL;
    size_t base = GET_SET(c, addr) * c.n_blocks_per_set;
    for(struct block *b = c.blocks + base; b < c.blocks + c.n_blocks_per_set + base; b++) {
        if (!res && b->tag == GET_TAG(c, addr) && b->valid) {
            res = b;
        } else if(b->age < 0xFFFFFFFF) b->age ++; // Increment age
    }
    return res;
}

// Finds block to replace in cache
// Attempt to replace invalid blocks first then go by age
struct block* cache_find_oldest(struct cache c, uint32_t addr) {
    struct block *res = NULL;
    size_t base = GET_SET(c, addr) * c.n_blocks_per_set;
    for(struct block *b = c.blocks + base; b < c.blocks + c.n_blocks_per_set + base; b++) {
        if ((!b->valid) || !res || (b->age > res->age)) res = b;
    }
    return res;
}

// Fetches value in cache
uint32_t cache_get(struct cache c, uint32_t addr) {
    uint32_t val;
    struct block *res = cache_find_block(c, addr);
    if (!res) {
        D printf("DEBUG : Cache : Not in cache, read from lower\n");
        res = cache_load(c, addr);
    } else {
        D printf("DEBUG : Cache : We have a hit scotty!\n");
        hits++;
    }
    memcpy(&val, res->data + GET_BLOCK(c, addr), sizeof(uint32_t)); // TODO : Load half-words and bytes also
    res->age = 0;
    return val;
}

// Updates value in cache
// NOTE : Mutally recursive with cache_load
struct block* cache_update(struct cache c, uint32_t addr, uint32_t val) {
    D printf("DEBUG : Cache : Update cache\n");
    struct block *res = cache_find_block(c, addr);
    if (!res) {
        res = cache_load(c, addr);
    }
    memcpy(res->data + GET_BLOCK(c, addr), &val, sizeof(uint32_t));
    res->modified = true;
    res->age = 0;
    return res;
}

// Load a block from lower memory into the cache (and returns the block)
// Works for both L2, ICache and DCache
struct block* cache_load(struct cache c, uint32_t addr) {
    D printf("DEBUG : Cache : Load memory into cache\n");

    // Find block to replace
    struct block *res = cache_find_oldest(c, addr);
    D printf("DEBUG : Cache : Block to replace\n");
    D printf("DEBUG   - Valid    = [%d]\n", res->valid);
    D printf("DEBUG   - Modified = [%d]\n", res->modified);
    D printf("DEBUG   - Age      = [%u]\n", res->age);
    D printf("DEBUG   - Tag      = [0x%x]\n", res->tag);
    D printf("DEBUG   - Data At  = [%p]\n", res->data);

    // Check if modified
    if(res->valid && res->modified) {
        D printf("DEBUG   - Writing changes to lower memory\n");

        // Find first address in block
        // Addr = [Tag | Set Offset | Block Offset  | Byte Index]
        uint32_t faddr = 0;
        faddr |= res->tag;
        faddr <<= log2n(c.n_sets);
        faddr |= GET_SET(c, addr);
        faddr <<= log2n(c.n_blocks_per_set);
        faddr |= GET_BLOCK(c, addr);
        faddr <<= log2n(c.n_words_per_block);
        faddr <<= 2;

        // Write to lower memory (L2-cache or main memory)
        for (uint32_t* ptr = (uint32_t*) res->data; ptr < (uint32_t*) res->data + c.n_words_per_block; ptr++) {
            if (c.blocks == l2cache.blocks) {
                SET_BIGWORD(mem, faddr, *ptr) // TODO : AVOID GOING OUTSIDE MEMORY
            } else {
                cache_update(l2cache, faddr, *ptr);
            }
            faddr += 4;
        }
    }

    // Update block
    // Read from lower memory (L2-cache or main memory)
    #ifdef DEBUG
    if(c.blocks == l2cache.blocks) {
        D printf("DEBUG : Cache : Loading from MEM into L2-cache\n");
    } else {
        D printf("DEBUG : Cache : Loading from L2-cache into L1-cache\n");
    }
    #endif
    uint32_t *p = (uint32_t*) res->data;
    for (uint32_t i = 0; i < c.n_words_per_block; i++) {
        if (c.blocks == l2cache.blocks) {
            *p = GET_BIGWORD(mem, addr); // TODO : AVOID GOING OUTSIDE MEMORY E.G [X|OUT|OUT|OUT] LOAD WORD AT X
        } else {
            *p = cache_get(l2cache, addr);
        }
        addr += 4;
        p++;
    }
    res->valid = true;
    res->modified = false;
    res->age = 0;

    // Update misses
    misses++;
    cycles += (c.blocks == l2cache.blocks ? 400 : 20);
    return res;
}

// Read from instruction cache
int inst_read(uint32_t addr, uint32_t *read_inst) {
    D printf("DEBUG : Cache : Reading instruction from cache, Addr = [0x%x]\n", addr);
    if (addr >= END_OF_MEM || addr <= MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Reading instruction from outside memory [0x%x]\n", addr);
        return ERROR_INVALID_MEM_ADDR;
    }

    // *read_inst = GET_BIGWORD(mem, addr);
    *read_inst = cache_get(icache, addr);
    return 0;
}

// Read from data cache
int data_read(uint32_t addr, uint32_t *read_data) {
    D printf("DEBUG : Cache : Reading instruction from cache, Addr = [0x%x]\n", addr);
    if (addr >= END_OF_MEM || addr <= MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Reading instruction from outside memory [0x%x]\n", addr);
        return ERROR_INVALID_MEM_ADDR;
    }

    // *read_data = GET_BIGWORD(mem, addr);
    *read_data = cache_get(dcache, addr);
    return 0;
}

// Write to data cache
int data_write(uint32_t addr, uint32_t data) {
    D printf("DEBUG : Cache : Reading instruction from cache, Addr = [0x%x]\n", addr);
    if (addr >= END_OF_MEM || addr <= MIPS_RESERVE) {
        D printf("DEBUG   - Cache : WARNING : Writing to data outside memory [0x%x]\n", addr);
        return ERROR_INVALID_MEM_ADDR;
    }

    // SET_BIGWORD(mem, addr, data);
    cache_update(dcache, addr, data);
    return 0;
}
