#include "exceptions.h"

extern CPU cpu;

uint16_t get_nmi() {
    cpu.cycles += 2;
    return mem_read16(0x000E);
}

uint8_t get_iegr() {
    cpu.cycles += 2;
    return mem_read8(IEGR);
}

uint8_t get_ienr1() {
    cpu.cycles += 2;
    return mem_read8(IENR1);
}

uint8_t get_ienr2() {
    cpu.cycles += 2;
    return mem_read8(IENR2);
}

uint8_t get_irr1() {
    cpu.cycles += 2;
    return mem_read8(IRR1);
}

uint8_t get_irr2() {
    cpu.cycles += 2;
    return mem_read8(IRR2);
}

// handles interrupts & TRAP instruction
void handle_exception(uint8_t vector) {
    uint16_t address = mem_read16(vector);

    cpu.er[0x7] -= 2;
    mem_write16(cpu.er[0x7], cpu.pc & 0xFFFF);

    cpu.er[0x7] -= 2;
    mem_write8(cpu.er[0x7], cpu.ccr);

    //if (true) {} else {}
    set_I(true); // just for now

    cpu.pc = address; // execution branches to that address
}

void check_exceptions() {
    if (I(cpu.ccr)) return; // if already there's an interrupt

    uint8_t iegr = get_iegr();
    uint8_t ienr1 = get_ienr1();
    uint8_t ienr2 = get_ienr2();
    uint8_t irr1 = get_irr1();
    uint8_t irr2 = get_irr2();

    // IRQ0
    if ((irr1 & 1) && (ienr1 & 1)) {
        handle_exception(IRQ0);
        return;
    }

    // RTC 0.25
    if (ienr1 & (1 << 7)) {
        handle_exception(RTC_025);
        return;
    }

    // IRQ1
    if ((irr1 & (1 << 1)) && (ienr1 & (1 << 1))) {
        handle_exception(0x0022);
        return;
    }

    // IRQAEC
    if ((irr1 & (1 << 2)) && (ienr1 & (1 << 2))) {
        handle_exception(0x0024);
        return;
    }
}