#include "memory.h"

static MEMORY memory;

bool load_rom(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    size_t read = fread(memory.rom, 1, ROM_SIZE, f);
    fclose(f);

    if (read != ROM_SIZE) return false; // error incomplete rom or corrupted
    return true;
}

void init_memory() {
    for (int i = 0x0000; i < MMIO1_SIZE; i++) {
        memory.mmio1[i] = 0;
    }

    for (int i = 0x0000; i < RAM_SIZE; i++) {
        memory.ram[i] = 0;
    }

    for (int i = 0x0000; i < MMIO2_SIZE; i++) {
        memory.mmio2[i] = 0;
    }
}

uint8_t mem_read8(uint32_t address) {
    address = address & 0xFFFF; // mask
    if (address < 0x0000 || address > 0xFFFF) {
        printf("Invalid memory address access! mem_read8(0x%04X)\n", address);
        exit(-1);
    }

    if (0x0000 <= address && address <= 0xBFFF) {
        return memory.rom[address];
    } else if (0xF020 <= address && address <= 0xF0FF) {
        return memory.mmio1[address - 0xF020];
    } else if (0xF780 <= address && address <= 0xFF7F) {
        return memory.ram[address - 0xF780];
    } else if (0xFF80 <= address && address <= 0xFFFF) {
        return memory.mmio2[address - 0xFF80];
    } else {
        printf("Warning: read from unmapped address 0x%04X\n", address);
        return 0xFF;
    }
}

uint16_t mem_read16(uint32_t address) {
    address = address & 0xFFFF; // mask
    if (address == 0xFFFF) {
        printf("an attempt is made to read out of memory: mem_read8(0x10000)\n");
        //exit(-1);
    }

    if (!(address % 2 == 0)) { // it's an odd address **
        printf("*ERROR: ODD ADDRESS!* mem_read16(%08X)\n", address);
        //exit(-1);
        //address &= ~1;
    }

    uint8_t hi = mem_read8(address);
    uint8_t lo = mem_read8(address + 1);
    return (hi << 8) | lo;
}

uint32_t mem_read32(uint32_t address) {
    address = address & 0xFFFF; // mask
    if (!(address % 2 == 0)) { // it's an odd address **
        //address &= ~1;
        printf("*ERROR: ODD ADDRESS!* mem_read32(%08X)\n", address);
        //exit(-1);
    }
    uint16_t hi = mem_read16(address);
    uint16_t lo = mem_read16(address + 2);
    return (hi << 16) | lo;
}

void mem_write8(uint32_t address, uint8_t value) {
    address = address & 0xFFFF; // mask
    if (address < 0x0000 || address > 0xFFFF) {
        printf("Invalid memory address access! mem_write8(0x%04X)\n", address);
        exit(-1);
    }

    if (0x0000 <= address && address <= 0xBFFF) {
        printf("Attempt to write to ROM: mem_write8(0x%04X)\n", address);
        //exit(-1);
        memory.rom[address] = value;
    } else if (0xF020 <= address && address <= 0xF0FF) {
        memory.mmio1[address - 0xF020] = value;
    } else if (0xF780 <= address && address <= 0xFF7F) {
        memory.ram[address - 0xF780] = value;
    } else if (0xFF80 <= address && address <= 0xFFFF) {
        memory.mmio2[address - 0xFF80] = value;
    } else {
        printf("Warning: write to unmapped address 0x%04X\n", address);
        printf("-> It's going to be ignored\n");
        exit(-1);
    }
}

void mem_write16(uint32_t address, uint16_t value) {
    address = address & 0xFFFF; // mask
    if (address == 0xFFFF) {
        printf("an attempt is made to write out of memory: mem_write8(0x10000)\n");
        exit(-1);
    }

    if (!(address % 2 == 0)) { // it's an odd address
        //address &= ~1;
        printf("*ERROR: ODD ADDRESS!* mem_write16(%08X)\n", address);
        //exit(-1);
    }

    mem_write8(address, value >> 8);       // hi
    mem_write8(address + 1, value & 0xFF); // lo
}

void mem_write32(uint32_t address, uint32_t value) {
    address = address & 0xFFFF; // mask
    if (address == 0xFFFF || address == 0xFFFE) {
        printf("an attempt is made to write out of memory *mem_write32*\n");
        exit(-1);
    }

    if (!(address % 2 == 0)) { // it's an odd address
        //address &= ~1;
        printf("*ERROR: ODD ADDRESS!* mem_write32(%08X)\n", address);
        //exit(-1);
    }

    mem_write16(address, value >> 16);        // hi
    mem_write16(address + 2, value & 0xFFFF); // lo
}