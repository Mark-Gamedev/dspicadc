#include <stdlib.h>
#include "circularBuffer.h"

CircularBuffer *cb;
static CircularBuffer circularBuffer;
static int intbuf[BUFFERSZ];

//void cbInit(CircularBuffer *cb, int size) {
void cbInit() {
    /*
    cb->size = size;
    cb->start = 0;
    cb->count = 0;
    cb->elems = (int *) calloc(cb->size, sizeof (int));
    */
    cb = &circularBuffer;
    cb->size = BUFFERSZ;
    cb->start = 0;
    cb->count = 0;
    cb->elems = intbuf;
}

int cbIsFull() {
    return cb->count == cb->size;
}

int cbIsEmpty() {
    return cb->count == 0;
}

void cbWrite(int elem) {
    int end = (cb->start + cb->count) % cb->size;
    cb->elems[end] = elem;
    if (cb->count == cb->size){
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
    }else{
        cb->count++;
    }
}

int cbRead() {
    int ret = cb->elems[cb->start];
    cb->start = (cb->start + 1) % cb->size;
    cb->count--;
    return ret;
}
