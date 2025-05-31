#include "timer_w.h"

extern CPU cpu;

uint8_t get_tmrw() {
    //cpu.cycles += 2;
    return mem_read8(TMRW);
}

uint8_t get_tcrw() {
    //cpu.cycles += 2;
    return mem_read8(TCRW);
}

uint8_t get_tierw() {
    //cpu.cycles += 2;
    return mem_read8(TIERW);
}

uint8_t get_tsrw() {
    //cpu.cycles += 2;
    return mem_read8(TSRW);
}

uint8_t get_tior0() {
    //cpu.cycles += 2;
    return mem_read8(TIOR0);
}

uint8_t get_tior1() {
    //cpu.cycles += 2;
    return mem_read8(TIOR1);
}

uint16_t get_tcnt() {
    //cpu.cycles += 2;
    return mem_read16(TCNT);
}

uint16_t get_gra() {
    //cpu.cycles += 2;
    return mem_read16(GRA);
}

uint16_t get_grb() {
    //cpu.cycles += 2;
    return mem_read16(GRB);
}

uint16_t get_grc() {
    //cpu.cycles += 2;
    return mem_read16(GRC);
}

uint16_t get_grd() {
    //cpu.cycles += 2;
    return mem_read16(GRD);
}

void set_tmrw(uint8_t value) {
    mem_write8(TMRW, value);
    //cpu.cycles += 2;
}

void set_tcrw(uint8_t value) {
    mem_write8(TCRW, value);
    //cpu.cycles += 2;
}

void set_tierw(uint8_t value) {
    mem_write8(TIERW, value);
    //cpu.cycles += 2;
}

void set_tsrw(uint8_t value) {
    mem_write8(TSRW, value);
    //cpu.cycles += 2;
}

void set_tior0(uint8_t value) {
    mem_write8(TIOR0, value);
    //cpu.cycles += 2;
}

void set_tior1(uint8_t value) {
    mem_write8(TIOR1, value);
    //cpu.cycles += 2;
}

void set_tcnt(uint16_t value) {
    mem_write16(TCNT, value);
    //cpu.cycles += 2;
}

void set_gra(uint16_t value) {
    mem_write16(GRA, value);
    //cpu.cycles += 2;
}

void set_grb(uint16_t value) {
    mem_write16(TMRW, value);
    //cpu.cycles += 2;
}

void set_grc(uint16_t value) {
    mem_write16(GRC, value);
    //cpu.cycles += 2;
}

void set_grd(uint16_t value) {
    mem_write8(GRD, value);
    //cpu.cycles += 2;
}

void init_timer_w_registers() {
    mem_write8(TMRW, 0x48);
    mem_write8(TCRW, 0x00);
    mem_write8(TIERW, 0x70);
    mem_write8(TSRW, 0x70);
    mem_write8(TIOR0, 0x88);
    mem_write8(TIOR1, 0x88);
    mem_write16(TCNT, 0x0000);
    mem_write16(GRA, 0xFFFF);
    mem_write16(GRB, 0xFFFF);
    mem_write16(GRC, 0xFFFF);
    mem_write16(GRD, 0xFFFF);
}

void update_timer_w() {
    if ((mem_read8(CKSTPR2) & 0x40) && (get_tmrw() >> 7)) { // if timerW is not halted
        if (get_tmrw() >> 7) { // CTS
            set_tcnt(get_tcnt() + 1);
        }
        //if ((get_tcnt() + 1) == 0x0000) {
        //    set_tsrw(get_tsrw() + 1);
        //}
        if (get_tcnt() == get_gra()) {
            if (get_tcrw() >> 7) { // CCLR
                set_tcnt(0x0000);
            }
            set_tsrw(get_tsrw() | 0x01);
            if (get_tierw() & 0x01) { // IMIEA
                // TODO: interrupt
                if (!I(cpu.ccr)) {
                    printf("interrupt Timer W\n");
                    // save pc & ccr
                    cpu.er[0x7] += 2;
                    mem_write16(cpu.er[0x7], cpu.pc & 0xFFFF);
                    cpu.er[0x7] += 2;
                    mem_write16(cpu.er[0x7], (cpu.ccr << 8) | cpu.ccr);
                    set_I(true);
                    cpu.pc = mem_read16(TIMER_W);
                }
            }
        }
    }
}