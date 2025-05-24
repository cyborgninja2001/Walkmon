#ifndef SSU_H
#define SSU_H

#include <stdint.h>
#include "memory.h"
#include "cpu.h"

// SSU registers
#define SSCRH 0xF0E0
#define SSCRL 0xF0E1
#define SSMR  0xF0E2
#define SSER  0xF0E3
#define SSSR  0xF0E4
#define SSRDR 0xF0E9
#define SSTDR 0xF0EB
//#define SSTRSR ????

void ssu_init();

uint8_t get_sscrh();
void set_sscrh(uint8_t value);

uint8_t get_sscrl();
void set_sscrl(uint8_t value);

uint8_t get_ssmr();
void set_ssmr(uint8_t value);

uint8_t get_sser();
void set_sser(uint8_t value);

uint8_t get_sssr();
void set_sssr(uint8_t value);

uint8_t get_ssrdr();
void set_ssrdr(uint8_t value);

uint8_t get_sstdr();
void set_sstdr(uint8_t value);

#endif