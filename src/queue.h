#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// double linked list

typedef struct Queue {
    uint16_t interrupt_vector;
    struct Queue *prev;
    struct Queue *next;
} Queue;

void init();
void insert(uint16_t interrupt_vector);
uint16_t pop();

#endif