#include "cpu.h"

static CPU cpu;

void set_I(bool v) {
    if (v) {
        cpu.ccr |= (1 << 7);
    } else {
        cpu.ccr &= ~(1 << 7);
    }
}

void set_UI(bool v) {
    if (v) {
        cpu.ccr |= (1 << 6);
    } else {
        cpu.ccr &= ~(1 << 6);
    }
}

void set_H(bool v) {
    if (v) {
        cpu.ccr |= (1 << 5);
    } else {
        cpu.ccr &= ~(1 << 5);
    }
}

void set_U(bool v) {
    if (v) {
        cpu.ccr |= (1 << 4);
    } else {
        cpu.ccr &= ~(1 << 4);
    }
}

void set_N(bool v) {
    if (v) {
        cpu.ccr |= (1 << 3);
    } else {
        cpu.ccr &= ~(1 << 3);
    }
}

void set_Z(bool v) {
    if (v) {
        cpu.ccr |= (1 << 2);
    } else {
        cpu.ccr &= ~(1 << 2);
    }
}

void set_V(bool v) {
    if (v) {
        cpu.ccr |= (1 << 1);
    } else {
        cpu.ccr &= ~(1 << 1);
    }
}

void set_C(bool v) {
    if (v) {
        cpu.ccr |= 1;
    } else {
        cpu.ccr &= ~1;
    }
}

void cpu_reset() {
    // pc is loaded from the vector table
    cpu.pc = 0x00000000;

    cpu.ccr = 0;
    set_I(true);

    // the other CCR bits and the general registers and
    // extended registers are not initialized

    // just in case:
    for (int i = 0; i < 8; i++) {
        cpu.er[i] = 0x00000000;
    }
}

void cpu_debug() {
    for (int i = 0; i < 8; i++) {
        printf("er%d: %08X\n", i, cpu.er[i]);
    }
    printf("pc: %08X\n", cpu.pc);
    printf("ccr: %02X\n", cpu.ccr);
    printf("I:%d UI:%d H:%d U:%d N:%d Z:%d V:%d C:%d \n",
          I(cpu.ccr), UI(cpu.ccr), H(cpu.ccr), U(cpu.ccr),
          N(cpu.ccr), Z(cpu.ccr), V(cpu.ccr), C(cpu.ccr));
}