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

// MOV.B @ers, Rd
static void mov_addr_ers_rd(uint8_t ers, uint8_t rd) {
    uint8_t value = mem_read8(cpu.er[ers & 0x7] & 0x00FFFFFF); // we use the lower 24 bits

    if ((rd & 0x8) >> 3) {  // RnL
        set_rXl(rd & 0x7, value);
    } else {                // RnH
        set_rXh(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 4;
}

// MOV.B @(d:16, ERs), Rd
static void mov_b_disp16_addr_ers_rd(uint16_t disp, uint8_t ers, uint8_t rd) {
    uint32_t address = (cpu.er[ers & 0x7] + disp) & 0x00FFFFFF; // we use the lower 24 bits
    uint8_t value = mem_read8(address);

    if ((rd & 0x8) >> 3) {  // RnL
        set_rXl(rd & 0x7, value);
    } else {                // RnH
        set_rXh(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.B @(d:24, ERs), Rd
static void mov_b_disp24_addr_ers_rd(uint32_t disp, uint8_t ers, uint8_t rd) {
    uint32_t address = (cpu.er[ers & 0x7] + disp) & 0x00FFFFFF; // we use the lower 24 bits
    uint8_t value = mem_read8(address);

    if ((rd & 0x8) >> 3) {  // RnL
        set_rXl(rd & 0x7, value);
    } else {                // RnH
        set_rXh(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.B @ERs+, Rd
static void mov_b_addr_ers_plus_rd(uint8_t ers, uint8_t rd) {
    uint32_t address = (cpu.er[ers & 0x7] & 0x00FFFFFF); // we use the lower 24 bits
    uint8_t value = mem_read8(address);

    if ((rd & 0x8) >> 3) {  // RnL
        set_rXl(rd & 0x7, value);
    } else {                // RnH
        set_rXh(rd & 0x7, value);
    }

    // increment @Ers
    cpu.er[ers & 0x7] += 1;

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

uint8_t cpu_fetch8() {
    uint8_t data = mem_read8(cpu.pc & 0xFFFF); // normal mode (16 bits)
    cpu.pc += 1;
    return data;
}

uint16_t cpu_fetch16() {
    uint16_t opcode = mem_read16(cpu.pc & 0xFFFF); // normal mode (16 bits)
    cpu.pc += 2;
    return opcode;
}

void cpu_step() {
    uint8_t opcode = cpu_fetch8();

    switch (opcode) {
        case 0x00: nop(); break; // NOP
        case 0x0C: { // MOV.B Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            mov_b_r_r((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            break;
        }
        case 0x0D: { // MOV.W Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            mov_w_r_r((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            break;
        }
        case 0x0F: { // MOV.L Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            mov_l_r_r((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            break;
        }
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF: { // MOV.B #xx:8, Rd
            uint8_t imm = cpu_fetch8();
            mov_b_xx_r(imm, opcode & 0x0F);
            break;
        }
        case 0x68: { // MOV.B @ers, Rd
            uint8_t second_byte = cpu_fetch8();
            mov_addr_ers_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            break;
        }
        case 0x6E: { // MOV.B @(d:16, ERs), Rd
            uint8_t second_byte = cpu_fetch8();
            uint16_t disp = cpu_fetch16(); // 3rd & 4th byte
            mov_b_disp16_addr_ers_rd(disp, (second_byte & 0xF0) >> 4, second_byte & 0x0F);
            break;
        }
        case 0x78: { // MOV.B @(d:24, ERs), Rd
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            uint8_t fifth_byte = cpu_fetch8(); // it only has 0's

            if ((third_byte != 0x6A) || ((fourth_byte & 0xF0) != 0x20) || (fifth_byte != 0x00)) {
                printf("Error: unknown extended MOV.B format at PC=0x%06X\n", cpu.pc - 5);
                exit(-1);
            }

            //disp
            uint8_t sixth_byte = cpu_fetch8();
            uint8_t seventh_byte = cpu_fetch8();
            uint8_t eight_byte = cpu_fetch8();

            uint32_t disp = (sixth_byte << 16) | (seventh_byte << 8) | eight_byte;
            mov_b_disp24_addr_ers_rd(disp, (second_byte & 0xF0) >> 4, fourth_byte & 0x0F);
            break;
        }
        case 0x6C: { // MOV.B @ERs+, Rd
            uint8_t second_byte = cpu_fetch8();
            mov_b_addr_ers_plus_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            break;
        }
        default:
            printf("Error: opcode not implemented: 0x%02X in PC=0x%06X\n", opcode, cpu.pc - 2);
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