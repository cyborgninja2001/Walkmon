#include <stdio.h>

#include "cpu.h"
#include "eeprom.h"
#include "memory.h"
#include "exceptions.h"
#include "ssu.h"
#include "clock.h"
#include "timer_b1.h"
#include "timer_w.h"

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
    ssu_init();
    init_memory();
    init_power_down_registers();
    osccr_init();
    init_timer_b1();
    init_timer_w_registers();
    //mem_write8(0xF0E4, mem_read8(0xF0E4) | 0b00000100);
    while (true) {
        // why er7 (sp) is odd???? i think it shouldn't
        //if ((cpu.er[0x7] % 2) != 0) { exit(-1); }
        //if ((cpu.er[0x7] % 2) != 0) cpu.er[0x7] &= ~1;
        if ((cpu.pc % 2) != 0) { exit(-1); }
        //mem_write8(0xF0E4, 0xFF);
        cpu_step();
        update_timer_b1();
        update_timer_w();
        check_exceptions();
        cpu_debug();
    }
    return 0;
}
