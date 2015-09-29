#include <stdint.h>
#include <stdlib.h>

#ifndef CONSTS_H
#define CONSTS_H

// Debugging
// #define DEBUG
#ifdef DEBUG
#define D
#else
#define D if(0)
#endif

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

// Misc
#define KB 1024
#define INSTRUCTION_SIZE sizeof(uint32_t)

// Registers
uint32_t regs[32]; // General purpose
uint32_t PC;       // Program counter
uint32_t nPC;      // Next program counter
size_t instr_cnt;  // Number of instructions executed
size_t cycles;     // Number of cycles executed

// Memory
size_t alloc;
uchar* mem;

#endif
