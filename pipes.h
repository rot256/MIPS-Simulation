#include <stdbool.h>
#include "consts.h"

#ifndef PIPES_H
#define PIPES_H
#define EXT64I(value) ((uint64_t)(uint32_t)value)

// Pipeline register structures

struct preg_if_id {
    uint32_t inst;         // Instruction value
    uint32_t next_pc;      // Following program counter
};

struct preg_id_exe {
    bool mem_read;         // Control : Memory read
    bool mem_write;        // Control : Memory writeback
    bool reg_write;        // Control : Register writeback
    bool alu_src;          // Control : Alu source (true = imm)
    bool mem_to_reg;       // Control : Write memory output to register

    uint32_t rt;           // RT
    uint32_t rs;           // Source register
    uint32_t funct;        // Funct value
    uint32_t rs_value;     // Value in rs register
    uint32_t rt_value;     // Value in rt register
    uint32_t sign_ext_imm; // Sign extended immediate
    uint32_t reg_dst;      // Destination register
    uint32_t shamt;        // Shift amount
};

struct preg_exe_mem
{
    bool mem_read;          // Control : Memory read
    bool mem_write;         // Control : Memory writeback
    bool reg_write;         // Control : Register writeback
    bool mem_to_reg;        // Control : Write memory output to register

    uint32_t rt;            // RT value
    uint32_t rt_value;      // Value in rt register
    uint32_t alu_res;       // Output of ALU
    uint32_t reg_dst;       // Destionation register
};

struct preg_mem_wb
{
    bool reg_write;       // Control : Register writeback
    bool mem_to_reg;      // Control : Write memory output to register

    uint32_t read_data;   // Data read from memory
    uint32_t rt;          // RT value
    uint32_t alu_res;     // Output of ALU
    uint32_t reg_dst;     // Destionation register
};

// Pipeline steps

void interp_if();
int interp_id();
int interp_exe();
void interp_mem();
void interp_wb();

// Pipeline forwarding

int forward();

//

int cycle();

#endif
