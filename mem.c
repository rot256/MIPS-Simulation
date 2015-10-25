#include "consts.h"
#include "mem.h"
#include "elf.h"
#include "mips32.h"
#include <stdio.h>
#include <string.h>

/* Simulates a simple cache
 *
 * Addr = [Tag | Set Index | Word Index | Byte Index (2 bit)]
 *
 * The strategy for "block replacement" is:
 *  1. Find a block that is invalid in the requested set
 *  2. Find the block with the greates age
 *
 * Age is implemented as follows:
 * At every cache interaction the age of every block
 * in the requested set is increased by 1 (we make sure that no overflow can occure),
 * the age of a block is reset every time it is accessed (updated or fetched).
 */

#define GET_BYTE(addr)     (addr & 0x3)
#define RM_BYTE(addr)      (addr >> 2)

#define GET_BLOCK(c, addr) (RM_BYTE(addr) & (c.n_words_per_block - 1))
#define RM_BLOCK(c, addr)  (addr >> log2n(c.n_words_per_block))

#define GET_SET(c, addr)   (RM_BLOCK(c, RM_BYTE(addr)) & (c.n_sets - 1))
#define RM_SET(c, addr)    (addr >> log2n(c.n_sets))

#define GET_TAG(c, addr)   (RM_SET(c, RM_BLOCK(c, RM_BYTE(addr))))
#define FADDR(c, tag, set) (((tag << log2n(c.n_sets) | set ) << log2n(c.n_words_per_block)) << 2)

static unsigned char mem[MEMSZ];
struct block* cache_load(struct cache *c, uint32_t addr);

// Integer Log2
static inline uint32_t log2n(uint32_t a);
static inline uint32_t log2n(uint32_t a) {
    uint32_t l;
    for (l = 0; a >>= 1; l++);
    return l;
}

// Print valid blocks in cache (used for debugging)
static void print_cache(struct cache c) {
    printf("Cache blocks : %p\n", (void*) c.blocks);
    for (uint32_t s = 0; s < c.n_sets; s++) {
        printf("Set %d :\n", s);
        for(uint32_t b = 0; b < c.n_blocks_per_set; b++) {
            struct block res = c.blocks[b + s*c.n_blocks_per_set];
            uint32_t faddr = FADDR(c, res.tag, s);
            if(!res.valid) continue;
            printf(" Block %d :\n", b);
            printf("  Valid    = [%d]\n", res.valid);
            printf("  Modified = [%d]\n", res.modified);
            printf("  Words:\n");
            for(uint32_t w = 0; w < c.n_words_per_block; w++) {
                printf("   [%2d] = 0x%08x [0x%x]\n", w, *(((uint32_t*) res.data) + w), faddr);
                faddr += 4;
            }
        }
    }
}

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
    size_t data_size = c->n_blocks_per_set * c->n_words_per_block * sizeof(uint32_t);;
    size_t block_cnt = c->n_sets * c->n_blocks_per_set;
    c->blocks = (struct block*) calloc(block_cnt, sizeof(struct block));
    for(size_t i = 0; i < block_cnt; i++) {
        (c->blocks[i]).data = (uint32_t *) malloc(data_size);
        if(c->blocks[i].data == NULL) return ERROR_MEMORY_ERROR;
        D printf("SETUP %p ALLOC %zu\n", (void*) (c->blocks[i]).data, data_size);
    }

    // Set statistics
    c->hits = 0;
    c->misses = 0;
    return 0;
}

// Finds block of address (if in cache)
struct block* cache_find_block(struct cache *c, uint32_t addr) {
    struct block *res = NULL;
    struct block *b = c->blocks + (GET_SET((*c), addr) * c->n_blocks_per_set);
    for(uint32_t i = 0; i < c->n_blocks_per_set; i++) {
        if (!res && b->tag == GET_TAG((*c), addr) && b->valid) {
            res = b;
        } else if(b->age < 0xFFFFFFFF) b->age ++;
        b++;
    }
    return res;
}

// Finds block to replace in cache
// Attempt to replace invalid blocks first then go by age
struct block* cache_find_oldest(struct cache *c, uint32_t addr) {
    struct block *res = NULL;
    size_t base = GET_SET((*c), addr) * c->n_blocks_per_set;
    for(struct block *b = c->blocks + base; b < c->blocks + c->n_blocks_per_set + base; b++) {
        if (!b->valid) return b;
        if (!res || (b->age > res->age)) res = b;
    }
    return res;
}

