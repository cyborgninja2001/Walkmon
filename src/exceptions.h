#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>
#include "cpu.h"
#include "memory.h"

// interrupts are exceptions
// There are four external interrupts: NMI, IRQAEC, IRQ1, and IRQ0.

// External interrupt (H'000E to H'000F)
uint16_t get_nmi();

// H'000E to H'000F
#define NMI 0x000E

// H'0020 to H'0021
#define IRQ0 0x0020

// H'0022 to H'0023
#define IRQ1 0x0022

// H'0024 to H'0025
#define IRQAEC 0x0024

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

// RTC 0.25 H'002E to H'002F
#define RTC_025 0x002E

uint8_t get_iegr();
uint8_t get_ienr1();
uint8_t get_ienr2();
uint8_t get_irr1();
uint8_t get_irr2();

void handle_exception(uint8_t vector);
void check_exceptions();

#endif