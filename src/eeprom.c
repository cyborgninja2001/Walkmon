#include "eeprom.h"

static EEPROM eeprom;

bool eeprom_load(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    size_t read = fread(eeprom.data, 1, EEPROM_SIZE, f);
    fclose(f);

    if (read != EEPROM_SIZE) return false; // something went wrong
    return true;
}

bool eeprom_save(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;

    size_t written = fwrite(eeprom.data, 1, EEPROM_SIZE, f);
    fclose(f);

    return written == EEPROM_SIZE;
}

static uint16_t eeprom_address = 0;
static uint8_t eeprom_data = 0;
static uint8_t eeprom_status = 0;

uint8_t eeprom_mmio_read(uint16_t address) {
    switch (address) {
        case 0xF0A0: return eeprom_address & 0xFF;          // lo
        case 0xF0A1: return (eeprom_address >> 8) & 0xFF;   // hi
        case 0xF0A2: return eeprom_data;
        case 0xF0A3: return 0x00;                           // without reading commands
        case 0xF0A4: return eeprom_status;
        default:
            return 0xFF;
    }
}

void eeprom_mmio_write(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xF0A0:
            eeprom_address = (eeprom_address & 0xFF00) | value;
            break;
        case 0xF0A1:
            eeprom_address = (eeprom_address & 0x00FF) | (value << 8);
            break;
        case 0xF0A2:
            eeprom_data = value;
            break;
        case 0xF0A3:                   // command
            if (value == 0x1) {        // read
                if (eeprom_address < EEPROM_SIZE) {
                    eeprom_data = eeprom.data[eeprom_address];
                } else {
                    eeprom_data = 0xFF;
                }
            } else if (value == 0x2) { // write
                if (eeprom_address < EEPROM_SIZE) {
                    eeprom.data[eeprom_address] = eeprom_data;
                }
            }
            break;
        case 0xF0A4:
            // we do not manually write the status
            break;
    }
}

uint8_t eeprom_read(uint16_t address) {
    return eeprom.data[address];
}