#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include "memory.h"
#include "cpu.h"

// Clock Pulse Generators

// oscillator control register
#define OSCCR 0xFFF5

void osccr_init();
uint8_t get_osccr();
void set_osccr(uint8_t value);

#endif