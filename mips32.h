// Handout for G2, ARK15, DIKU.
//
// Copyright (c) 2015 DIKU.
// Copenhagen Liberal License - v0.5 - September 6, 2015.
//
// Authors:
//  Annie Jane Pinder <anpi@di.ku.dk>
//  Oleksandr Shturmov <oleks@oleks.info>

#ifndef ARK2015_MIPS32_H
#define ARK2015_MIPS32_H


#include <stdint.h>


// MIPS32 memory

#define MIPS_RESERVE  (0x400000)

#define GET_BIGWORD(mem, addr) ((uint32_t) \
  ((mem)[(addr) - MIPS_RESERVE + 0] << 24)  | \
  ((mem)[(addr) - MIPS_RESERVE + 1] << 16)  | \
  ((mem)[(addr) - MIPS_RESERVE + 2] << 8)   | \
  ((mem)[(addr) - MIPS_RESERVE + 3]))

#define SET_BIGWORD(mem, addr, value) \
  (mem)[(addr) - MIPS_RESERVE + 0] = value >> 24; \
  (mem)[(addr) - MIPS_RESERVE + 1] = value << 8 >> 24; \
  (mem)[(addr) - MIPS_RESERVE + 2] = value << 16 >> 24; \
  (mem)[(addr) - MIPS_RESERVE + 3] = value << 24 >> 24;

// MIPS32 instruction parsing

#define OPCODE_R        (0x00)
#define OPCODE_J        (0x02)
#define OPCODE_JAL      (0x03)
#define OPCODE_BEQ      (0x04)
#define OPCODE_BNE      (0x05)
#define OPCODE_ADDI     (0x08)
#define OPCODE_ADDIU    (0x09)
#define OPCODE_SLTI     (0x0A)
#define OPCODE_SLTIU    (0x0B)
#define OPCODE_ANDI     (0x0C)
#define OPCODE_ORI      (0x0D)
#define OPCODE_LUI      (0x0F)
#define OPCODE_LW       (0x23)
#define OPCODE_SW       (0x2B)

#define FUNCT_JR        (0x08)
#define FUNCT_SYSCALL   (0x0C)
#define FUNCT_ADD       (0x20)
#define FUNCT_ADDU      (0x21)
#define FUNCT_SUB       (0x22)
#define FUNCT_SUBU      (0x23)
#define FUNCT_AND       (0x24)
#define FUNCT_OR        (0x25)
#define FUNCT_NOR       (0x27)
#define FUNCT_SLT       (0x2A)
#define FUNCT_SLTU      (0x2B)
#define FUNCT_SLL       (0x00)
#define FUNCT_SRL       (0x02)

// Least-significant bit-masks.
#define LS_4B   ((1 << 4 ) - 1)
#define LS_5B   ((1 << 5 ) - 1)
#define LS_6B   ((1 << 6 ) - 1)
#define LS_16B  ((1 << 16) - 1)
#define LS_26B  ((1 << 26) - 1)

// Most-significant bit-masks.
#define MS_4B   (LS_4B << 28)

#define GET_OPCODE(inst)    ( (inst) >> 26)
#define GET_RS(inst)        (((inst) >> 21) & LS_5B)
#define GET_RT(inst)        (((inst) >> 16) & LS_5B)
#define GET_RD(inst)        (((inst) >> 11) & LS_5B)
#define GET_SHAMT(inst)     (((inst) >> 6)  & LS_5B)

#define GET_FUNCT(inst)     ((inst) & LS_6B)
#define GET_IMM(inst)       ((inst) & LS_16B)
#define GET_ADDRESS(inst)   ((inst) & LS_26B)

#define SIGN_EXTEND(value)  ((int32_t)(int16_t)value)
#define ZERO_EXTEND(value)  ((uint32_t)value)

#endif // ARK2015_MIPS32_H
