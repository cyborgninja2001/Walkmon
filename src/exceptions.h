#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>
#include "cpu.h"
#include "memory.h"

// interrupts are exceptions
// There are four external interrupts: NMI, IRQAEC, IRQ1, and IRQ0.

// External interrupt (H'000E to H'000F)
uint16_t get_nmi();

// Interrupt edge select register (IEGR)
#define IEGR 0xFFF2

// Interrupt enable register 1 (IENR1)
#define IENR1 0xFFF3

//Interrupt enable register 2 (IENR2)
#define IENR2 0xFFF4

// Interrupt flag register 1 (IRR1)
#define IRR1 0xFFF6

// Interrupt flag register 2 (IRR2)
#define IRR2 0xFFF7

uint8_t get_iegr();
uint8_t get_ienr1();
uint8_t get_ienr2();
uint8_t get_irr1();
uint8_t get_irr2();

void handle_exception(uint8_t vector);
void check_exceptions();

#endif