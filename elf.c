// Handout for G1, ARK15, DIKU.
//
// Copyright (c) 2015 DIKU.
// Copenhagen Liberal License - v0.5 - September 6, 2015.
//
// Authors:
//  Annie Jane Pinder <anpi@di.ku.dk>
//  Oleksandr Shturmov <oleks@oleks.info>

#include "elf.h"
#include "mips32.h"   // MIPS_RESERVE

#include <stdio.h>
#include <stdint.h>
#include <error.h>    // error
#include <errno.h>
#include <stdlib.h>   // exit
#include <string.h>   // memset

#define EI_MAG0   (0)
#define EI_MAG1   (1)
#define EI_MAG2   (2)
#define EI_MAG3   (3)
#define EI_CLASS  (4)
#define EI_DATA   (5)
#define EI_NIDENT (16)

#define ELFCLASS32  (1)

#define ELFDATA2MSB (2)

#define ET_EXEC (2)

#define EM_MIPS (8)

typedef uint16_t Elf32_Half;

typedef uint32_t Elf32_Word;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;

#define ET_EXEC   (2)

FILE *fdesc;

struct elf_header {
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half    e_type;
  Elf32_Half    e_machine;
  Elf32_Word    e_version;        // Not used.
  Elf32_Addr    e_entry;
  Elf32_Off     e_phoff;
  Elf32_Off     e_shoff;          // Not used.
  Elf32_Addr    e_flags;
  Elf32_Half    e_ehsize;         // Not used.
  Elf32_Half    e_phentsize;
  Elf32_Half    e_phnum;
  Elf32_Half    e_shentsize;      // Not used.
  Elf32_Half    e_shnum;          // Not used.
  Elf32_Half    e_shstrndx;       // Not used.
} e_header;

#define PT_NULL           (0x0)
#define PT_LOAD           (0x1)
#define PT_NOTE           (0x4)

#define PT_MIPS_ABIFLAGS  (0x70000003)

struct prog_header {
  Elf32_Word    p_type;
  Elf32_Off     p_offset;
  Elf32_Addr    p_vaddr;
  Elf32_Addr    p_paddr;
  Elf32_Word    p_filesz;
  Elf32_Word    p_memsz;
  Elf32_Word    p_flags;          // Not used.
  Elf32_Word    p_align;          // Not used.
} p_header;

int check_magic_number() {
  if (
    (e_header.e_ident[EI_MAG0] != 0x7F) ||
    (e_header.e_ident[EI_MAG1] != 'E') ||
    (e_header.e_ident[EI_MAG2] != 'L') ||
    (e_header.e_ident[EI_MAG3] != 'F')) {
    // error(0, 0, "this doesn't look like an ELF-file.");
    return ELF_ERROR_NOT_ELF;
  }

  return 0;
}

int check_elf_class_32() {
  if (e_header.e_ident[EI_CLASS] != ELFCLASS32) {
    // error(0, 0, "only 32-bit ELF is supported.");
    return ELF_ERROR_NOT_SUPPORTED;
  }

  return 0;
}

int check_data_encoding() {
  if (e_header.e_ident[EI_DATA] != ELFDATA2MSB) {
    // error(0, 0, "only 2's complement, big endian is supported.");
    return ELF_ERROR_NOT_SUPPORTED;
  }

  return 0;
}

int read_elf_ident() {
  int retval = 0;

  // It is straight-forward to read the ELF Identification. It is just an array
  // of 16 bytes. So there is no trouble with endianness; that comes later.

  if (fread(&e_header.e_ident, sizeof(e_header.e_ident), 1, fdesc) != 1) {
    // error(0, errno, "couldn't read ELF Identification");
    return ELF_ERROR_IO_ERROR;
  }

  retval = check_magic_number();
  if (retval != 0) return retval;

  retval = check_elf_class_32();
  if (retval != 0) return retval;

  retval = check_data_encoding();
  if (retval != 0) return retval;

  return 0;
}

int read_half_words(uint16_t *dst, size_t n) {
  unsigned char buf[2];
  size_t i;

  for (i = 0; i < n; ++i) {
    if (fread(&buf, sizeof(buf), 1, fdesc) != 1) {
      // error(0, 0, "couldn't read ELF32 half word.");
      return ELF_ERROR_IO_ERROR;
    }

    *(dst++) = (buf[0] << 8) | buf[1];
  }

  return 0;
}

int read_words(uint32_t *dst, size_t n) {
  unsigned char buf[4];
  size_t i;

  for (i = 0; i < n; ++i) {
    if (fread(&buf, sizeof(buf), 1, fdesc) != 1) {
      // error(0, 0, "couldn't read ELF32 word.");
      return ELF_ERROR_IO_ERROR;
    }

    *(dst++) = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
  }

  return 0;
}

int check_type() {
  if (e_header.e_type != ET_EXEC) {
    // error(0, 0, "only executables are supported.");
    return ELF_ERROR_NOT_SUPPORTED;
  }

  return 0;
}

int check_machine() {
  if (e_header.e_machine != EM_MIPS) {
    // error(0, 0, "only MIPS RS3000 machine is supported.");
    return ELF_ERROR_NOT_SUPPORTED;
  }

  return 0;
}

