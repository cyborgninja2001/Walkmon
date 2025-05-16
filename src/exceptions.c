#include "exceptions.h"

uint16_t get_nmi() {
    return mem_read16(0x000E);
}

uint8_t get_iegr() {
    cpu_cycles(2);
    return mem_read8(IEGR);
}

uint8_t get_ienr1() {
    cpu_cycles(2);
    return mem_read8(IENR1);
}

uint8_t get_ienr2() {
    cpu_cycles(2);
    return mem_read8(IENR2);
}

uint8_t get_irr1() {
    cpu_cycles(2);
    return mem_read8(IRR1);
}

uint8_t get_irr2() {
    cpu_cycles(2);
    return mem_read8(IRR2);
}

void handle_exception(uint8_t vector) {}

void check_exceptions() {}
