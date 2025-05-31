#include "timer_b1.h"

static bool timer_b1_halted;

void init_timer_b1() {
    timer_b1_halted = true;
    mem_write8(TMB1, 0x38);
}

static uint64_t subClocks;

void update_timer_b1() {
    if ((!timer_b1_halted) && (subClocks % 256) == 0) {
        // TODO
    }
}
