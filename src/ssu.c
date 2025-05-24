#include "ssu.h"

extern CPU cpu;

void ssu_init() {
    mem_write8(SSCRH, 0x08);
    mem_write8(SSCRL, 0x00);
    mem_write8(SSMR, 0x00);
    mem_write8(SSER, 0x00);
    mem_write8(SSSR, 0x04);

    mem_write8(SSRDR, 0x00);
    mem_write8(SSTDR, 0x00);
}

uint8_t get_sscrh() {
    return mem_read8(SSCRH);
    cpu.cycles += 2;
}

void set_sscrh(uint8_t value) {
    mem_write8(SSCRH, value);
    cpu.cycles += 2;
}

uint8_t get_sscrl() {
    return mem_read8(SSCRL);
    cpu.cycles += 2;
}

void set_sscrl(uint8_t value) {
    mem_write8(SSCRL, value);
    cpu.cycles += 2;
}

uint8_t get_ssmr() {
    return mem_read8(SSMR);
    cpu.cycles += 2;
}

void set_ssmr(uint8_t value) {
    mem_write8(SSMR, value);
    cpu.cycles += 2;
}

uint8_t get_sser() {
    return mem_read8(SSER);
    cpu.cycles += 2;
}

void set_sser(uint8_t value) {
    mem_write8(SSER, value);
    cpu.cycles += 2;
}

uint8_t get_sssr() {
    return mem_read8(SSSR);
    cpu.cycles += 2;
}

void set_sssr(uint8_t value) {
    mem_write8(SSSR, value);
    cpu.cycles += 2;
}

uint8_t get_ssrdr() {
    return mem_read8(SSRDR);
    cpu.cycles += 2;
}

void set_ssrdr(uint8_t value) {
    mem_write8(SSRDR, value);
    cpu.cycles += 2;
}

uint8_t get_sstdr() {
    return mem_read8(SSTDR);
    cpu.cycles += 2;
}

void set_sstdr(uint8_t value) {
    mem_write8(SSTDR, value);
    cpu.cycles += 2;
}