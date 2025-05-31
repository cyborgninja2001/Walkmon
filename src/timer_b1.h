#ifndef TIMER_B1_H
#define TIMER_B1_H

#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#include "exceptions.h"

// registers
#define TMB1 0xF0D0
#define TCB1 0xF0D1 // (R)
#define TLB1 0xF0D1 // (W)

// bits in TMB1
#define TMB17 (mem_read8(TMB1) >> 7)
#define TMB16 ((mem_read8(TMB1) >> 6) & 1)
#define TMB12 ((mem_read8(TMB1) >> 2) & 1)
#define TMB11 ((mem_read8(TMB1) >> 1) & 1)
#define TMB10 ((mem_read8(TMB1) >> 0) & 1)

void init_timer_b1();
void update_timer_b1();

#endif