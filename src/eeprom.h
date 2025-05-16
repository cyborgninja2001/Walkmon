#ifndef EEPROM_H
#define EEPROM_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// it is accessed through MMIO registers

#define EEPROM_SIZE (64 * 1024)

typedef struct {
    uint8_t data[EEPROM_SIZE]; // 64 KB
} EEPROM;

bool eeprom_load(const char *path);
bool eeprom_save(const char *path);

uint8_t eeprom_mmio_read(uint16_t address);
void eeprom_mmio_write(uint16_t address, uint8_t value);

uint8_t eeprom_read(uint16_t address);

#endif