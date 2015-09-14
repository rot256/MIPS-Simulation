// Handout for G1, ARK15, DIKU.
//
// Copyright (c) 2015 DIKU.
// Copenhagen Liberal License - v0.5 - September 6, 2015.
//
// Authors:
//  Annie Jane Pinder <anpi@di.ku.dk>
//  Oleksandr Shturmov <oleks@oleks.info>

#ifndef ARK2015_ELF_H
#define ARK2015_ELF_H

#include <stddef.h>
#include <stdint.h>

#define ELF_ERROR_IO_ERROR        (-2)
#define ELF_ERROR_OUT_OF_MEM      (-3)
#define ELF_ERROR_NOT_ELF         (-4)
#define ELF_ERROR_NOT_SUPPORTED   (-5)

int elf_dump(const char *path, uint32_t *entry,
  unsigned char *mem, size_t memsz);

#endif // ARK2015_ELF_H
