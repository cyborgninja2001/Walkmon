#include "clock.h"

extern CPU cpu;

void osccr_init() {
    mem_write8(OSCCR, 0); // OSCF bit depends on the value of E7_2 Pin
}

uint8_t get_osccr() {
    cpu.cycles += 2;
    return mem_read8(OSCCR);
}

void set_osccr(uint8_t value) {
    mem_write8(OSCCR, value);
    cpu.cycles += 2;
}