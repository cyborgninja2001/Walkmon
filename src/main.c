#include <stdio.h>

#include "cpu.h"
#include "eeprom.h"
#include "memory.h"
#include "exceptions.h"

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

    //printf("PC     -> memory: 0x%02X\n", mem_read8(0x2C4));     // opcode
    //printf("PC + 1 -> memory: 0x%02X\n", mem_read8(0x2C4 + 1)); // registro
    //printf("PC + 2 -> memory: 0x%02X\n", mem_read16(0x2C4 + 2)); // imm or xx

    //printf("OPCODE: 0x%02X %02X\n", mem_read8(0x2C8), mem_read8(0x2C9));
    //printf("OPCODE 1: 0x%02X %02X %02X\n", mem_read8(0xB0F2), mem_read8(0xB0F2 + 1), mem_read8(0xB0F2 + 2));

    //for (int i = 0x0000; i <= 0x0007; i++) {
    //    printf("%c", eeprom_read(i));
    //}
    //printf("\n");
    //exit(-1);
    //printf("OPCODE %02X\n", mem_read8(mem_read16(NMI)));
    //exit(-1);
    //mem_write8(IEGR, 0xFF);
    //set_I(false);
    //mem_write8(IRR1, 0xFF);
    //mem_write8(IENR1, 0xFF);
    extern CPU cpu;
    mem_write8(0xF0E4, 0xFF);
    while (true) {
        //mem_write8(0xF0E4, 0xFF);
        cpu_step();
        check_exceptions();
        cpu_debug();
    }
    return 0;
}
