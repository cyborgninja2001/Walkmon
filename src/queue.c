#include "queue.h"

static Queue *queue_begin;
static Queue *queue_end; // saves the elem that goes first in the queue

void init() {
    queue_begin = NULL;
    queue_end = NULL;
}

void insert(uint16_t interrupt_vector) {
    Queue *new = (Queue*)malloc(sizeof(Queue)); // create new elem
    if (queue_end == NULL) queue_end = new;
    new->interrupt_vector = interrupt_vector;
    if (queue_begin != NULL) queue_begin->prev = new;
    new->next = queue_begin;
    new->prev = NULL;
    queue_begin = new;
}

uint16_t pop() {
    if (queue_end == NULL) { printf("*ERROR: POP EMPTY QUEUE!*\n"); exit(-1);} // empty queue

    uint16_t interrupt_vector = queue_end->interrupt_vector;
    if (queue_begin != queue_end) {
        Queue *aux = queue_end->prev;
        aux->next = NULL;
        free(queue_end);
        queue_end = aux;
    } else {
        free(queue_end);
        queue_begin = NULL;
        queue_end = NULL;
    }
    return interrupt_vector;
}

static void debug_queue() {
    Queue *aux = queue_begin;
    while (aux != NULL) {
        printf("-> [%04X] ", aux->interrupt_vector);
        aux = aux->next;
    }
    printf("-> NULL\n");
}

/*
int main (void) {
    init();
    insert(0x0001);
    insert(0x0002);
    insert(0x0003);
    insert(0x0004);
    debug_queue();

    printf("DELETING: \n");
    pop();
    debug_queue();
    pop();
    debug_queue();
    pop();
    debug_queue();
    pop();
    debug_queue();
    pop();
    debug_queue(); //error
}
*/