int read_type_and_machine() {
  int retval;

  retval = read_half_words(&e_header.e_type, 2);
  if (retval != 0) {
    // error(0, 0, "couldn't read type and machine.");
    return retval;
  }

  retval = check_type();
  if (retval != 0) return retval;

  retval = check_machine();
  if (retval != 0) return retval;

  return 0;
}

int read_e_header() {
  int retval = 0;

  retval = read_elf_ident(fdesc);
  if (retval != 0) {
    // error(0, 0, "couldn't read ELF ident array.");
    return retval;
  }

  retval = read_type_and_machine(fdesc);
  if (retval != 0) {
    // error(0, 0, "couldn't read ELF type and machine.");
    return retval;
  }

  retval = read_words(&e_header.e_version, 5);
  if (retval != 0) {
    // error(0, 0, "couldn't read rest of ELF header.");
    return retval;
  }

  retval = read_half_words(&e_header.e_ehsize, 6);
  if (retval != 0) {
    // error(0, 0, "couldn't read rest of ELF header.");
    return retval;
  }

  return retval;
}

// Assumes first 6 words of p_header are set.
// After this function, fdesc will point to one past the end of the segment.
int copy_segment(unsigned char *mem, size_t memsz) {
  uint32_t offset;
  unsigned char *segmem;

  offset = p_header.p_vaddr - MIPS_RESERVE;

  if (memsz < offset + p_header.p_memsz) {
    return ELF_ERROR_OUT_OF_MEM;
  }

  segmem = mem + offset;
  memset(segmem, 0, p_header.p_memsz);

  if (fseek(fdesc, p_header.p_offset, SEEK_SET) != 0) {
    return ELF_ERROR_IO_ERROR;
  }

  if (fread(segmem, p_header.p_filesz, 1, fdesc) != 1) {
    return ELF_ERROR_IO_ERROR;
  }

  return 0;
}

// Assumes fdesc points to the first byte of a program header entry.
// After this function, fdesc points to one past the end of the associated
// segment. Also, the first 6 words of the p_header will be set.
int copy_cur_segment_aux(unsigned char *mem, size_t memsz) {
  int retval = 0;

  if (read_words(&p_header.p_type, 6) != 0) {
    // error(0, 0, "couldn't read program header entry.");
    return ELF_ERROR_IO_ERROR;
  }

  switch(p_header.p_type) {
  case PT_NULL:
  case PT_NOTE:
  case PT_MIPS_ABIFLAGS:
    // Naïvely skip the above.
    break;
  case PT_LOAD:
    retval = copy_segment(mem, memsz);
    break;
  default:
    // error(0, 0, "unknown program header entry type 0x%x", p_header.p_type);
    retval = ELF_ERROR_NOT_SUPPORTED;
    break;
  }

  return retval;
}

// After this function, fdesc points to one past the end of the current program
// header entry. Also, the first 6 words of the p_header will be set.
int copy_cur_segment(unsigned char *mem, size_t memsz) {
  long origin;
  int retval;

  origin = ftell(fdesc);
  if (origin == -1) {
    // error(0, errno, "couldn't tell current position in file");
    return ELF_ERROR_IO_ERROR;
  }

  retval = copy_cur_segment_aux(mem, memsz);

  // Restore fdesc to next program header entry (if any).
  if (fseek(fdesc, origin + e_header.e_phentsize, SEEK_SET) != 0) {
    // error(0, errno, "couldn't seek past program header entity");
    return ELF_ERROR_IO_ERROR;
  }

  return retval;
}

// After this function, fdesc point to one past the end of the last program
// header entry.
int copy_all_segments(unsigned char *mem, size_t memsz) {
  size_t i;
  int retval = 0;

  if (fseek(fdesc, e_header.e_phoff, SEEK_SET) != 0) {
    // error(0, errno, "couldn't seek to program header table");
    return ELF_ERROR_IO_ERROR;
  }

  for (i = 0; i < e_header.e_phnum; ++i) {
    retval = copy_cur_segment(mem, memsz);
    if (retval != 0) return retval;
  }

  return 0;
}

int elf_open(const char *path) {
  int retval = 0;

  fdesc = fopen(path, "r");
  if (fdesc == NULL) {
    // error(0, errno, "couldn't open file (%s) for reading", path);
    return ELF_ERROR_IO_ERROR;
  }

  retval = read_e_header();
  if (retval != 0) {
    // error(0, errno, "couldn't read ELF header");
    return retval;
  }

  return 0;
}

int elf_close() {
  if (fclose(fdesc) != 0) {
    // error(0, errno, "couldn't close opened file");
    return ELF_ERROR_IO_ERROR;
  }

  return 0;
}

int elf_dump(const char *path, uint32_t *entry, unsigned char *mem, size_t memsz) {
  int retval = 0;

  retval = elf_open(path);
  if (retval != 0) {
    // error(0, 0, "couldn't get started on the ELF file.");
    return retval;
  }

  *entry = e_header.e_entry;

  retval = copy_all_segments(mem, memsz);
  if (retval != 0) {
    // error(0, 0, "couldn't read prog segments.");
    return retval;
  }

  return elf_close();
}
