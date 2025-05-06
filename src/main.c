#include <stdio.h>

#include "cpu.h"
#include "eeprom.h"
#include "memory.h"

int main(int argc, char *argv[]) {
    bool loaded_rom = load_rom(argv[1]);
    if (loaded_rom) {
        printf("ROM LOADED SUCCESSFULLY!\n");
    } else {
        printf("COULDN'T LOAD ROM\n");
    }
    bool loaded_eeprom = eeprom_load(argv[2]);
    if (loaded_eeprom) {
        printf("EEPROM LOADED SUCCESSFULLY!\n");
    } else {
        printf("COULDN'T LOAD EEPROM\n");
    }
    cpu_reset();
    cpu_debug();

    while (true) {
        cpu_step();
        cpu_debug();
    }
    return 0;
}
