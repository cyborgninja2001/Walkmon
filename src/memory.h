#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ssu.h"

/*
Memory Map
0x0000 - 0xBFFF (ROM)
0xF020 - 0xF0FF (MMIO)
0xF780 - 0xFF7F (RAM)
0xFF80 - 0xFFFF (MMIO)
*/

#define ROM_SIZE 0xC000
#define MMIO1_SIZE 0xE0
#define RAM_SIZE 0x800
#define MMIO2_SIZE 0x80

typedef struct {
    uint8_t rom[ROM_SIZE];     // 48 KB
    uint8_t mmio1[MMIO1_SIZE]; // 224 bytes
    uint8_t ram[RAM_SIZE];     // 2 KB
    uint8_t mmio2[MMIO2_SIZE]; // 128 bytes
} MEMORY;

bool load_rom(const char *path);
void init_memory();

uint8_t mem_read8(uint32_t address);
uint16_t mem_read16(uint32_t address);
uint32_t mem_read32(uint32_t address);

void mem_write8(uint32_t address, uint8_t value);
void mem_write16(uint32_t address, uint16_t value);
void mem_write32(uint32_t address, uint32_t value);


#endif