#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

// it is accessed through MMIO registers

#define EEPROM_SIZE (64 * 1024)

typedef struct {
    uint8_t data[EEPROM_SIZE]; // 64 KB
} EEPROM;

#endif