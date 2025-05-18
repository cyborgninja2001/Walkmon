#include "exceptions.h"

extern CPU cpu;

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
    cpu.er[0x7] -= 2;
    mem_write16(cpu.er[0x7], cpu.pc & 0xFFFF);

    cpu.er[0x7] -= 2;
    mem_write16(cpu.er[0x7], (cpu.ccr << 8) | cpu.ccr); // push the ccr twice in normal mode

    //if (true) {} else {}
    set_I(true); // just for now

    cpu.pc = mem_read16(vector); // execution branches to that address
    cpu.cycles += 14; // ??
}

// Priority:
// 1. Reset
// 2. interrupts
// 3. Trap instruction

// Interrupts are not detected at the end of the ANDC, ORC, XORC,
// and LDC instructions, or immediately after reset exception handling.

// Interrupts can be external or internal
// TODO: create a queue with all interrupts request
void check_exceptions() {
    //set_I(false);
    //mem_write8(IEGR, 0xFF);
    //mem_write8(IRR1, 0xFF);
    //mem_write8(IENR1, 0xFF);

    uint8_t iegr = get_iegr();
    uint8_t ienr1 = get_ienr1();
    uint8_t ienr2 = get_ienr2();
    uint8_t irr1 = get_irr1();
    uint8_t irr2 = get_irr2();

    if (iegr & (1 << 7)) {
        if (!I(cpu.ccr)) handle_exception(NMI);
        return;
    } else if ((irr1 & 1) && (ienr1 & 1)) {
        if (!I(cpu.ccr)) handle_exception(IRQ0);
        return;
    } else if ((irr1 & (1 << 1)) && (ienr1 & (1 << 1))) {
        if (!I(cpu.ccr)) handle_exception(IRQ1);
        return;
    } else if ((irr1 & (1 << 2)) && (ienr1 & (1 << 2))) {
        if (!I(cpu.ccr)) handle_exception(IRQAEC);
        return;
    } else if ((irr2 & (1 << 6)) && (ienr2 & (1 << 6))) {
        if (!I(cpu.ccr)) handle_exception(AD_CONVERTER);
        return;
    } else {
        return;
    }
}
