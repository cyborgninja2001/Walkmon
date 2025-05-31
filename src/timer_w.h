#ifndef TIMER_W_H
#define TIMER_W_H

#include <stdint.h>
#include "memory.h"
#include "cpu.h"
#include "exceptions.h"

// registers
#define TMRW  0xF0F0
#define TCRW  0xF0F1
#define TIERW 0xF0F2
#define TSRW  0xF0F3
#define TIOR0 0xF0F4
#define TIOR1 0xF0F5
#define TCNT  0xF0F6
#define GRA   0xF0F8
#define GRB   0xF0FA
#define GRC   0xF0FC
#define GRD   0xF0FE

void init_timer_w_registers();

uint8_t get_tmrw();
uint8_t get_tcrw();
uint8_t get_tierw();
uint8_t get_tsrw();
uint8_t get_tior0();
uint8_t get_tior1();
uint16_t get_tcnt();
uint16_t get_gra();
uint16_t get_grb();
uint16_t get_grc();
uint16_t get_grd();

void set_tmrw(uint8_t value);
void set_tcrw(uint8_t value);
void set_tierw(uint8_t value);
void set_tsrw(uint8_t value);
void set_tior0(uint8_t value);
void set_tior1(uint8_t value);
void set_tcnt(uint16_t value);
void set_gra(uint16_t value);
void set_grb(uint16_t value);
void set_grc(uint16_t value);
void set_grd(uint16_t value);

void update_timer_w();

#endif