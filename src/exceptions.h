#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>
#include "cpu.h"
#include "memory.h"

// interrupts are exceptions
// There are four external interrupts: NMI, IRQAEC, IRQ1, and IRQ0.

// The SSU and IIC share the same vector address. When using the IIC, shift the SSU to
// standby mode using CKSTPR2.

// VECTORS
#define RESET                         0x0000
#define NMI                           0x000E
#define TRAPA0                        0x0010
#define TRAPA1                        0x0012
#define TRAPA2                        0x0014
#define TRAPA3                        0x0016
#define SLEEP                         0x001A
#define IRQ0                          0x0020
#define IRQ1                          0x0022
#define IRQAEC                        0x0024
#define COMP0                         0x002A
#define COMP1                         0x002C
#define QUARTER_SECOND_OVERFLOW       0x002E
#define HALF_SECOND_OVERFLOW          0x0030
#define SECOND_PERIODIC_OVERFLOW      0x0032
#define MINUTE_PERIODIC_OVERFLOW      0x0034
#define HOUR_PERIODIC_OVERFLOW        0x0036
#define DAY_OF_WEEK_PERIODIC_OVERFLOW 0x0038
#define WEEK_PERIODIC_OVERFLOW        0x003A
#define FREE_RUNNING_OVERFLOW         0x003C
#define WDT_OVERFLOW                  0x003E
#define ASYNCHRONOUS_EVENT_COUNTER    0x0040
#define TIMER_B1                      0x0042
#define SSU_IIC2                      0x0044
#define TIMER_W                       0x0046
#define SCI3                          0x004A
#define AD_CONVERTER                  0x004C

// Interrupt registers
#define IEGR 0xFFF2
#define IENR1 0xFFF3
#define IENR2 0xFFF4
#define IRR1 0xFFF6
#define IRR2 0xFFF7

// RTC interrupt flags
#define RTCFLG 0xF067

// RTCFLG bits
#define RTC_025SEIFG (1 << 0)

uint8_t get_iegr();
uint8_t get_ienr1();
uint8_t get_ienr2();
uint8_t get_irr1();
uint8_t get_irr2();

void handle_exception(uint8_t vector);
void check_exceptions();

#endif