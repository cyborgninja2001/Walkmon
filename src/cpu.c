#include "cpu.h"

//static CPU cpu;
CPU cpu;

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
    uint32_t vector =  mem_read16(0x0000); // hi
    //vector |= mem_read16(0x0002);                 // lo
    cpu.pc = vector;                   // 0x02C4

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
    cpu.halted = false;
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
static void mov_b_addr_ers_rd(uint8_t ers, uint8_t rd) {
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
// *The MOV.B @ER7+, Rd instruction should never be used*
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

// MOV.B @aa:8, Rd
static void mov_b_aa8_rd(uint8_t aa, uint8_t rd) {
    uint8_t value = mem_read8(aa);

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

// MOV.B @aa:16, Rd
static void mov_b_aa16_rd(uint16_t aa, uint8_t rd) {
    uint8_t value = mem_read8(aa);

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

// MOV.B @aa:24, Rd
static void mov_b_aa24_rd(uint32_t aa, uint8_t rd) {
    uint8_t value = mem_read8(aa);

    if ((rd & 0x8) >> 3) {  // RnL
        set_rXl(rd & 0x7, value);
    } else {                // RnH
        set_rXh(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 8;
}

// *The source operand <EAs> must be located at an even address*

// MOV.W #xx:16, Rd
static void mov_w_xx_rd(uint16_t xx, uint8_t rd) {
    if ((rd & 0x8) >> 3) { // En
        set_eX(rd & 0x7, xx);
    } else {               // Rn
        set_rX(rd & 0x7, xx);
    }

    // set the flags
    set_N(xx & 0x8000);
    set_Z(xx == 0);
    set_V(false);

    cpu.cycles += 4;
}

// MOV.W @ERs, Rd
static void mov_w_addr_ers_rd(uint8_t ers, uint8_t rd) {
    uint16_t value = mem_read16(cpu.er[ers & 0x7] & 0x00FFFFFF);

    if ((rd & 0x8) >> 3) { // En
        set_eX(rd & 0x7, value);
    } else {               // Rn
        set_rX(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 4;
}

// MOV.W @(d:16, Ers), Rd
static void mov_w_disp16_addr_ers_rd(uint16_t disp, uint8_t ers, uint8_t rd) {
    uint32_t address = (cpu.er[ers & 0x7] + disp) & 0x00FFFFFF;
    uint16_t value = mem_read16(address);

    if ((rd & 0x8) >> 3) { // En
        set_eX(rd & 0x7, value);
    } else {               // Rn
        set_rX(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.W @(d:24, ERs), Rd
static void mov_w_disp24_addr_ers_rd(uint32_t disp, uint8_t ers, uint8_t rd) {
    uint32_t address = (cpu.er[ers & 0x7] + disp) & 0x00FFFFFF;
    uint16_t value = mem_read16(address);

    if ((rd & 0x8) >> 3) { // En
        set_eX(rd & 0x7, value);
    } else {               // Rn
        set_rX(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.W @ERs+, Rd
static void mov_w_addr_ers_plus_rd(uint8_t ers, uint8_t rd) {
    printf("****ERS: %02X ****\n", ers & 0x7);
    uint32_t address = cpu.er[ers & 0x7] & 0xFFFF; //& 0x00FFFFFF;
    printf("***ADDRESS: %04X\n ****", address & 0xFFFF);
    printf("ERS + 2 %08X\n", cpu.er[ers & 0x7] + 2);
    // register value should be even
    if (!((address % 2) == 0)) {
        printf("ERROR: MOV.W @ERs+, Rd. ERs should be even!\n");
        //exit(-1);
    }
    uint16_t value = mem_read16(address);

    cpu.er[ers & 0x7] += 2;

    if ((rd & 0x8) >> 3) { // En
        set_eX(rd & 0x7, value);
    } else {               // Rn
        set_rX(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.W @aa:16, Rd
static void mov_w_aa16_rd(uint16_t aa, uint8_t rd) {
    uint16_t value = mem_read16(aa);

    if ((rd & 0x8) >> 3) { // En
        set_eX(rd & 0x7, value);
    } else {               // Rn
        set_rX(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.W @aa:24, Rd
static void mov_w_aa24_rd(uint32_t aa, uint8_t rd) {
    uint16_t value = mem_read16(aa);

    if ((rd & 0x8) >> 3) { // En
        set_eX(rd & 0x7, value);
    } else {               // Rn
        set_rX(rd & 0x7, value);
    }

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 8;
}

// MOV.L #xx:32, Rd
static void mov_l_xx_rd(uint32_t xx, uint8_t rd) {
    cpu.er[rd & 0x7] = xx;

    // set the flags
    set_N(xx & 0x80000000);
    set_Z(xx == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.L @ERs, ERd
static void mov_l_addr_ers_erd(uint8_t ers, uint8_t erd) {
    uint32_t address = cpu.er[ers & 0x7];
    uint32_t value = mem_read32(address);
    cpu.er[erd & 0x7] = value;

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 8;
}

// MOV.L @(d:16, ERs), ERd
static void mov_l_disp16_addr_ers_erd(uint16_t disp, uint8_t ers, uint8_t erd) {
    uint32_t address = (cpu.er[ers & 0x7] + disp) & 0x00FFFFFF; // only 24 bits
    uint32_t value = mem_read32(address);

    cpu.er[erd & 0x7] = value;

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.L @(d:24, ERs), ERd
static void mov_l_disp24_addr_ers_erd(uint32_t disp, uint8_t ers, uint8_t erd) {
    uint32_t address = (cpu.er[ers & 0x7] + disp) & 0x00FFFFFF; // only 24 bits
    uint32_t value = mem_read32(address);

    cpu.er[erd & 0x7] = value;

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 14;
}

// MOV.L @ERs+, ERd
static void mov_l_addr_ers_plus_erd(uint8_t ers, uint8_t erd) {
    uint32_t value = mem_read32(cpu.er[ers & 0x7]);
    cpu.er[erd & 0x7] = value;
    cpu.er[ers & 0x7] += 4;

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.L @aa:16, ERd
static void mov_l_aa16_erd(uint16_t aa, uint8_t erd) {
    uint32_t value = mem_read32(aa);
    cpu.er[erd & 0x7] = value;

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.L @aa:24, ERd
static void mov_l_aa24_erd(uint32_t aa, uint8_t erd) {
    uint32_t value = mem_read32(aa);
    cpu.er[erd & 0x7] = value;

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 12;
}

// MOV.B Rs, @ERd
static void mov_b_rs_addr_erd(uint8_t rs, uint8_t erd) {
    uint8_t value;

    if ((rs & 0x8) >> 3) {  // RnL
        value = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        value = rXh(cpu.er[rs & 0x7]);
    }

    uint32_t address = cpu.er[erd & 0x7];

    mem_write8(address, value);

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 4;
}

// MOV.B Rs, @(d:16, ERd)
static void mov_b_rs_disp16_addr_erd(uint16_t disp , uint8_t rs, uint8_t erd) {
    uint8_t value;

    if ((rs & 0x8) >> 3) {  // RnL
        value = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        value = rXh(cpu.er[rs & 0x7]);
    }

    uint32_t address = cpu.er[erd & 0x7] + disp;

    mem_write8(address, value);

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.B Rs, @(d:24, ERd)
static void mov_b_rs_disp24_addr_erd(uint32_t disp, uint8_t rs, uint8_t erd) {
    uint8_t value;

    if ((rs & 0x8) >> 3) {  // RnL
        value = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        value = rXh(cpu.er[rs & 0x7]);
    }

    uint32_t address = cpu.er[erd & 0x7] + disp;

    mem_write8(address, value);

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.B Rs, @-ERs
static void mov_b_rs_minus_addr_ers(uint8_t rs, uint8_t ers) {
    cpu.er[ers & 0x7] -= 1;
    uint32_t address  = cpu.er[ers & 0x7];

    uint8_t value;

    if ((rs & 0x8) >> 3) {  // RnL
        value = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        value = rXh(cpu.er[rs & 0x7]);
    }

    mem_write8(address, value);

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.B Rs, @aa:8
static void mov_b_rs_aa8(uint8_t rs, uint8_t aa) {
    uint8_t value;

    if ((rs & 0x8) >> 3) {  // RnL
        value = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        value = rXh(cpu.er[rs & 0x7]);
    }

    mem_write8(aa, value);

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 4;
}

// MOV.B Rs, @aa:16
static void mov_b_rs_aa16(uint8_t rs, uint16_t aa) {
    uint8_t value;

    if ((rs & 0x8) >> 3) {  // RnL
        value = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        value = rXh(cpu.er[rs & 0x7]);
    }

    mem_write8(aa, value);

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.B Rs, @aa:24
static void mov_b_rs_aa24(uint8_t rs, uint32_t aa) {
    uint8_t value;

    if ((rs & 0x8) >> 3) {  // RnL
        value = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        value = rXh(cpu.er[rs & 0x7]);
    }

    mem_write8(aa, value);

    // set the flags
    set_N(value & 0x80);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 8;
}

// MOV.W Rs, @ERd
static void mov_w_rs_addr_erd(uint8_t rs, uint8_t erd) {
    uint16_t value;
    if ((rs & 0x8) >> 3) { // En
        value = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        value = rX(cpu.er[rs & 0x7]);
    }

    mem_write16(cpu.er[erd & 0x7], value);

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 4;
}

// MOV.W Rs, @(d:16, Erd)
static void mov_w_rs_disp16_addr_erd(uint8_t rs, uint16_t disp, uint8_t erd) {
    uint16_t value;
    if ((rs & 0x8) >> 3) { // En
        value = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        value = rX(cpu.er[rs & 0x7]);
    }

    uint32_t address = cpu.er[erd & 0x7] + disp;
    mem_write16(address, value);

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.W Rs, @(d:24, Erd)
static void mov_w_rs_disp24_addr_erd(uint8_t rs, uint32_t disp, uint8_t erd) {
    uint16_t value;
    if ((rs & 0x8) >> 3) { // En
        value = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        value = rX(cpu.er[rs & 0x7]);
    }

    uint32_t address = cpu.er[erd & 0x7] + disp;
    mem_write16(address, value);

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.W Rs, @-ERd
static void mov_w_rs_minus_addr_erd(uint8_t rs, uint8_t erd) {
    cpu.er[erd & 0x7] -= 2;
    uint32_t address = cpu.er[erd & 0x7];

    uint16_t value;
    if ((rs & 0x8) >> 3) { // En
        value = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        value = rX(cpu.er[rs & 0x7]);
    }

    mem_write16(address, value);

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.W Rs, @aa:16
static void mov_w_rs_aa16(uint8_t rs, uint16_t aa) {
    uint16_t value;
    if ((rs & 0x8) >> 3) { // En
        value = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        value = rX(cpu.er[rs & 0x7]);
    }

    mem_write16(aa, value);

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 6;
}

// MOV.W Rs, @aa:24
static void mov_w_rs_aa24(uint8_t rs, uint32_t aa) {
    uint16_t value;
    if ((rs & 0x8) >> 3) { // En
        value = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        value = rX(cpu.er[rs & 0x7]);
    }

    mem_write16(aa, value);

    // set the flags
    set_N(value & 0x8000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 8;
}

// MOV.L Ers, @Erd
static void mov_l_ers_addr_erd(uint8_t ers, uint8_t erd) {
    uint32_t value = cpu.er[ers & 0x7];
    uint32_t address = cpu.er[erd & 0x7];
    mem_write32(address, value);

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 8;
}

// MOV.L ERs, @(d:16, Erd)
static void mov_l_ers_disp16_addr_erd(uint8_t ers, uint16_t disp, uint8_t erd) {
    uint32_t value = cpu.er[ers & 0x7];
    uint32_t address = cpu.er[erd & 0x7] + disp;
    mem_write32(address, value);

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.L ERs, @(d:24, Erd)
static void mov_l_ers_disp24_addr_erd(uint8_t ers, uint32_t disp, uint8_t erd) {
    uint32_t value = cpu.er[ers & 0x7];
    uint32_t address = cpu.er[erd & 0x7] + disp;
    mem_write32(address, value);

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 14;
}

// MOV.L Ers, @-Erd
static void mov_l_ers_minus_addr_erd(uint8_t ers, uint8_t erd) {
    cpu.er[erd & 0x7] -=4;
    uint32_t address = cpu.er[erd & 0x7]; // this should be even
    uint32_t value = cpu.er[ers & 0x7];

    mem_write32(address, value);

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.L Ers, @aa:16
static void mov_l_ers_aa16(uint8_t ers, uint16_t aa) {
    uint32_t value = cpu.er[ers & 0x7];
    mem_write32(aa, value);

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 10;
}

// MOV.L Ers, @aa:24
static void mov_l_ers_aa24(uint8_t ers, uint32_t aa) {
    uint32_t value = cpu.er[ers & 0x7];
    mem_write32(aa, value);

    // set the flags
    set_N(value & 0x80000000);
    set_Z(value == 0);
    set_V(false);

    cpu.cycles += 12;
}

// ADD.B #xx:8, Rd
static void add_b_xx_rd(uint8_t xx, uint8_t rd) {
    uint8_t a;
    uint8_t b = xx;
    uint8_t result;

    if ((rd & 0x8) >> 3) {  // RnL
        a = rXl(cpu.er[rd & 0x7]);
        result = a + b;
        set_rXl(rd & 0x7, result);
    } else {                // RnH
        a = rXh(cpu.er[rd & 0x7]);
        result = a + b;
        set_rXh(rd & 0x7, result);
    }

    // set the flags
    set_H(((a & 0x0F) + (b & 0x0F)) > 0x0F);
    set_N(result & 0x80);
    set_Z(result == 0);
    set_V(((a ^ result) & (b ^ result) & 0x80) != 0);
    set_C((a + b) > 0xFF);

    cpu.cycles += 2;
}

// ADD.B Rs, Rd
static void add_b_rs_rd(uint8_t rs, uint8_t rd) {
    uint8_t a;
    uint8_t b;
    uint8_t result;

    if ((rs & 0x8) >> 3) {  // RnL
        a = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        a = rXh(cpu.er[rs & 0x7]);
    }

    if ((rd & 0x8) >> 3) {  // RnL
        b = rXl(cpu.er[rd & 0x7]);
        result = a + b;
        set_rXl(rd & 0x7, result);
    } else {                // RnH
        b = rXh(cpu.er[rd & 0x7]);
        result = a + b;
        set_rXh(rd & 0x7, result);
    }

    // set the flags
    set_H(((a & 0x0F) + (b & 0x0F)) > 0x0F);
    set_N(result & 0x80);
    set_Z(result == 0);
    set_V(((a ^ result) & (b ^ result) & 0x80) != 0);
    set_C((a + b) > 0xFF);

    cpu.cycles += 2;
}

// ADD.W #xx:16, Rd
static void add_w_xx_rd(uint16_t xx, uint8_t rd) {
    uint16_t a = xx;
    uint16_t b;
    uint16_t result;
    if ((rd & 0x8) >> 3) { // En
        b = eX(cpu.er[rd & 0x7]);
        result = a + b;
        set_eX(rd & 0x7, result);
    } else {               // Rn
        b = rX(cpu.er[rd & 0x7]);
        result = a + b;
        set_rX(rd & 0x7, result);
    }

    // set the flags
    set_H(((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF); // ? check
    set_N(result & 0x8000);
    set_Z(result == 0);
    set_V(((a ^ result) & (b ^ result) & 0x8000) != 0);
    set_C((uint32_t)a + (uint32_t)b > 0xFFFF);

    cpu.cycles += 4;
}

// ADD.W Rs, Rd
static void add_w_rs_rd(uint8_t rs, uint8_t rd) {
    uint16_t a;
    uint16_t b;
    uint16_t result;
    if ((rs & 0x8) >> 3) { // En
        a = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        a = rX(cpu.er[rs & 0x7]);
    }

    if ((rd & 0x8) >> 3) { // En
        b = eX(cpu.er[rd & 0x7]);
        result = a + b;
        set_eX(rd & 0x7, result);
    } else {               // Rn
        b = rX(cpu.er[rd & 0x7]);
        result = a + b;
        set_rX(rd & 0x7, result);
    }

    // set the flags
    set_H(((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF); // ? check
    set_N(result & 0x8000);
    set_Z(result == 0);
    set_V(((a ^ result) & (b ^ result) & 0x8000) != 0);
    set_C((uint32_t)a + (uint32_t)b > 0xFFFF);

    cpu.cycles += 2;
}

// ADD.L #xx:32, ERd
static void add_l_xx_rd(uint32_t xx, uint8_t erd) {
    uint32_t a = xx;
    uint32_t b = cpu.er[erd & 0x7];
    uint32_t result = a + b;

    cpu.er[erd & 0x7] = result;

    // set the flags
    set_H(((a & 0x0FFFFFFF) + (b & 0x0FFFFFFF)) > 0x0FFFFFFF); // carry on bit 27
    set_N(result & 0x80000000);                                // bit 31
    set_Z(result == 0);
    set_V(((a ^ result) & (b ^ result) & 0x80000000) != 0);     // overflow with sign
    set_C(((uint64_t)a + (uint64_t)b) > 0xFFFFFFFF);            // carry on bit 31

    cpu.cycles += 6;
}

// ADD.L Rs, Erd
static void add_l_rs_erd(uint8_t ers, uint8_t erd) {
    uint32_t a = cpu.er[ers & 0x7];
    uint32_t b = cpu.er[erd & 0x7];
    uint32_t result = a + b;

    cpu.er[erd & 0x7] = result;

    // set the flags
    set_H(((a & 0x0FFFFFFF) + (b & 0x0FFFFFFF)) > 0x0FFFFFFF); // carry on bit 27
    set_N(result & 0x80000000);                                // bit 31
    set_Z(result == 0);
    set_V(((a ^ result) & (b ^ result) & 0x80000000) != 0);     // overflow with sign
    set_C(((uint64_t)a + (uint64_t)b) > 0xFFFFFFFF);            // carry on bit 31

    cpu.cycles += 2;
}

// SUB.B Rs, Rd
static void sub_b_rs_rd(uint8_t rs, uint8_t rd) {
    uint8_t a;
    uint8_t b;
    uint8_t result;

    if ((rs & 0x8) >> 3) {  // RnL
        a = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        a = rXh(cpu.er[rs & 0x7]);
    }

    if ((rd & 0x8) >> 3) {  // RnL
        b = rXl(cpu.er[rd & 0x7]);
        result = b - a;
        set_rXl(rd & 0x7, result);
    } else {                // RnH
        b = rXh(cpu.er[rd & 0x7]);
        result = b - a;
        set_rXh(rd & 0x7, result);
    }

    // set the flags
    set_H((b & 0x0F) < (a & 0x0F));
    set_N(result & 0x80);
    set_Z(result == 0);
    set_V(((b ^ a) & (b ^ result) & 0x80) != 0);
    set_C(b < a);

    cpu.cycles += 2;
}

// SUB.W #xx:16, Rd
static void sub_w_xx_rd(uint16_t xx, uint8_t rd) {
    uint16_t a = xx;
    uint16_t b;
    uint16_t result;

    if ((rd & 0x8) >> 3) {  // En
        b = eX(cpu.er[rd & 0x7]);
        result = b - a;
        set_eX(rd & 0x7, result);
    } else {                // Rn
        b = rX(cpu.er[rd & 0x7]);
        result = b - a;
        set_rX(rd & 0x7, result);
    }

    // set the flags
    set_H((b & 0x0FFF) < (a & 0x0FFF));
    set_N(result & 0x8000);
    set_Z(result == 0);
    set_V(((b ^ a) & (b ^ result) & 0x8000) != 0);
    set_C(b < a);

    cpu.cycles += 4;
}

// SUB.W Rs, Rd
static void sub_w_rs_rd(uint8_t rs, uint8_t rd) {
    uint16_t a;
    uint16_t b;
    uint16_t result;

    if ((rs & 0x8) >> 3) {  // En
        a = eX(cpu.er[rs & 0x7]);
    } else {                // Rn
        a = rX(cpu.er[rs & 0x7]);
    }

    if ((rd & 0x8) >> 3) {  // En
        b = eX(cpu.er[rd & 0x7]);
        result = b - a;
        set_eX(rd & 0x7, result);
    } else {                // Rn
        b = rX(cpu.er[rd & 0x7]);
        result = b - a;
        set_rX(rd & 0x7, result);
    }

    // set the flags
    set_H((b & 0x0FFF) < (a & 0x0FFF));
    set_N(result & 0x8000);
    set_Z(result == 0);
    set_V(((b ^ a) & (b ^ result) & 0x8000) != 0);
    set_C(b < a);

    cpu.cycles += 2;
}

// SUB.L #xx:32, ERd
static void sub_l_xx_erd(uint32_t xx, uint8_t erd) {
    uint32_t a = xx;
    uint32_t b = cpu.er[erd & 0x7];
    uint32_t result  = b - a;
    cpu.er[erd & 0x7] = result;

    // set the flags
    set_H((b & 0x0FFFFFFF) < (a & 0x0FFFFFFF));
    set_N(result & 0x80000000);
    set_Z(result == 0);
    set_V(((b ^ a) & (b ^ result) & 0x80000000) != 0);
    set_C(b < a);

    cpu.cycles += 6;
}

// SUB.L ERs, ERd
static void sub_l_ers_erd(uint8_t ers, uint8_t erd) {
    uint32_t a = cpu.er[ers & 0x7];
    uint32_t b = cpu.er[erd & 0x7];
    uint32_t result  = b - a;
    cpu.er[erd & 0x7] = result;

    // set the flags
    set_H((b & 0x0FFFFFFF) < (a & 0x0FFFFFFF));
    set_N(result & 0x80000000);
    set_Z(result == 0);
    set_V(((b ^ a) & (b ^ result) & 0x80000000) != 0);
    set_C(b < a);

    cpu.cycles += 2;
}

// SUBS #1, ERd
static void subs_1_erd(uint8_t erd) {
    cpu.er[erd & 0x7] -= 1;
    cpu.cycles += 2;
}

// SUBS #2, ERd
static void subs_2_erd(uint8_t erd) {
    cpu.er[erd & 0x7] -= 2;
    cpu.cycles += 2;
}

// SUBS #4, ERd
static void subs_4_erd(uint8_t erd) {
    cpu.er[erd & 0x7] -= 4;
    cpu.cycles += 2;
}

// (Jump to Subroutine)
// JSR @ERn
static void jsr_addr_ern(uint8_t ern) {
    // MOV.W Rs, @-ERd = PUSH.W (PC & 0xFFFF)
    cpu.er[0x7] -= 2; // SP -= 2
    uint32_t sp_address = cpu.er[0x7] & 0x00FFFFFF; // only 24 bits
    uint16_t value = cpu.pc & 0xFFFF; // in normal mode only save 16 bits
    mem_write16(sp_address, value);
    cpu.pc = cpu.er[ern & 0x7] & 0x00FFFFFF; // only 24 bits (*CHECK*)
    cpu.cycles += 6;
}

// JSR @aa:24
static void jsr_aa24(uint32_t aa) {
    cpu.er[0x7] -= 2; // SP -= 2
    uint32_t sp_address = cpu.er[0x7] & 0x00FFFFFF; // only 24 bits
    uint16_t value = cpu.pc & 0xFFFF; // in normal mode only save 16 bits
    mem_write16(sp_address, value);
    cpu.pc = aa;
    cpu.cycles += 8;
}

// JSR @@aa:8 (****CHECK****)
static void jsr_aa8(uint8_t aa) {
    cpu.er[0x7] -= 2; // SP -= 2
    uint32_t sp_address = cpu.er[0x7] & 0x00FFFFFF; // only 24 bits
    uint16_t value = cpu.pc & 0xFFFF; // in normal mode only save 16 bits
    mem_write16(sp_address, value);

    uint16_t address = mem_read16(aa);
    cpu.pc = address;
    cpu.cycles += 8;
}

//Bcc:

// BRA (BT) d:8
static void bra8(uint8_t disp) {
    // condition always true
    cpu.pc += (int8_t)disp;
    cpu.cycles += 4;
}

// BRA (BT) d:16
static void bra16(uint16_t disp) {
    // condition always true
    cpu.pc += (int16_t)disp;
    cpu.cycles += 6;
}

// BRN (BF) d:8
static void brn8(uint8_t disp) {
    // condition always false
    if (false) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BRN (BF) d:16
static void brn16(uint16_t disp) {
    // condition always false
    if (false) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BHI d:8
static void bhi8(uint8_t disp) {
    if ((C(cpu.ccr) | Z(cpu.ccr)) == 0) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BHI d:16
static void bhi16(uint16_t disp) {
    if ((C(cpu.ccr) | Z(cpu.ccr)) == 0) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BLS d:8
static void bls8(uint8_t disp) {
    if ((C(cpu.ccr) | Z(cpu.ccr)) == 1) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BLS d:16
static void bls16(uint16_t disp) {
    if ((C(cpu.ccr) | Z(cpu.ccr)) == 1) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// Bcc (BHS) d:8
static void bcc_bhs8(uint8_t disp) {
    if (C(cpu.ccr) == 0) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// Bcc (BHS) d:16
static void bcc_bhs16(uint16_t disp) {
    if (C(cpu.ccr) == 0) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BCS (BLO) d:8
static void bcs_blo8(uint8_t disp) {
    if (C(cpu.ccr) == 1) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BCS (BLO) d:16
static void bcs_blo16(uint16_t disp) {
    if (C(cpu.ccr) == 1) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BNE d:8
static void bne8(uint8_t disp) {
    if (Z(cpu.ccr) == 0) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BNE d:16
static void bne16(uint16_t disp) {
    if (Z(cpu.ccr) == 0) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BEQ d:8
static void beq8(uint8_t disp) {
    if (Z(cpu.ccr) == 1) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BEQ d:16
static void beq16(uint16_t disp) {
    if (Z(cpu.ccr) == 1) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BVC d:8
static void bvc8(uint8_t disp) {
    if (V(cpu.ccr) == 0) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BVC d:16
static void bvc16(uint16_t disp) {
    if (V(cpu.ccr) == 0) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BVS d:8
static void bvs8(uint8_t disp) {
    if (V(cpu.ccr) == 1) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BVS d:16
static void bvs16(uint16_t disp) {
    if (V(cpu.ccr) == 1) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BPL d:8
static void bpl8(uint8_t disp) {
    if (N(cpu.ccr) == 0) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BPL d:16
static void bpl16(uint16_t disp) {
    if (N(cpu.ccr) == 0) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BMI d:8
static void bmi8(uint8_t disp) {
    if (N(cpu.ccr) == 1) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BMI d:16
static void bmi16(uint8_t disp) {
    if (N(cpu.ccr) == 1) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BGE d:8
static void bge8(uint8_t disp) {
    if ((N(cpu.ccr) ^ V(cpu.ccr)) == 0) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BGE d:16
static void bge16(uint16_t disp) {
    if ((N(cpu.ccr) ^ V(cpu.ccr)) == 0) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BLT d:8
static void blt8(uint8_t disp) {
    if ((N(cpu.ccr) ^ V(cpu.ccr)) == 1) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BLT d:16
static void blt16(uint8_t disp) {
    if ((N(cpu.ccr) ^ V(cpu.ccr)) == 1) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BGT d:8
static void bgt8(uint8_t disp) {
    if ((Z(cpu.ccr) | (N(cpu.ccr) ^ V(cpu.ccr))) == 0) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BGT d:16
static void bgt16(uint16_t disp) {
    if ((Z(cpu.ccr) | (N(cpu.ccr) ^ V(cpu.ccr))) == 0) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// BLE d:8
static void ble8(uint8_t disp) {
    if ((Z(cpu.ccr) | (N(cpu.ccr) ^ V(cpu.ccr))) == 1) {
        cpu.pc += (int8_t)disp;
    }
    cpu.cycles += 4;
}

// BLE d:16
static void ble16(uint16_t disp) {
    if ((Z(cpu.ccr) | (N(cpu.ccr) ^ V(cpu.ccr))) == 1) {
        cpu.pc += (int16_t)disp;
    }
    cpu.cycles += 6;
}

// CMP.B #xx:8, Rd
static void cmp_b_xx8_rd(uint8_t xx, uint8_t rd) {
    uint8_t a;
    uint8_t b = xx;
    uint8_t result;

    if ((rd & 0x8) >> 3) {  // RnL
        a = rXl(cpu.er[rd & 0x7]);
    } else {                // RnH
        a = rXh(cpu.er[rd & 0x7]);
    }
    result = a - b;

    // set the flags
    set_H((a & 0x0F) < (b & 0x0F));
    set_N(result & 0x80);
    set_Z(result == 0);
    set_V(((a ^ b) & (a ^ result) & 0x80) != 0);
    set_C(a < b);

    cpu.cycles += 2;
}

// CMP.B Rs, Rd
static void cmp_b_rs_rd(uint8_t rs, uint8_t rd) {
    uint8_t a;
    uint8_t b;
    uint8_t result;

    if ((rd & 0x8) >> 3) {  // RnL
        a = rXl(cpu.er[rd & 0x7]);
    } else {                // RnH
        a = rXh(cpu.er[rd & 0x7]);
    }

    if ((rs & 0x8) >> 3) {  // RnL
        b = rXl(cpu.er[rs & 0x7]);
    } else {                // RnH
        b = rXh(cpu.er[rs & 0x7]);
    }
    result = a - b;

    // set the flags
    set_H((a & 0x0F) < (b & 0x0F));
    set_N(result & 0x80);
    set_Z(result == 0);
    set_V(((a ^ b) & (a ^ result) & 0x80) != 0);
    set_C(a < b);

    cpu.cycles += 2;
}

// CMP.W #xx:16, Rd
static void cmp_w_xx16_rd(uint16_t xx, uint8_t rd) {
    uint16_t a;
    uint16_t b = xx;
    uint16_t result;

    if ((rd & 0x8) >> 3) { // En
        a = eX(cpu.er[rd & 0x7]);
    } else {               // Rn
        a = rX(cpu.er[rd & 0x7]);
    }
    result = a - b;

    // set the flags
    set_H((a & 0x0FFF) < (b & 0x0FFF));
    set_N(result & 0x8000);
    set_Z(result == 0);
    set_V(((a ^ b) & (a ^ result) & 0x8000) != 0);
    set_C(a < b);

    cpu.cycles += 4;
}

// CMP.W Rs, Rd
static void cmp_w_rs_rd(uint8_t rs, uint8_t rd) {
    uint16_t a;
    uint16_t b;
    uint16_t result;

    if ((rd & 0x8) >> 3) { // En
        a = eX(cpu.er[rd & 0x7]);
    } else {               // Rn
        a = rX(cpu.er[rd & 0x7]);
    }

    if ((rs & 0x8) >> 3) { // En
        b = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        b = rX(cpu.er[rs & 0x7]);
    }
    result = a - b;

    // set the flags
    set_H((a & 0x0FFF) < (b & 0x0FFF));
    set_N(result & 0x8000);
    set_Z(result == 0);
    set_V(((a ^ b) & (a ^ result) & 0x8000) != 0);
    set_C(a < b);

    cpu.cycles += 2;
}

// CMP.L #xx:32, ERd
static void cmp_l_xx32_erd(uint32_t xx, uint8_t erd) {
    uint32_t a = cpu.er[erd & 0x7];
    uint32_t b = xx;
    uint32_t result = a - b;

    // set the flags
    set_H((a & 0x0FFFFFFF) < (b & 0x0FFFFFFF));
    set_N(result & 0x80000000);
    set_Z(result == 0);
    set_V(((a ^ b) & (a ^ result) & 0x80000000) != 0);
    set_C(a < b);

    cpu.cycles += 6;
}

// CMP.L ERs, ERd
static void cmp_l_ers_erd(uint8_t ers, uint8_t erd) {
    uint32_t a = cpu.er[erd & 0x7];
    uint32_t b = cpu.er[ers & 0x7];
    uint32_t result = a - b;

    // set the flags
    set_H((a & 0x0FFFFFFF) < (b & 0x0FFFFFFF));
    set_N(result & 0x80000000);
    set_Z(result == 0);
    set_V(((a ^ b) & (a ^ result) & 0x80000000) != 0);
    set_C(a < b);

    cpu.cycles += 2;
}

// ADDS #1, ERd
static void adds_1_erd(uint8_t erd) {
    cpu.er[erd & 0x7] += 1;
    cpu.cycles += 2;
}

// ADDS #2, ERd
static void adds_2_erd(uint8_t erd) {
    cpu.er[erd & 0x7] += 2;
    cpu.cycles += 2;
}

// ADDS #4, ERd
static void adds_4_erd(uint8_t erd) {
    cpu.er[erd & 0x7] += 4;
    cpu.cycles += 2;
}

// RTS (ReTurn from Subroutine)
static void rts() { // *CHECK*
    uint16_t value = mem_read16(cpu.er[0x7]);
    cpu.er[0x7] += 2;
    cpu.pc = value;
    cpu.cycles += 8;
}

// BSET #xx:3, Rd
static void bset_xx3_rd(uint8_t xx, uint8_t rd) {
    uint8_t bit = 1 << (xx & 0x07);

    if ((rd & 0x8) >> 3) { // RnL
        uint8_t val = rXl(cpu.er[rd & 0x7]);
        val |= bit;
        set_rXl(rd & 0x7, val);
    } else {               // RnH
        uint8_t val = rXh(cpu.er[rd & 0x7]);
        val |= bit;
        set_rXh(rd & 0x7, val);
    }

    cpu.cycles += 2;
}

// BSET #xx:3, @ERd
static void bset_xx3_addr_erd(uint8_t xx, uint8_t erd) {
    uint8_t value = mem_read8(cpu.er[erd & 0x7]);
    uint8_t bit = 1 << (xx & 0x07);
    value |= bit;
    mem_write8(cpu.er[erd & 0x7], value);
    cpu.cycles += 8;
}

// BSET #xx:3, @aa:8
static void bset_xx3_aa8(uint8_t xx, uint8_t aa) {
    uint8_t value = mem_read8(aa);
    uint8_t bit = 1 << (xx & 0x07);
    value |= bit;
    mem_write8(aa, value);
    cpu.cycles += 8;
}

// BSET Rn, Rd
static void bset_rn_rd(uint8_t rn, uint8_t rd) {
    uint8_t n;

    if ((rn & 0x8) >> 3) { // RnL
        n = rXl(cpu.er[rn & 0x7]);
    } else {               // RnH
        n = rXh(cpu.er[rn & 0x7]);
    }

    uint8_t bit = 1 << (n & 0x07);
    uint8_t value;

    if ((rd & 0x8) >> 3) { // RnL
        value = rXl(cpu.er[rd & 0x7]);
        value |= bit;
        set_rXl(rd & 0x7, value);
    } else {               // RnH
        value = rXh(cpu.er[rd & 0x7]);
        value |= bit;
        set_rXh(rd & 0x7, value);
    }

    cpu.cycles += 2;
}

// BSET Rn, @ERd
static void bset_rn_addr_erd(uint8_t rn, uint8_t erd) {
    uint8_t n;

    if ((rn & 0x8) >> 3) { // RnL
        n = rXl(cpu.er[rn & 0x7]);
    } else {               // RnH
        n = rXh(cpu.er[rn & 0x7]);
    }

    uint8_t bit = 1 << (n & 0x07);
    uint8_t value = mem_read8(cpu.er[erd & 0x7]);
    value |= bit;
    mem_write8(cpu.er[erd & 0x7], value);
    cpu.cycles += 8;
}

// BSET Rn, @aa:8
static void bset_rn_aa8(uint8_t rn, uint8_t aa) {
    uint8_t n;

    if ((rn & 0x8) >> 3) { // RnL
        n = rXl(cpu.er[rn & 0x7]);
    } else {               // RnH
        n = rXh(cpu.er[rn & 0x7]);
    }

    uint8_t bit = 1 << (n & 0x07);
    uint8_t value = mem_read8(aa);
    value |= bit;
    mem_write8(aa, value);
    cpu.cycles += 8;
}

// AND.B #xx:8, Rd
static void and_b_xx8_rd(uint8_t xx, uint8_t rd) {
    uint8_t a;
    uint8_t b = xx;
    uint8_t result;

    if ((rd & 0x8) >> 3) { // RnL
        a = rXl(cpu.er[rd & 0x7]);
        result = a & b;
        set_rXl(rd & 0x7, result);
    } else {               // RnH
        a = rXh(cpu.er[rd & 0x7]);
        result = a & b;
        set_rXh(rd & 0x7, result);
    }

    // set the flags
    set_N(result & 0x80);
    set_Z(result == 0);
    set_V(false);

    cpu.cycles += 2;
}

// AND.B Rs, Rd
static void and_b_rs_rd(uint8_t rs, uint8_t rd) {
    uint8_t a;
    uint8_t b;
    uint8_t result;

    if ((rs & 0x8) >> 3) { // RnL
        b = rXl(cpu.er[rs & 0x7]);
    } else {               // RnH
        b = rXh(cpu.er[rs & 0x7]);
    }

    if ((rd & 0x8) >> 3) { // RnL
        a = rXl(cpu.er[rd & 0x7]);
        result = a & b;
        set_rXl(rd & 0x7, result);
    } else {               // RnH
        a = rXh(cpu.er[rd & 0x7]);
        result = a & b;
        set_rXh(rd & 0x7, result);
    }

    // set the flags
    set_N(result & 0x80);
    set_Z(result == 0);
    set_V(false);

    cpu.cycles += 2;
}

// AND.W #xx:16, rd
static void and_w_xx16_rd(uint16_t xx, uint8_t rd) {
    uint16_t a;
    uint16_t b = xx;
    uint16_t result;

    if ((rd & 0x8) >> 3) { // En
        a = eX(cpu.er[rd & 0x7]);
        result = a & b;
        set_eX(rd & 0x7, result);
    } else {               // Rn
        a = rX(cpu.er[rd & 0x7]);
        result = a & b;
        set_rX(rd & 0x7, result);
    }

    // set the flags
    set_N(result & 0x8000);
    set_Z(result == 0);
    set_V(false);

    cpu.cycles += 4;
}

// AND.W Rs, Rd
static void and_w_rs_rd(uint8_t rs, uint8_t rd) {
    uint16_t a;
    uint16_t b;
    uint16_t result;

    if ((rs & 0x8) >> 3) { // En
        b = eX(cpu.er[rs & 0x7]);
    } else {               // Rn
        b = rX(cpu.er[rs & 0x7]);
    }

    if ((rd & 0x8) >> 3) { // En
        a = eX(cpu.er[rd & 0x7]);
        result = a & b;
        set_eX(rd & 0x7, result);
    } else {               // Rn
        a = rX(cpu.er[rd & 0x7]);
        result = a & b;
        set_rX(rd & 0x7, result);
    }

    // set the flags
    set_N(result & 0x8000);
    set_Z(result == 0);
    set_V(false);

    cpu.cycles += 2;
}

// AND.L #xx:32, ERd
static void and_l_xx32_erd(uint32_t xx, uint8_t erd) {
    uint32_t a = cpu.er[erd & 0x7];
    uint32_t b = xx;
    uint32_t result = a & b;

    cpu.er[erd & 0x7] = result;

    // set the flags
    set_N(result & 0x80000000);
    set_Z(result == 0);
    set_V(false);

    cpu.cycles += 6;
}

// AND.L Rs, ERd
static void and_l_rs_erd(uint8_t rs, uint8_t erd) {
    uint32_t a = cpu.er[erd & 0x7];
    uint32_t b = cpu.er[rs & 0x7];
    uint32_t result = a & b;

    cpu.er[erd & 0x7] = result;

    // set the flags
    set_N(result & 0x80000000);
    set_Z(result == 0);
    set_V(false);

    cpu.cycles += 4;
}

// BSR (Branch to SubRoutine)
// BSR d:8
static void bsr_d8(uint8_t disp) {
    cpu.er[0x7] -= 2;
    mem_write16(cpu.er[0x7], cpu.pc & 0xFFFF);
    cpu.pc += (int8_t) disp;
    cpu.cycles += 6;
}

// BSR d:16
static void bsr_d16(uint16_t disp) {
    cpu.er[0x7] -= 2;
    mem_write16(cpu.er[0x7], cpu.pc & 0xFFFF);
    cpu.pc += (int16_t) disp;
    cpu.cycles += 8;
}

// BCLR #xx:3, Rd
static void bclr_xx3_rd(uint8_t xx, uint8_t rd) {
    uint8_t bit = ~(1 << (xx & 0x07));

    if ((rd & 0x8) >> 3) { // RnL
        uint8_t val = rXl(cpu.er[rd & 0x7]);
        val &= bit;
        set_rXl(rd & 0x7, val);
    } else {               // RnH
        uint8_t val = rXh(cpu.er[rd & 0x7]);
        val &= bit;
        set_rXh(rd & 0x7, val);
    }

    cpu.cycles += 2;
}

// BCLR #xx:3 @ERd
static void bclr_xx3_addr_erd(uint8_t xx, uint8_t erd) {
    uint8_t value = mem_read8(cpu.er[erd & 0x7]);
    uint8_t bit = ~(1 << (xx & 0x07));
    value &= bit;
    mem_write8(cpu.er[erd & 0x7], value);
    cpu.cycles += 8;
}

// BCLR #xx:3 @aa:8
static void bclr_xx3_aa8(uint8_t xx, uint8_t aa) {
    uint8_t value = mem_read8(aa);
    uint8_t bit = ~(1 << (xx & 0x07));
    value &= bit;
    mem_write8(aa, value);
    cpu.cycles += 8;
}

// BCLR Rn, Rd
static void bclr_rn_rd(uint8_t rn, uint8_t rd) {
    uint8_t n;

    if ((rn & 0x8) >> 3) { // RnL
        n = rXl(cpu.er[rn & 0x7]);
    } else {               // RnH
        n = rXh(cpu.er[rn & 0x7]);
    }

    uint8_t bit = ~(1 << (n & 0x07));
    uint8_t value;

    if ((rd & 0x8) >> 3) { // RnL
        value = rXl(cpu.er[rd & 0x7]);
        value &= bit;
        set_rXl(rd & 0x7, value);
    } else {               // RnH
        value = rXh(cpu.er[rd & 0x7]);
        value &= bit;
        set_rXh(rd & 0x7, value);
    }

    cpu.cycles += 2;
}

// BCLR Rn, @ERd
static void bclr_rn_addr_erd(uint8_t rn, uint8_t erd) {
    uint8_t n;

    if ((rn & 0x8) >> 3) { // RnL
        n = rXl(cpu.er[rn & 0x7]);
    } else {               // RnH
        n = rXh(cpu.er[rn & 0x7]);
    }

    uint8_t bit = ~(1 << (n & 0x07));
    uint8_t value = mem_read8(cpu.er[erd & 0x7]);
    value &= bit;
    mem_write8(cpu.er[erd & 0x7], value);
    cpu.cycles += 8;
}

// BCLR Rn, @aa:8
static void bclr_rn_aa8(uint8_t rn, uint8_t aa) {
    uint8_t n;

    if ((rn & 0x8) >> 3) { // RnL
        n = rXl(cpu.er[rn & 0x7]);
    } else {               // RnH
        n = rXh(cpu.er[rn & 0x7]);
    }

    uint8_t bit = ~(1 << (n & 0x07));
    uint8_t value = mem_read8(aa);
    value &= bit;
    mem_write8(aa, value);
    cpu.cycles += 8;
}

// BILD (Bit Invert LoaD)
// BILD #xx3.Rd
static void bild_xx3_rd(uint8_t xx, uint8_t rd) {
    uint8_t n;

    if ((rd & 0x8) >> 3) { // RnL
        n = rXl(cpu.er[rd & 0x7]);
    } else {               // RnH
        n = rXh(cpu.er[rd & 0x7]);
    }

    uint8_t bit = (n >> (xx & 0x7)) & 1;
    set_C(!bit);

    cpu.cycles += 2;
}

// BILD #xx:3.@ERd
static void bild_xx3_addr_erd(uint8_t xx, uint8_t erd) {
    uint8_t n = mem_read8(cpu.er[erd & 0x7]);

    uint8_t bit = (n >> (xx & 0x7)) & 1;
    set_C(!bit);

    cpu.cycles += 6;
}

// BILD #xx:3.@aa:8
static void bild_xx3_aa8(uint8_t xx, uint8_t aa) {
    uint8_t n = mem_read8(aa);

    uint8_t bit = (n >> (xx & 0x7)) & 1;
    set_C(!bit);

    cpu.cycles += 6;
}

// BLD (Bit LoaD)
// BLD #xx:3,Rd
static void bld_xx3_rd(uint8_t xx, uint8_t rd) {
    uint8_t n;

    if ((rd & 0x8) >> 3) { // RnL
        n = rXl(cpu.er[rd & 0x7]);
    } else {               // RnH
        n = rXh(cpu.er[rd & 0x7]);
    }

    uint8_t bit = (n >> (xx & 0x7)) & 1;
    set_C(bit);

    cpu.cycles += 2;
}

// BLD #xx:3,@ERd
static void bld_xx3_addr_erd(uint8_t xx, uint8_t erd) {
    uint8_t n = mem_read8(cpu.er[erd & 0x7]);

    uint8_t bit = (n >> (xx & 0x7)) & 1;
    set_C(bit);

    cpu.cycles += 6;
}

// BLD #xx:3,@aa:8
static void bld_xx3_aa8(uint8_t xx, uint8_t aa) {
    uint8_t n = mem_read8(aa);

    uint8_t bit = (n >> (xx & 0x7)) & 1;
    set_C(bit);

    cpu.cycles += 6;
}

// RTE (ReTurn from Exception)
static void rte() {
    cpu.ccr = mem_read8(cpu.er[0x7]);
    cpu.er[0x7] += 2;
    cpu.pc = mem_read16(cpu.er[0x7]);
    cpu.er[0x7] += 2;
    cpu.cycles += 10;
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
    printf("OPCODE: %02X\n", opcode);

    switch (opcode) {
        case 0x00: { // NOP
            nop();
            printf("NOP\n");
            break;
        }
        case 0x56: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte) {
                case 0x70: { // RTE (ReTurn from Exception)
                    rte();
                    printf("RTE (ReTurn from Exception)\n");
                    break;
                }
            }
            break;
        }
        case 0x72: { // BCLR #xx:3, Rd
            uint8_t second_byte = cpu_fetch8();
            bclr_xx3_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("BCLR #xx:3, Rd\n");
            break;
        }
        case 0x77: {
            uint8_t second_byte = cpu_fetch8();
            switch(second_byte & 0x80) {
                case 0x00: { // BLD #xx:3,Rd
                    bld_xx3_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("BLD #xx3,Rd\n");
                    break;
                }
                case 0x80: { // BILD #xx3.Rd
                    bild_xx3_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("BILD #xx3.Rd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x7C: {
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            switch (third_byte) {
                case 0x77: {
                    switch (fourth_byte & 0x80) {
                        case 0x00: { // BLD #xx:3,@ERd
                            bld_xx3_addr_erd((fourth_byte & 0xF0) >> 4, (second_byte & 0xF0) >> 4);
                            printf("BLD #xx:3,@ERd\n");
                            break;
                        }
                        case 0x80: { // BILD #xx:3.@ERd
                            bild_xx3_addr_erd((fourth_byte & 0xF0) >> 4, (second_byte & 0xF0) >> 4);
                            printf("BILD #xx:3.@ERd\n");
                            break;
                        }
                        default:
                            printf("Error: opcode not implemented: 0x%02X\n", opcode);
                            printf("Current PC: %06X\n", cpu.pc);
                            exit(-1);
                    }
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x7E: {
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            switch (third_byte) {
                case 0x77: {
                    switch (fourth_byte & 0x80) {
                        case 0x00: { // BLD #xx:3,@aa:8
                            bld_xx3_aa8((fourth_byte & 0xF0) >> 4, second_byte);
                            printf("BLD #xx:3,@aa:8\n");
                            break;
                        }
                        case 0x80: { // BILD #xx:3.@aa:8
                            bild_xx3_aa8((fourth_byte & 0xF0) >> 4, second_byte);
                            printf("BILD #xx:3.@aa:8\n");
                            break;
                        }
                        default:
                            printf("Error: opcode not implemented: 0x%02X\n", opcode);
                            printf("Current PC: %06X\n", cpu.pc);
                            exit(-1);
                    }
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x62: { // BCLR Rn, Rd
            uint8_t second_byte = cpu_fetch8();
            bclr_rn_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("BCLR Rn, Rd\n");
            break;
        }
        case 0x55: { // BSR d:8
            uint8_t disp = cpu_fetch8();
            bsr_d8(disp);
            printf("BSR d:8\n");
            break;
        }
        case 0x5C: { // BSR d:16
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            uint16_t disp = (third_byte << 8) | fourth_byte;
            bsr_d16(disp);
            printf("BSR d:16\n");
            break;
        }
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF: { // AND.B #xx:8, Rd
            uint8_t imm = cpu_fetch8();
            and_b_xx8_rd(imm, opcode & 0x0F);
            printf("AND.B #xx:8, Rd\n");
            break;
        }
        case 0x16: { // AND.B Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            and_b_rs_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("AND.B Rs, Rd\n");
            break;
        }
        case 0x66: { // AND.W Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            and_w_rs_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("AND.W Rs, Rd\n");
            break;
        }
        case 0x60: { // BSET Rn, Rd
            uint8_t second_byte = cpu_fetch8();
            bset_rn_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("BSET Rn, Rd\n");
            break;
        }
        case 0x70: { // BSET #xx:3, Rd
            uint8_t second_byte = cpu_fetch8();
            bset_xx3_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("BSET #xx:3, Rd\n");
            break;
        }
        case 0x7D: {
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            switch(third_byte) {
                case 0x60: { // BSET Rn, @ERd
                    bset_rn_addr_erd((fourth_byte & 0xF0) >> 4, (second_byte & 0xF0) >> 4);
                    printf("BSET Rn, @ERd\n");
                    break;
                }
                case 0x62: { // BCLR Rn, @ERd
                    bclr_rn_addr_erd((fourth_byte & 0xF0) >> 4, (second_byte & 0xF0) >> 4);
                    printf("BCLR Rn, @ERd\n");
                    break;
                }
                case 0x70: { // BSET #xx:3, @ERd
                    bset_xx3_addr_erd((fourth_byte & 0xF0) >> 4, (second_byte & 0xF0) >> 4);
                    printf("BSET #xx:3, @ERd\n");
                    break;
                }
                case 0x72: { // BCLR #xx:3 @ERd
                    bclr_xx3_addr_erd((fourth_byte & 0xF0) >> 4, (second_byte & 0xF0) >> 4);
                    printf("BCLR #xx:3 @ERd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x7F: {
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            switch(third_byte) {
                case 0x60: { // BSET Rn, @aa:8
                    bset_rn_aa8((fourth_byte & 0xF0) >> 4, second_byte);
                    printf("BSET Rn, @aa:8\n");
                    break;
                }
                case 0x62: { // BCLR Rn, @aa:8
                    bclr_rn_aa8((fourth_byte & 0xF0) >> 4, second_byte);
                    printf("BCLR Rn, @aa:8\n");
                    break;
                }
                case 0x70: { // BSET #xx:3, @aa:8
                    bset_xx3_aa8((fourth_byte & 0xF0) >> 4, second_byte);
                    printf("BSET #xx:3, @aa:8\n");
                    break;
                }
                case 0x72: { // BCLR #xx:3 @aa:8
                    bclr_xx3_aa8((fourth_byte & 0xF0) >> 4, second_byte);
                    printf("BCLR #xx:3 @aa:8\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X __ %02X\n", opcode, third_byte);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x0B: {
            uint8_t second_byte = cpu_fetch8();
            switch(opcode & 0xF0) {
                case 0x00: { // ADDS #1, ERd
                    adds_1_erd(second_byte & 0x0F);
                    printf("ADDS #1, ERd\n");
                    break;
                }
                case 0x80: { // ADDS #2, ERd
                    adds_2_erd(second_byte & 0x0F);
                    printf("ADDS #2, ERd\n");
                    break;
                }
                case 0x90: { // ADDS #4, ERd
                    adds_4_erd(second_byte & 0x0F);
                    printf("ADDS #4, ERd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x54: {
            uint8_t second_byte = cpu_fetch8();
            switch(second_byte) {
                case 0x70: { // RTS (ReTurn from Subroutine)
                    rts();
                    printf("RTS (ReTurn from Subroutine)\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA6:
        case 0xA7:
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAE:
        case 0xAF: { // CMP.B #xx:8, Rd
            uint8_t imm = cpu_fetch8();
            cmp_b_xx8_rd(imm, opcode & 0x0F);
            printf("CMP.B #xx:8, Rd\n");
            break;
        }
        case 0x1C: { // CMP.B Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            cmp_b_rs_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("CMP.B Rs, Rd\n");
            break;
        }
        case 0x1D: { // CMP.W Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            cmp_w_rs_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("CMP.W Rs, Rd\n");
            break;
        }
        case 0x1F: { // CMP.L ERs, ERd
            uint8_t second_byte = cpu_fetch8();
            cmp_l_ers_erd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("CMP.L ERs, ERd\n");
            break;
        }
        case 0x40: { // BRA (BT) d:8
            uint8_t disp = cpu_fetch8();
            bra8(disp);
            printf("BRA (BT) d:8\n");
            break;
        }
        case 0x58: {
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            uint16_t disp = (third_byte << 8) | fourth_byte;
            switch(second_byte) {
                case 0x00: { // BRA (BT) d:16
                    bra16(disp);
                    printf("BRA (BT) d:16\n");
                    break;
                }
                case 0x10: { // BRN (BF) d:16
                    brn16(disp);
                    printf("BRN (BF) d:16\n");
                    break;
                }
                case 0x20: { // BHI d:16
                    bhi16(disp);
                    printf("BHI d:16\n");
                    break;
                }
                case 0x30: { // BLS d:16
                    bls16(disp);
                    printf("BLS d:16\n");
                    break;
                }
                case 0x40: { // Bcc (BHS) d:16
                    bcc_bhs16(disp);
                    printf("Bcc (BHS) d:16\n");
                    break;
                }
                case 0x50: { // BCS (BLO) d:16
                    bcs_blo16(disp);
                    printf("BCS (BLO) d:16\n");
                    break;
                }
                case 0x60: { // BNE d:16
                    bne16(disp);
                    printf("BNE d:16\n");
                    break;
                }
                case 0x70: { // BEQ d:16
                    beq16(disp);
                    printf("BEQ d:16\n");
                    break;
                }
                case 0x80: { // BVC d:16
                    bvc16(disp);
                    printf("BVC d:16\n");
                    break;
                }
                case 0x90: { // BVS d:16
                    bvs16(disp);
                    printf("BVS d:16\n");
                    break;
                }
                case 0xA0: { // BPL d:16
                    bpl16(disp);
                    printf("BPL d:16\n");
                    break;
                }
                case 0xB0: { // BMI d:16
                    bmi16(disp);
                    printf("BMI d:16\n");
                    break;
                }
                case 0xC0: { // BGE d:16
                    bge16(disp);
                    printf("BGE d:16\n");
                    break;
                }
                case 0xD0: { // BLT d:16
                    blt16(disp);
                    printf("BLT d:16\n");
                    break;
                }
                case 0xE0: { // BGT d:16
                    bgt16(disp);
                    printf("BGT d:16\n");
                    break;
                }
                case 0xF0: { // BLE d:16
                    ble16(disp);
                    printf("BLE d:16\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x41: { // BRN (BF) d:8
            uint8_t disp = cpu_fetch8();
            brn8(disp);
            printf("BRN (BF) d:8\n");
            break;
        }
        case 0x42: { // BHI d:8
            uint8_t disp = cpu_fetch8();
            bhi8(disp);
            printf("BHI d:8\n");
            break;
        }
        case 0x43: { // BLS d:8
            uint8_t disp = cpu_fetch8();
            bls8(disp);
            printf("BLS d:8\n");
            break;
        }
        case 0x44: { // Bcc (BHS) d:8
            uint8_t disp = cpu_fetch8();
            bcc_bhs8(disp);
            printf("Bcc (BHS) d:8\n");
            break;
        }
        case 0x45: { // BCS (BLO) d:8
            uint8_t disp = cpu_fetch8();
            bcs_blo8(disp);
            printf("BCS (BLO) d:8\n");
            break;
        }
        case 0x46: { // BNE d:8
            uint8_t disp = cpu_fetch8();
            bne8(disp);
            printf("BNE d:8\n");
            break;
        }
        case 0x47: { // BEQ d:8
            uint8_t disp = cpu_fetch8();
            beq8(disp);
            printf("BEQ d:8\n");
            break;
        }
        case 0x48: { // BVC d:8
            uint8_t disp = cpu_fetch8();
            bvc8(disp);
            printf("BVC d:8\n");
            break;
        }
        case 0x49: { // BVS d:8
            uint8_t disp =cpu_fetch8();
            bvs8(disp);
            printf("BVS d:8\n");
            break;
        }
        case 0x4A: { // BPL d:8
            uint8_t disp = cpu_fetch8();
            bpl8(disp);
            printf("BPL d:8\n");
            break;
        }
        case 0x4B: { // BMI d:8
            uint8_t disp = cpu_fetch8();
            bmi8(disp);
            printf("BMI d:8\n");
            break;
        }
        case 0x4C: { // BGE d:8
            uint8_t disp = cpu_fetch8();
            bge8(disp);
            printf("BGE d:8\n");
            break;
        }
        case 0x4D: { // BLT d:8
            uint8_t disp = cpu_fetch8();
            blt8(disp);
            printf("BLT d:8\n");
            break;
        }
        case 0x4E: { // BGT d:8
            uint8_t disp = cpu_fetch8();
            bgt8(disp);
            printf("BGT d:8\n");
            break;
        }
        case 0x4F: { // BLE d:8
            uint8_t disp = cpu_fetch8();
            ble8(disp);
            printf("BLE d:8\n");
            break;
        }
        case 0x5D: { // JSR @ERn
            uint8_t second_byte = cpu_fetch8();
            jsr_addr_ern((second_byte & 0xF0) >> 4);
            printf("JSR @ERn\n");
            break;
        }
        case 0x5E: { // JSR @aa:24
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            uint32_t abs = (second_byte << 16) | (third_byte << 8) | fourth_byte;
            jsr_aa24(abs);
            printf("JSR @aa:24\n");
            break;
        }
        case 0x5F: { // JSR @@aa:8
            uint8_t abs = cpu_fetch8();
            jsr_aa8(abs);
            printf("JSR @aa:8\n");
            break;
        }
        case 0x1B: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0xF0) {
                case 0x00: { // SUBS #1, ERd
                    subs_1_erd(second_byte & 0x0F);
                    printf("SUBS #1, ERd\n");
                    break;
                }
                case 0x80: { // SUBS #2, ERd
                    subs_2_erd(second_byte & 0x0F);
                    printf("SUBS #2, ERd\n");
                    break;
                }
                case 0x90: { // SUBS #4, ERd
                    subs_4_erd(second_byte & 0x0F);
                    printf("SUBS #4, ERd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x18: { // SUB.B Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            sub_b_rs_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("SUB.B Rs, Rd\n");
            break;
        }
        case 0x19: { // SUB.W Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            sub_w_rs_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("SUB.W Rs, Rd\n");
            break;
        }
        case 0x1A: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0x80) {
                case 0x00: {}
                case 0x80: { // SUB.L ERs, ERd
                    sub_l_ers_erd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("SUB.L ERs, ERd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x08: { // ADD.B Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            add_b_rs_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("ADD.B Rs, Rd\n");
            break;
        }
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F: { // ADD.B #xx:8, Rd
            uint8_t imm = cpu_fetch8();
            add_b_xx_rd(imm, opcode & 0x0F);
            printf("ADD.B #xx:8, Rd\n");
            break;
        }
        case 0x09: { // ADD.W Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            add_w_rs_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("ADD.W Rs, Rd\n");
            break;
        }
        case 0x0A: { // ADD.L Rs, Erd
            uint8_t second_byte = cpu_fetch8();
            add_l_rs_erd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("ADD.L Rs, Erd\n");
            break;
        }
        case 0x0C: { // MOV.B Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            mov_b_r_r((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("MOV.B Rs, Rd\n");
            break;
        }
        case 0x0D: { // MOV.W Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            mov_w_r_r((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("MOV.W Rs, Rd\n");
            break;
        }
        case 0x0F: { // MOV.L Rs, Rd
            uint8_t second_byte = cpu_fetch8();
            mov_l_r_r((second_byte & 0xF0) >> 4, second_byte & 0x0F);
            printf("MOV.L Rs, Rd\n");
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
            printf("MOV.B #xx:8, Rd\n");
            break;
        }
        case 0x68: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0x80) {
                case 0x00: { // MOV.B @ers, Rd
                    mov_b_addr_ers_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("MOV.B @ers, Rd\n");
                    break;
                }
                case 0x80: { // MOV.B Rs, @ERd
                    mov_b_rs_addr_erd(second_byte & 0x0F, (second_byte & 0xF0) >> 4);
                    printf("MOV.B Rs, @ERd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x6E: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0x80) {
                case 0x00: { // MOV.B @(d:16, ERs), Rd
                    uint16_t disp = cpu_fetch16(); // 3rd & 4th byte
                    mov_b_disp16_addr_ers_rd(disp, (second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("MOV.B @(d:16, ERs), Rd\n");
                    break;
                }
                case 0x80: { // MOV.B Rs, @(d:16, ERd)
                    uint16_t disp = cpu_fetch16(); // 3rd & 4th byte
                    mov_b_rs_disp16_addr_erd(disp, second_byte & 0x0F, (second_byte & 0xF0) >> 4);
                    printf("MOV.B Rs, @(d:16, ERd)\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x78: {
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            uint8_t fifth_byte = cpu_fetch8(); // it only has 0's

            switch (third_byte) {
                case 0x6A: {
                    //disp
                    uint8_t sixth_byte = cpu_fetch8();
                    uint8_t seventh_byte = cpu_fetch8();
                    uint8_t eight_byte = cpu_fetch8();

                    uint32_t disp = (sixth_byte << 16) | (seventh_byte << 8) | eight_byte;

                    switch (fourth_byte & 0xF0) {
                        case 0x20: { // MOV.B @(d:24, ERs), Rd
                            mov_b_disp24_addr_ers_rd(disp, (second_byte & 0xF0) >> 4, fourth_byte & 0x0F);
                            printf("MOV.B @(d:24, ERs), Rd\n");
                            break;
                        }
                        case 0xA0: { // MOV.B Rs, @(d:24, ERd)
                            mov_b_rs_disp24_addr_erd(disp, fourth_byte & 0x0F, (second_byte & 0xF0) >> 4);
                            printf("MOV.B Rs, @(d:24, Erd)\n");
                            break;
                        }
                        default:
                            printf("Error: opcode not implemented: 0x%02X\n", opcode);
                            printf("Current PC: %06X\n", cpu.pc);
                            exit(-1);
                    }
                    break;
                }
                case 0x6B: {
                    //disp
                    uint8_t sixth_byte = cpu_fetch8();
                    uint8_t seventh_byte = cpu_fetch8();
                    uint8_t eight_byte = cpu_fetch8();

                    uint32_t disp = (sixth_byte << 16) | (seventh_byte << 8) | eight_byte;
                    switch (fourth_byte & 0xF0) {
                        case 0x20: { // MOV.W @(d:24, ERs), Rd
                            mov_w_disp24_addr_ers_rd(disp, (second_byte & 0xF0) >> 4, fourth_byte & 0x0F);
                            printf("MOV.W @(d:24, Ers) Rd\n");
                            break;
                        }
                        case 0xA0: { // MOV.W Rs, @(d:24, Erd)
                            mov_w_rs_disp24_addr_erd(fourth_byte & 0x0F, disp, (second_byte & 0xF0) >> 4);
                            printf("MOV.W Rs, @(d:24, ERd)\n");
                            break;
                        }
                        default:
                            printf("Error: opcode not implemented: 0x%02X\n", opcode);
                            printf("Current PC: %06X\n", cpu.pc);
                            exit(-1);
                    }
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x6C: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0x80) {
                case 0x00: { // MOV.B @ERs+, Rd
                    mov_b_addr_ers_plus_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("MOV.B @ERs+, Rd\n");
                    break;
                }
                case 0x80: { // MOV.B Rs, @-ERs
                    mov_b_rs_minus_addr_ers(second_byte & 0x0F, (second_byte & 0xF0) >> 4);
                    printf("MOV.B Rs, @-Ers\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F: { // MOV.B @aa:8, Rd
            uint8_t abs = cpu_fetch8(); // 2nd byte
            mov_b_aa8_rd(abs, opcode & 0x0F);
            printf("MOV.B @aa:8, Rd\n");
            break;
        }
        case 0x6A: {
            uint8_t second_byte = cpu_fetch8();
            switch(second_byte & 0xF0) {
                case 0x00: { // MOV.B @aa:16, Rd
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t abs = (third_byte << 8) | fourth_byte;
                    mov_b_aa16_rd(abs, second_byte & 0x0F);
                    printf("MOV.B @aa:16, Rd\n");
                    break;
                }
                case 0x20: { // MOV.B @aa:24, Rd
                    uint8_t third_byte = cpu_fetch8();
                    if (third_byte != 0x00) {
                        printf("Error: unknown extended MOV.B @aa:24, Rd format. 3rd byte must be 0x00: %02X \n", third_byte);
                        exit(-1);
                    }
                    uint8_t fourth_byte = cpu_fetch8();
                    uint8_t fifth_byte = cpu_fetch8();
                    uint8_t sixth_byte = cpu_fetch8();
                    uint32_t abs = (fourth_byte << 16) | (fifth_byte << 8) | sixth_byte;
                    mov_b_aa24_rd(abs, second_byte & 0x0F);
                    printf("MOV.B @aa:24, Rd\n");
                    break;
                }
                case 0x80: { // MOV.B Rs, @aa:16
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t abs = (third_byte << 8) | fourth_byte;
                    mov_b_rs_aa16(second_byte & 0x0F, abs);
                    printf("MOV.B Rs, @aa:16\n");
                    break;
                }
                case 0xA0: { // MOV.B Rs, @aa:24
                    uint8_t third_byte = cpu_fetch8();
                    if (third_byte != 0x00) {
                        printf("Error: unknown extended MOV.B @aa:24, Rd format. 3rd byte must be 0x00: %02X \n", third_byte);
                        exit(-1);
                    }
                    uint8_t fourth_byte = cpu_fetch8();
                    uint8_t fifth_byte = cpu_fetch8();
                    uint8_t sixth_byte = cpu_fetch8();
                    uint32_t abs = (fourth_byte << 16) | (fifth_byte << 8) | sixth_byte;
                    mov_b_rs_aa24(second_byte & 0x0F, abs);
                    printf("MOV.B Rs, @aa:24\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x79: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0xF0) {
                case 0x00: { // MOV.W #xx:16, Rd
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t imm = (third_byte << 8) | fourth_byte;
                    mov_w_xx_rd(imm, second_byte & 0x0F);
                    printf("MOV.W #xx:16, Rd\n");
                    break;
                }
                case  0x10: { // ADD.W #xx:16, Rd
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t imm = (third_byte << 8) | fourth_byte;
                    add_w_xx_rd(imm, second_byte & 0x0F);
                    printf("ADD.W #xx:16, Rd\n");
                    break;
                }
                case 0x20: { // CMP.W #xx:16, Rd
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t imm = (third_byte << 8) | fourth_byte;
                    cmp_w_xx16_rd(imm, second_byte & 0x0F);
                    printf("CMP.W #xx:16, Rd\n");
                    break;
                }
                case 0x30: { // SUB.W #xx:16, Rd
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t imm = (third_byte << 8) | fourth_byte;
                    sub_w_xx_rd(imm, second_byte & 0x0F);
                    printf("SUB.W #xx:16, Rd\n");
                    break;
                }
                case 0x60: { // AND.W #xx:16, rd
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t imm = (third_byte << 8) | fourth_byte;
                    and_w_xx16_rd(imm, second_byte & 0x0F);
                    printf("AND.W #xx:16, Rd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x69: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0x80) {
                case 0x00: { // MOV.W @ERs, Rd
                    mov_w_addr_ers_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("MOV.W @Ers, Rd\n");
                    break;
                }
                case 0x80: { // MOV.W Rs, @ERd
                    mov_w_rs_addr_erd(second_byte & 0x0F, (second_byte & 0xF0) >> 4);
                    printf("MOV.W Rs, @Erd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x6F: {
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();

            uint16_t disp = (third_byte << 8) | fourth_byte;
            switch (second_byte & 0x80) {
                case 0x00: { // MOV.W @(d:16, Ers), Rd
                    mov_w_disp16_addr_ers_rd(disp, (second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("MOV.W @(d:16, Ers), Rd\n");
                    break;
                }
                case 0x80: { // MOV.W Rs, @(d:16, Erd)
                    mov_w_rs_disp16_addr_erd(second_byte & 0x0F, disp, (second_byte & 0xF0) >> 4);
                    printf("MOV.W Rs, @(d:16, Erd)\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x6D: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0x80) {
                case 0x00: { // MOV.W @ERs+, Rd
                    mov_w_addr_ers_plus_rd((second_byte & 0xF0) >> 4, second_byte & 0x0F);
                    printf("MOV.W @ERs+, Rd\n");
                    break;
                }
                case 0x80: { // MOV.W Rs, @-ERd
                    mov_w_rs_minus_addr_erd(second_byte & 0x0F, (second_byte & 0xF0) >> 4);
                    printf("MOV.W Rs, @-ERd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x6B: {
            uint8_t second_byte = cpu_fetch8();
            switch (second_byte & 0xF0) {
                case 0x00: { // MOV.W @aa:16, Rd
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t abs = (third_byte << 8) | fourth_byte;
                    mov_w_aa16_rd(abs, second_byte & 0x0F);
                    printf("MOV.W @aa:16, Rd\n");
                    break;
                }
                case 0x20: { // MOV.W @aa:24, Rd
                    uint8_t third_byte = cpu_fetch8(); // should be 0
                    uint8_t fourth_byte = cpu_fetch8();
                    uint8_t fifth_byte = cpu_fetch8();
                    uint8_t sixth_byte = cpu_fetch8();
                    uint32_t abs = (fourth_byte << 16) | (fifth_byte << 8) | sixth_byte;
                    mov_w_aa24_rd(abs, second_byte & 0x0F);
                    printf("MOV.W @aa:24, Rd\n");
                    break;
                }
                case 0x80: { // MOV.W Rs, @aa:16
                    uint8_t third_byte = cpu_fetch8();
                    uint8_t fourth_byte = cpu_fetch8();
                    uint16_t abs = (third_byte << 8) | fourth_byte;
                    mov_w_rs_aa16(second_byte & 0x0F, abs);
                    printf("MOV.W Rs, @aa:16\n");
                    break;
                }
                case 0xA0: { // MOV.W Rs, @aa:24
                    uint8_t third_byte = cpu_fetch8(); // should be 0
                    uint8_t fourth_byte = cpu_fetch8();
                    uint8_t fifth_byte = cpu_fetch8();
                    uint8_t sixth_byte = cpu_fetch8();
                    uint32_t abs = (fourth_byte << 16) | (fifth_byte << 8) | sixth_byte;
                    mov_w_rs_aa24(second_byte & 0x0F, abs);
                    printf("MOV.W Rs, @aa:24\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: ****\n");
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x7A: {
            uint8_t second_byte = cpu_fetch8();
            uint8_t third_byte = cpu_fetch8();
            uint8_t fourth_byte = cpu_fetch8();
            uint8_t fifth_byte = cpu_fetch8();
            uint8_t sixth_byte = cpu_fetch8();

            uint32_t imm = (third_byte << 24) | (fourth_byte << 16) | (fifth_byte << 8) | sixth_byte;
            switch (second_byte & 0xF0) {
                case 0x00: { // MOV.L #xx:32, Rd
                    mov_l_xx_rd(imm, second_byte & 0x0F);
                    printf("MOV.L #xx:32, Rd\n");
                    break;
                }
                case 0x10: { // ADD.L #xx:32, ERd
                    add_l_xx_rd(imm, second_byte & 0x0F);
                    printf("ADD.L #xx:32, ERd\n");
                    break;
                }
                case 0x20: { // CMP.L #xx:32, ERd
                    cmp_l_xx32_erd(imm, second_byte & 0x0F);
                    printf("CMP.L #xx:32, ERd\n");
                    break;
                }
                case 0x30: { // SUB.L #xx:32, ERd
                    sub_l_xx_erd(imm, second_byte & 0x0F);
                    printf("SUB.L #xx:32, ERd\n");
                    break;
                }
                case 0x60: { // AND.L #xx:32, ERd
                    and_l_xx32_erd(imm, second_byte & 0x0F);
                    printf("AND.L #xx:32, ERd\n");
                    break;
                }
                default:
                    printf("Error: opcode not implemented: 0x%02X\n", opcode);
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x01: {
            uint8_t second_byte = cpu_fetch8(); // should be 0
            uint8_t third_byte = cpu_fetch8();
            switch (third_byte) {
                case 0x69: {
                    uint8_t fourth_byte = cpu_fetch8();
                    switch (fourth_byte & 0x80) {
                        case 0x00: { // MOV.L @ERs, ERd
                            mov_l_addr_ers_erd((fourth_byte & 0xF0) >> 4, fourth_byte & 0x0F);
                            printf("MOV.L @ERs, ERd\n");
                            break;
                        }
                        case 0x80: { // MOV.L Ers, @Erd
                            mov_l_ers_addr_erd(fourth_byte & 0x0F, (fourth_byte & 0xF0) >> 4);
                            printf("MOV.L Ers, @Erd\n");
                            break;
                        }
                    }
                    break;
                }
                case 0x66: { // AND.L Rs, ERd
                    uint8_t fourth_byte = cpu_fetch8();
                    and_l_rs_erd((fourth_byte & 0xF0) >> 4, fourth_byte & 0x0F);
                    printf("AND.L Rs, ERd\n");
                    break;
                }
                case 0x6F: {
                    uint8_t fourth_byte = cpu_fetch8();
                    uint8_t fifth_byte = cpu_fetch8();
                    uint8_t sixth_byte = cpu_fetch8();

                    uint16_t disp = (fifth_byte << 8) | sixth_byte;
                    switch (fourth_byte & 0x80) {
                        case 0x00: { // MOV.L @(d:16, ERs), ERd
                            mov_l_disp16_addr_ers_erd(disp, (fourth_byte & 0xF0) >> 4, fourth_byte & 0x0F);
                            printf("MOV.L @(d:16, ERs), ERd\n");
                            break;
                        }
                        case 0x80: { // MOV.L ERs, @(d:16, Erd)
                            mov_l_ers_disp16_addr_erd(fourth_byte & 0x0F, disp, (fourth_byte & 0xF0) >> 4);
                            printf("MOV.L ERs, @(d:16, Erd)\n");
                            break;
                        }
                    }
                    break;
                }
                case 0x78: {
                    uint8_t fourth_byte = cpu_fetch8();
                    uint8_t fifth_byte = cpu_fetch8();
                    uint8_t sixth_byte = cpu_fetch8();
                    uint8_t seventh_byte = cpu_fetch8();
                    uint8_t eigth_byte = cpu_fetch8();
                    uint8_t nineth_byte = cpu_fetch8();
                    uint8_t tenth_byte = cpu_fetch8();

                    uint32_t disp = (eigth_byte << 16) | (nineth_byte << 8) | tenth_byte;
                    switch (sixth_byte & 0xF0) {
                        case 0x20: { // MOV.L @(d:24, ERs), ERd
                            mov_l_disp24_addr_ers_erd(disp, (fourth_byte & 0xF0) >> 4, sixth_byte & 0x0F);
                            printf("MOV.L @(d:24, ERs), ERd\n");
                            break;
                        }
                        case 0xA0: { // MOV.L ERs, @(d:24, Erd)
                            mov_l_ers_disp24_addr_erd(sixth_byte & 0x0F, disp, (fourth_byte & 0xF0) >> 4);
                            printf("MOV.L ERs, @(d:24, Erd)\n");
                            break;
                        }
                        default:
                            printf("Error: opcode not implemented: 0x%02X\n", opcode);
                            printf("Current PC: %06X\n", cpu.pc);
                            exit(-1);
                    }
                    break;
                }
                case 0x6D: {
                    uint8_t fourth_byte = cpu_fetch8();
                    switch (fourth_byte & 0x80) {
                        case 0x00: { // MOV.L @ERs+, ERd
                            mov_l_addr_ers_plus_erd((fourth_byte & 0xF0) >> 4, fourth_byte & 0x0F);
                            printf("MOV.L @ERs+, ERd\n");
                            break;
                        }
                        case 0x80: { // MOV.L Ers, @-Erd
                            mov_l_ers_minus_addr_erd(fourth_byte & 0x0F, (fourth_byte & 0xF0) >> 4);
                            printf("MOV.L Ers, @-Erd\n");
                            break;
                        }
                        default:
                            printf("Error: opcode not implemented: 0x%02X\n", opcode);
                            printf("Current PC: %06X\n", cpu.pc);
                            exit(-1);
                    }
                    break;
                }
                case 0x6B: {
                    uint8_t fourth_byte = cpu_fetch8();
                    switch (fourth_byte & 0xF0) {
                        case 0x00: { // MOV.L @aa:16, ERd
                            uint8_t fifth_byte = cpu_fetch8();
                            uint8_t sixth_byte = cpu_fetch8();

                            uint16_t abs = (fifth_byte << 8) | sixth_byte;
                            mov_l_aa16_erd(abs, fourth_byte & 0x0F);
                            printf("MOV.L @aa:16, ERd\n");
                            break;
                        }
                        case 0x20: { // MOV.L @aa:24, ERd
                            uint8_t fifth_byte = cpu_fetch8(); // should be 0
                            uint8_t sixth_byte = cpu_fetch8();
                            uint8_t seventh_byte = cpu_fetch8();
                            uint8_t eigth_byte = cpu_fetch8();

                            uint32_t abs = (sixth_byte << 16) | (seventh_byte << 8) | eigth_byte;
                            mov_l_aa24_erd(abs, fourth_byte & 0x0F);
                            printf("MOV.L @aa:24, ERd\n");
                            break;
                        }
                        case 0x80: { // MOV.L Ers, @aa:16
                            uint8_t fifth_byte = cpu_fetch8();
                            uint8_t sixth_byte = cpu_fetch8();

                            uint16_t abs = (fifth_byte << 8) | sixth_byte;
                            mov_l_ers_aa16(fourth_byte & 0x0F, abs);
                            printf("MOV.L Ers, @aa:16\n");
                            break;
                        }
                        case 0xA0: { // MOV.L Ers, @aa:24
                            uint8_t fifth_byte = cpu_fetch8(); // should be 0
                            uint8_t sixth_byte = cpu_fetch8();
                            uint8_t seventh_byte = cpu_fetch8();
                            uint8_t eigth_byte = cpu_fetch8();

                            uint32_t abs = (sixth_byte << 16) | (seventh_byte << 8) | eigth_byte;
                            mov_l_ers_aa24(fourth_byte & 0x0F, abs);
                            printf("MOV.L Ers, @aa:24\n");
                            break;
                        }
                        default:
                            printf("Error: opcode not implemented: 0x%02X\n", opcode);
                            printf("Current PC: %06X\n", cpu.pc);
                            exit(-1);
                    }
                    break;
                }
                default:
                    printf("Error: opcode not implemented: ****\n");
                    printf("Current PC: %06X\n", cpu.pc);
                    exit(-1);
            }
            break;
        }
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F: { // MOV.B Rs, @aa:8
            uint8_t abs = cpu_fetch8(); // second byte
            mov_b_rs_aa8(opcode & 0x0F, abs);
            printf("MOV.B Rs, @aa:8\n");
            break;
        }
        default:
            printf("Error: opcode not implemented: 0x%02X\n", opcode);
            printf("Current PC: %06X\n", cpu.pc);
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
    printf("cpu cycles: %ld\n", cpu.cycles);
}