// Fetches value in cache
uint32_t cache_get(struct cache *c, uint32_t addr) {
    D printf("DEBUG : Cache : Fetch from cache\n");
    D printf("DEBUG   - Addr       = [0x%x]\n", addr);
    D printf("DEBUG   - Cache      = [%p]\n", (void*) c->blocks);
    D printf("DEBUG   - Sets       = [%u]\n", c->n_sets);
    D printf("DEBUG   - Set Size   = [%u]\n", c->n_blocks_per_set);
    D printf("DEBUG   - Block Size = [%u]\n", c->n_words_per_block);
    D printf("DEBUG   - Word       = [%u]\n", GET_BLOCK((*c), addr));
    D printf("DEBUG   - Set        = [%u]\n", GET_SET((*c), addr));
    D printf("DEBUG   - Tag        = [0x%x]\n", GET_TAG((*c), addr));
    D printf("DEBUG   - First Addr = [0x%x]\n", FADDR((*c), GET_TAG((*c), addr), GET_SET((*c), addr)));
    struct block *res = cache_find_block(c, addr);
    if (!res) {
        D printf("DEBUG : Cache : Not in cache, read from lower\n");
        res = cache_load(c, addr);
    } else {
        c->hits++;
        D printf("DEBUG : Cache : Hit : %p\n", (void*) c->blocks);
    }
    res->age = 0;
    return *(res->data + GET_BLOCK((*c), addr));
}

// Updates value in cache
struct block* cache_update(struct cache *c, uint32_t addr, uint32_t val) {
    D printf("DEBUG : Cache : Update cache, set [0x%x] = [0x%x]\n", addr, val);
    struct block *res = cache_find_block(c, addr);
    if (!res) {
        D printf("DEBUG : Cache : Not in cache, read from lower\n");
        res = cache_load(c, addr);
    } else {
        c->hits++;
        D printf("DEBUG : Cache : Hit : %p\n", (void*) c->blocks);
    }
    res->age = 0;
    res->modified = true;
    *(res->data + GET_BLOCK((*c), addr)) = val;
    return res;
}

// Load a block from lower memory into the cache (and returns the block)
struct block* cache_load(struct cache *c, uint32_t addr) {
    D printf("DEBUG : Cache : Load memory into cache\n");

    // Find block to replace
    struct block *res = cache_find_oldest(c, addr);
    D printf("DEBUG : Cache : Block to replace\n");
    D printf("DEBUG   - Valid    = [%d]\n", res->valid);
    D printf("DEBUG   - Modified = [%d]\n", res->modified);
    D printf("DEBUG   - Age      = [%u]\n", res->age);
    D printf("DEBUG   - Tag      = [0x%x]\n", res->tag);
    D printf("DEBUG   - Data At  = [%p]\n", (void*) res->data);

    // Check if modified and write changes
    if(res->valid && res->modified) {
        D printf("DEBUG   - Writing changes to lower memory\n");
        uint32_t faddr = FADDR((*c), res->tag, GET_SET((*c), addr));
        uint32_t *ptr = res->data;
        for (uint32_t i = 0; i < c->n_words_per_block; i++) {
            if (c->blocks == l2cache.blocks && faddr < END_OF_MEM && faddr > MIPS_RESERVE) {
                SET_BIGWORD(mem, faddr, *ptr);
            } else {
                cache_update(&l2cache, faddr, *ptr);
            }
            faddr += 4;
            ptr++;
        }
    }

    // Update block metadata
    res->valid = true;
    res->modified = false;
    res->age = 0;
    res->tag = GET_TAG((*c), addr);
    #ifdef DEBUG
    if(c->blocks == l2cache.blocks) {
        D printf("DEBUG : Cache : Loading from MEM into L2-cache\n");
    } else {
        D printf("DEBUG : Cache : Loading from L2-cache into L1-cache\n");
    }
    #endif

    // Read from lower memory (L2-cache or main memory)
    uint32_t *ptr = res->data;
    uint32_t faddr = FADDR((*c), res->tag, GET_SET((*c), addr));
    for (uint32_t i = 0; i < c->n_words_per_block; i++) {
        D printf("DEBUG : Cache : Load %d / %d [%p]\n", i, c->n_words_per_block, (void*) ptr);
        if (c->blocks == l2cache.blocks && faddr < END_OF_MEM && faddr > MIPS_RESERVE) {
            D printf("DEBUG : Cache : Loaded word [0x%08x] from lower cache\n", *ptr);
            *ptr = GET_BIGWORD(mem, faddr);
        } else {
            D printf("DEBUG : Cache : Load addr = [0x%08x] from memory\n", addr);
            uint32_t a = cache_get(&l2cache, faddr);
            *ptr = a;
        }
        faddr += 4;
        ptr++;
    }

    // Update cycles and misses
    c->misses++;
    cycles += (c->blocks == l2cache.blocks ? 400 : 20);

    // DEBUG : TODO
    if (c->blocks == l2cache.blocks && 1 == 0) {
        print_cache((*c));
    }
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
    *read_inst = cache_get(&icache, addr);
    D print_cache(icache);
    // D print_cache(l2cache);
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
    *read_data = cache_get(&dcache, addr);
    D print_cache(dcache);
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
    cache_update(&dcache, addr, data);
    D print_cache(dcache);
    return 0;
}
