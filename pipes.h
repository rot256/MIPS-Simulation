#include "consts.h"
#include <stdbool.h>

// Pipeline register structures

struct preg_if_id {
    uint32_t inst;        // Instruction value
    uint32_t PC;          // Program counter
};

struct preg_id_exe {
    bool mem_read;        // Control : Memory read
    bool mem_write;       // Control : Memory writeback
    bool reg_write;       // Control : Register writeback

    uint32_t rs;          // RS value
    uint32_t funct;       // Funct value
    uint32_t rs_value;    // Value in rs register
    uint32_t rt_value;    // Value in rt register
    int32_t sign_ext_imm; // Sign extended immediate
};

struct preg_exe_mem
{
    bool mem_read;        // Control : Memory read
    bool mem_write;       // Control : Memory writeback
    bool reg_write;       // Control : Register writeback

    uint32_t rt;          // RT value
    uint32_t rt_value;    // Value in rt register
    uint32_t alu_res;     // Output of ALU
};

struct preg_mem_wb
{
    bool reg_write;       // Control : Register writeback

    uint32_t rt;          // RT value

    
};

int interp_if();