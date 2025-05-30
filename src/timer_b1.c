#include "timer_b1.h"

void init_timer_b1() {
    mem_write8(TMB1, 0x38);
}