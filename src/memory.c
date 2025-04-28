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