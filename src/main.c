#include <stdio.h>

#include "cpu.h"
#include "eeprom.h"
#include "memory.h"

int main(int argc, char *argv[]) {
    cpu_reset();
    cpu_debug();
    return 0;
}
