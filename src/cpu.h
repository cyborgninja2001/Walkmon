#ifndef CPU_H
#define CPU_H

// H8/300 advanced derivative

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "memory.h"

/*
erX (32 bits) can be splitted into eX (hi 16 bits) & rX (lo 16 bits)

rX can be splitted into rXh (hi 8 bits) & rXl (lo 8 bits)
*/

typedef struct {
    uint32_t er[8]; // er0 - er7 (er7 is the stack pointer)
    // control registers
    uint32_t pc;    // pc is 24 bits
    uint8_t ccr;    // condition code register

    uint64_t cycles;
} CPU;

// hi 16 bits
#define eX(erX) ((uint16_t)((erX) >> 16))

// lo 16 bits
#define rX(erX) ((uint16_t)((erX) & 0xFFFF))

// hi 8 bits
#define rXh(erX) ((uint8_t)((erX >> 8) & 0xFF))

// lo 8 bits
#define rXl(erX) ((uint8_t)((erX) & 0xFF))

// ACCESS CCR BITS:
// get interrupt mask bit (I) (bit 7)
#define I(ccr) ((ccr >> 7) & 0x1)

// get User bit (UI) (bit 6)
#define UI(ccr) ((ccr >> 6) & 0x1)

// get Half Carry flag (H) (bit 5)
#define H(ccr) ((ccr >> 5) & 0x1)

// get User bit (U) (bit 4)
#define U(ccr) ((ccr >> 4) & 0x1)

// get Negative flag (N) (bit 3)
#define N(ccr) ((ccr >> 3) & 0x1)

// get Zero flag (Z) (bit 2)
#define Z(ccr) ((ccr >> 2) & 0x1)

// get Overflow flag (V) (bit 1)
#define V(ccr) ((ccr >> 1) & 0x1)

// get Carry flag (C) (bit 0)
#define C(ccr) (ccr & 0x1)

// set the rXl regsiters
void set_rXl(uint8_t reg, uint8_t value);

// set the rXh registers
void set_rXh(uint8_t reg, uint8_t value);

// set the rX regsiters
void set_rX(uint8_t reg, uint16_t value);

// set the eX regsiters
void set_eX(uint8_t reg, uint16_t value);

// SET CCR BITS
// set interrupt mask bit (I) (bit 7) with value 'v'
void set_I(bool v);

// set User bit (UI) (bit 6) with value 'v'
void set_UI(bool v);

// set Half Carry flag (H) (bit 5) with value 'v'
void set_H(bool v);

// set User bit (U) (bit 4) with value 'v'
void set_U(bool v);

// set Negative flag (N) (bit 3) with value 'v'
void set_N(bool v);

// set Zero flag (Z) (bit 2) with value 'v'
void set_Z(bool v);

// set Overflow flag (V) (bit 1) with value 'v'
void set_V(bool v);

// set Carry flag (C) (bit 0) with value 'v'
void set_C(bool v);

void cpu_reset();
uint16_t cpu_fetch16();
void cpu_step();        // fetch, decode & execute

void cpu_debug();

#endif