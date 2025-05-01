#include "cpu.h"

static CPU cpu;

void set_rXl(uint8_t reg, uint8_t value) {
    cpu.er[reg] &= 0xFFFFFF00; // clear rXl
    cpu.er[reg] |= value;
}

void set_rXh(uint8_t reg, uint8_t value) {
    cpu.er[reg] &= 0xFFFF00FF; // clear rXh
    cpu.er[reg] |= ((uint32_t)value) << 8;
}

void set_rX(uint8_t reg, uint16_t value) {
    cpu.er[reg] &= 0xFFFF0000; // clear rX
    cpu.er[reg] |= value;
}

void set_eX(uint8_t reg, uint16_t value) {
    cpu.er[reg] &= 0x0000FFFF; // clear eX
    cpu.er[reg] |= ((uint32_t) value) << 16;
}

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
    uint32_t vector = (mem_read16(0x0000) << 16); // hi
    vector |= mem_read16(0x0002);                 // lo
    cpu.pc = vector & 0xFFFFFF;                   // only use 24 bits

    // just in case
    if (cpu.pc == 0xFFFFFF || cpu.pc == 0x000000) {
        printf("Warning: invalid reset vector: 0x%06X\n", cpu.pc);
    }

    cpu.ccr = 0;
    set_I(true);

    // the other CCR bits and the general registers and
    // extended registers are not initialized

    // just in case:
    for (int i = 0; i < 8; i++) {
        cpu.er[i] = 0x00000000;
    }

    cpu.cycles = 0;
}

// *INSTRUCTIONS*
static void nop() {
    // it does nothing
    cpu.cycles += 2;
}

// MOV.B Rs, Rd
static void mov_b_r_r(uint8_t rs, uint8_t rd) {
    uint8_t value;

    if ((rs & 0x8) >> 3) {              // RnL
        value =  rXl(cpu.er[rs & 0x7]);
    } else {                            // RnH
        value = rXh(cpu.er[rs & 0x7]);
    }

    if ((rd & 0x8) >> 3) {  // RnL
        set_rXl(rd & 0x7, value);
    } else {                // RnH
        set_rXh(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 2;
}

// MOV.W Rs, Rd
static void mov_w_r_r(uint8_t rs, uint8_t rd) {
    uint16_t value;

    if ((rs & 0x8) >> 3) {              // En
        value =  eX(cpu.er[rs & 0x7]);
    } else {                            // Rn
        value = rX(cpu.er[rs & 0x7]);
    }

    if ((rd & 0x8) >> 3) {  // En
        set_eX(rd & 0x7, value);
    } else {                // Rn
        set_rX(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 2;
}

// MOV.L Rs, Rd
static void mov_l_r_r(uint8_t rs, uint8_t rd) {
    uint32_t value = cpu.er[rs & 0x7];
    cpu.er[rd] = value;

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 2;
}

// *MOV.B (EAs), Rd*:

// MOV.B #xx:8, Rd
static void mov_b_xx_r(uint8_t xx, uint8_t rd) {
    if ((rd & 0x8) >> 3) {  // RnL
        set_rXl(rd & 0x7, xx);
    } else {                // RnH
        set_rXh(rd & 0x7, xx);
    }

    // set the flags
    set_N(xx & 0x80);
    set_Z(xx == 0);
    set_V(false);

    cpu.cycles += 2;
}

uint16_t cpu_fetch16() {
    uint16_t opcode = mem_read16(cpu.pc & 0xFFFF); // normal mode (16 bits)
    cpu.pc += 2;
    return opcode;
}

void cpu_step() {
    uint16_t opcode = cpu_fetch16();

    switch (opcode & 0xFF00) {
        case 0x0000: nop(); break;                                          // NOP
        case 0x0C00: mov_b_r_r((opcode & 0xF0) >> 4, opcode & 0x0F); break; // MOV.B Rs, Rd
        case 0x0D00: mov_w_r_r((opcode & 0xF0) >> 4, opcode & 0x0F); break; // MOV.W Rs, Rd
        case 0x0F00: mov_l_r_r((opcode & 0xF0) >> 4, opcode & 0x0F); break; // MOV.L ERs, ERd

        default:
            printf("Error: opcode not implemented: 0x%04X in PC=0x%06X\n", opcode, cpu.pc - 2);
            exit(-1);
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